//
// Created by Brandon on 1/5/25.
//

#include "IntroScene.h"
#include "Clock.h"

IntroScene::IntroScene(rgb::PixelList& ring) : ring(ring) {

}

auto IntroScene::setup() -> void {
  trailingEffect.offset = 4;
  trailingEffect.trailRatio = 6.0f / 12;
  trailingEffect.speed = rgb::Duration::Milliseconds(30);
  trailingEffect.shader = [&](auto& led, auto& params) {
    auto time = rgb::Clock::Now().mod(speed).to<float>();
    auto hue = rgb::LerpWrap(0.0f, 1.0f, time / speed.to<float>());
    auto color  = rgb::Color::HslToRgb(hue);
    led = color * .03f;
  };
  trailingEffect.init();
}

auto IntroScene::update() -> void {
  trailingEffect.update();
}

auto IntroScene::draw() -> void {
  trailingEffect.draw(ring);
}

auto IntroScene::cleanup() -> void {

}