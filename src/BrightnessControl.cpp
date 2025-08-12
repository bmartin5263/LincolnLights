//
// Created by Brandon on 8/12/25.
//

#include "BrightnessControl.h"

auto BrightnessControl::setLevel(BrightnessLevel level) -> void {
  mLevel = level;
}

auto BrightnessControl::decreaseLevel(bool includeOff) -> void {
  switch (mLevel) {
    case BrightnessLevel::OFF:
      break;
    case BrightnessLevel::DIM:
      if (includeOff) {
        mLevel = BrightnessLevel::OFF;
      }
      break;
    case BrightnessLevel::MEDIUM:
      mLevel = BrightnessLevel::DIM;
      break;
    case BrightnessLevel::BRIGHT:
      mLevel = BrightnessLevel::MEDIUM;
      break;
  }
}

auto BrightnessControl::increaseLevel() -> void {
  switch (mLevel) {
    case BrightnessLevel::OFF:
      mLevel = BrightnessLevel::DIM;
      break;
    case BrightnessLevel::DIM:
      mLevel = BrightnessLevel::MEDIUM;
      break;
    case BrightnessLevel::MEDIUM:
      mLevel = BrightnessLevel::BRIGHT;
    case BrightnessLevel::BRIGHT:
      break;
  }
}

auto BrightnessControl::getBrightness(float onlyBrightness) const -> float {
  if (mLevel == BrightnessLevel::OFF) {
    return 0.0f;
  }
  return onlyBrightness;
}

auto BrightnessControl::getBrightness(float low, float medium) const -> float {
  switch (mLevel) {
    case BrightnessLevel::OFF:
      return 0.0f;
    case BrightnessLevel::DIM:
      return low;
    case BrightnessLevel::MEDIUM:
    case BrightnessLevel::BRIGHT:
      return medium;
  }
}

auto BrightnessControl::getBrightness(float low, float medium, float high) const -> float {
  switch (mLevel) {
    case BrightnessLevel::OFF:
      return 0.0f;
    case BrightnessLevel::DIM:
      return low;
    case BrightnessLevel::MEDIUM:
      return medium;
    case BrightnessLevel::BRIGHT:
      return high;
  }
}