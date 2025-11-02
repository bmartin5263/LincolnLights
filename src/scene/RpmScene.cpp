//
// Created by Brandon on 2/21/25.
//

#include "RpmScene.h"
#include "PushButton.h"
#include "Clock.h"
#include "PixelList.h"
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
  return Lerp(brightness, brightness * scale, Pulse((now - lastPulseReset).asSeconds(), 1.5f));
}

auto calculateInversePulseBrightness(
  normal brightness,
  normal scale,
  Timestamp now,
  Timestamp lastPulseReset
) -> normal {
  auto max = brightness;
  auto min = brightness * scale;
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
      if (brightness <= calculations.effectiveBrightBrightness * 1.16f) {
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
  calcs.effectiveDimBrightness = BrightnessControl::GetBrightness(
    rgb::ByteToFloat(1), rgb::ByteToFloat(2), rgb::ByteToFloat(4)
  );
  calcs.effectiveBrightBrightness = BrightnessControl::GetBrightness(
    rgb::ByteToFloat(6), rgb::ByteToFloat(20), rgb::ByteToFloat(40)
  );

  auto ledCount = ring.getSize();
  auto levelCount = shape == RpmShape::LINE ? ledCount : layout->calculateLevels(ledCount);
  calcs.rpmPerLevel = limit / levelCount;
  calcs.yellowLevel = calcs.effectiveYellowLineStart / calcs.rpmPerLevel;
  calcs.redLevel = calcs.effectiveRedLineStart / calcs.rpmPerLevel;

  auto currentRpm = vehicle.rpm();
  rpm = RPM_SMOOTHING_FACTOR * currentRpm + (1 - RPM_SMOOTHING_FACTOR) * rpm;
  calcs.rpmLevelAchieved = static_cast<uint>(rpm / calcs.rpmPerLevel);
  if (calcs.rpmLevelAchieved == 0 && rpm > 100) {
    ++calcs.rpmLevelAchieved;
  }
  calcs.glow = calcs.rpmLevelAchieved > calcs.yellowLevel && !connectedAt.isZero();

  auto offset = shape == RpmShape::LINE ? 0 : layout->calculateOffset(ledCount);
  for (int level = 0; level < levelCount; ++level) {
    calcs.level = level;
    auto color = colorMode->calculateColor(calcs, *this);

    // Handle the "not connected" effect, where if the OBD-II connection is not established
    // then we default all the pixels to white
    if (connectedAt == Timestamp{}) {
      color = INACTIVE_COLOR;
      calcs.rpmLevelAchieved = 100;
    }
    else {
      auto percent = calculateEffectPercent(connectedAt);
      if (percent < 1.0f) {
        calcs.rpmLevelAchieved = 100;
      }
      color = INACTIVE_COLOR.lerpClamp(color, percent);
    }

    auto brightness = calculateNextBrightness(calcs);
    auto percent = calculateEffectPercent(enteredAt);
    brightness = rgb::LerpClamp(0.0f, brightness, percent);

    color *= brightness;
    ring[mapToPixelPosition(level, ledCount, offset)] = color;
  }

  auto currentSpeed = static_cast<float>(vehicle.speed());
  rpm = (RPM_SMOOTHING_FACTOR * currentRpm + (1 - RPM_SMOOTHING_FACTOR) * rpm);
//  rpmSuperSmooth = RPM_SUPER_SMOOTHING_FACTOR * currentRpm + (1 - RPM_SUPER_SMOOTHING_FACTOR) * rpmSuperSmooth;

  if (rpm >= 900 && footLevel < leftStrip.getSize() && !inAnimation) {
    inAnimation = true;
    activationAnimationHandle = Timer::SetImmediateTimeout([&](auto& context){
      ++footLevel;
      if (footLevel < leftStrip.getSize()) {
        context.repeatIn = Duration::Milliseconds(15);
      }
      else {
        inAnimation = false;
      }
    });
  }
  else if (currentSpeed <= 0.0f && footLevel > 0 && !inAnimation) {
    inAnimation = true;
    activationAnimationHandle = Timer::SetImmediateTimeout([&](auto& context){
      --footLevel;
      if (footLevel > 0) {
        context.repeatIn = Duration::Milliseconds(20);
      }
      else {
        inAnimation = false;
      }
    });
  }

  leftFrontFoot.fill(Color(.8f, 0, 1.0f));
  rightFrontFoot.fill(Color(.8f, 0, 1.0f));

  leftBackFoot.fill(Color(.8f, 0, 1.0f));
  rightBackFoot.fill(Color(.8f, 0, 1.0f));

  if (footLevel > 0) {
    Color rpmColor;
    if (currentRpm >= 3000.0f || calcs.now < (rainbowedAt + Duration::Seconds(5))) {
      if (currentRpm >= 3000.0f) {
        rainbowedAt = Clock::Now();
      }
      const auto SPEED = Duration::Milliseconds(500);
      auto time = calcs.now.mod(SPEED).to<float>() / SPEED.to<float>();
      auto hue = LerpWrap(0.0f, 1.0f, time);
      rpmColor = Color::HslToRgb(hue);
    }
    else {
      auto start = 1500.0f;
      auto max = 2500.0f;
      auto time = (rpm - start) / (max - start);
      rpmColor = Color(0.0f, 1.0f, 1.0f).lerpClamp(Color(1.0f, 0, .4f), time);
    }
    leftStrip.fill(rpmColor, footLevel);
    rightStrip.fill(rpmColor, footLevel);
  }
}