/*
Allocore Example: Frame Feedback

Description:
This demonstrates how to create a feedback post-processing effect.
This is accomplished by copying the frame buffer into a texture after rendering
and then displaying the texture at the start of the next frame. Different
feedback effects can be accomplished by distorting the quad the texture is
rendered onto.

Author:
Lance Putnam, Nov. 2014
Keehong Youn, 2017
*/

#include "al/app/al_App.hpp"
#include "al/math/al_Random.hpp"
#include <iostream>
#include "Gamma/Oscillator.h"

using namespace al;

struct MyApp : public App {
  Mesh shape;
  Texture texBlur;
  float angle = 0;
  rnd::Random<rnd::Tausworthe> rng;
  rnd::Random<rnd::Tausworthe> rng_int;
  std::vector<Vec3f> PrevVertices;
  std::vector<Color> PrevColors;

  std::vector<Mesh> shapes;

  gam::LFO<> lfo_1{};

  float FPS = 60;

  static const int PRIMITIVE_SIZE = 10;
  int PRIMITIVE[PRIMITIVE_SIZE] = {5,4,13,4,5,1,2,3,1,2};

  int BLUR_STATE = 0;
  float PARAM_JITTER_PROB = 0.1;

  float LFO_1_FREQ = 0.1;
  float LFO_1_WIDTH = 1;

  Mesh::Primitive rand_primitive;

  void onCreate() override {
    //Set fps
    fps(FPS);
    gam::sampleRate(FPS);

    shape.primitive(Mesh::LINE_STRIP);
    const int N = 5;
    for (int i = 0; i < N; ++i) {
      float theta = float(i) / N * 2 * M_PI;
      shape.vertex(cos(theta), sin(theta));
      shape.color(HSV(theta / 2 / M_PI, 0.1));
    }
    texBlur.filter(Texture::NEAREST_MIPMAP_NEAREST);

    lfo_1.set(LFO_1_FREQ,0,0.5);
  }

  void onAnimate(double dt_sec) override {
    float angle_fuckery = rng.uniform(0.0, 120.0);
    angle += dt_sec * angle_fuckery;
    // angle += 90 * dt / 1000; // dt is in millis
    if (angle >= 360)
      angle = fmod((double)angle, 360.0);
  }

  void onDraw(Graphics &g) override {
    float lfo_1_val = 0.5 * (lfo_1.cos() + 1);

    g.clear(0);

    // Choose primitve.
    if (rng.prob(0.5)) {
      rng.shuffle(PRIMITIVE, PRIMITIVE_SIZE);
      rand_primitive = static_cast<Mesh::Primitive>(PRIMITIVE[0]);
      shape.primitive(rand_primitive);
    }

    PrevVertices.clear();
    PrevColors.clear();
    for (int i = 0; i < shape.vertices().size(); i++) {
      PrevVertices.push_back(shape.vertices()[i]);
      PrevColors.push_back(shape.colors()[i]);
    }
    shape.vertices().clear();
    shape.colors().clear();
    const int N = static_cast<int>(rng.uniform(3, 11));
    for (int i = 0; i < N; ++i) {
      float theta = float(i) / N * 2 * M_PI;
      Vec3f vert(powf(cos(theta), 3), powf(sin(theta), 7), (sin(theta)));
      Color col(HSV(theta / 2 / M_PI, 0.1));
      if (i >= PrevVertices.size()) {
        int rand_index = rng.uniform(0.0f, static_cast<float>(PrevVertices.size() - 1));

        if (rng.prob(0.005))
          vert *= PrevVertices[rand_index];
        else
          vert.cross(PrevVertices[rand_index]);
        // col = col / PrevColors[rand_index];
      } else {

        if (rng.prob(0.005))
          vert.cross(PrevVertices[i]);
        else
          vert *= PrevVertices[i];
        // col = col / PrevColors[i];
      }

      shape.vertex(vert);
      shape.color(col);
    }

    // 1. Match texture dimensions to window
    texBlur.resize((fbWidth()*2) * lfo_1_val, (fbHeight()*2) * lfo_1_val);

    // 2. Draw feedback texture. Try the different varieties!
    if(rand_primitive > 3)
      g.tint(0.98);
    else 
      g.tint(0.999);

    int pick_view = rng.uniform(-1, 2);
    if (rng.prob(PARAM_JITTER_PROB)) {
      BLUR_STATE = pick_view;
    }

    switch (BLUR_STATE) {
    case 0:
      g.quadViewport(texBlur, -0.995, -0.995, 1.99, 1.99); // Inward
      break;
    case 1:
      g.quadViewport(texBlur, -1.005, -0.995, 2.01, 1.99); // Squeeze
      break;
    case 2:
      g.quadViewport(texBlur, -1.005, -1.00, 2.01, 2.0); // Oblate
      break;
    default:
      g.quadViewport(texBlur, -1, -1, 2, 2);

    }

    // g.quadViewport(texBlur, -1.005, -1.005, 2.01, 2.01);  // Outward
    // g.quadViewport(texBlur, -0.995, -0.995, 1.99, 1.99); // Inward
    // g.quadViewport(texBlur, -1.005, -1.00, 2.01, 2.0);   // Oblate
    // g.quadViewport(texBlur, -1.005, -0.995, 2.01, 1.99); // Squeeze
    // g.quadViewport(texBlur, -1, -1, 2, 2);               // non-transformed
    //g.tint(1); // set tint back to 1

    // 3. Do your drawing...
    g.camera(Viewpoint::UNIT_ORTHO); // ortho camera that fits [-1:1] x [-1:1]
    g.rotate(angle, 0, 0, 1);
    g.meshColor(); // use mesh's color array
    g.draw(shape);

    // 4. Copy current (read) frame buffer to texture
    texBlur.copyFrameBuffer();
  }
};

int main() {
  MyApp app;
  app.start();
}
