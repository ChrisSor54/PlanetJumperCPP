#include "PhysicsObject.h"

//-----------------------------------------------------------------------
// PhysicsObject

PhysObj::PhysicsObject(Vector2d position, Vector2d velocity, double mass, double radius)
    : position(position), velocity(velocity), mass(mass), radius(radius) {};

void PhysObj::setPosition(Vector2d position) {
    this->position = position;
};

void PhysObj::applyImpulse(Vector2d impulseVector) {
    velocity += impulseVector/mass;
};

//-----------------------------------------------------------------------
// Body

Body::Body(Vector2d position, Vector2d velocity, double mass, double radius, sf::Color color) 
    : PhysicsObject(position, velocity, mass, radius), color(color) {

    shape.setFillColor(color);
    shape.setRadius(radius);
};

void Body::draw(sf::RenderWindow& window, float distanceScale) {

    sf::Vector2f scaledPosition = static_cast<sf::Vector2f>(position)/distanceScale;
    sf::Vector2f center = static_cast<sf::Vector2f>(window.getSize())/2.f;
    shape.setPosition(scaledPosition + center);
    shape.setRadius(radius / distanceScale);
    window.draw(shape);
};
