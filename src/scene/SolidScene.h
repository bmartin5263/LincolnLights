//
// Created by Brandon on 3/17/25.
//

#ifndef RGBLIB_SOLIDSCENE_H
#define RGBLIB_SOLIDSCENE_H

#include "Scene.h"
#include "Vehicle.h"
#include "PixelList.h"


class SolidScene : public rgb::Scene {
public:

  explicit SolidScene(rgb::PixelList& leds);

  auto update() -> void override;
  auto draw() -> void override;

private:
  rgb::PixelList& leds;
  rgb::Color color{rgb::Color::BLUE(.01)};

};

#endif //RGBLIB_SOLIDSCENE_H
