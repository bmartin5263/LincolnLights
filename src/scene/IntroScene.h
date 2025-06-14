//
// Created by Brandon on 1/5/25.
//

#ifndef RGBLIB_INTROSCENE_H
#define RGBLIB_INTROSCENE_H

#include "Scene.h"
#include "LEDCircuit.h"
#include "FillEffect.h"
#include "TrailingEffect.h"

class IntroScene : public rgb::Scene {
public:
  explicit IntroScene(rgb::PixelList& ring);
  auto setup() -> void override;
  auto update() -> void override;
  auto draw() -> void override;
  auto cleanup() -> void override;

  rgb::PixelList& ring;
  rgb::TrailingEffect trailingEffect{};
  rgb::Duration speed{rgb::Duration::Seconds(1)};
};


#endif //RGBLIB_INTROSCENE_H
