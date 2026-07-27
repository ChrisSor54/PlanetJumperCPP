#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
#include <cmath>

class PhysicsObject {
    public:
        // Properties
        sf::Vector2f position;
        sf::Vector2f velocity;
        float mass;
        float radius;

        // Methods
        PhysicsObject(sf::Vector2f position, sf::Vector2f velocity, float mass, float radius);
};

class Body : public PhysicsObject {
    public:
        // Properties
        sf::Color color;

        // Methods
        Body(sf::Vector2f position, sf::Vector2f velocity, float mass, float radius, sf::Color);
        void setPosition(sf::Vector2f newPos);
        void draw(sf::RenderWindow& window, float distanceScale);

    private:
        sf::CircleShape shape;
};
