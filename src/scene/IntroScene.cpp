//
// Created by Brandon on 1/5/25.
//

#include "IntroScene.h"
#include "Clock.h"

IntroScene::IntroScene(rgb::PixelList& ring, rgb::PixelList& strip) : ring(ring), strip(strip) {

}

auto IntroScene::setup() -> void {
  trailingEffect.offset = 4;
  trailingEffect.trailRatio = 6.0f / 12;
  trailingEffect.speed = rgb::Duration::Milliseconds(30);
//  trailingEffect.cycles = 2;
  // Rainbow
//  trailingEffect.shader = [&](auto& led, auto& params) {
//    auto time = rgb::Clock::Now().mod(speed).to<float>();
//    auto hue = rgb::LerpWrap(0.0f, 1.0f, time / speed.to<float>());
//    auto color  = rgb::Color::HslToRgb(hue);
//    led = color * .03f;
//  };
  // Green
  trailingEffect.shader = [&](auto& led, auto& params) {
    auto ratio = params.positionRatio;
    led = rgb::Color::GREEN() * brightness * ratio;
  };
  trailingEffect.init();
}

auto IntroScene::update() -> void {
  trailingEffect.update();
}

auto IntroScene::draw() -> void {
  brightness = .03f;
  trailingEffect.draw(ring);
  brightness = .5f;
  trailingEffect.draw(strip);
}

auto IntroScene::cleanup() -> void {

}