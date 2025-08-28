//
// Created by Brandon on 3/17/25.
//

#include "Assertions.h"
#include "TrailingScene.h"
#include "Clock.h"
#include "LEDs.h"

auto TrailingScene::setup() -> void {
  nextMoveTime.value = 0;
  pixel = 0;
}

auto TrailingScene::update() -> void {
  auto now = rgb::Clock::Now();
  if (now >= nextMoveTime) {
    move();
    nextMoveTime = now + speed;
  }
}

auto TrailingScene::draw() -> void {
  auto ledSize = ring.getSize();
  auto now = rgb::Clock::Now();

  auto shaderParameters = ShaderParameters {
    .now = now,
    .speed = speed,
    .length = length
  };
  if (continuous) {
    for (int i = 0; i < length; ++i) {
      auto led = (pixel + i + shift) % ledSize;
      shaderParameters.relativePosition = i;
      shaderParameters.absolutePosition = led;
      ring[led] = shader(shaderParameters);
    }
  }
  else {
    for (int i = 0; i < length; ++i) {
      auto led = (pixel + i + 1) - ((rgb::i32) length);
      if (led >= 0 && led < ledSize + 1) {
        led = (led + shift) % ledSize;
        shaderParameters.relativePosition = i;
        shaderParameters.absolutePosition = led;
        ring[led] = shader(shaderParameters);
      }
    }
  }
}

auto TrailingScene::move() -> void {
  auto ledSize = ring.getSize();
  pixel += 1;
  if (pixel >= ledSize + (continuous ? 0 : length + endBuffer)) {
    pixel = 0;
  }
}