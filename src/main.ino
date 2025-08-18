#include <bitset>
#include "BrightnessControl.h"
#include "Timer.h"
#include "scene/RpmScene.h"
#include "scene/TrailingScene.h"
#include "DebugScreen.h"
#include "scene/IntroScene.h"
#include "IRReceiver.h"
#include "App.h"
#include "AppBuilder.h"
#include "util/VehicleThread.h"
#include "LEDStrip.h"
#include "scene/SolidScene.h"
#include <ArduinoOTA.h> // idk why but without this compilation fails
#include "LEDs.h"

using namespace rgb;

LEDStrip<CLOCK_LED_COUNT> ring = LEDStrip<CLOCK_LED_COUNT>{D2_RGB};

LEDStrip<FOOT_STRIP_LED_COUNT> leftStrip = LEDStrip<FOOT_STRIP_LED_COUNT>{D4_RGB, NEO_GRB + NEO_KHZ800};
PixelSlice leftFrontFoot = leftStrip.slice(HALF_FOOT_STRIP_LED_COUNT);
PixelSlice leftBackFoot = leftStrip.slice(HALF_FOOT_STRIP_LED_COUNT, HALF_FOOT_STRIP_LED_COUNT);

LEDStrip<FOOT_STRIP_LED_COUNT> rightStrip = rgb::LEDStrip<FOOT_STRIP_LED_COUNT>{D5_RGB, NEO_GRB + NEO_KHZ800};
PixelSlice rightFrontFoot = leftStrip.slice(HALF_FOOT_STRIP_LED_COUNT);
PixelSlice rightBackFoot = leftStrip.slice(HALF_FOOT_STRIP_LED_COUNT, HALF_FOOT_STRIP_LED_COUNT);

PixelSlice slice = CLOCK_LED_SHAPE == RpmShape::LINE ? ring.slice(3) : ring.slice(9, 3);

std::array<rgb::LEDCircuit*, 3> leds = std::array {
  static_cast<LEDCircuit*>(&ring),
  static_cast<LEDCircuit*>(&leftStrip),
  static_cast<LEDCircuit*>(&rightStrip),
};

u16 len = 6;

// Scenes
auto vehicle = Vehicle{};
auto rpmScene = RpmScene{vehicle};
auto introScene = IntroScene{};
auto solidScene = SolidScene{ring};
auto comboGauge = TrailingScene { TrailingSceneParameters {
  .leds = &ring,
  .colorGenerator = [
    cancelEffectAt = Timestamp{},
    averageRpm = vehicle.rpm(),
    rainbowSpeed = 0.0f,
    averageSpeed = vehicle.speed()
  ](TrailingSceneColorGeneratorParameters params) mutable {
    auto alpha = .001;
    auto currentRpm = vehicle.rpm();
    auto currentSpeed = vehicle.speed();
    averageRpm = alpha * currentRpm + (1 - alpha) * averageRpm;
    averageSpeed = alpha * currentSpeed + (1 - alpha) * averageSpeed;
    auto now = Clock::Now();

    const auto RAINBOW_RPM_THRESHOLD = 3000.0f;
    bool activateRainbowMode = currentRpm >= RAINBOW_RPM_THRESHOLD;
    bool maintainRainbowMode = now < cancelEffectAt
      || (rainbowSpeed != 0.0f && abs(currentSpeed - rainbowSpeed) < 15.0f);
    if (activateRainbowMode || maintainRainbowMode) {
      len = 12;
      if (activateRainbowMode) {
        // Should be active for at least 3 seconds after reaching RPM threshold
        cancelEffectAt = now + Duration::Seconds(3);
        if (rainbowSpeed != 0.0f) {
          rainbowSpeed = vehicle.speed();
        }
      }

      const auto SPEED = Duration::Seconds(2);
      auto time = now.mod(SPEED).to<float>() / SPEED.to<float>();
      auto myTime = time + (static_cast<float>(params.relativePosition) / params.length * .3f);
      auto hue = LerpWrap(0.0f, 1.0f, myTime);
      auto color  = Color::HslToRgb(hue);
      return color * Brightness::Current();
    }
    else {
      len = 6;
      cancelEffectAt = rgb::Timestamp{};
      auto r = EaseInOutCubic(LerpClamp(1.0f, 0.0f, averageRpm - 1000, 1500.0f));
      auto g = 0.0f;
      auto b = EaseInOutCubic(LerpClamp(0.0f, 1.0f, averageRpm - 1000, 1500.0f));
      auto positionalBrightnessAdjust = LerpClamp(.5f, 1.0f, static_cast<float>(params.relativePosition) / params.length);
      auto brightness = LerpClamp(.05f, .18f, averageRpm - 1500, 1000.0f) * positionalBrightnessAdjust;

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
//  DebugScreen::Start();
  rpmScene.yellowLineStart = 3000 * .8f;
  rpmScene.redLineStart = 4000 * .8f;
  rpmScene.limit = 4200 * .8f;
  rpmScene.colorMode = RpmColorMode::PARTITIONED();
  rpmScene.glow = true;

  if (CLOCK_LED_SHAPE == RpmShape::CIRCLE) {
    ring.setOffset(offset);
  }

//  irReceiver.button1.onPress([](){
//    if (rpmScene.dimBrightness == 0) {
//      rpmScene.dimBrightness = ByteToFloat(1);
//    }
//    else {
//      rpmScene.dimBrightness = 0;
//    }
//  });
  irReceiver.button2.onPress([](){
    if (rpmScene.colorMode == RpmColorMode::SINGLE()) {
      rpmScene.colorMode = RpmColorMode::PARTITIONED();
    }
    else {
      rpmScene.colorMode = RpmColorMode::SINGLE();
    }
  });
  irReceiver.button3.onPress([](){
    if (rpmScene.layout == RpmLayout::TRADITIONAL()) {
      rpmScene.layout = RpmLayout::SPORT();
    }
    else {
      rpmScene.layout = RpmLayout::TRADITIONAL();
    }
  });
//  irReceiver.button5.onPress([](){
//    if (rpmScene.greenColor == Color::GREEN(1.0f)) {
//      rpmScene.greenColor = Color::MAGENTA(1.0f);
//      rpmScene.yellowColor = Color::GREEN(1.0f);
//      rpmScene.redColor = Color::YELLOW(1.0f);
//    }
//    else {
//      rpmScene.greenColor = Color::GREEN(1.0f);
//      rpmScene.yellowColor = Color::YELLOW(1.0f);
//      rpmScene.redColor = Color::RED(1.0f);
//    }
//  });
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
  irReceiver.buttonStar.onPress([](){
    Debug::Recover();
  });
  irReceiver.buttonRight.onPress([](){ App::NextScene(); });
  irReceiver.buttonLeft.onPress([](){ App::PrevScene(); });

  irReceiver.buttonUp.onPress([](){
    Brightness::SetToMax();
    BrightnessControl::IncreaseLevel();
  });
  irReceiver.buttonOk.onPress([](){
    Brightness::SetToDefault();
    BrightnessControl::SetLevel(BrightnessLevel::DIM);
  });
  irReceiver.buttonDown.onPress([](){
    Brightness::SetToMin();
    BrightnessControl::DecreaseLevel();
  });


  irReceiver.button1.onPress([&](){
    solidScene.color.r = min(1.0f, solidScene.color.r + .1f);
  });
  irReceiver.button4.onPress([&](){
    solidScene.color.r = Brightness::Current();
  });
  irReceiver.button7.onPress([&](){
    solidScene.color.r = max(0.0f, solidScene.color.r - .1f);
  });
  irReceiver.button2.onPress([&](){
    solidScene.color.g = min(1.0f, solidScene.color.g + .1f);
  });
  irReceiver.button5.onPress([&](){
    solidScene.color.g = Brightness::Current();
  });
  irReceiver.button8.onPress([&](){
    solidScene.color.g = max(0.0f, solidScene.color.g - .1f);
  });
  irReceiver.button3.onPress([&](){
    solidScene.color.b = min(1.0f, solidScene.color.b + .1f);
  });
  irReceiver.button6.onPress([&](){
    solidScene.color.b = Brightness::Current();
  });
  irReceiver.button9.onPress([&](){
    solidScene.color.b = max(0.0f, solidScene.color.b - .1f);
  });

  irReceiver.start(D3);

  Brightness::Configure()
    .DefaultBrightness(.05f)
    .MaxBrightness(.15f)
    .MinBrightness(ByteToFloat(1));
  AppBuilder::Create()
    .DebugOutputLED(&slice)
    .EnableIntroScene(introScene, Duration::Milliseconds(1200))
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