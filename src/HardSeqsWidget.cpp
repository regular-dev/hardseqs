/*
 * This file is part of HardSeqs.
 *
 * HardSeqs is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * HardSeqs is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with HardSeqs. If not, see <http://www.gnu.org/licenses/>.
 *
 * Copyright (C) 2025 regular-dev team
 */

#include "HardSeqs.hpp"

#include "UiComponents.hpp"

#include <iostream>
#include <chrono>
#include <cstdint>


struct HardSeqsWidget : ModuleWidget 
{
    protected:
        HardSeqs *m_module {nullptr};
        std::uint64_t m_start_time;
        SpriteSwitcher *m_label {nullptr};

    public:
        HardSeqsWidget(HardSeqs *module);

        void stepSwitchHandler(int step_idx);
        void appendContextMenu(Menu *menu) override;
};

HardSeqsWidget::HardSeqsWidget(HardSeqs *module) 
{
    m_module = module;

    setModule(module);
    box.size = Vec(20 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/HardSeqs.svg")));

    /* -- Top Panel Rect Start -- */
    {
        // Start trigger input
        addInput(createInput<CDPort>(Vec(80, 18), module, HardSeqs::INP_RUN));

        // Stop trigger input
        addInput(createInput<CDPort>(Vec(129, 18), module, HardSeqs::INP_POS));

        // Clock input
        addInput(createInput<CDPort>(Vec(186, 18), module, HardSeqs::INP_CLOCK));

        // Reset
        addInput(createInput<CDPort>(Vec(244, 18), module, HardSeqs::INP_RST));
    }
    /* -- Top Panel Rect End -- */

    /* Left panel rect start */
    {
        // Sequence length
        addParam(createParam<LightKnobSnap>(Vec(19.0, 71.0), module, HardSeqs::PARAM_LEN));

        // IsOnce
        //CommonSwitch
        addParam(createParam<LightKnobSnap>(Vec(19.7, 135.7), module, HardSeqs::PARAM_REPEAT_N));

        // Is running led(light)
        addChild(createLight<MediumLight<RedLight>>(Vec(28.0, 201.145), module, HardSeqs::LED_IS_RUNNING));

        // Is once led(light)
        addChild(createLight<MediumLight<RedLight>>(Vec(28.0, 236.002), module, HardSeqs::LED_IS_ONCE));

        m_label = createParam<SpriteSwitcher>(Vec(15.0, 275.0), module, HardSeqs::PARAM_LABEL);

        m_label->setCallback([&] (int cur_label) { m_module->params[HardSeqs::PARAM_LABEL].setValue(cur_label - 1); } );

        m_label->addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/SeqNameKik1.svg")));
        m_label->addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/SeqNameKik2.svg")));
        m_label->addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/SeqNameCHat1.svg")));
        m_label->addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/SeqNameCHat2.svg")));
        m_label->addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/SeqNameOHat.svg")));
        m_label->addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/SeqNameSnr1.svg")));
        m_label->addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/SeqNameSnr2.svg")));
        m_label->addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/SeqNameTom1.svg")));
        m_label->addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/SeqNamePerc.svg")));
        m_label->addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/SeqNameKey1.svg")));
        m_label->addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/SeqNameKey2.svg")));

        if (module)
            m_label->setFrame(static_cast<int>(m_module->params.at(HardSeqs::PARAM_LABEL).value));

        addParam(m_label);

    }
    /* Left panel rect end */

    /* Right panel rect start */
    {
        addOutput(createOutput<CDPort>(Vec(250, 79.121), module, HardSeqs::OUT_GATE));
        addOutput(createOutput<CDPort>(Vec(250, 118.484), module, HardSeqs::OUT_MOD1));
        addOutput(createOutput<CDPort>(Vec(250, 157.096), module, HardSeqs::OUT_MOD2));
        addOutput(createOutput<CDPort>(Vec(250, 194.137), module, HardSeqs::OUT_MOD3));
    }
    /* Right panel rect end */

    /* StepButtons Start */
    constexpr const int kSwitchInRow = 4;
    constexpr const int kSwitchInCol = 4;

    constexpr const float kSwitchLeftY = 80.0;
    constexpr const float kSwitchLeftX = 79.0;
    constexpr const float kOutLeftX = kSwitchLeftX - 2;
    constexpr const float kOutLeftY = kSwitchLeftY + 18;
    constexpr const float kLedLeftX = 99.0;
    constexpr const float kLedLeftY = kSwitchLeftX + 7;

    constexpr const float kShiftX = 40.0;
    constexpr const float kShiftY = 40.0;

    int cur_param_idx = HardSeqs::ParamIds::PARAM_STEP1;
    int cur_out_idx = HardSeqs::OutputIds::OUT_STEP1;
    int cur_led_idx = HardSeqs::LightIds::LED_STEP1;

    for (auto i = 0; i < kSwitchInRow; ++i) {
        for (auto j = 0; j < kSwitchInCol; ++j ) {
            // Step Switcher
            auto param = createParam<SmallSwitch>(Vec(kSwitchLeftX + j * kShiftY, kSwitchLeftY + i * kShiftX), module, cur_param_idx);
            param->setCallback(std::bind(&HardSeqsWidget::stepSwitchHandler, this, cur_param_idx));
            addParam(param);
            cur_param_idx += 1;

            // Step Out
            auto out_port = createOutput<SmallPort>(Vec(kOutLeftX + j * kShiftY, kOutLeftY + i * kShiftX), module, cur_out_idx);
            addOutput(out_port);
            cur_out_idx += 1;

            // LED
            auto led = createLight<SmallLight<YellowLight>>(Vec(kLedLeftX + j * kShiftY, kLedLeftY + i * kShiftX), module, cur_led_idx);
            addChild(led);
            cur_led_idx += 1;
        }
    }
    /* StepButtons End*/

    /* Step Bottom Panel Start*/
    auto step_switch_step_enabled = createParam<LightSwitch>(Vec(85.0, 269.0), module, HardSeqs::ParamIds::PARAM_STEP_ENABLED);
    step_switch_step_enabled->setCallback(std::bind(&HardSeqs::stepParamChangedHandler, m_module, std::placeholders::_1));
    addChild(step_switch_step_enabled);

    constexpr const int kEachLen = 5;
    constexpr const float kEachLeftX = 129.0;
    constexpr const float kEachLeftY = 269.0;
    constexpr const float kShiftEachStepX = 18.0;

    int cur_each_len_id = HardSeqs::ParamIds::PARAM_STEP_EACH1;
    for (int i = 0; i < kEachLen; ++i) {
        auto step_each_n = createParam<LightSwitch>(Vec(kEachLeftX + i * kShiftEachStepX, kEachLeftY), module, cur_each_len_id);
        step_each_n->setCallback(std::bind(&HardSeqs::stepParamChangedHandler, m_module, std::placeholders::_1));
        addChild(step_each_n);

        cur_each_len_id += 1;
    }

    auto step_prob = createParam<CustomLightKnob>(Vec(80.5, 321.5), module, HardSeqs::ParamIds::PARAM_STEP_PROB);
    step_prob->setCallback(std::bind(&HardSeqs::stepParamChangedHandler, module, std::placeholders::_1));
    addChild(step_prob);

    auto step_mod1 = createParam<CustomLightSnapFreeKnob>(Vec(139.0, 321.5), module, HardSeqs::ParamIds::PARAM_STEP_MOD1);
    step_mod1->snap = false;
    step_mod1->setCallback(std::bind(&HardSeqs::stepParamChangedHandler, module, std::placeholders::_1));
    addChild(step_mod1);

    auto step_mod2 = createParam<CustomLightKnob>(Vec(191.0, 321.5), module, HardSeqs::ParamIds::PARAM_STEP_MOD2);
    step_mod2->setCallback(std::bind(&HardSeqs::stepParamChangedHandler, module, std::placeholders::_1));
    addChild(step_mod2);

    auto step_mod3 = createParam<CustomLightKnob>(Vec(241.0, 321.5), module, HardSeqs::ParamIds::PARAM_STEP_MOD3);
    step_mod3->setCallback(std::bind(&HardSeqs::stepParamChangedHandler, module, std::placeholders::_1));
    addChild(step_mod3);

    auto step_elen = createParam<CustomLightKnob>(Vec(241.0, 265.5), module, HardSeqs::ParamIds::PARAM_STEP_ELEN);
    step_elen->setCallback(std::bind(&HardSeqs::stepParamChangedHandler, module, std::placeholders::_1));
    addChild(step_elen);
    /* Step Bottom Panel End */
}

void HardSeqsWidget::stepSwitchHandler(int idx)
{
    #ifdef HS_DEBUG
    std::cout << "selected step : " << idx << "\n";
    #endif

    for (int i = HardSeqs::ParamIds::PARAM_STEP1; i < HardSeqs::ParamIds::PARAM_STEP16 + 1; ++i) {
        // m_module->getParam(i).setValue(0.0);
        m_module->getParam(i).setValue(0.0);
    }


    m_module->getParam(idx).setValue(1.0);
    m_module->setSelectedStep(idx - HardSeqs::ParamIds::PARAM_STEP1);
}

void HardSeqsWidget::appendContextMenu(Menu* menu) 
{
    ModuleWidget::appendContextMenu(menu);

    menu->addChild(new MenuSeparator());

    menu->addChild(createMenuItem("Disable all gates","",
    [this] ()
    {
        m_module->generateRandomGateSequence(0);
    }));
    menu->addChild(createMenuItem("Generate random gate sequence (temp = 10)","",
    [this] ()
    {
        m_module->generateRandomGateSequence(10);
    }));
    menu->addChild(createMenuItem("Generate random gate sequence (temp = 25)","",
    [this] ()
    {
        m_module->generateRandomGateSequence(25);
    }));
    menu->addChild(createMenuItem("Generate random gate sequence (temp = 50)","",
    [this] ()
    {
        m_module->generateRandomGateSequence(50);
    }));
    menu->addChild(createMenuItem("Generate random gate sequence (temp = 75)","",
    [this] ()
    {
        m_module->generateRandomGateSequence(75);
    }));
    menu->addChild(createMenuItem("Generate random gate sequence (temp = 90)","",
    [this] ()
    {
        m_module->generateRandomGateSequence(90);
    }));
}

Model *modelHardSeqs = createModel<HardSeqs, HardSeqsWidget>("HardSeqs");