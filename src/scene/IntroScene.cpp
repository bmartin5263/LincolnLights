//
// Created by Brandon on 1/5/25.
//

#include "IntroScene.h"
#include "Clock.h"
#include "LEDs.h"
#include "BrightnessControl.h"

auto IntroScene::setup() -> void {
  trailingEffect1.offset = 4;
  trailingEffect1.trailRatio = .5;
  trailingEffect1.speed = rgb::Duration::Milliseconds(30);
  trailingEffect1.cycles = 3;
  trailingEffect1.endBuffer = 10;
  // Rainbow
//  trailingEffect1.shader = [&](auto& led, auto& params) {
//    auto time = rgb::Clock::Now().mod(speed).to<float>();
//    auto hue = rgb::LerpWrap(0.0f, 1.0f, time / speed.to<float>());
//    auto color  = rgb::Color::HslToRgb(hue);
//    led = color * .03f;
//  };
  // Green
  trailingEffect1.shader = [&](auto& led, auto& params) {
    auto ratio = params.positionRatio;
    led = rgb::Color::GREEN() * brightness * ratio;
  };
  trailingEffect1.init();

  trailingEffect2.offset = 0;
  trailingEffect2.trailRatio = .5;
  trailingEffect2.speed = rgb::Duration::Milliseconds(20);
  trailingEffect2.cycles = 2;
  trailingEffect2.shader = [&](auto& led, auto& params) {
    auto ratio = params.positionRatio;
    led = rgb::Color::GREEN() * brightness * ratio;
  };
  trailingEffect2.init();
}

auto IntroScene::update() -> void {
  trailingEffect1.update();
  trailingEffect2.update();
}

auto IntroScene::draw() -> void {
  brightness = .03f;
  trailingEffect1.draw(ring);
  brightness = 1.0f;
  trailingEffect2.draw(leftStrip);
  trailingEffect2.draw(rightStrip);
}

auto IntroScene::cleanup() -> void {

}