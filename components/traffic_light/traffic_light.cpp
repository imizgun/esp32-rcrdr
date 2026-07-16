#include "traffic_light.hpp"

// #define RED GPIO_NUM_17
// #define YELLOW GPIO_NUM_5
// #define GREEN GPIO_NUM_18
// 
TrafficLight::TrafficLight() {
  _state = TrafficLightState::ChangingState;
}

void TrafficLight::update() {
  switch (_state) {
      case TrafficLightState::ChangingState: _state = TrafficLightState::Allowed; break;
      case TrafficLightState::Forbidden: _state = TrafficLightState::ChangingState; break;
      case TrafficLightState::Allowed: _state = TrafficLightState::BeReadyToStop; break;
      case TrafficLightState::BeReadyToStop: _state = TrafficLightState::Forbidden;
  }
}

TrafficLightState TrafficLight::getState() const {
  return _state;
}

int TrafficLight::getStateTime() const {
  switch (_state) {
    case TrafficLightState::ChangingState: return 2000; break;
    case TrafficLightState::Forbidden: return 5000; break;
    case TrafficLightState::Allowed: return 7000; break;
    case TrafficLightState::BeReadyToStop: return 1500; break;
  }
  return 0;
}

// TrafficLight light = TrafficLight();

// gpio_reset_pin(RED);
// gpio_set_direction(RED, GPIO_MODE_OUTPUT);
// gpio_reset_pin(YELLOW);
// gpio_set_direction(YELLOW, GPIO_MODE_OUTPUT);
// gpio_reset_pin(GREEN);
// gpio_set_direction(GREEN, GPIO_MODE_OUTPUT);

// while (1) {
//     switch (light.getState()) {
//         case TrafficLightState::ChangingState:
//             gpio_set_level(RED, 1);
//             gpio_set_level(YELLOW, 1);
//             break;
//         case TrafficLightState::Allowed:
//             gpio_set_level(RED, 0);
//             gpio_set_level(YELLOW, 0);
//             gpio_set_level(GREEN, 1);
//             break;
//         case TrafficLightState::BeReadyToStop:        
//             gpio_set_level(YELLOW, 1);
//             gpio_set_level(GREEN, 0);
//             break;
//         case TrafficLightState::Forbidden:
//             gpio_set_level(RED, 1);
//             gpio_set_level(GREEN, 0);
//             gpio_set_level(YELLOW, 0);
//             break;
//     }
//     vTaskDelay(light.getStateTime() / portTICK_PERIOD_MS);
//     light.update();
// }
// 