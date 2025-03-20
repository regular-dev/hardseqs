#include "HardSeqs.hpp"

#include <iostream>

#define ADD_CV(a, b) clamp(a.value + b.value, 0.0f, 10.0f)
constexpr const float kCvThreshold = 1.7;
constexpr const float kStepEnabled = 0.3;
constexpr const float kStepPlaying = 1.0;

HardSeqs::HardSeqs() 
{
    config(PARAM_COUNT, INP_COUNT, OUT_COUNT, LED_COUNT);
    m_cv_start.reset(new SynthDevKit::CV(kCvThreshold));
    m_cv_stop.reset(new SynthDevKit::CV(kCvThreshold));
    m_cv_clock.reset(new SynthDevKit::CV(kCvThreshold));
    m_cv_reset.reset(new SynthDevKit::CV(kCvThreshold));

    configParam(HardSeqs::PARAM_LEN, 1.0, 4.0, 4.0);
    configParam(HardSeqs::PARAM_REPEAT_N, 0.0, 4.0, 0.0);

    configParam(HardSeqs::PARAM_STEP_PROB, 0.0, 100.0, kStepDefaultProb);
    configParam(HardSeqs::PARAM_STEP_MOD1, -100.0, 100.0, kStepDefaultMod1);
    configParam(HardSeqs::PARAM_STEP_MOD2, -100.0, 100.0, kStepDefaultMod2);
    configParam(HardSeqs::PARAM_STEP_MOD3, -100.0, 100.0, kStepDefaultMod3);
    configParam(HardSeqs::PARAM_STEP_ELEN, 0.0, 5.0, kStepDefaultElen);

    getParam(PARAM_STEP1 + m_current_step).setValue(1.0);
}

void HardSeqs::setCurrentStep(int step)
{
    m_current_step = step;
    std::cout << "current hardseq step : " << static_cast<int>(m_current_step) << "\n";

    // Update step params from local
    const auto& local_entry = m_steps.at(step);
    // Each step enabled param
    getParam(PARAM_STEP_ENABLED).setValue(static_cast<float>(local_entry.is_enabled));
    // Each step param
    getParam(PARAM_STEP_EACH1).setValue(static_cast<float>(local_entry.each_n[0]));
    getParam(PARAM_STEP_EACH2).setValue(static_cast<float>(local_entry.each_n[1]));
    getParam(PARAM_STEP_EACH3).setValue(static_cast<float>(local_entry.each_n[2]));
    getParam(PARAM_STEP_EACH4).setValue(static_cast<float>(local_entry.each_n[3]));
    getParam(PARAM_STEP_EACH5).setValue(static_cast<float>(local_entry.each_n[4]));
    // Each step probability
    getParam(PARAM_STEP_PROB).setValue(static_cast<float>(local_entry.prob));
    getParam(PARAM_STEP_MOD1).setValue(local_entry.mod1);
    getParam(PARAM_STEP_MOD2).setValue(local_entry.mod2);
    getParam(PARAM_STEP_MOD3).setValue(local_entry.mod3);
    getParam(PARAM_STEP_ELEN).setValue(local_entry.len_each_n);
}

void HardSeqs::process(const ProcessArgs &args)
{
    float cv_start = inputs[INP_START].getVoltage();
    float cv_stop = inputs[INP_STOP].getVoltage();
    float cv_clock = inputs[INP_CLOCK].getVoltage();
    float cv_reset = inputs[INP_RST].getVoltage();

    m_cv_start->update(cv_start);
    m_cv_stop->update(cv_stop);
    m_cv_clock->update(cv_clock);
    m_cv_reset->update(cv_reset);

    // cv start
    if (m_cv_start->newTrigger())
    {
        m_is_running = true;
        lights[LED_IS_RUNNING].value = 1.0;  
    }

    // cv stop
    if (m_cv_stop->newTrigger())
    {
        m_is_running = false;
        lights[LED_IS_RUNNING].value = 0.0;
    }

    // cv reset
    if (m_cv_reset->newTrigger())
    {
        m_current_step = 0;
        m_current_loop = 0;
    }

    // cv clock
    if (m_cv_clock->newTrigger())
    {
        outputs[OUT_GATE].setVoltage(m_steps[m_current_step].is_enabled ? 1.0 : 0.0);
        outputs[OUT_MOD1].setVoltage(m_steps[m_current_step].mod1);
        outputs[OUT_MOD2].setVoltage(m_steps[m_current_step].mod2);
        outputs[OUT_MOD3].setVoltage(m_steps[m_current_step].mod3);

        outputs[m_current_step].setVoltage(m_steps[m_current_step].is_enabled ? 1.0 : 0.0);

        // TODO : light led impl

        m_current_step++;

        if (m_current_step == m_length) {
            m_current_step = 0;

            m_current_loop++;
            if (m_current_loop == m_loop_length)
                m_current_loop = 0;
        }
    }

    // lights
    const auto repeat_n_val = getParam(ParamIds::PARAM_REPEAT_N).value;
    if (repeat_n_val == 0) {
        lights[LED_IS_ONCE].value = 0.0;
    } else if (repeat_n_val == 1) {
        lights[LED_IS_ONCE].value = 0.25;
    } else if (repeat_n_val == 2) {
        lights[LED_IS_ONCE].value = 0.45;
    } else if (repeat_n_val == 3) {
        lights[LED_IS_ONCE].value = 0.7;
    } else if (repeat_n_val == 4) {
        lights[LED_IS_ONCE].value = 1.0;
    }

    lights[LED_IS_RUNNING].value = m_is_running ? 1.0 : 0.0;
    lights[LED_STEP1].value = kStepEnabled;
}

void HardSeqs::stepParamChangedHandler(int step_param_id)
{
    std::cout << "hardseqs changed param : " << step_param_id << "\n";

    if (step_param_id >= PARAM_STEP_ENABLED && step_param_id <= PARAM_STEP_EACH5) {
        const auto param_val = getParam(step_param_id).value;
        const auto new_val = param_val == 0.0 ? 1.0 : 0.0;
        getParam(step_param_id).setValue(new_val);
    }

    syncParamWithLocalSteps(step_param_id);
}

void HardSeqs::syncParamWithLocalSteps(int step_param_id)
{
    auto &cur_entry = m_steps[m_current_step];

    if (step_param_id == PARAM_STEP_ENABLED) {
        cur_entry.is_enabled = static_cast<bool>(getParam(step_param_id).value);
    } else if (step_param_id == PARAM_STEP_EACH1) {
        cur_entry.each_n[0] = static_cast<bool>(getParam(step_param_id).value);
    } else if (step_param_id == PARAM_STEP_EACH2) {
        cur_entry.each_n[1] = static_cast<bool>(getParam(step_param_id).value);
    } else if (step_param_id == PARAM_STEP_EACH3) {
        cur_entry.each_n[2] = static_cast<bool>(getParam(step_param_id).value);
    } else if (step_param_id == PARAM_STEP_EACH4) {
        cur_entry.each_n[3] = static_cast<bool>(getParam(step_param_id).value);
    } else if (step_param_id == PARAM_STEP_EACH5) {
        cur_entry.each_n[4] = static_cast<bool>(getParam(step_param_id).value);
    } else if (step_param_id == PARAM_STEP_PROB) {
        cur_entry.prob = getParam(step_param_id).value;
    } else if (step_param_id == PARAM_STEP_MOD1) {
        cur_entry.mod1 = getParam(step_param_id).value;
    } else if (step_param_id == PARAM_STEP_MOD2) {
        cur_entry.mod2 = getParam(step_param_id).value;
    } else if (step_param_id == PARAM_STEP_MOD3) {
        cur_entry.mod3 = getParam(step_param_id).value;
    } else if (step_param_id == PARAM_STEP_ELEN) {
        cur_entry.len_each_n = static_cast<int>(getParam(step_param_id).value);
    }
}

json_t* HardSeqs::toJson()
{
    return Module::toJson();
}

void HardSeqs::fromJson(json_t* from)
{
    Module::fromJson(from);
}