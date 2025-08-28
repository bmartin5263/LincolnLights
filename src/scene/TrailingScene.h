//
// Created by Brandon on 3/17/25.
//

#ifndef RGBLIB_TRAILINGSCENE_H
#define RGBLIB_TRAILINGSCENE_H

#include <optional>
#include "Types.h"
#include "Scene.h"
#include "Vehicle.h"
#include "PixelList.h"

class TrailingScene : public rgb::Scene {
public:
  struct ShaderParameters {
    rgb::Timestamp now{};
    rgb::Duration speed{};
    rgb::u16 length{};
    rgb::u16 absolutePosition{};
    rgb::u16 relativePosition{};
  };
  using Shader = std::function<rgb::Color(const ShaderParameters&)>;
  constexpr static auto DefaultShader(const ShaderParameters&) -> rgb::Color {
    return rgb::Color::CYAN(.01);
  }

  auto setup() -> void override;
  auto update() -> void override;
  auto draw() -> void override;

  Shader shader{DefaultShader};
  rgb::Duration speed{rgb::Duration::Seconds(1)};
  int shift{0};
  rgb::u16 length{1};
  int endBuffer{0};
  bool continuous{false};
private:
  int pixel{0};
  rgb::Timestamp nextMoveTime{0};

  auto move() -> void;
};

#endif //RGBLIB_TRAILINGSCENE_H
