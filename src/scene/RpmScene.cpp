//
// Created by Brandon on 2/21/25.
//

#include "RpmScene.h"
#include "PushButton.h"
#include "Clock.h"
#include "DebugScreen.h"
#include "PixelList.h"
#include "Timer.h"
#include "LEDs.h"
#include "BrightnessControl.h"

using namespace rgb;

RpmScene::RpmScene(Vehicle& vehicle): vehicle(vehicle) {

}

auto RpmScene::setup() -> void {
  INFO("RPM setup");
  lastPulseReset = Clock::Now();
  connectedAt = Timestamp{};
  if (enteredAt == Timestamp{}) {
    enteredAt = Clock::Now();
  }
}

auto RpmScene::update() -> void {
  if (connectedAt == Timestamp{} && vehicle.isConnected()) {
    connectedAt = Clock::Now();
  }
  else if (connectedAt != Timestamp{} && !vehicle.isConnected()) {
    connectedAt = Timestamp{};
  }
}

constexpr u16 mapToPixelPosition(int level, int ledCount, int offset = 0) {
  return (level + offset) % ledCount;
}

auto RpmScene::getCoolantTemp() -> fahrenheit {
  if (dynamicRedLine && vehicle.isConnected()) {
    return vehicle.coolantTemp();
  }
  else {
    return maxCoolantLevel;
  }
}

auto calculatePulseBrightness(
  normal brightness,
  normal scale,
  Timestamp now,
  Timestamp lastPulseReset
) -> normal {
  auto min = brightness;
  auto max = brightness * scale;
  return Lerp(min, max, Pulse((now - lastPulseReset).asSeconds(), 1.5f));
}

auto RpmScene::calculateNextBrightness(const RpmGaugeCalculations& calculations) -> normal {
  if (calculations.level >= calculations.rpmLevelAchieved) {
    return calculations.effectiveDimBrightness;
  }

  if (calculations.glow) {
    if (!lastFrameWasYellow) {
      lastPulseReset = calculations.now - Duration::Milliseconds(500);
    }
    auto brightness = calculatePulseBrightness(calculations.effectiveBrightBrightness, 3.0f, calculations.now, lastPulseReset);
    lastFrameWasYellow = true;
    return brightness;
  }
  else {
    if (lastFrameWasYellow) {
      auto brightness = calculatePulseBrightness(calculations.effectiveBrightBrightness, 3.0f, calculations.now, lastPulseReset);
      if (brightness <= calculations.effectiveBrightBrightness + 1) {
        lastFrameWasYellow = false;
      }
      return brightness;
    }
    else {
      return calculations.effectiveBrightBrightness;
    }
  }
}

auto calculateEffectPercent(Timestamp time) -> normal {
  auto begin = Clock::Now() - time;
  auto end = Duration::Milliseconds(150);
  return static_cast<normal>(begin.value) / static_cast<normal>(end.value);
}

auto RpmScene::draw() -> void {
  auto calcs = RpmGaugeCalculations();
  calcs.now = Clock::Now();
  calcs.coolantPercent = RemapPercent(minCoolantLevel, maxCoolantLevel, getCoolantTemp());
  calcs.effectiveYellowLineStart = static_cast<u16>(yellowLineStart * LerpClamp(.6f, 1.0f, calcs.coolantPercent));
  calcs.effectiveRedLineStart = static_cast<u16>(redLineStart * LerpClamp(.8f, 1.0f, calcs.coolantPercent));
  calcs.effectiveBrightBrightness = BrightnessControl::GetBrightness(
    rgb::ByteToFloat(1), rgb::ByteToFloat(4), rgb::ByteToFloat(8)
  );
  calcs.effectiveDimBrightness = BrightnessControl::GetBrightness(
    rgb::ByteToFloat(4), rgb::ByteToFloat(16), rgb::ByteToFloat(32)
  );

  auto ledCount = ring.getSize();
  auto levelCount = shape == RpmShape::LINE ? ledCount : layout->calculateLevels(ledCount);
  calcs.rpmPerLevel = limit / levelCount;
  calcs.yellowLevel = calcs.effectiveYellowLineStart / calcs.rpmPerLevel;
  calcs.redLevel = calcs.effectiveRedLineStart / calcs.rpmPerLevel;

  rpm = RPM_SMOOTHING_FACTOR * vehicle.rpm() + (1 - RPM_SMOOTHING_FACTOR) * rpm;
  calcs.rpmLevelAchieved = static_cast<uint>(rpm / calcs.rpmPerLevel);
  if (calcs.rpmLevelAchieved == 0 && rpm > 100) {
    ++calcs.rpmLevelAchieved;
  }

  auto offset = shape == RpmShape::LINE ? 0 : layout->calculateOffset(ledCount);
  for (int level = 0; level < levelCount; ++level) {
    calcs.level = level;
    auto color = colorMode->calculateColor(calcs, *this);

    if (connectedAt == Timestamp{}) {
      color = Color::WHITE(.8f);
      calcs.rpmLevelAchieved = 100;
    }
    else {
      auto percent = calculateEffectPercent(connectedAt);
      if (percent < 1.0f) {
        calcs.rpmLevelAchieved = 100;
      }
      color = Color::WHITE(.8f).lerpClamp(color, percent);
    }

    calcs.glow = calcs.rpmLevelAchieved > calcs.yellowLevel && !connectedAt.isZero();
    auto brightness = calculateNextBrightness(calcs);
    auto percent = calculateEffectPercent(enteredAt);
    brightness = rgb::LerpClamp(0.0f, brightness, percent);

    color *= brightness;
    ring[mapToPixelPosition(level, ledCount, offset)] = color;
  }

  auto stripBrightness = BrightnessControl::GetBrightness(.3f, .6f, .9f);
  stripBrightness = calculatePulseBrightness(stripBrightness, 1.5f, calcs.now, lastPulseReset);
  auto start = 1500.0f;
  auto max = 2500.0f;
  auto color = Color::GREEN().lerpWrap(Color::RED(), (rpm - start) / (max - start));
  leftStrip.fill(color * stripBrightness);
}