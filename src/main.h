#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include "body.h"
#include "player.h"

class Game {
    public:
        // Members
        sf::RenderWindow window;

        // Methods
        Game(unsigned int window_w, unsigned int window_h);
        void run();
        
    private:
        // Members
        std::vector<Body> bodies;
        float distanceScale = 1.0;
        signed int zoomScale = 0;

        // Methods
        void handleInput(float dt);
        void update(float dt);
        void draw(sf::RenderWindow& window);

        float setZoomScale(int newZoomScale);
        void moveCamera(sf::Vector2f offset);

        sf::Vector2f getGravityVector(Body& body1, Body& body2, float dt);
};
