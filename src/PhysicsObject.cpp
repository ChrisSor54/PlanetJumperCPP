#include "PhysicsObject.h"

//--------------------------------------------------------------------------------------------------
// PhysicsObject
//--------------------------------------------------------------------------------------------------


// Constructors
PhysObj::PhysicsObject(double mass, double radius)
    :PhysicsObject(Vector2d(0,0), Vector2d(0,0), mass, radius, sf::degrees(0), DEFAULT_FRICTION_COEFFICIENT) {};

PhysObj::PhysicsObject(Vector2d position, Vector2d velocity, double mass, double radius, sf::Angle rotationalVelocity, float surfaceFriction)
    : position(position), velocity(velocity), mass(mass), radius(radius), rotationalVelocity(rotationalVelocity), surfaceFriction(surfaceFriction) {
    
    surfaceFriction = std::clamp(surfaceFriction, 0.f, 1.f); // Clamp friction
    static int numBodies = 0;
    this->id = numBodies;
    numBodies++;
    this->velocityBuffer = Vector2d(0, 0);
    hasCollided = false;
    elasticity = ELASTICITY;
}

// Public Methods ----------------------------------------------------------------------------------

// Updates -----------------------------------------------------------------------------------------
#pragma region Updates

/// @brief Updates velocity with the gravitational forces between PhysicsObjects
/// @param other The PhysicsObject being interacted with
/// @param dt Deltatime
void PhysObj::updateGravity(PhysObj& other, double dt) {
    Vector2d gravityVector = getGravityVector(other)*dt;
    double surfaceDist = getSurfaceDistance(other);
    velocity += gravityVector;
    //std::cout << "Body: " << std::to_string(id) << std::endl;
    if (surfaceDist <= 2*GROUNDED_MARGIN) {
        //std::cout << "Gravity" << std::endl;
        hasCollided = true;
        isGrounded = true;
        if (other.mass >= mass) parentObject = &other;
    }
    
    if (other.mass >= mass) {
        if (!parentObject) {
            parentObject = &other;
        } else if (!isGrounded && parentObject != &other && getGravityVector(other, 3).lengthSquared() > (getGravityVector(*parentObject, 3)).lengthSquared()) {
            parentObject = &other;
        }
    }
}

/// @brief Updates velocity with the kinetic forces between PhysicsObjects
/// @param other The PhysicsObject being interacted with
/// @param dt Deltatime
void PhysObj::updateCollision(PhysObj& other, double dt) {
    double resolution = 1.0;
    
    Vector2d relVelocity = (velocity - other.velocity);
    if ((relVelocity*dt).lengthSquared() > pow(radius + other.radius, 2)) {
        resolution = relVelocity.length()*dt/(radius + other.radius);
    }
    if (checkCollision(other, dt, resolution)) {
        Vector2d normal = (other.position - position).normalized();
        Vector2d collisionImpulse = getCollisionImpulse(other);        
        double restThreshold = (0.1*getGravityVector(other)*mass).lengthSquared();
        if (collisionImpulse.lengthSquared() <= restThreshold) {
            //std::cout << std::to_string(id) << ": Damp" << std::endl;
            collisionImpulse = getCollisionImpulse(other, 0.0); // Nullify the collision
            // std::cout << std::to_string(id) << ": " << std::to_string(collisionImpulse.length() - (getGravityVector(other)*dt).length()) << std::endl;
            double surfaceDistance = getSurfaceDistance(other);
            if (surfaceDistance > 0) {
                fixOverlap(other, true);
            }
            if (other.mass >= mass) parentObject = &other;
                isGrounded = true;
        }

        double jNormal = collisionImpulse.dot(normal);
        Vector2d normalImpulse = normal * jNormal;
        double surfaceVelocity = other.rotationalVelocity.asRadians()*(other.radius + radius);
        relVelocity -= normal.rotatedBy(sf::degrees(-90)) * surfaceVelocity;
        Vector2d vTangentVel = relVelocity - normal * relVelocity.dot(normal);
        double tangentSpeed = vTangentVel.length();

        Vector2d frictionImpulse(0, 0);
        if (tangentSpeed > 0) {
            Vector2d tangentDir = vTangentVel.normalized();
            double reducedMass = 1.0 / (1.0/mass + 1.0/other.mass);
            double jTangentNeeded = tangentSpeed * reducedMass; // impulse to fully stop sliding
            double avgFriction = sqrt(surfaceFriction*other.surfaceFriction);
            double jTangentMax = avgFriction * std::abs(jNormal); // Coulomb's law
            double jFriction = std::min(jTangentNeeded, jTangentMax);
            frictionImpulse = -tangentDir * jFriction;
        }

        Vector2d totalImpulse = normalImpulse + frictionImpulse;
        //std::cout << std::to_string(id) << ": Before Friction: " << std::to_string(relVelocity.length()) << std::endl;
        applyImpulse(totalImpulse);
        //std::cout << std::to_string(id) << ": After Friction: " << std::to_string((velocity + velocityBuffer).length()) << std::endl;
        hasCollided = true;
    }
}

/// @brief Update the position of the object by its velocity
/// @param dt Deltatime
void PhysObj::updatePosition(double dt) {
    rotation += rotationalVelocity*dt;
    position += (velocity*dt);
}

void PhysObj::updateVelocity() {
    velocity += velocityBuffer;
    velocityBuffer = Vector2d(0,0);
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
    if (surfaceDist <= GROUNDED_MARGIN || force) {
        //std::cout << "Overlap" << std::endl;
        Vector2d normal = (position - other.position).normalized();
        Vector2d offset = normal * (surfaceDist);

        if (mass == other.mass) {
            position -= offset/2.0;
            other.position += offset/2.0;
        } else if (mass < other.mass) {
            position -= offset;
        }
        hasCollided = true;
    }
}

/// @brief Apply an instant impulse to a PhysicsObject
/// @param impulseVector The vector of the impulse
void PhysObj::applyImpulse(Vector2d impulseVector) {
    velocityBuffer += impulseVector/mass;
}

#pragma endregion

// Protected Methods -------------------------------------------------------------------------------

// Physics -----------------------------------------------------------------------------------------
#pragma region Physics


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
        throw std::invalid_argument("resolution must be greater than 0");
    }
    double step = 0;
    double sqrRadii = pow(radius + other.radius + GROUNDED_MARGIN, 2);
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
    float avgElasticity = sqrt(elasticity*other.elasticity);
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
Vector2d PhysObj::getGravityVector(PhysObj& other) {
    return getGravityVector(other, 2);
}

/// @brief Get the acceleration vector of the gravitational force between two PhysicsObjects
/// @param other The other PhysicsObject
/// @param distancePower Exponent for distance
/// @return The acceleration vector of gravitational attraction
Vector2d PhysObj::getGravityVector(PhysObj& other, int distancePower) {
    Vector2d normal = other.position - position;
    double distanceFactor = normal.lengthSquared();
    if (distancePower != 2) {
        distanceFactor = pow(normal.length(), distancePower);
    }
    //double sqrDist = pow(dist, 2);
    if (distanceFactor == 0) {
        return Vector2d(0, 0);
    }
    Vector2d gravityVector = normal.normalized() * G*other.mass/distanceFactor;
    return gravityVector;
}

#pragma endregion

// Drawing -----------------------------------------------------------------------------------------

// void PhysObj::drawVelocity(sf::RenderWindow& window, Vector2d referenceVelocity, double scale) {
//     Vector2d relativeVelocity = velocity - referenceVelocity;
//     Vector2d center = static_cast<Vector2d>(window.getSize())/2.0;

//     sf::Color color(255,255,255);

//     sf::VertexArray velocityLine(sf::PrimitiveType::Lines, 2); 
//     velocityLine[0].position = static_cast<sf::Vector2f>(position + center);
//     velocityLine[0].color = color;
//     velocityLine[1].position = static_cast<sf::Vector2f>(position + center + relativeVelocity*scale);
//     velocityLine[1].color = color;

//     window.draw(velocityLine);
// }

//--------------------------------------------------------------------------------------------------
// Body
//--------------------------------------------------------------------------------------------------

#pragma region Body Class

bool Body::textureLoaded = false;
sf::Texture Body::texture;
Body::Body(Vector2d position, Vector2d velocity,
    double mass, double radius, sf::Angle rotationalVelocity, float surfaceFriction,
    sf::Color color) 
    : Body(position, velocity, mass, radius, rotationalVelocity, surfaceFriction, color, false) {
}   

Body::Body(Vector2d position, Vector2d velocity,
    double mass, double radius, sf::Angle rotationalVelocity, float surfaceFriction,
    sf::Color color, bool drawTexture) 
    : PhysicsObject(position, velocity, mass, radius, rotationalVelocity, surfaceFriction), color(color)  {
    shape.setRadius(radius);
    shape.setOrigin(sf::Vector2f(radius, radius));
    shape.setPointCount(30 + (int) (radius/5));
    if (!Body::textureLoaded) {
        sf::Image textureImage;
        if (!textureImage.loadFromFile("assets/body_texture.png")) {
            throw std::invalid_argument("Bad body texture");
        }
        sf::Image tileTextureImage(sf::Vector2u(32, 32));
        if (!tileTextureImage.copy(textureImage, {0, 0}, sf::IntRect({0, 0}, {32,32}))) {
            throw std::invalid_argument("Failed to build tile texture");
        }

        if (!Body::texture.loadFromImage(tileTextureImage)) {
            throw std::invalid_argument("Failed to build tile texture");
        }
        Body::texture.setRepeated(true);
        Body::textureLoaded = true;
    }
    int textureWidth = static_cast<int>(radius);
    if (drawTexture) {
        shape.setTexture(&Body::texture);
        shape.setTextureRect(sf::IntRect({0,0}, {textureWidth, textureWidth}));
    }
    shape.setFillColor(color);
}

void Body::draw(sf::RenderWindow& window) {
    sf::Vector2f center = static_cast<sf::Vector2f>(window.getSize())/2.f;
    shape.setPosition(static_cast<sf::Vector2f>(position) + center);
    shape.setRotation(rotation);
    window.draw(shape);
}

void Body::drawVelocity(sf::RenderWindow& window, Vector2d referenceVelocity, double scale) {
    Vector2d relativeVelocity = velocity - referenceVelocity;
    Vector2d center = static_cast<Vector2d>(window.getSize())/2.0;

    sf::Color lineColor = sf::Color(255,0,0);
    // float brightness = (color.r + color.g + color.b) / 3.f;
    // if (brightness > 255/2) {
    //     lineColor = sf::Color(0,0,0);
    // }
    // lineColor.a = 255;

    sf::VertexArray velocityLine(sf::PrimitiveType::Lines, 2); 
    velocityLine[0].position = static_cast<sf::Vector2f>(position + center);
    velocityLine[0].color = lineColor;
    velocityLine[1].position = static_cast<sf::Vector2f>(position + center + relativeVelocity*scale);
    velocityLine[1].color = lineColor;

    window.draw(velocityLine);
}

#pragma endregion
