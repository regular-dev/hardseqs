#pragma once

#include "asset.hpp"
#include "rack.hpp"
#include "ExternalComponents.hpp"

#include <functional>

using namespace rack;

struct CommonSwitch : app::SvgSwitch
{
    protected:
        CDShadow m_shadow = CDShadow();

    public:

        CommonSwitch()
        {
            //momentary = true;
		    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/CommonSwitch.svg")));
		    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/CommonSwitchSel.svg")));
            m_shadow.setBox(box);
        }

        void draw(const DrawArgs &args) override 
        {
            m_shadow.draw(args.vg);
            app::SvgSwitch::draw(args);
        }
};

struct SmallSwitch : app::SvgSwitch
{
    protected:
        using Callback = std::function<void(int idx)>;
        Callback m_callback;
        int m_idx {-1};

    public:
        SmallSwitch()
        {
            addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/SmallSwitch.svg")));
		    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/SmallSwitchSel.svg")));
        }
        
        void onButton(const ButtonEvent &e) override
        {
            if (m_callback && e.action == GLFW_PRESS)
                m_callback(m_idx);
        }

        void setStepIndex(int idx)
        {
            m_idx = idx;
        }

        void setCallback(Callback f)
        {
            m_callback = f;
        }
};

struct SmallPort : CDPort
{
    SmallPort()
    {
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/SmallPort.svg")));
    }
};

struct LightSwitch : app::SvgSwitch
{
    protected:
        CDShadow m_shadow = CDShadow();
        using Callback = std::function<void(int idx)>;
        Callback m_callback;

    public:
        LightSwitch()
        {
            addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/LightSwitch.svg")));
            addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/LightSwitchSel.svg")));
            m_shadow.setBox(box);
        }

        void setCallback(Callback f)
        {
            m_callback = f;
        }

        void onButton(const ButtonEvent &e) override
        {
            if (m_callback && e.action == GLFW_PRESS)
                m_callback(paramId);
        }
};

struct CustomLightKnob : LightKnobSnap
{
    protected:
        using Callback = std::function<void(int param_id)>;
        Callback m_callback;

    public:
        CustomLightKnob() : LightKnobSnap() { }

        void setCallback(Callback f)
        {
            m_callback = f;
        }

        void onChange(const ChangeEvent &e) override
        {
            RoundKnob::onChange(e);

            if (m_callback)
                m_callback(paramId);
        }
};
