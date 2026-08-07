#include "main.h"


BodyTexture defaultTexture = {
    .xOrigin = 0,
    .yOrigin = 0,
    .width = 48,
    .height = 48,
    .repeat = true
};

BodyTexture homePlanetTexture = {
    .xOrigin = 48,
    .yOrigin = 0,
    .width = 48,
    .height = 48,
    .repeat = false
};

BodyTexture homeMoonTexture = {
    .xOrigin = 96,
    .yOrigin = 0,
    .width = 48,
    .height = 48,
    .repeat = false
};

BodyTexture gasGiantTexture = {
    .xOrigin = 0,
    .yOrigin = 48,
    .width = 48,
    .height = 48,
    .repeat = false
};

// MAIN --------------------------------------------------------------------------------------------
#pragma region Main

int main() {
    Game game(1600, 1200);

    Player* player1 = game.addPlayer(sf::Color(255, 255, 255));
    Player* player2 = game.addPlayer(sf::Color(255, 0, 0));

    // Star A
    Body* starA = game.addBody(
        Vector2d(0.f, 0.f), // Position
        Vector2d(0, 0), // Velocity
        5*pow(10, 11), // Mass
        20000.0, // Radius
        sf::degrees(0), // Rotational Velocity
        0.1, // Surface Friction
        MatterState::PLASMA, // State
        sf::Color(251, 255, 148) // Color
    ); 

    // Planet A
    Body* planetA = game.addSatellite(
        starA, // Parent Body
        sf::degrees(0), // Angle
        75000, // SemiMajorAxis
        0.2, // Eccentricity
        false, // CounterClockwise orbit
        10*pow(10, 7), // Mass
        450, // Radius
        sf::radians(0.0),
        0.6, // Surface Friction
        MatterState::SOLID, // State
        sf::Color(168, 168, 162), // Color
        homePlanetTexture
    ); 

    // Moon A1
    Body* moonA1 = game.addSatellite(
        planetA,
        sf::degrees(180),
        750,
        0.0,
        false,
        5*pow(10, 5),
        40.0,
        sf::radians(0.48686449556),
        1.0,
        MatterState::SOLID, // State
        sf::Color(255, 255, 255),
        homeMoonTexture
    );

    // Planet B
    Body* planetB = game.addSatellite(
        starA,
        sf::degrees(0),
        35000,
        0.2,
        false,
        65*pow(10, 5),
        100.0,
        sf::radians(0.108),
        0.8,
        MatterState::SOLID, // State
        sf::Color(21, 24, 79),
        defaultTexture
    );

    // Planet C
    Body* planetC = game.addSatellite(
        starA,
        sf::degrees(90),
        114000,
        0.05,
        false,
        18*pow(10, 7),
        1000.0,
        sf::degrees(0),
        0.1,
        MatterState::GAS, // State
        sf::Color(150, 140, 126),
        gasGiantTexture
    );

    // Planet D
    Body* planetD = game.addSatellite(
        starA,
        sf::degrees(180),
        130000,
        0.7,
        false,
        3*pow(10, 5),
        20.0,
        sf::degrees(0),
        0.1,
        MatterState::SOLID, // State
        sf::Color(70, 57, 145),
        homeMoonTexture
    );

    // Planet E
    Body* planetE = game.addSatellite(
        starA, // Parent Body
        sf::degrees(0), // Angle
        205000, // SemiMajorAxis
        0.2, // Eccentricity
        false, // CounterClockwise orbit
        2*pow(10, 9), // Mass
        550, // Radius
        sf::radians(0.481819315439),
        0.8, // Surface Friction
        MatterState::SOLID, // State
        sf::Color(0, 168, 0), // Color
        defaultTexture
    );

    // Moon E1
    Body* moonE1 = game.addSatellite(
        planetE, // Parent Body
        sf::degrees(0), // Angle
        2050, // SemiMajorAxis
        0.0, // Eccentricity
        false, // CounterClockwise orbit
        7*pow(10, 6), // Mass
        100, // Radius
        sf::radians(1),
        0.8, // Surface Friction
        MatterState::SOLID, // State
        sf::Color(0, 168, 210), // Color
        homeMoonTexture
    );

    // Moon E2
    Body* moonE2 = game.addSatellite(
        planetE, // Parent Body
        sf::degrees(180), // Angle
        2050, // SemiMajorAxis
        0.0, // Eccentricity
        false, // CounterClockwise orbit
        7*pow(10, 6), // Mass
        50, // Radius
        sf::radians(2),
        0.8, // Surface Friction
        MatterState::SOLID, // State
        sf::Color(255, 168, 0), // Color
        homeMoonTexture
    );



    game.teleportPlayerTo(0, planetA);
    game.teleportPlayerTo(1, moonA1);

    game.run();
    
    return 0;
}
#pragma endregion
