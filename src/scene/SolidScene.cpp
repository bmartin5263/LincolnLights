//
// Created by Brandon on 3/17/25.
//

#include "SolidScene.h"
#include "WebServer.h"

using namespace rgb;

#ifdef RGB_DEBUG
constexpr auto table = ParameterTable<SolidScene, 3> {
  std::pair {"r", [](SolidScene& scene, const String& value){
    scene.color.r = value.toFloat();
  }},
  std::pair {"g", [](SolidScene& scene, const String& value){
    scene.color.g = value.toFloat();
  }},
  std::pair {"b", [](SolidScene& scene, const String& value){
    scene.color.b = value.toFloat();
  }}
};
#endif

SolidScene::SolidScene(PixelList& leds): leds(leds) {

}

auto SolidScene::setup() -> void {
#ifdef RGB_DEBUG
  webServerHandle = WebServer::ParameterServer("/rgb", *this, table);
#endif
}

auto SolidScene::cleanup() -> void {
#ifdef RGB_DEBUG
  webServerHandle.reset();
#endif
}

auto SolidScene::update() -> void {
}

auto SolidScene::draw() -> void {
  leds.fill(color);
}