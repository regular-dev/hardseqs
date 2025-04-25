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

struct CustomLightSnapFreeKnob : CustomLightKnob
{
    public:
        CustomLightSnapFreeKnob() : CustomLightKnob() { snap = false; }
};

struct SpriteSwitcher : SvgSwitch
{
    protected:
        std::vector<std::shared_ptr<Svg>> m_frames;
        int m_cur_frame = 0;
        SvgWidget* m_svg_widget = nullptr;
        bool m_is_hovered = false;
        float m_glow_size = 6.0f;
        NVGcolor m_glow_color = nvgRGBA(255, 255, 128, 128);
        std::function<void(int)> m_callback;

    public:
        SpriteSwitcher()
        {
            m_svg_widget = new SvgWidget;
            addChild(m_svg_widget);
        }

        void setCallback(std::function<void(int)> callback)
        {
            m_callback = callback;
        }
        
        void addFrame(std::shared_ptr<Svg> svg) 
        {
            m_frames.push_back(svg);
            if (m_frames.size() == 1) 
            {
                m_svg_widget->setSvg(svg);
                box.size = svg->getSize();
            }
        }

        void setFrame(int frame_id)
        {
            if (frame_id < 0 || frame_id >= m_frames.size())
                return;

            m_cur_frame = frame_id;
            m_svg_widget->setSvg(m_frames[m_cur_frame]);
        }
        
        void onButton(const ButtonEvent& e) override 
        {
            if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_LEFT) 
            {
                m_cur_frame = (m_cur_frame + 1) % m_frames.size();

                if (m_callback)
                    m_callback(m_cur_frame);

                m_svg_widget->setSvg(m_frames[m_cur_frame]);
                e.consume(this);
            }
        }
        
        void onEnter(const EnterEvent& e) override 
        {
            m_is_hovered = true;
            SvgSwitch::onEnter(e);
        }
        
        void onHover(const HoverEvent& e) override 
        {
            e.consume(this);
        }

        void onLeave(const LeaveEvent& e) override 
        {
            m_is_hovered = false;
            SvgSwitch::onLeave(e);
        }

        void draw(const DrawArgs& args) override 
        {
            if (m_is_hovered) 
            {
                nvgBeginPath(args.vg);
                nvgRoundedRect(args.vg, 
                            -m_glow_size, 
                            -m_glow_size, 
                            box.size.x + m_glow_size*2, 
                            box.size.y + m_glow_size*2, 
                            3.0);
                
                NVGpaint paint = nvgBoxGradient(
                    args.vg,
                    m_glow_size, m_glow_size,
                    box.size.x - m_glow_size*2, box.size.y - m_glow_size*2,
                    3.0, 10.0,
                    m_glow_color, nvgRGBA(0, 0, 0, 0)
                );
                nvgFillPaint(args.vg, paint);
                nvgFill(args.vg);
            }
            
            Widget::draw(args);
        }
};