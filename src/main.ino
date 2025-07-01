#include <bitset>
#include "Timer.h"
#include "scene/RpmScene.h"
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
auto rpmScene = RpmScene{ring, vehicle};
auto introScene = IntroScene{ring};
auto comboGauge = TrailingScene { TrailingSceneParameters {
  .leds = &ring,
  .colorGenerator = [cancelEffectAt = rgb::Timestamp{}, rpm = 1.0f](TrailingSceneColorGeneratorParameters params) mutable {
    auto alpha = .001;
    rpm = alpha * vehicle.rpm() + (1 - alpha) * rpm;
    auto rpmStr = "RPM: " + std::to_string(static_cast<int>(rpm));
    DebugScreen::PrintLine(3, rpmStr);
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

auto scenes = std::array {
  static_cast<Scene*>(&rpmScene),
  static_cast<Scene*>(&comboGauge),
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
  rpmScene.yellowLineStart = 3000;
  rpmScene.redLineStart = 4000;
  rpmScene.limit = 4200;
  rpmScene.colorMode = RpmColorMode::SEGMENTED;
  rpmScene.glow = true;

  ring.setOffset(offset);

  irReceiver.button1.onPress([](){
    if (rpmScene.dimBrightness == 0) {
      rpmScene.dimBrightness = 1;
    }
    else {
      rpmScene.dimBrightness = 0;
    }
  });
  irReceiver.button2.onPress([](){
    if (rpmScene.colorMode == RpmColorMode::SINGLE) {
      rpmScene.colorMode = RpmColorMode::SEGMENTED;
    }
    else {
      rpmScene.colorMode = RpmColorMode::SINGLE;
    }
  });
  irReceiver.button3.onPress([](){
    rpmScene.bright = !rpmScene.bright;
  });
  irReceiver.button4.onPress([](){
    if (rpmScene.layout == RpmLayout::TRADITIONAL) {
      rpmScene.layout = RpmLayout::SPORT;
    }
    else {
      rpmScene.layout = RpmLayout::TRADITIONAL;
    }
  });
  irReceiver.button5.onPress([](){
    if (rpmScene.greenColor == Color::GREEN(1.0f)) {
      rpmScene.greenColor = Color::MAGENTA(1.0f);
      rpmScene.yellowColor = Color::GREEN(1.0f);
      rpmScene.redColor = Color::YELLOW(1.0f);
    }
    else {
      rpmScene.greenColor = Color::GREEN(1.0f);
      rpmScene.yellowColor = Color::YELLOW(1.0f);
      rpmScene.redColor = Color::RED(1.0f);
    }
  });
  irReceiver.button6.onPress([](){
    if (rpmScene.yellowLineStart == 3000) {
      rpmScene.yellowLineStart = 1800;
      rpmScene.redLineStart = 3000;
      rpmScene.limit = 5000;
    }
    else if (rpmScene.yellowLineStart == 1800) {
      rpmScene.yellowLineStart = 3001;
      rpmScene.redLineStart = 5000;
      rpmScene.limit = 7000;
    }
    else {
      rpmScene.yellowLineStart = 3000;
      rpmScene.redLineStart = 4000;
      rpmScene.limit = 4200;
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