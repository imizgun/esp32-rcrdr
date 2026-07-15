#include "traffic_light.hpp"

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
