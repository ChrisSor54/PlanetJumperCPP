#pragma once
#include "includes.h"

#include "PhysicsObject.h"
#include "Player.h"

class Game {
    public:
        // Members
        sf::RenderWindow window;

        // Methods
        Game(unsigned int window_w, unsigned int window_h);
        void run();
        
    private:
        // Members
        std::vector<std::unique_ptr<PhysicsObject>> objects;
        Vector2d globalOrigin;
        Vector2d globalVelocity;
        float distanceScale = 1.0;
        signed int zoomScale = 0;
        Player player;
        float dt;

        // Methods
        void handleInput(double dt);
        void update(double dt);
        void draw(sf::RenderWindow& window);

        // Initialization
        void addBody(Vector2d position, Vector2d velocity, double mass, double radius, sf::Color color);

        // Updates
        void updateRelativeVelocities(PhysicsObject& referenceObject);

        // Camera
        float setZoomScale(int newZoomScale);
        void centerCamera(float lerpScale);
        void moveCamera(Vector2d offset);
};
