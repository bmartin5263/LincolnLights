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

struct TrailingSceneShaderParameters {
  rgb::Timestamp now;
  rgb::Duration speed;
  rgb::u16 length;
  rgb::u16 absolutePosition;
  rgb::u16 relativePosition;

  [[nodiscard]]
  auto positionRatio() const -> float {
    return static_cast<float>(relativePosition) / static_cast<float>(length);
  }
};

using TrailingSceneShader = std::function<rgb::Color(const TrailingSceneShaderParameters&)>;
constexpr auto DefaultShader(const TrailingSceneShaderParameters&) -> rgb::Color {
  return rgb::Color::CYAN(.01);
}

class TrailingScene : public rgb::Scene {
public:
  auto setup() -> void override;
  auto update() -> void override;
  auto draw() -> void override;

  TrailingSceneShader shader{DefaultShader};
  rgb::Duration speed{rgb::Duration::Seconds(1)};
  int shift{0};
  rgb::u16 length{1};
  int endBuffer{0};
  bool continuous{false};
private:
  int position{0};
  rgb::Timestamp nextMoveTime{0};

  auto move() -> void;
};

#endif //RGBLIB_TRAILINGSCENE_H
