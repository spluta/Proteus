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

    out_temp_size = int(44100./controlRate())+2; //an extra one for safety

    in_rs = (float*)RTAlloc(mWorld, (double)out_temp_size * sizeof(float));
    //in1_rs = (float*)RTAlloc(mWorld, (double)out_temp_size * sizeof(float));
    out_temp = (float*)RTAlloc(mWorld, (double)out_temp_size * sizeof(float));

    int error;
    src_state.reset (src_new (SRC_SINC_MEDIUM_QUALITY, 1, &error));
    src_set_ratio (src_state.get(), m_ratio);

    int error_out;
    src_state_out.reset (src_new (SRC_SINC_MEDIUM_QUALITY, 1, &error_out));
    src_set_ratio (src_state_out.get(), 1./m_ratio);

    // //only need this for the 2 input case
    // int error1;
    // src_state1.reset (src_new (2, 2, &error1));
    // src_set_ratio (src_state1.get(), m_ratio);

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

    try {
      unit->LSTM.reset();
      unit->LSTM.load_json(path);
      
      Print("Load Proteus Model: %s\n", path);
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
        int block44k_size = resample (in_0, in_rs, nSamples);
        switch (LSTM.input_size) {
          case 1: {
            LSTM.process(in_rs, out_temp, block44k_size);
            break;
          }
          case 2: {
            // int temp = resample_1 (in_1, in1_rs, nSamples);
            // //bad hack to make sure the second input is the same size as the first
            // //probably need to use libsamplerate callback interface, but I'm not that smart
            // if (temp < block44k_size) {
            //   in1_rs[block44k_size-1] = in1_rs[block44k_size-2];
            // }
            LSTM.process(in_rs, in_1, out_temp, block44k_size);
            break;
          }
          //so far there are no models that use 3 inputs
          // case 3: {
          //   LSTM.process(in_rs, in_1, in_2, outbuf, nSamples);
          //   break;
          // }
        }
        //resample the output back to the original sample rate and write to the output buffer
        int block44k_size2 = resample_out (out_temp, outbuf, block44k_size, nSamples);
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
