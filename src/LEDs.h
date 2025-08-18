//
// Created by Brandon on 8/12/25.
//

#ifndef LINCOLNLIGHTS_PIXELS_H
#define LINCOLNLIGHTS_PIXELS_H

#include "effect/RpmShape.h"
#include "LEDStrip.h"
#include "WebServer.h"

static constexpr rgb::u16 CLOCK_LED_COUNT = LL_LED_COUNT;
static constexpr RpmShape CLOCK_LED_SHAPE = LL_LED_SHAPE;
static constexpr rgb::u16 FOOT_STRIP_LED_COUNT = 40;
static constexpr rgb::u16 HALF_FOOT_STRIP_LED_COUNT = FOOT_STRIP_LED_COUNT / 2;

extern rgb::LEDStrip<CLOCK_LED_COUNT> ring;

extern rgb::LEDStrip<FOOT_STRIP_LED_COUNT> leftStrip;
extern rgb::PixelSlice leftFrontFoot;
extern rgb::PixelSlice leftBackFoot;

extern rgb::LEDStrip<FOOT_STRIP_LED_COUNT> rightStrip;
extern rgb::PixelSlice rightFrontFoot;
extern rgb::PixelSlice rightBackFoot;

extern rgb::PixelSlice slice;

extern std::array<rgb::LEDCircuit*, 3> leds;

extern rgb::WebServerHandle webServerHandle;

#endif //LINCOLNLIGHTS_PIXELS_H
