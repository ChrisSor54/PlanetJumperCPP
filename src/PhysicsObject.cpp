#include "PhysicsObject.h"

//--------------------------------------------------------------------------------------------------
// PhysicsObject
//--------------------------------------------------------------------------------------------------


// Constructor
PhysObj::PhysicsObject(Vector2d position, Vector2d velocity, double mass, double radius)
    : position(position), velocity(velocity), mass(mass), radius(radius) {
        static int numBodies = 0;
        this->id = numBodies;
        numBodies++;
    };

// Public Methods ----------------------------------------------------------------------------------

void PhysObj::setPosition(Vector2d position) {
    this->position = position;
};

/// @brief Updates velocity with the resulting forces of interactions between PhysicsObjects
/// @param other The PhysicsObject being interacted with
/// @param dt Deltatime
void PhysObj::updateForces(PhysObj& other, double dt) {
    Vector2d gravityVector = getGravityVector(other)*dt;
    if (checkCollision(other, dt)) {
        Vector2d collisionVector = getCollisionImpulse(other);
        applyImpulse(collisionVector);
        other.applyImpulse(-collisionVector);
    } else {
        velocity += gravityVector;
        if (checkCollision(other, dt)) {
            // If gravity is the only force causing collision, don't apply it
            velocity -= gravityVector;
        }
        double surfaceDist = getSurfaceDistance(other);
        if (surfaceDist < 0) {
            Vector2d offset = (position - other.position).normalized() * surfaceDist;
                // How far position needs to be offset to avoid overlap
            if (mass < other.mass) {
                position -= offset;
            } else {
                other.position += offset;
            }
        }
    }
};

/// @brief Update the position of the object by its velocity
/// @param dt Deltatime
void PhysObj::updatePosition(double dt) {
    position += (velocity*dt);
};


// Protected Methods -------------------------------------------------------------------------------

/// @brief Checks if this PhyicsObject will colide with other
/// @param other The PhysicsObject to check with
/// @param dt Deltatime
/// @return Whether a collision will occur
bool PhysObj::checkCollision(PhysObj& other, double dt) {
    Vector2d futurePos = position + (velocity * dt);
    Vector2d otherFuturePos = other.position + (other.velocity * dt);
    double sqrRadii = pow(radius + other.radius, 2);
    return (futurePos - otherFuturePos).lengthSquared() <= (sqrRadii + COLLISION_MARGIN);
};

/// @brief Get the collision impulse vector
/// @param other The colliding PhysicsObject
/// @return The impulse vector
Vector2d PhysObj::getCollisionImpulse(PhysObj& other) {
    Vector2d relativeVelocity = velocity - other.velocity;
    double e = ELASTICITY;
    Vector2d collisionNormal = (position - other.position).normalized();
    double vn = relativeVelocity.dot(collisionNormal);
    Vector2d baseImpulse = -(vn*(1 + e)/(1/mass + 1/other.mass)) * collisionNormal;
    return baseImpulse;
};

/// @brief Get the distance between object surfaces
/// @param other The other PhsyicsObject
/// @return The distance between their surfaces
double PhysObj::getSurfaceDistance(PhysObj& other) {
    return (other.position - position).length() - (radius + other.radius);
};

/// @brief Apply an instant impulse to a PhysicsObject
/// @param impulseVector The vector of the impulse
void PhysObj::applyImpulse(Vector2d impulseVector) {
    velocity += impulseVector/mass;
};


/// @brief Get the acceleration vector of the gravitational force between two PhysicsObjects
/// @param other The other PhysicsObject
/// @return The acceleration vector of gravitational attraction
Vector2d PhysObj::getGravityVector(PhysicsObject& other) {
    Vector2d dPosition = position - other.position;
    double dist = sqrt(pow(dPosition.x, 2) + pow(dPosition.y, 2));
    double sqrDist = pow(dist, 2);
    if (sqrDist == 0) {
        return Vector2d(0.0, 0.0);
    }
    Vector2d normal = dPosition/dist;
    Vector2d gravityVector = normal * -sqrt(G*other.mass/sqrDist);
    return gravityVector;
};



//--------------------------------------------------------------------------------------------------
// Body
//--------------------------------------------------------------------------------------------------

const sf::Font font("arial.ttf");

Body::Body(Vector2d position, Vector2d velocity, double mass, double radius, sf::Color color) 
    : PhysicsObject(position, velocity, mass, radius), color(color), text(font, std::to_string(id), 10) {
    shape.setFillColor(color);
    shape.setRadius(radius);
};

void Body::draw(sf::RenderWindow& window, float distanceScale) {

    sf::Vector2f scaledPosition = static_cast<sf::Vector2f>(position)/distanceScale;
    sf::Vector2f center = static_cast<sf::Vector2f>(window.getSize())/2.f;
    text.setPosition(scaledPosition + center);
    shape.setPosition(scaledPosition + center);
    shape.setRadius(radius / distanceScale);
    window.draw(shape);
};
