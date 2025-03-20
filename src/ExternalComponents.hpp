#pragma once
/*
Shadow code taken and modified from https://github.com/lindenbergresearch/LRTRack

and is available under the following license:

Copyright (c) 2017-2018, Lindenberg Research / Patrick Lindenberg
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Commercial redistribution of the code, or parts, in any form
  must be granted by the author.

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

* Neither the name of Lindenberg Research nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.


THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

struct CDShadow {
private:
  Rect box;
  float size = 0.65;
  float strength = 0.8f;

  Vec shadowPos = Vec(3, 5);

public:
  void setShadowPosition(float x, float y) {
    shadowPos = Vec(x, y);
  }

  void setBox(const Rect &box) {
    CDShadow::box = box;
  }

  void setSize(float size) {
    CDShadow::size = size;
  }

  void setStrength(float strength) {
    CDShadow::strength = strength;
  }

  void drawShadow(NVGcontext *vg, float strength, float size) {
    // add shadow
    nvgBeginPath(vg);
    nvgRect(vg, -20, -20, box.size.x + 40, box.size.y + 40);

    NVGcolor icol = nvgRGBAf(0.0f, 0.0f, 0.0f, strength);
    NVGcolor ocol = nvgRGBAf(0.0f, 0.0f, 0.0f, 0.f);
    ;

    NVGpaint paint = nvgRadialGradient(
        vg, box.size.x / 2 + shadowPos.x, box.size.y / 2 + shadowPos.y,
        box.size.x * 0.3f, box.size.x * size, icol, ocol);
    nvgFillPaint(vg, paint);
    nvgFill(vg);
  }

  void draw(NVGcontext *vg) {
    drawShadow(vg, strength, size);
  };
};

struct CDKnob : RoundKnob {
protected:
  CDShadow shadow = CDShadow();

public:
  CDKnob() {
  }

  void setSVG(std::shared_ptr<Svg> svg) {
    app::SvgKnob::setSvg(svg);

    shadow.setBox(box);
  }

  void draw(const DrawArgs &args) override {
    /** shadow */
    shadow.draw(args.vg);

    /** component */
    app::SvgKnob::draw(args);
  }
};

struct LightKnob : CDKnob {
  LightKnob() {
    setSVG(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Knob.svg")));
  }
};

struct LightKnobSnap : LightKnob {
  LightKnobSnap() {
    snap = true;
  }
};

struct LightKnobSmall : CDKnob {
  LightKnobSmall() {
    setSVG(APP->window->loadSvg(asset::plugin(pluginInstance, "res/KnobSmall.svg")));
  }
};

struct LocalPort : PortWidget {
  widget::FramebufferWidget *fb;
  widget::SvgWidget *sw;

  LocalPort() {
    fb = new widget::FramebufferWidget;
    addChild(fb);

    sw = new widget::SvgWidget;
    fb->addChild(sw);
  }

  void setSvg(std::shared_ptr<Svg> svg) {
    sw->setSvg(svg);
    fb->box.size = sw->box.size;
    box.size = sw->box.size;

    fb->dirty = true;
  }

};

struct CDPort : LocalPort {
private:
  CDShadow shadow = CDShadow();

public:
  CDPort() {
    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Port.svg")));

    /** inherit dimensions */
    shadow.setBox(box);
    shadow.setSize(0.4);
    shadow.setShadowPosition(2, 1);
  }

  void draw(const DrawArgs &args) override {
    shadow.draw(args.vg);
    PortWidget::draw(args);
  }
};