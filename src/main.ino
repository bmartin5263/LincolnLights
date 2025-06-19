#include <bitset>
#include "Timer.h"
#include "scene/RpmDisplay.h"
#include "scene/TrailingScene.h"
#include "DebugScreen.h"
#include "scene/IntroScene.h"
#include "IRReceiver.h"
#include "App.h"
#include "AppBuilder.h"
#include "VehicleThread.h"
#include "LEDStrip.h"
#include <ArduinoOTA.h> // idk why but without this compilation fails

using namespace rgb;

static constexpr u16 LED_COUNT = 12;

// Output
auto ring = LEDStrip<LED_COUNT>{D5};
auto slice = ring.slice(9, 3);
auto leds = std::array {
  static_cast<LEDCircuit*>(&ring)
};

u16 len = 6;

// Scenes
auto vehicle = Vehicle{};
auto rpmDisplay = RpmDisplay{ring, vehicle};
auto introScene = IntroScene{ring};
auto comboGauge = TrailingScene { TrailingSceneParameters {
  .leds = &ring,
  .colorGenerator = [cancelEffectAt = rgb::Timestamp{}](TrailingSceneColorGeneratorParameters params) mutable {
    auto rpm = vehicle.rpm();
    if (rpm >= 3000.f || Clock::Now() < cancelEffectAt) {
      len = 12;
      if (cancelEffectAt == Timestamp{}) {
        cancelEffectAt = Clock::Now() + Duration::Seconds(3);
      }
      auto speed = Duration::Milliseconds(2000);
      auto time = rgb::Clock::Now().mod(speed).to<float>() / speed.to<float>();
      auto myTime = time + (static_cast<float>(params.relativePosition) / params.length * .3f);
      auto hue = rgb::LerpWrap(0.0f, 1.0f, myTime);
      auto color  = rgb::Color::HslToRgb(hue);
      return color * .04f;
    }
    else {
      len = 6;
      cancelEffectAt = rgb::Timestamp{};
      auto r = EaseInOutCubic(LerpClamp(1.0f, 0.0f, rpm - 1000, 1500.0f));
      auto g = 0.0f;
      auto b = EaseInOutCubic(LerpClamp(0.0f, 1.0f, rpm - 1000, 1500.0f));
      auto positionalBrightnessAdjust = LerpClamp(.5f, 1.0f, static_cast<float>(params.relativePosition) / params.length);
//      auto positionalBrightnessAdjust = 1.0f;
      auto brightness = LerpClamp(.05f, .18f, rpm - 1500, 1000.0f) * positionalBrightnessAdjust;

      return Color { r, g, b } * brightness;
    }
  },
  .speed = Duration::Milliseconds(500),
  .shift = 6,
  .length = len,
  .endBuffer = 4,
  .continuous = true
}};
auto redGreenGauge = TrailingScene { TrailingSceneParameters {
  .leds = &ring,
  .colorGenerator = [cancelEffectAt = rgb::Timestamp{}](TrailingSceneColorGeneratorParameters params) mutable {
    auto rpm = vehicle.rpm();
    auto r = LerpClamp(0.0f, 1.0f, rpm - 1000, 1500.0f);
    auto g = LerpClamp(1.0f, 0.0f, rpm - 1000, 1500.0f);
    auto b = 0.0f;
    auto brightness = LerpClamp(.05f, .2f, rpm - 1500, 1000.0f);
    return Color { r, g, b } * brightness;
  },
  .speed = Duration::Milliseconds(500),
  .shift = 6,
  .length = 6,
  .endBuffer = 4,
  .continuous = true
}};
auto rainbowGauge = TrailingScene { TrailingSceneParameters {
  .leds = &ring,
  .colorGenerator = [cancelEffectAt = rgb::Timestamp{}](TrailingSceneColorGeneratorParameters params) mutable {
    auto speed = Duration::Milliseconds(2000);
    auto time = rgb::Clock::Now().mod(speed).to<float>() / speed.to<float>();
    auto myTime = time + (static_cast<float>(params.relativePosition) / params.length * .3f);
    auto hue = rgb::LerpWrap(0.0f, 1.0f, myTime);
    auto color  = rgb::Color::HslToRgb(hue);
    return color * .085f;
  },
  .speed = Duration::Milliseconds(500),
  .shift = 6,
  .length = 6,
  .endBuffer = 4,
  .continuous = true
}};
auto whiteGreenPulse = TrailingScene { TrailingSceneParameters {
  .leds = &ring,
  .colorGenerator = [](TrailingSceneColorGeneratorParameters params){
    auto rpm = vehicle.rpm();
    auto brightness = LerpClamp(.02f, .03f, rpm, 4000.0f);
    auto x = Pulse(params.now.asSeconds(), 1.f);
    brightness += LerpClamp(0.f, .05f, x);
    return Color {1.0f - x, 1.0f, 1.0f - x} * brightness;
  },
  .speed = Duration::Milliseconds(500),
  .shift = 6,
  .length = 6,
  .endBuffer = 4,
  .continuous = true
}};

auto scenes = std::array {
  static_cast<Scene*>(&rpmDisplay),
  static_cast<Scene*>(&comboGauge),
  static_cast<Scene*>(&redGreenGauge),
  static_cast<Scene*>(&rainbowGauge),
  static_cast<Scene*>(&whiteGreenPulse),
  static_cast<Scene*>(&introScene)
};

// Input
auto irReceiver = IRReceiver{};
auto sensors = std::array {
  Runnable { []() {
    irReceiver.update();
  }}
};

int offset = 1;

auto setup() -> void {
  DebugScreen::Start();
  rpmDisplay.yellowLineStart = 3000;
  rpmDisplay.redLineStart = 4000;
  rpmDisplay.limit = 4200;
  rpmDisplay.colorMode = RpmColorMode::SEGMENTED;
  rpmDisplay.glow = true;

  ring.setOffset(offset);

  irReceiver.button1.onPress([](){
    if (rpmDisplay.dimBrightness == 0) {
      rpmDisplay.dimBrightness = 1;
    }
    else {
      rpmDisplay.dimBrightness = 0;
    }
  });
  irReceiver.button2.onPress([](){
    if (rpmDisplay.colorMode == RpmColorMode::SINGLE) {
      rpmDisplay.colorMode = RpmColorMode::SEGMENTED;
    }
    else {
      rpmDisplay.colorMode = RpmColorMode::SINGLE;
    }
  });
  irReceiver.button3.onPress([](){
    rpmDisplay.bright = !rpmDisplay.bright;
  });
  irReceiver.button4.onPress([](){
    if (rpmDisplay.layout == RpmLayout::TRADITIONAL) {
      rpmDisplay.layout = RpmLayout::SPORT;
    }
    else {
      rpmDisplay.layout = RpmLayout::TRADITIONAL;
    }
  });
  irReceiver.button5.onPress([](){
    if (rpmDisplay.greenColor == Color::GREEN(1.0f)) {
      rpmDisplay.greenColor = Color::MAGENTA(1.0f);
      rpmDisplay.yellowColor = Color::GREEN(1.0f);
      rpmDisplay.redColor = Color::YELLOW(1.0f);
    }
    else {
      rpmDisplay.greenColor = Color::GREEN(1.0f);
      rpmDisplay.yellowColor = Color::YELLOW(1.0f);
      rpmDisplay.redColor = Color::RED(1.0f);
    }
  });
  irReceiver.button6.onPress([](){
    if (rpmDisplay.yellowLineStart == 3000) {
      rpmDisplay.yellowLineStart = 1800;
      rpmDisplay.redLineStart = 3000;
      rpmDisplay.limit = 5000;
    }
    else if (rpmDisplay.yellowLineStart == 1800) {
      rpmDisplay.yellowLineStart = 3001;
      rpmDisplay.redLineStart = 5000;
      rpmDisplay.limit = 7000;
    }
    else {
      rpmDisplay.yellowLineStart = 3000;
      rpmDisplay.redLineStart = 4000;
      rpmDisplay.limit = 4200;
    }
  });
  irReceiver.buttonHash.onPress([](){
    offset = (offset + 1) % LED_COUNT;
    ring.setOffset(offset);
  });
  irReceiver.buttonStar.onPress([](){
    Debug::Recover();
  });
  irReceiver.buttonRight.onPress([](){ App::NextScene(); });
  irReceiver.buttonLeft.onPress([](){ App::PrevScene(); });
  irReceiver.buttonUp.onPress([](){ introScene.speed += Duration::Seconds(1); });
  irReceiver.buttonDown.onPress([](){ introScene.speed -= Duration::Seconds(1); });
  irReceiver.start(D3);

  log::init();
  AppBuilder::Create()
    .DebugOutputLED(&slice)
    .EnableIntroScene(introScene, Duration::Seconds(1))
    .SetScenes(scenes)
    .SetLEDs(leds)
    .SetSensors(sensors)
    .Start();

  VehicleThread::Instance().autoUpdate = true;
  VehicleThread::Start(vehicle);
}

auto updateDisplay() -> void {
  auto fpsStr = "FPS: " + std::to_string(Clock::Fps())
                + "  MPH: " + std::to_string(static_cast<int>(vehicle.speed()));
  auto rpmStr = "RPM: " + std::to_string(static_cast<int>(vehicle.rpm()))
                + "  Coolant: " + std::to_string(static_cast<int>(vehicle.coolantTemp())) + "°F";
  auto fuelStr = "Fuel: " + std::to_string(static_cast<int>(vehicle.fuelLevel())) + "%"
                 + "  Throttle: " + std::to_string(static_cast<int>(vehicle.throttlePosition() * 100)) + "%";
  DebugScreen::PrintLine(0, fpsStr);
  DebugScreen::PrintLine(1, rpmStr);
  DebugScreen::PrintLine(2, fuelStr);
}

auto loop() -> void {
  auto t = EaseOutCubic(vehicle.speed() / 160.0f);
  auto m = LerpClamp(100, 4, t);
  comboGauge.params.speed = Duration::Milliseconds(m);
  comboGauge.params.length = len;
//  whiteGreenPulse.params.speed = Duration::Milliseconds(m);
//  rainbowGauge.params.speed = Duration::Milliseconds(m);
//  redGreenGauge.params.speed = Duration::Milliseconds(m);
  if (DebugScreen::ReadyForUpdate()) {
    updateDisplay();
    DebugScreen::Display();
  }
  App::Loop();
}