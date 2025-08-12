//
// Created by Brandon on 8/12/25.
//

#ifndef LINCOLNLIGHTS_BRIGHTNESSLEVEL_H
#define LINCOLNLIGHTS_BRIGHTNESSLEVEL_H

enum class BrightnessLevel {
  OFF,
  DIM,
  MEDIUM,
  BRIGHT,
};

class BrightnessControl {
public:
  static auto GetBrightness(float onlyBrightness) -> float { return Instance().getBrightness(onlyBrightness); };
  static auto GetBrightness(float low, float medium) -> float { return Instance().getBrightness(low, medium); };
  static auto GetBrightness(float low, float medium, float high) -> float { return Instance().getBrightness(low, medium, high); };
  static auto GetLevel() -> BrightnessLevel { return Instance().getLevel(); };

  static auto SetLevel(BrightnessLevel level) -> void { Instance().setLevel(level); };
  static auto IncreaseLevel() -> void { Instance().increaseLevel(); };
  static auto DecreaseLevel(bool includeOff = false) -> void { Instance().decreaseLevel(includeOff); };

private:
  BrightnessLevel mLevel{BrightnessLevel::DIM};

  static auto Instance() -> BrightnessControl& {
    static BrightnessControl instance;
    return instance;
  }

  auto getBrightness(float onlyBrightness) const -> float;
  auto getBrightness(float low, float medium) const -> float;
  auto getBrightness(float low, float medium, float high) const -> float;
  auto setLevel(BrightnessLevel level) -> void;
  auto getLevel() const -> BrightnessLevel;
  auto increaseLevel() -> void;
  auto decreaseLevel(bool includeOff) -> void;

};

#endif //LINCOLNLIGHTS_BRIGHTNESSLEVEL_H
