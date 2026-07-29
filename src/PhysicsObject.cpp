#include "PhysicsObject.h"

//--------------------------------------------------------------------------------------------------
// PhysicsObject
//--------------------------------------------------------------------------------------------------


// Constructors
PhysObj::PhysicsObject(double mass, double radius)
    :PhysicsObject(Vector2d(0,0), Vector2d(0,0), mass, radius) {};

PhysObj::PhysicsObject(Vector2d position, Vector2d velocity, double mass, double radius)
    : position(position), velocity(velocity), mass(mass), radius(radius) {
        static int numBodies = 0;
        this->id = numBodies;
        numBodies++;
        this->velocityBuffer = Vector2d(0.0, 0.0);
        hasCollided = false;
        elasticity = ELASTICITY;
    }

// Public Methods ----------------------------------------------------------------------------------

void PhysObj::setPosition(Vector2d position) {
    this->position = position;
}

/// @brief Updates velocity with the resulting forces of interactions between PhysicsObjects
/// @param other The PhysicsObject being interacted with
/// @param dt Deltatime
void PhysObj::updateForces(PhysObj& other, double dt) {
    Vector2d gravityVector = getGravityVector(other)*dt;
    double surfaceDist = getSurfaceDistance(other);
    hasCollided = false;
    //std::cout << "Body: " << std::to_string(id) << std::endl;
    if (checkCollision(other, dt)) {
        Vector2d collisionVector = getCollisionImpulse(other);
        if (collisionVector.length() <= gravityVector.length()*mass) {
            collisionVector = getCollisionImpulse(other, 0.0); // Nullify the collision
            //std::cout << "Collision dampened" << std::endl;
        } else {
            //std::cout << "Collision : " << std::to_string(collisionVector.length()) << std::endl;
        }
        applyImpulse(collisionVector);
        hasCollided = true;
    } else if (surfaceDist > 0) {
        //std::cout << "Gravity : " << std::to_string(gravityVector.length()) << std::endl;
        velocityBuffer += gravityVector;
        if (!parentObject) {
            parentObject = &other;
        } else if (parentObject != &other && gravityVector.lengthSquared() > (getGravityVector(*parentObject)*dt).lengthSquared()) {
            parentObject = &other;
        }
    }
}

/// @brief Update the position of the object by its velocity
/// @param dt Deltatime
void PhysObj::updatePosition(double dt) {
    //position += positionBuffer;
    velocity += velocityBuffer;
    position += (velocity*dt);
    velocityBuffer = Vector2d(0.0, 0.0);
}


/// @brief Checks for and removes overlap between objects
/// @param other The other object to check
/// @param dt Deltatime
void PhysObj::fixOverlap(PhysObj& other, double dt) {
    if (checkCollision(other, dt)) {
        double surfaceDist = getSurfaceDistance(other);
        if (surfaceDist < 0) {
            Vector2d offset = (position - other.position).normalized() * surfaceDist;
                // How far position needs to be offset to avoid overlap
            if (mass == other.mass) {
                position -= offset/2.0;
                other.position += offset/2.0;
            } else if (mass < other.mass) {
                position -= offset;
            } else {
                other.position += offset;
            }
        }
    }
}

/// @brief Apply an instant impulse to a PhysicsObject
/// @param impulseVector The vector of the impulse
void PhysObj::applyImpulse(Vector2d impulseVector) {
    velocityBuffer += impulseVector/mass;
}


// Protected Methods -------------------------------------------------------------------------------

/// @brief Checks if this PhyicsObject will collide with other
/// @param other The PhysicsObject to check with
/// @param dt Deltatime
/// @return Whether a collision will occur
bool PhysObj::checkCollision(PhysObj& other, double dt) {
    Vector2d futurePos = position + (velocity * dt);
    Vector2d otherFuturePos = other.position + (other.velocity * dt);
    double sqrRadii = pow(radius + other.radius, 2);
    return (futurePos - otherFuturePos).lengthSquared() <= sqrRadii;
}

/// @brief Get the collision impulse vector
/// @param other The colliding PhysicsObject
/// @return The impulse vector
Vector2d PhysObj::getCollisionImpulse(PhysObj& other) {
    return getCollisionImpulse(other, elasticity);
}

/// @brief Get the collision impulse vector
/// @param other The colliding PhysicsObject
/// @param elasticity The elasticity of the collision
/// @return The impulse vector
Vector2d PhysObj::getCollisionImpulse(PhysObj& other, double elasticity) {
    Vector2d relativeVelocity = velocity - other.velocity;
    Vector2d collisionNormal = (position - other.position).normalized();
    double vn = relativeVelocity.dot(collisionNormal);
    Vector2d collisionImpulse = -(vn*(1 + elasticity)/(1/mass + 1/other.mass)) * collisionNormal;
    return collisionImpulse;
}

/// @brief Get the distance between object surfaces
/// @param other The other PhsyicsObject
/// @return The distance between their surfaces
double PhysObj::getSurfaceDistance(PhysObj& other) {
    return (other.position - position).length() - (radius + other.radius);
}

/// @brief Get the acceleration vector of the gravitational force between two PhysicsObjects
/// @param other The other PhysicsObject
/// @return The acceleration vector of gravitational attraction
Vector2d PhysObj::getGravityVector(PhysicsObject& other) {
    Vector2d dPosition = position - other.position;
    double sqrDist = pow(dPosition.x, 2) + pow(dPosition.y, 2);
    //double sqrDist = pow(dist, 2);
    if (sqrDist == 0) {
        return Vector2d(0.0, 0.0);
    }
    Vector2d gravityVector = dPosition.normalized() * -G*other.mass/sqrDist;
    return gravityVector;
}



//--------------------------------------------------------------------------------------------------
// Body
//--------------------------------------------------------------------------------------------------


Body::Body(Vector2d position, Vector2d velocity, double mass, double radius, sf::Color color) 
    : PhysicsObject(position, velocity, mass, radius), color(color) {
    shape.setFillColor(color);
    shape.setRadius(radius);
    shape.setOrigin(sf::Vector2f(radius, radius));
}

void Body::draw(sf::RenderWindow& window, float distanceScale) {
    sf::Vector2f scaledPosition = static_cast<sf::Vector2f>(position)/distanceScale;
    sf::Vector2f center = static_cast<sf::Vector2f>(window.getSize())/2.f;
    shape.setPosition(scaledPosition + center);
    shape.setRadius(radius / distanceScale);
    shape.setOrigin(sf::Vector2f(radius/distanceScale, radius/distanceScale));
    window.draw(shape);
}
