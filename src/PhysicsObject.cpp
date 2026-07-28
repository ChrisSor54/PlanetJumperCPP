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
        this->velocityBuffer = Vector2d(0.0, 0.0);
    };

// Public Methods ----------------------------------------------------------------------------------

void PhysObj::setPosition(Vector2d position) {
    this->position = position;
};

/// @brief Updates velocity with the resulting forces of interactions between PhysicsObjects
/// @param other The PhysicsObject being interacted with
/// @param dt Deltatime
/// @return Returns true if a collision occurred
bool PhysObj::updateForces(PhysObj& other, double dt) {
    Vector2d gravityVector = getGravityVector(other)*dt;
    double surfaceDist = getSurfaceDistance(other);
    if (surfaceDist < -COLLISION_OVERLAP_MARGIN) {
        Vector2d collisionVector = getCollisionImpulse(other);
        if (collisionVector.length() > 0.1/mass) {
            applyImpulse(collisionVector);
            std::cout << "Collision occured: " << std::to_string(id) << std::endl;
            return true;
        }
    } else if (surfaceDist > 0) {
        velocityBuffer += gravityVector;
        return false;
    }
    return false;
};

/// @brief Update the position of the object by its velocity
/// @param dt Deltatime
void PhysObj::updatePosition(double dt) {
    //position += positionBuffer;
    velocity += velocityBuffer;
    position += (velocity*dt);
    velocityBuffer = Vector2d(0.0, 0.0);
};


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


// Protected Methods -------------------------------------------------------------------------------

/// @brief Checks if this PhyicsObject will collide with other
/// @param other The PhysicsObject to check with
/// @param dt Deltatime
/// @return Whether a collision will occur
bool PhysObj::checkCollision(PhysObj& other, double dt) {
    Vector2d futurePos = position + (velocity * dt);
    Vector2d otherFuturePos = other.position + (other.velocity * dt);
    //double sqrRadii = pow(radius + other.radius - COLLISION_MARGIN, 2);
    return (futurePos - otherFuturePos).length() <= (radius + other.radius);
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
    velocityBuffer += impulseVector/mass;
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

sf::Font Body::font;
bool Body::fontLoaded = false;

Body::Body(Vector2d position, Vector2d velocity, double mass, double radius, sf::Color color) 
    : PhysicsObject(position, velocity, mass, radius), color(color), text(sf::Text(font, "", 10)) {
    shape.setFillColor(color);
    shape.setRadius(radius);
    text.setString(std::to_string(id));
    text.setFillColor(color);
};

void Body::draw(sf::RenderWindow& window, float distanceScale) {
    sf::Vector2f scaledPosition = static_cast<sf::Vector2f>(position)/distanceScale;
    sf::Vector2f center = static_cast<sf::Vector2f>(window.getSize())/2.f;
    shape.setPosition(scaledPosition + center);
    shape.setRadius(radius / distanceScale);
    window.draw(shape);
    if (!fontLoaded) {
        if (!font.openFromFile("Hack-Regular.ttf")) {
            return;
        }
        fontLoaded = true;
    }
    text.setPosition(scaledPosition + center + sf::Vector2f(0.0, -radius - 2.0));
    window.draw(text);
    
};
