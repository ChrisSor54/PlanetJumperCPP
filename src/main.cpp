#include "main.h"
#include <SFML/Graphics.hpp>
#include <iostream>
#include <cmath>
#include "player.h"
#include "body.h"


//-----------------------------------------------------------------------
// CONSTANTS

const float G = 1.0;


//-----------------------------------------------------------------------
// VARIABLES



//-----------------------------------------------------------------------
// Game

Game::Game(unsigned int window_w, unsigned int window_h) {
    window.create(sf::VideoMode({window_w, window_h}), "Planet Jumper");
};

void Game::run() {
    sf::Clock clock;
    window.setFramerateLimit(60);
    window.setVerticalSyncEnabled(true);

    bodies.push_back(Body(sf::Vector2f(10.f, 10.f), sf::Vector2f(0.0, 0.0), 6000.0, 10.0, sf::Color(255, 255, 255)));
    bodies.push_back(Body(sf::Vector2f(50.f, 10.f), sf::Vector2f(0.0, -9.0), 10.0, 10.0, sf::Color(255, 255, 255)));

    while (window.isOpen()) {
        
        float dt = clock.restart().asSeconds(); // get deltatime
        update(dt);
        handleInput();
        draw(window);
        
    }
};

void Game::handleInput() {
    while (const std::optional event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>())
            window.close();
    }
};

void Game::update(float dt) {
    for (Body& body1 : bodies) {
        for (Body& body2 : bodies) {
            if (&body1 == &body2) {
                continue;
            }
            sf::Vector2f gravVector = getGravityVector(body1, body2, dt);
            body1.velocity += gravVector;
        }
    }


    for (Body& body : bodies) {
        body.setPosition(body.position + body.velocity*dt);
    }
};

void Game::draw(sf::RenderWindow& window) {
    window.clear();
    for (Body& body : bodies) {
        body.draw(window, distance_scale);
    }

    window.display();
}

sf::Vector2f Game::getGravityVector(Body& body1, Body& body2, float dt) {
    sf::Vector2f dv = body2.position - body1.position;
    float dist = sqrt(pow(dv.x, 2) + pow(dv.y, 2));
    float sqrDist = pow(dist, 2);
    if (sqrDist == 0) {
        return sf::Vector2f(0.0, 0.0);
    }
    sf::Vector2f normal = dv/dist;
    sf::Vector2f gravityVector = normal * static_cast<float>(sqrt(G*body2.mass/sqrDist))*dt;
    return gravityVector;
}



int main() {
    Game game(800, 600);
    game.run();
    
    return 0;
}