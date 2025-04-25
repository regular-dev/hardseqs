#include <cstdint>
#include <array>
#include <memory>

#include "RandomGenerator.hpp"

#include "CV.hpp"
#include "Plugin.hpp"


constexpr const int kLenSteps = 16;
constexpr const int kLenEach = 5;
constexpr const float kMaximumVoltage = 10.0;
constexpr const float kCvThreshold = 0.5;

// Default step values
constexpr const bool kStepDefaultEnabled = 0.0;
constexpr const bool kStepDefaultEach1 = 1.0;
constexpr const bool kStepDefaultEach2 = 1.0;
constexpr const bool kStepDefaultEach3 = 1.0;
constexpr const bool kStepDefaultEach4 = 1.0;
constexpr const bool kStepDefaultEach5 = 1.0;
constexpr const float kStepDefaultProb = 100.0;
constexpr const float kStepDefaultMod1 = 0.0;
constexpr const float kStepDefaultMod2 = 0.0;
constexpr const float kStepDefaultMod3 = 0.0;
constexpr const float kStepDefaultElen = kLenEach;

struct HardSeqs : Module 
{
  enum ParamIds { 
    PARAM_REPEAT_N,
    PARAM_IS_RUN,
    PARAM_LEN,
    PARAM_SEL,

    PARAM_STEP_ENABLED,
    PARAM_STEP_EACH1,
    PARAM_STEP_EACH2,
    PARAM_STEP_EACH3,
    PARAM_STEP_EACH4,
    PARAM_STEP_EACH5,
    PARAM_STEP_PROB,
    PARAM_STEP_MOD1,
    PARAM_STEP_MOD2,
    PARAM_STEP_MOD3,
    PARAM_STEP_ELEN,

    PARAM_STEP1,
    PARAM_STEP2,
    PARAM_STEP3,
    PARAM_STEP4,
    PARAM_STEP5,
    PARAM_STEP6,
    PARAM_STEP7,
    PARAM_STEP8,
    PARAM_STEP9,
    PARAM_STEP10,
    PARAM_STEP11,
    PARAM_STEP12,
    PARAM_STEP13,
    PARAM_STEP14,
    PARAM_STEP15,
    PARAM_STEP16,

    PARAM_LABEL,

    PARAM_COUNT
  };

  // cv input
  enum InputIds {
    INP_RUN,
    INP_POS,
    INP_CLOCK,
    INP_RST,

    INP_COUNT
  };

  // output signals slots
  enum OutputIds {
    OUT_STEP1,
    OUT_STEP2,
    OUT_STEP3,
    OUT_STEP4,
    OUT_STEP5,
    OUT_STEP6,
    OUT_STEP7,
    OUT_STEP8,
    OUT_STEP9,
    OUT_STEP10,
    OUT_STEP11,
    OUT_STEP12,
    OUT_STEP13,
    OUT_STEP14,
    OUT_STEP15,
    OUT_STEP16,

    OUT_GATE,
    OUT_MOD1,
    OUT_MOD2,
    OUT_MOD3,

    OUT_COUNT
  };

  enum LightIds { 
    LED_IS_RUNNING,
    LED_IS_ONCE,

    LED_STEP1,
    LED_STEP2,
    LED_STEP3,
    LED_STEP4,
    LED_STEP5,
    LED_STEP6,
    LED_STEP7,
    LED_STEP8,
    LED_STEP9,
    LED_STEP10,
    LED_STEP11,
    LED_STEP12,
    LED_STEP13,
    LED_STEP14,
    LED_STEP15,
    LED_STEP16,

    LED_COUNT
  };

  struct StepEntry {
    bool is_enabled = kStepDefaultEnabled;

    std::array<bool, kLenEach> each_n = {kStepDefaultEach1, kStepDefaultEach2, kStepDefaultEach3, kStepDefaultEach4, kStepDefaultEach5};
    int len_each_n = kStepDefaultElen;
    int cur_n = 0;

    int prob = kStepDefaultProb;
    float mod1 = kStepDefaultMod1;
    float mod2 = kStepDefaultMod2;
    float mod3 = kStepDefaultMod3;

    void incrementLoop();
    bool isTrigger() const;

    StepEntry() = default;
  };

  HardSeqs();
  void process(const ProcessArgs &args) override;

  void setSelectedStep(int step);
  void stepParamChangedHandler(int step_param_id);
  void syncParamWithLocalSteps(int step_param_id);
  void clearAllStepLights();
  void clearAllStepOutputs();
  void resetSteps();

  json_t* dataToJson() override;
  void dataFromJson(json_t* root_json) override;

  SynthDevKit::CV m_cv_run {kCvThreshold};
  SynthDevKit::CV m_cv_clock {kCvThreshold};
  SynthDevKit::CV m_cv_reset {kCvThreshold};

  uint8_t m_start_pos = 0;
  uint8_t m_selected_step = 0;
  uint8_t m_current_step = 0;
  bool m_is_running = false;

  uint8_t m_cur_loop = 0;

  RandomGenerator rand_gen_;

  std::array<StepEntry, kLenSteps> m_steps = 
  {
    StepEntry(), StepEntry(), StepEntry(), StepEntry(),
    StepEntry(), StepEntry(), StepEntry(), StepEntry(),
    StepEntry(), StepEntry(), StepEntry(), StepEntry(),
    StepEntry(), StepEntry(), StepEntry(), StepEntry(),
  };
};