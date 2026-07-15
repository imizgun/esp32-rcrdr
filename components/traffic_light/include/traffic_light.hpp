#pragma once

enum class TrafficLightState {
    Forbidden,
    ChangingState,
    BeReadyToStop,
    Allowed
};

class TrafficLight {
    public: 
        TrafficLight();
        void update();
        TrafficLightState getState() const;
        int getStateTime() const;

    private:
        TrafficLightState _state;
};
