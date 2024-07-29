
//#pragma once

#include "SC_PlugIn.hpp"
#include <RTNeural/RTNeural.h>
#include "RTNeuralLSTM.h"
#include "libsamplerate/include/samplerate.h"


// namespace Proteus {
 
class Proteus : public SCUnit {
public:
  Proteus();

  // Destructor
  ~Proteus();

  //RTNeural::Model<float> model;

  RT_LSTM LSTM;
  bool m_model_loaded{false};

private:
  // Calc function
  void next_a(int nSamples);

  void load_model(Proteus* unit, sc_msg_iter* args);

  size_t resample (const float* input, float* output, size_t numSamples) noexcept;
  size_t resample_1 (const float* input, float* output, size_t inSamples) noexcept;
  size_t resample_out (const float* input, float* output, size_t inSamples, size_t outSamples) noexcept;

  //create a pointer to the SRC_STATE struct, which we will use to manage the sample rate conversion
  //we only need to do this because Proteus models are trained at 44.1kHz and need that as the input sample rate
  //std::unique_ptr<SRC_STATE, decltype (&src_delete)> src_state { nullptr, &src_delete };

  //SRC_STATE* src_state;

  std::unique_ptr<SRC_STATE, decltype (&src_delete)> src_state { nullptr, &src_delete };
  std::unique_ptr<SRC_STATE, decltype (&src_delete)> src_state_out { nullptr, &src_delete };

  //not using this one right now
  //std::unique_ptr<SRC_STATE, decltype (&src_delete)> src_state1 { nullptr, &src_delete };

  enum InputParams { In0, In1, Bypass, NumInputParams };
  enum Outputs { Out1, NumOutputParams };

  float m_step_val{1.f/(float)sampleRate()};
  float m_sample_rate{(float)sampleRate()};

  int out_temp_size{128};
  float *in_rs;
  // float *in1_rs;
  float *out_temp;

  double m_ratio{1.0};

  int m_bypass{1};
};

// } // namespace Proteus

