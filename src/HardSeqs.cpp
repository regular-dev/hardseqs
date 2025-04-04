#include "HardSeqs.hpp"

#include <iostream>
#include "jansson.h"

constexpr const float kStepEnabled = 0.1;
constexpr const float kStepPlaying = 0.9;
constexpr const float kModOutputDenum = 10.0;

template<typename T>
T clamp(const T &v, const T &val_max, const T &val_min) {
    if (v > val_max)
        return val_max;
    else if (v < val_min)
        return val_min;
    else
        return v;
}

HardSeqs::HardSeqs() 
{
    std::cout << "CREATED MODULE\n";

    config(PARAM_COUNT, INP_COUNT, OUT_COUNT, LED_COUNT);

    configParam(HardSeqs::PARAM_LEN, 0.0, 16.0, 16.0);
    configParam(HardSeqs::PARAM_REPEAT_N, 0.0, 4.0, 0.0);

    configParam(HardSeqs::PARAM_STEP_PROB, 0.0, 100.0, kStepDefaultProb);
    configParam(HardSeqs::PARAM_STEP_MOD1, -100.0, 100.0, kStepDefaultMod1);
    configParam(HardSeqs::PARAM_STEP_MOD2, -100.0, 100.0, kStepDefaultMod2);
    configParam(HardSeqs::PARAM_STEP_MOD3, -100.0, 100.0, kStepDefaultMod3);
    configParam(HardSeqs::PARAM_STEP_ELEN, 0.0, 5.0, kStepDefaultElen);

    getParam(PARAM_STEP1 + m_selected_step).setValue(1.0);
}

void HardSeqs::setSelectedStep(int step)
{
    m_selected_step = step;
    std::cout << "selected hardseq step : " << static_cast<int>(m_selected_step) << "\n";

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
    const auto cv_start = inputs[INP_RUN].getVoltage();
    const auto cv_pos = inputs[INP_POS].getVoltage();
    const auto cv_clock = inputs[INP_CLOCK].getVoltage();
    const auto cv_reset = inputs[INP_RST].getVoltage();

    m_cv_run.update(cv_start);
    m_cv_clock.update(cv_clock);
    m_cv_reset.update(cv_reset);

    // cv run
    if (m_cv_run.newTrigger())
    {
        m_is_running = !m_is_running;
        lights[LED_IS_RUNNING].value = static_cast<float>(m_is_running); 
    }

    if (cv_pos > 0.0)
    {
        m_start_pos = static_cast<int>((16.0 * cv_pos) / 5.0);
        m_start_pos = clamp(m_start_pos, 0.0, kLenSteps - 1);
        // cv pos can modulate from 0...5V, where 0 = first step, 5V = last step.
    } else {
        //std::cout << "setting start pos = 0\n";
        m_start_pos = 0;
    }

    // cv reset
    if (m_cv_reset.newTrigger())
    {
        resetSteps();
    }

    clearAllStepLights();
    clearAllStepOutputs();

    if (cv_clock < kCvThreshold) {
        outputs[OUT_GATE].setVoltage(0.0);

        for (int i = OUT_STEP1; i <= OUT_STEP16; ++i) {
            outputs[i].setVoltage(0.0);
        }
    }

    // cv clock
    if (m_cv_clock.newTrigger() && m_is_running)
    {
        auto &step_entry = m_steps[m_current_step];
        
        bool is_trigger = false;
        const auto is_loop_trigger = step_entry.isTrigger();

        if (is_loop_trigger) {
            const auto prob_val = rand_gen_.randomPercent(step_entry.prob);

            if (step_entry.prob == 100 || prob_val) {
                is_trigger = step_entry.is_enabled;
            }
        }

        outputs[OUT_STEP1 + m_current_step].setVoltage(is_trigger ? kMaximumVoltage : 0.0);
        outputs[OUT_GATE].setVoltage(is_trigger ? kMaximumVoltage : 0.0);

        outputs[OUT_MOD1].setVoltage(step_entry.mod1 / kModOutputDenum);
        outputs[OUT_MOD2].setVoltage(step_entry.mod2 / kModOutputDenum);
        outputs[OUT_MOD3].setVoltage(step_entry.mod3 / kModOutputDenum);

        m_current_step++;

        if (m_current_step >= std::min(static_cast<int>(m_start_pos + getParam(PARAM_LEN).value), kLenSteps)) {
            m_current_step = m_start_pos;

            for (auto &it_step : m_steps)
                it_step.incrementLoop();

            m_cur_loop++;
            if (m_cur_loop >= getParam(PARAM_REPEAT_N).value && getParam(PARAM_REPEAT_N).value != 0.0) {
                m_is_running = false;
            }
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
    auto &cur_entry = m_steps[m_selected_step];

    if (step_param_id == PARAM_STEP_ENABLED) {
        cur_entry.is_enabled = static_cast<bool>(getParam(step_param_id).value);
    } else if (step_param_id == PARAM_STEP_EACH1) {
        cur_entry.each_n[0] = getParam(step_param_id).value == 1.0;
    } else if (step_param_id == PARAM_STEP_EACH2) {
        cur_entry.each_n[1] = getParam(step_param_id).value == 1.0;
    } else if (step_param_id == PARAM_STEP_EACH3) {
        cur_entry.each_n[2] = getParam(step_param_id).value == 1.0;
    } else if (step_param_id == PARAM_STEP_EACH4) {
        cur_entry.each_n[3] = getParam(step_param_id).value == 1.0;
    } else if (step_param_id == PARAM_STEP_EACH5) {
        cur_entry.each_n[4] = getParam(step_param_id).value == 1.0;
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

json_t* HardSeqs::dataToJson()
{
    json_t* out = json_object();

    json_t* steps_array = json_array();

    for (const auto &it : m_steps) {
        json_t* json_entry = json_object();

        json_object_set_new(json_entry, "is_enabled", json_integer(static_cast<int>(it.is_enabled)));
        json_object_set_new(json_entry, "prob", json_integer(it.prob));
        json_object_set_new(json_entry, "mod1", json_real(it.mod1));
        json_object_set_new(json_entry, "mod2", json_real(it.mod2));
        json_object_set_new(json_entry, "mod3", json_real(it.mod3));
        json_object_set_new(json_entry, "len_each_n", json_integer(it.len_each_n));

        json_object_set_new(json_entry, "each_step1_enabled", json_integer(static_cast<int>(it.each_n[0])));
        json_object_set_new(json_entry, "each_step2_enabled", json_integer(static_cast<int>(it.each_n[1])));
        json_object_set_new(json_entry, "each_step3_enabled", json_integer(static_cast<int>(it.each_n[2])));
        json_object_set_new(json_entry, "each_step4_enabled", json_integer(static_cast<int>(it.each_n[3])));
        json_object_set_new(json_entry, "each_step5_enabled", json_integer(static_cast<int>(it.each_n[4])));

        json_array_append_new(steps_array, json_entry);
    }

    json_object_set_new(out, "steps", steps_array);
    json_object_set_new(out, "is_running", json_integer(static_cast<int>(m_is_running)));

    return out;
}

void HardSeqs::dataFromJson(json_t* from)
{
    json_t* steps_array = json_object_get(from, "steps");
    json_t* is_running = json_object_get(from, "is_running");

    std::size_t index;
    json_t* json_entry;

    json_array_foreach(steps_array, index, json_entry) {
        json_t* val_is_enabled = json_object_get(json_entry, "is_enabled");
        json_t* val_prob = json_object_get(json_entry, "prob");
        json_t* val_mod1 = json_object_get(json_entry, "mod1");
        json_t* val_mod2 = json_object_get(json_entry, "mod2");
        json_t* val_mod3 = json_object_get(json_entry, "mod3");
        json_t* val_len_each_n = json_object_get(json_entry, "len_each_n");

        json_t* val_each_step1_enabled = json_object_get(json_entry, "each_step1_enabled");
        json_t* val_each_step2_enabled = json_object_get(json_entry, "each_step2_enabled");
        json_t* val_each_step3_enabled = json_object_get(json_entry, "each_step3_enabled");
        json_t* val_each_step4_enabled = json_object_get(json_entry, "each_step4_enabled");
        json_t* val_each_step5_enabled = json_object_get(json_entry, "each_step5_enabled");

        m_steps[index].is_enabled = static_cast<bool>(json_integer_value(val_is_enabled));
        m_steps[index].prob = static_cast<int>(json_integer_value(val_prob));
        m_steps[index].mod1 = static_cast<float>(json_real_value(val_mod1));
        m_steps[index].mod2 = static_cast<float>(json_real_value(val_mod2));
        m_steps[index].mod3 = static_cast<float>(json_real_value(val_mod3));
        m_steps[index].len_each_n = static_cast<int>(json_integer_value(val_len_each_n));

        m_steps[index].each_n[0] = static_cast<bool>(json_integer_value(val_each_step1_enabled));
        m_steps[index].each_n[1] = static_cast<bool>(json_integer_value(val_each_step2_enabled));
        m_steps[index].each_n[2] = static_cast<bool>(json_integer_value(val_each_step3_enabled));
        m_steps[index].each_n[3] = static_cast<bool>(json_integer_value(val_each_step4_enabled));
        m_steps[index].each_n[4] = static_cast<bool>(json_integer_value(val_each_step5_enabled));
    }

    m_is_running = static_cast<bool>(json_integer_value(is_running));

    getParam(PARAM_STEP1).setValue(1.0);
    setSelectedStep(0);
}

void HardSeqs::clearAllStepLights()
{
    for (int i = 0; i < kLenSteps; ++i) {
        lights[i + LED_STEP1].value = m_steps[i].is_enabled ? kStepEnabled : 0.0;
    }

    lights[LED_STEP1 + m_current_step].value = kStepPlaying;
}

void HardSeqs::clearAllStepOutputs()
{
    for (int i = OUT_STEP1; i <= OUT_STEP16; ++i)
        outputs[i].setVoltage(0.0);
}

void HardSeqs::resetSteps()
{
    m_current_step = m_start_pos;

    for (auto &it : m_steps)
        it.cur_n = 0;
}

void HardSeqs::StepEntry::incrementLoop()
{
    cur_n += 1;
    if (cur_n >= len_each_n)
        cur_n = 0;
}

bool HardSeqs::StepEntry::isTrigger() const
{
    return each_n[cur_n];
}