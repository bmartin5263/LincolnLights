//
// Created by Brandon on 3/17/25.
//

#include "Assertions.h"
#include "TrailingScene.h"
#include "Clock.h"
#include "LEDs.h"

auto TrailingScene::setup() -> void {
  nextMoveTime.value = 0;
  position = 0;
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

  auto colorGeneratorParameters = TrailingSceneColorGeneratorParameters {
    .now = now,
    .speed = speed,
    .length = length
  };
  if (continuous) {
    for (int i = 0; i < length; ++i) {
      auto led = (position + i + shift) % ledSize;
      colorGeneratorParameters.relativePosition = i;
      colorGeneratorParameters.absolutePosition = led;
      ring[led] = colorGenerator(colorGeneratorParameters);
    }
  }
  else {
    for (int i = 0; i < length; ++i) {
      auto led = (position + i + 1) - ((rgb::i32) length);
      if (led >= 0 && led < ledSize + 1) {
        led = (led + shift) % ledSize;
        colorGeneratorParameters.relativePosition = i;
        colorGeneratorParameters.absolutePosition = led;
        ring[led] = colorGenerator(colorGeneratorParameters);
      }
    }
  }
}

auto TrailingScene::move() -> void {
  auto ledSize = ring.getSize();
  position += 1;
  if (position >= ledSize + (continuous ? 0 : length + endBuffer)) {
    position = 0;
  }
}