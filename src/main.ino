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
#include <ArduinoOTA.h> // idk why but without this compilation fails

using namespace rgb;

static constexpr u16 LED_COUNT = 12;

// Output
auto ring = LEDCircuit<LED_COUNT>{D5};
auto slice = ring.slice(9, 3);
auto leds = std::array {
  static_cast<Drawable*>(&ring)
};

// Scenes
auto vehicle = Vehicle{};
auto rpmDisplay = RpmDisplay{ring, vehicle};
auto introScene = IntroScene{ring};
auto trailingScene = TrailingScene { TrailingSceneParameters {
  .leds = &ring,
  .colorGenerator = [](TrailingSceneColorGeneratorParameters params){
    auto r = LerpClamp(1.0f, .75f, vehicle.rpm() - 600, 3500.0f);
    auto g = 0.0f;
    auto b = LerpClamp(0.0f, .25f, vehicle.rpm() - 600, 3500.0f);
    return Color { r, g, b } * .05f;
  },
  .speed = Duration::Milliseconds(500),
  .shift = 6,
  .length = 6,
  .endBuffer = 4,
  .continuous = true
}};
auto trailingScene2 = TrailingScene { TrailingSceneParameters {
  .leds = &ring,
  .colorGenerator = [](TrailingSceneColorGeneratorParameters params){
    auto rpm = vehicle.rpm();
    auto r = LerpClamp(0.0f, 1.0f, rpm, 4000.0f);
    auto g = LerpClamp(1.0f, 0.0f, rpm, 4000.0f);
    auto b = 0.0f;
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
//  static_cast<Scene*>(&trailingScene),
//  static_cast<Scene*>(&trailingScene2)
};

// Input
auto irReceiver = IRReceiver{D6};
auto sensors = std::array {
//  Runnable { []() {
//    static auto lastVehicleUpdate = Timestamp{};
//    if (Clock::Now().timeSince(lastVehicleUpdate) > config::VEHICLE_REFRESH_RATE) {
//      vehicle.update();
//      lastVehicleUpdate = Clock::Now();
//    }
//  }},
  Runnable { []() {
    irReceiver.update();
  }}
};

auto setup() -> void {
  DebugScreen::Start();
  rpmDisplay.yellowLineStart = 3000;
  rpmDisplay.redLineStart = 4000;
  rpmDisplay.limit = 4200;
  rpmDisplay.colorMode = RpmColorMode::SEGMENTED;
  rpmDisplay.glow = true;

  ring.setOffset(11);

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
    rpmDisplay.dynamicRedLine = !rpmDisplay.dynamicRedLine;
  });
  irReceiver.buttonStar.onPress([](){
    Debug::Recover();
  });
  irReceiver.buttonRight.onPress([](){ App::NextScene(); });
  irReceiver.buttonLeft.onPress([](){ App::PrevScene(); });
  irReceiver.start();

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

  auto r = "Reversed: " + std::to_string(ring.isReversed());
  DebugScreen::PrintLine(3, r);
}

auto loop() -> void {
  auto t = EaseOutCubic(vehicle.speed() / 140.0f);
  trailingScene.params.speed = Duration::Milliseconds(LerpClamp(100, 4, t));
  trailingScene2.params.speed = Duration::Milliseconds(LerpClamp(100, 4, t));
  if (DebugScreen::ReadyForUpdate()) {
    updateDisplay();
    DebugScreen::Display();
  }
  App::Loop();
}