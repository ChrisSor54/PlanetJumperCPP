#include "PhysicsObject.h"

//--------------------------------------------------------------------------------------------------
// PhysicsObject
//--------------------------------------------------------------------------------------------------


// Constructors
PhysObj::PhysicsObject(double mass, double radius)
    :PhysicsObject(Vector2d(0,0), Vector2d(0,0), mass, radius, DEFAULT_FRICTION_COEFFICIENT) {};

PhysObj::PhysicsObject(Vector2d position, Vector2d velocity, double mass, double radius, float frictionCoefficient)
    : position(position), velocity(velocity), mass(mass), radius(radius), frictionCoefficient(frictionCoefficient) {
        frictionCoefficient = std::clamp(frictionCoefficient, 0.f, 1.f); // Clamp friction
        static int numBodies = 0;
        this->id = numBodies;
        numBodies++;
        this->velocityBuffer = Vector2d(0, 0);
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
    //std::cout << "Body: " << std::to_string(id) << std::endl;
    if (checkCollision(other, dt)) {
        Vector2d collisionVector = getCollisionImpulse(other);
        if (collisionVector.length() <= gravityVector.length()*mass) {
            collisionVector = getCollisionImpulse(other, 0.0); // Nullify the collision
        }
        // Apply frictional dampening
        //double res
        Vector2d integratedForceVector = getIntegratedForces(other, dt, 1);

        Vector2d totalVelocity = (velocity - other.velocity) + integratedForceVector;
        Vector2d normal = (other.position-position).normalized();
        Vector2d vNormal = normal * totalVelocity.dot(normal);
        Vector2d vTangent = totalVelocity - vNormal;
        vTangent *= 1.0 - other.frictionCoefficient; // Dampen the tangential velocity by the friction coefficient
        Vector2d finalImpulse = (vNormal + vTangent ) - (velocity - other.velocity); // Velocity will be added from the buffer
        applyImpulse(finalImpulse);
        hasCollided = true;
    } else if (surfaceDist > 0) {
        velocityBuffer += gravityVector;
    }
    
    if (!parentObject) {
        parentObject = &other;
    } else if (parentObject != &other && gravityVector.lengthSquared() > (getGravityVector(*parentObject)*dt).lengthSquared()) {
        parentObject = &other;
    }
}

/// @brief Update the position of the object by its velocity
/// @param dt Deltatime
void PhysObj::updatePosition(double dt) {
    //position += positionBuffer;
    velocity += velocityBuffer;
    position += (velocity*dt);
    velocityBuffer = Vector2d(0, 0);
}

/// @brief Checks for and removes overlap between objects
/// @param other The other object to check
void PhysObj::fixOverlap(PhysObj& other) {
    fixOverlap(other, false);
}

/// @brief Checks for and removes overlap between objects
/// @param other The other object to check
/// @param force Force reposition without checking distance
void PhysObj::fixOverlap(PhysObj& other, bool force) {
    double surfaceDist = getSurfaceDistance(other);
    if (surfaceDist < 0 || force) {
        Vector2d normal = (position - other.position).normalized();
        Vector2d offset = normal * (surfaceDist + GROUNDED_MARGIN);

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

/// @brief Apply an instant impulse to a PhysicsObject
/// @param impulseVector The vector of the impulse
void PhysObj::applyImpulse(Vector2d impulseVector) {
    velocityBuffer += impulseVector/mass;
}


// Protected Methods -------------------------------------------------------------------------------

void PhysObj::updateVirtualForces(PhysObj& other, double dt) {
    Vector2d gravityVector = getGravityVector(other)*dt;
    double surfaceDist = getSurfaceDistance(other);
    //std::cout << "Body: " << std::to_string(id) << std::endl;

    if (checkCollision(other, dt)) {
        Vector2d collisionVector = getCollisionImpulse(other);
        // if (collisionVector.length() <= gravityVector.length()*mass) {
        //     collisionVector = getCollisionImpulse(other, 0.0); // Nullify the collision
        // }
        // Apply frictional dampening
        // Vector2d totalVelocity = (velocity - other.velocity) + collisionVector;
        // Vector2d normal = (other.position-position).normalized();
        // Vector2d vNormal = normal * totalVelocity.dot(normal);
        // Vector2d vTangent = totalVelocity - vNormal;
        // vTangent *= (1.0 - other.frictionCoefficient); // Dampen the tangential velocity by the friction coefficient
        // Vector2d finalImpulse = (vNormal + vTangent ) - (velocity - other.velocity); // Velocity will be added from the buffer
        applyImpulse(collisionVector);
        hasCollided = true;
    } else if (surfaceDist > 0) {
        velocityBuffer += gravityVector;
    }
    
    if (!parentObject) {
        parentObject = &other;
    } else if (parentObject != &other && gravityVector.lengthSquared() > (getGravityVector(*parentObject)*dt).lengthSquared()) {
        parentObject = &other;
    }
}


Vector2d PhysObj::getIntegratedForces(PhysObj& other, double dt, double resolution) {
    // Store current values

    Vector2d sPosition = position;
    Vector2d sVelocity = velocity;
    Vector2d sVelocityBuffer = velocityBuffer;
    Vector2d soPosition = other.position;
    Vector2d soVelocity = other.velocity;
    Vector2d soVelocityBuffer = other.velocityBuffer;

    velocityBuffer = Vector2d(0,0);
    other.velocityBuffer = Vector2d(0,0);

    resolution = std::max(resolution, 1.0);
    double delta =  dt/resolution;
    for (int i=0; i < resolution; i++) {
        updateVirtualForces(other, delta);
        other.updateVirtualForces(*this, delta);
        updatePosition(delta);
        other.updatePosition(delta);
        //fixOverlap(other);
        //other.fixOverlap(*this);
    }

    Vector2d finalImpulse = velocity - sVelocity;

    // Restore starting values
    position = sPosition;
    velocity = sVelocity;
    velocityBuffer = sVelocityBuffer;
    other.position = soPosition;
    other.velocity = soVelocity;
    other.velocityBuffer = soVelocityBuffer;
    
    return finalImpulse;
}

/// @brief Checks if this PhyicsObject will collide with other
/// @param other The PhysicsObject to check with
/// @param dt Deltatime
/// @return Whether a collision will occur
bool PhysObj::checkCollision(PhysObj& other, double dt) {
    return checkCollision(other, dt, 1);
}

/// @brief Checks if this PhyicsObject will collide with other
/// @param other The PhysicsObject to check with
/// @param dt Deltatime
/// @param resolution How many subdivisions to check collision along
/// @return Whether a collision will occur
bool PhysObj::checkCollision(PhysObj& other, double dt, double resolution) {
    if (resolution <= 0) {
        throw std::invalid_argument("deltaScale must be greater than 0");
    }
    double step = 0;
    double sqrRadii = pow(radius + other.radius, 2);
    while (step < dt) {
        step += dt/resolution;
        Vector2d futurePos = position + (velocity * step);
        Vector2d otherFuturePos = other.position + (other.velocity * step);
        if ((futurePos-otherFuturePos).lengthSquared() <= sqrRadii) {
            return true;            
        }
    }
    return false;
}

/// @brief Get the collision impulse vector
/// @param other The colliding PhysicsObject
/// @return The impulse vector
Vector2d PhysObj::getCollisionImpulse(PhysObj& other) {
    float avgElasticity = (elasticity + other.elasticity)/2;
    return getCollisionImpulse(other, avgElasticity);
}

/// @brief Get the collision impulse vector
/// @param other The colliding PhysicsObject
/// @param elasticity The elasticity of the collision
/// @return The impulse vector
Vector2d PhysObj::getCollisionImpulse(PhysObj& other, double elasticity) {
    Vector2d relativeVelocity = velocity - other.velocity;
    Vector2d collisionNormal = (position - other.position).normalized();
    double vNormal = relativeVelocity.dot(collisionNormal);
    Vector2d collisionImpulse = -(vNormal*(1 + elasticity)/(1/mass + 1/other.mass)) * collisionNormal;
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
    Vector2d normal = other.position - position;
    double sqrDist = normal.lengthSquared();
    //double sqrDist = pow(dist, 2);
    if (sqrDist == 0) {
        return Vector2d(0, 0);
    }
    Vector2d gravityVector = normal.normalized() * G*other.mass/sqrDist;
    return gravityVector;
}



//--------------------------------------------------------------------------------------------------
// Body
//--------------------------------------------------------------------------------------------------


Body::Body(Vector2d position, Vector2d velocity, double mass, double radius, float frictionCoefficient, sf::Color color) 
    : PhysicsObject(position, velocity, mass, radius, frictionCoefficient), color(color)  {
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
