#include "al/app/al_App.hpp"
#include <iostream>

using namespace al;

float PARAM = 3.1;
float INIT_INPUT = 0.9;
int MAX_ITER = 100;

float h(float x) { return PARAM * x * (1 - x); }

struct MyApp : public App {
  Mesh shape;
  Texture texBlur;
  float angle = 0;
  float FPS = 5;
  float last_h = INIT_INPUT;
  int frame_counter = 0;

  void onCreate() override {
    fps(FPS);
    shape.primitive(Mesh::LINE_STRIP);
  }

  void onAnimate(double dt_sec) override {}

  void onDraw(Graphics &g) override {
    if (frame_counter > MAX_ITER) {
      g.clear(0);
      frame_counter = 0;
      last_h = INIT_INPUT;
      shape.reset();
    }

    frame_counter++;
    last_h = h(last_h);
    shape.vertex(static_cast<float>(frame_counter) / MAX_ITER, last_h);
    shape.color(RGB(1.0f, 1.0f, 1.0f));

    // 3. Do your drawing...
    g.camera(Viewpoint::UNIT_ORTHO); // ortho camera that fits [-1:1] x [-1:1]
    g.meshColor();                   // use mesh's color array
    g.draw(shape);
  }
};

int main() {
  MyApp app;
  app.start();
}