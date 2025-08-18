//
// Created by Brandon on 3/17/25.
//

#ifndef RGBLIB_SOLIDSCENE_H
#define RGBLIB_SOLIDSCENE_H

#include "Scene.h"
#include "Vehicle.h"
#include "PixelList.h"
#include "WebServerFwd.h"


class SolidScene : public rgb::Scene {
public:

  explicit SolidScene(rgb::PixelList& leds);

  auto setup() -> void override;
  auto cleanup() -> void override;
  auto update() -> void override;
  auto draw() -> void override;

  rgb::Color color{rgb::Color::BLUE(.01)};
private:
  rgb::PixelList& leds;
  rgb::WebServerHandle webServerHandle;
};

#endif //RGBLIB_SOLIDSCENE_H
