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
#include "Timer.h"

class RpmScene : public rgb::Scene {
  using Color = rgb::Color;

public:
  explicit RpmScene(rgb::Vehicle& vehicle);

  auto setup() -> void override;
  auto update() -> void override;
  auto draw() -> void override;

private:
  static constexpr auto RPM_SMOOTHING_FACTOR = 0.03f;
  static constexpr auto INACTIVE_COLOR = Color::WHITE(.8);

  rgb::Vehicle& vehicle;
  rgb::Timestamp lastPulseReset{};
  rgb::Timestamp connectedAt{};
  rgb::Timestamp enteredAt{};
  rgb::Timestamp rainbowedAt{};
  rgb::TimerHandle activationAnimationHandle{};
  uint footLevel{0};
  bool inAnimation{false};
public:
  Color greenColor{Color::GREEN()};
  Color yellowColor{Color::YELLOW()};
  Color redColor{Color::RED()};
  rgb::fahrenheit minCoolantLevel{70.f};
  rgb::fahrenheit maxCoolantLevel{150.f};
  const RpmLayout* layout{RpmLayout::TRADITIONAL()};
  const RpmColorMode* colorMode{RpmColorMode::PARTITIONED()};
  RpmShape shape{RpmShape::CIRCLE};
  rgb::revs_per_minute rpm{0.0f};
  rgb::u16 yellowLineStart{5500};
  rgb::u16 redLineStart{6500};
  rgb::u16 limit{7000};
  bool glow{true};
  bool dynamicRedLine{true};

private:
  bool lastFrameWasYellow{false};

  auto getCoolantTemp() -> rgb::fahrenheit;
  auto calculateNextBrightness(const RpmGaugeCalculations&) -> rgb::normal;
};

#endif //RGBLIB_RPMDISPLAY_H
