#include "body.h"
#include "main.h"
#include <SFML/Graphics.hpp>
#include <iostream>
#include <cmath>

using PhysObj = PhysicsObject;

//-----------------------------------------------------------------------
// PhysicsObject

PhysObj::PhysicsObject(sf::Vector2f position, sf::Vector2f velocity, float mass, float radius)
    : position(position), velocity(velocity), mass(mass), radius(radius) {};

//-----------------------------------------------------------------------
// Body

Body::Body(sf::Vector2f position, sf::Vector2f velocity, float mass, float radius, sf::Color color) 
    : PhysicsObject(position, velocity, mass, radius), color(color) {

    shape.setFillColor(color);
    shape.setRadius(radius);
};

void Body::setPosition(sf::Vector2f position) {
    this->position = position;
};

void Body::draw(sf::RenderWindow& window, float distanceScale) {

    sf::Vector2f scaled_position = 1/distanceScale * position;
    sf::Vector2f center = .5f * static_cast<sf::Vector2f>(window.getSize());
    shape.setPosition(scaled_position + center);
    shape.setRadius(radius / distanceScale);
    window.draw(shape);
};
