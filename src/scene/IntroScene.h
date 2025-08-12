//
// Created by Brandon on 1/5/25.
//

#ifndef RGBLIB_INTROSCENE_H
#define RGBLIB_INTROSCENE_H

#include "Scene.h"
#include "LEDCircuit.h"
#include "FillEffect.h"
#include "TrailingEffect.h"

class IntroScene : public rgb::Scene {
public:
  auto setup() -> void override;
  auto update() -> void override;
  auto draw() -> void override;
  auto cleanup() -> void override;

  IntroScene() = default;
  IntroScene(const IntroScene& rhs) = default;
  IntroScene(IntroScene&& rhs) noexcept = default;
  IntroScene& operator=(const IntroScene& rhs) = default;
  IntroScene& operator=(IntroScene&& rhs) noexcept = default;
  ~IntroScene() override = default;

  rgb::TrailingEffect trailingEffect{};
  rgb::Duration speed{rgb::Duration::Seconds(1)};
  rgb::normal brightness{.03f};
};


#endif //RGBLIB_INTROSCENE_H
