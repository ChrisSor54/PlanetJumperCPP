#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <vector>


// Includes are used by multiple files


// Constants
const double G = 1.0;
constexpr double PI = 3.14159265358979323846;


// Aliases

using Vector2d = sf::Vector2<double>;
using Vector2f = sf::Vector2f;

using sfKey = sf::Keyboard::Scan;


// Input

enum class InputAction {
    Up,
    Down,
    Left,
    Right,
    Jump,
    Walk,
    RotateR,
    RotateL,
    ZoomIn,
    ZoomOut,
    SpeedUp,
    SpeedDown,
    ResetTimescale,
    ToggleRotation,
    ToggleFreecam,
    DEBUG,
    DEBUG_ShowVelocities,
    DEBUG_ToggleReference,
    COUNT
};

struct InputState {
    bool pressed = false;
    bool released = false;
};

struct InputMap {
    std::unordered_map<InputAction, std::vector<sfKey>> bindings;
    std::unordered_map<InputAction, InputState> inputStates;
    Vector2f directionalInput = Vector2f(0,0);
};



struct InputManager {
    std::unordered_map<int, InputMap> playerInputs = {
        {0, {{
            {InputAction::Up,                       {sfKey::W}},
            {InputAction::Down,                     {sfKey::S}},
            {InputAction::Left,                     {sfKey::A}},
            {InputAction::Right,                    {sfKey::D}},
            {InputAction::Jump,                     {sfKey::Space}},
            {InputAction::Walk,                     {sfKey::LShift}},
            {InputAction::RotateR,                  {sfKey::E}},
            {InputAction::RotateL,                  {sfKey::Q}},
            }, {}, Vector2f(0,0)
        }},
        {1, {{
            {InputAction::Up,                       {sfKey::Up}},
            {InputAction::Down,                     {sfKey::Down}},
            {InputAction::Left,                     {sfKey::Left}},
            {InputAction::Right,                    {sfKey::Right}},
            {InputAction::Jump,                     {sfKey::RControl}},
            {InputAction::Walk,                     {sfKey::RShift}},
            {InputAction::RotateR,                  {sfKey::E}},
            {InputAction::RotateL,                  {sfKey::Q}},
            }, {}, Vector2f(0,0)
        }},
        {2, {{
            {InputAction::Up,                       {sfKey::W}},
            {InputAction::Down,                     {sfKey::S}},
            {InputAction::Left,                     {sfKey::A}},
            {InputAction::Right,                    {sfKey::D}},
            {InputAction::Jump,                     {sfKey::Space}},
            {InputAction::Walk,                     {sfKey::LShift}},
            {InputAction::RotateR,                  {sfKey::E}},
            {InputAction::RotateL,                  {sfKey::Q}},
            }, {}, Vector2f(0,0)
        }},
        {3, {{
            {InputAction::Up,                       {sfKey::W}},
            {InputAction::Down,                     {sfKey::S}},
            {InputAction::Left,                     {sfKey::A}},
            {InputAction::Right,                    {sfKey::D}},
            {InputAction::Jump,                     {sfKey::Space}},
            {InputAction::Walk,                     {sfKey::LShift}},
            {InputAction::RotateR,                  {sfKey::E}},
            {InputAction::RotateL,                  {sfKey::Q}},
            }, {}, Vector2f(0,0)
        }}
    };

    InputMap globalInputs = {{
        {InputAction::ZoomIn,                   {sfKey::PageDown}},
        {InputAction::ZoomOut,                  {sfKey::PageUp}},
        {InputAction::SpeedUp,                  {sfKey::Period}},
        {InputAction::SpeedDown,                {sfKey::Comma}},
        {InputAction::ResetTimescale,           {sfKey::Slash}},
        {InputAction::ToggleRotation,           {sfKey::R}},
        {InputAction::ToggleFreecam,            {sfKey::F}},
        {InputAction::DEBUG,                    {sfKey::LControl}},
        {InputAction::DEBUG_ShowVelocities,     {sfKey::S}},
        {InputAction::DEBUG_ToggleReference,    {sfKey::R}}
    }, {}, Vector2f(0, 0)
    };
};


struct BodyTexture {
    int xOrigin = 0;
    int yOrigin = 0;
    int width = 32;
    int height = 32;
    bool repeat = true;
};

