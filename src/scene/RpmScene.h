//
// Created by Brandon on 2/21/25.
//

#ifndef RGBLIB_RPMDISPLAY_H
#define RGBLIB_RPMDISPLAY_H

#include "Scene.h"
#include "LEDCircuit.h"
#include "WebServerFwd.h"
#include "Vehicle.h"
#include "effect/RpmLayout.h"
#include "effect/RpmColorMode.h"
#include "effect/RpmShape.h"
#include "effect/RpmGaugeCalculations.h"

namespace rgb {
  class PushButton;
}

class RpmScene : public rgb::Scene {
public:
  explicit RpmScene(rgb::PixelList& pixels, rgb::Vehicle& vehicle);

  auto setup() -> void override;
  auto update() -> void override;
  auto draw() -> void override;

private:
  static constexpr auto RPM_SMOOTHING_FACTOR = 0.03f;

  rgb::PixelList& pixels;
  rgb::Vehicle& vehicle;
  rgb::Timestamp lastPulseReset{0};
public:

  rgb::Timestamp connectedAt{};
  rgb::Timestamp enteredAt{};
  rgb::Color greenColor{rgb::Color::GREEN(1.0f)};
  rgb::Color yellowColor{rgb::Color::YELLOW(1.0f)};
  rgb::Color redColor{rgb::Color::RED(1.0f)};
  rgb::fahrenheit minCoolantLevel{70.f};
  rgb::fahrenheit maxCoolantLevel{150.f};
  const RpmLayout* layout{RpmLayout::TRADITIONAL()};
  const RpmColorMode* colorMode{RpmColorMode::PARTITIONED()};
  RpmShape shape{RpmShape::CIRCLE};
  uint yellowLineStart{5500};
  uint redLineStart{6500};
  uint limit{7000};
  rgb::normal dimBrightness{1 / 255.0f};
  rgb::normal brightBrightness{4 / 255.0f};
  rgb::revs_per_minute rpm{0.0f};
  bool glow{true};
  bool bright{false};
  bool dynamicRedLine{true};

private:
  bool lastFrameWasYellow{false};

  auto getCoolantTemp() -> rgb::fahrenheit;
  auto calculateNextBrightness(const RpmGaugeCalculations&) -> rgb::normal;
};

#endif //RGBLIB_RPMDISPLAY_H
