// Proteus.cpp
// c++ code and improvements by Sam Pluta 2024


#include "Proteus.hpp"
#include "SC_PlugIn.hpp"
#include "SC_PlugIn.h"
#include <nlohmann/json.hpp>
#include <string>
#include "RTNeuralLSTM.cpp"
#include "libsamplerate/include/samplerate.h"

static InterfaceTable *ft;


size_t Proteus::resample (const float* input, float* output, size_t numSamples) noexcept
{
    SRC_DATA src_data {
        input, // data_in
        output, // data_out
        (int) numSamples, // input_frames
        int ((double) numSamples * m_ratio) + 1, // output_frames
        0, // input_frames_used
        0, // output_frames_gen
        0, // end_of_input
        m_ratio // src_ratio
    };

    src_process (src_state.get(), &src_data);

    return (size_t) src_data.output_frames_gen;
}

// size_t Proteus::resample_1 (const float* input, float* output, size_t numSamples) noexcept
// {
//     SRC_DATA src_data {
//         input, // data_in
//         output, // data_out
//         (int) numSamples, // input_frames
//         int ((double) numSamples * m_ratio) + 1, // output_frames
//         0, // input_frames_used
//         0, // output_frames_gen
//         0, // end_of_input
//         m_ratio // src_ratio
//     };

//     src_process (src_state.get(), &src_data);

//     return (size_t) src_data.output_frames_gen;
// }

size_t Proteus::resample_out (const float* input, float* output, size_t inSamples, size_t outSamples) noexcept
{
    SRC_DATA src_data {
        input, // data_in
        output, // data_out
        (int) inSamples, // input_frames
        (int) outSamples, // output_frames
        0, // input_frames_used
        0, // output_frames_gen
        0, // end_of_input
        1./m_ratio // src_ratio
    };

    src_process (src_state_out.get(), &src_data);

    return (size_t) src_data.output_frames_gen;
}

  Proteus::Proteus()
  {
    m_sample_rate = (float) sampleRate();

    m_ratio = 44100. / sampleRate();

    if(m_ratio > 1.05 || m_ratio < 0.95) {
      m_resample = true;
    }

    out_temp_size = int(44100./controlRate())+2; //an extra one for safety

    in_rs = (float*)RTAlloc(mWorld, (double)out_temp_size * sizeof(float));
    //in1_rs = (float*)RTAlloc(mWorld, (double)out_temp_size * sizeof(float));
    out_temp = (float*)RTAlloc(mWorld, (double)out_temp_size * sizeof(float));


    //setting these to medium quality sample rate conversion
    //probably could be "fastest"
    int error;
    src_state.reset (src_new (SRC_SINC_MEDIUM_QUALITY, 1, &error));
    src_set_ratio (src_state.get(), m_ratio);

    int error_out;
    src_state_out.reset (src_new (SRC_SINC_MEDIUM_QUALITY, 1, &error_out));
    src_set_ratio (src_state_out.get(), 1./m_ratio);

    mCalcFunc = make_calc_function<Proteus, &Proteus::next_a>();
    next_a(1);
  }
  Proteus::~Proteus() {
    RTFree(mWorld, in_rs);
    //RTFree(mWorld, in1_rs);
    RTFree(mWorld, out_temp);
  }

  void load_model (Proteus* unit, sc_msg_iter* args) {
    const char *path = args->gets();
    const bool verbose = args->geti();    

    try {
      unit->LSTM.reset();
      unit->LSTM.load_json(path);
      
      if (verbose) {
        Print("Proteus Model Loaded: %s\n", path);
      }
      unit->m_model_loaded = true;

    } catch (const std::exception& e) {
      unit->m_model_loaded = false;
      std::cerr << "error loading the model: " << e.what() << "\n";
      //return -1;
    }

}

  void Proteus::next_a(int nSamples)
  {
    const float *in_0 = in(In0);
    const float in_1 = in0(In1);
    const float bypass = in0(Bypass);
    float *outbuf = out(Out1);

    if (m_model_loaded==true) {
      if ((int)bypass==1) {
        for (int i = 0; i < nSamples; ++i) {
          outbuf[i] = in_0[i];
        }
      } else {
        //resample the input to 44.1kHz
        if (m_resample) {

          int block44k_size = resample (in_0, in_rs, nSamples);
          switch (LSTM.input_size) {
            case 1: {
              LSTM.process(in_rs, out_temp, block44k_size);
              break;
            }
            case 2: {

              LSTM.process(in_rs, in_1, out_temp, block44k_size);
              break;
            }
            case 3: {
              //not implemented yet
              break;
            }
          }
          //resample the output back to the original sample rate and write to the output buffer
          int block44k_size2 = resample_out (out_temp, outbuf, block44k_size, nSamples);
        } else {
          switch (LSTM.input_size) {
            case 1: {
              LSTM.process(in_0, outbuf, nSamples);
              break;
            }
            case 2: {
              LSTM.process(in_0, in_1, outbuf, nSamples);
              break;
            }
            case 3: {
              //not implemented yet
              break;
            }
          }
        }
      }
    } else {
      for (int i = 0; i < nSamples; ++i) {
        outbuf[i] = in_0[i];
      }
    }
  }

PluginLoad(Proteus)
{
  ft = inTable;
  registerUnit<Proteus>(ft, "Proteus", false);
  DefineUnitCmd("Proteus", "load_model", (UnitCmdFunc)&load_model);

}
