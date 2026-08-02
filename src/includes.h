#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <vector>

// Constants
const double G = 1.0;
constexpr double PI = 3.14159265358979323846;


// Aliases

using Vector2d = sf::Vector2<double>;


// Structs

enum class InputAction {
    Up,
    Down,
    Left,
    Right,
    Jump,
    Walk,
    ZoomIn,
    ZoomOut,
    SpeedUp,
    SpeedDown,
    ResetTimeScale,
    ToggleRotation,
    DEBUG,
    DEBUG_ShowVelocities,
    DEBUG_ToggleReference,
    COUNT
};

struct InputState {
    bool pressed = false;
    bool released = false;
};

struct InputManager {
    std::unordered_map<InputAction, sf::Keyboard::Key> bindings = {
        {InputAction::Up, sf::Keyboard::Key::Up},
        {InputAction::Down, sf::Keyboard::Key::Down},
        {InputAction::Left, sf::Keyboard::Key::Left},
        {InputAction::Right, sf::Keyboard::Key::Right},
        {InputAction::Jump, sf::Keyboard::Key::Space},
        {InputAction::Walk, sf::Keyboard::Key::LShift},
        {InputAction::ZoomIn, sf::Keyboard::Key::PageDown},
        {InputAction::ZoomOut, sf::Keyboard::Key::PageUp},
        {InputAction::SpeedUp, sf::Keyboard::Key::Period},
        {InputAction::SpeedDown, sf::Keyboard::Key::Comma},
        {InputAction::ResetTimeScale, sf::Keyboard::Key::Slash},
        {InputAction::ToggleRotation, sf::Keyboard::Key::R},
        {InputAction::DEBUG, sf::Keyboard::Key::LControl},
        {InputAction::DEBUG_ShowVelocities, sf::Keyboard::Key::S},
        {InputAction::DEBUG_ToggleReference, sf::Keyboard::Key::R},
    };

    sf::Vector2f directionalInput = sf::Vector2f(0,0);
    std::unordered_map<InputAction, InputState> inputStates = {};
};