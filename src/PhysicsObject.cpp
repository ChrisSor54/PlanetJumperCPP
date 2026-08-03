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
    if (surfaceDist <= 2.0*GROUNDED_MARGIN) {
        //std::cout << "Gravity" << std::endl;
        hasCollided = true;
        if (other.mass >= mass) {
            parentObject = &other;
            isGrounded = true;
        }
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
    if (checkCollision(other, dt)) {
        Vector2d normal = (other.position - position).normalized();
        double restThreshold = (COLLISION_REST_COEFFICIENT*getGravityVector(other)*mass).lengthSquared();
        Vector2d collisionImpulse = getCollisionImpulse(other, restThreshold);        
        Vector2d collisionNormal = normal * collisionImpulse.dot(normal);
        if (collisionNormal.lengthSquared() <= restThreshold) {
            double surfaceDistance = getSurfaceDistance(other);
            // if (surfaceDistance > 0) {
            //     fixOverlap(other, true);
            // }
            if (other.mass >= mass) {
                parentObject = &other;
                isGrounded = true;
            }
        }
        applyImpulse(collisionImpulse);
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
    Vector2d relVelocity = (velocity - other.velocity);
    double resolution = 1.0;
    if ((relVelocity*dt).lengthSquared() > pow(radius + other.radius, 2)) {
        resolution = relVelocity.length()*dt/(radius + other.radius);
    }
    return checkCollision(other, dt, resolution);
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

Vector2d PhysObj::getCollisionImpulse(PhysObj& other) {
    double sqrRestThreshold = (COLLISION_REST_COEFFICIENT*getGravityVector(other)*mass).lengthSquared();
    return getCollisionImpulse(other, sqrRestThreshold);
}

/// @brief Get the collision impulse vector
/// @param other The colliding PhysicsObject
/// @param elasticity The elasticity of the collision
/// @return The impulse vector
Vector2d PhysObj::getCollisionImpulse(PhysObj& other, double sqrRestThreshold) {
    Vector2d relativeVelocity = velocity - other.velocity;
    Vector2d normal = (other.position - position).normalized();
    double vNormal = relativeVelocity.dot(normal);
    double avgElasticity = sqrt(elasticity*other.elasticity);
    Vector2d collisionImpulse = -(vNormal*(1 + avgElasticity)/(1/mass + 1/other.mass)) * normal;
    if (collisionImpulse.lengthSquared() <= sqrRestThreshold) {
        collisionImpulse = -(vNormal)/(1/mass + 1/other.mass) * normal; // Nullify soft collisions
    }
    double jNormal = collisionImpulse.dot(normal);
    double totalSurfaceVelocity = (rotationalVelocity + other.rotationalVelocity).asRadians()*(other.radius + radius);
    relativeVelocity -= normal.rotatedBy(sf::degrees(-90)) * totalSurfaceVelocity;
    Vector2d vTangentVel = relativeVelocity - normal * relativeVelocity.dot(normal);
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

    Vector2d totalImpulse = collisionImpulse + frictionImpulse;
    return totalImpulse;
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

Vector2d PhysObj::getGravityVector(Vector2d position1, double mass1, Vector2d position2, double mass2) {
    Vector2d normal = (position2 - position1).normalized();
    double distanceFactor = normal.lengthSquared();
    return normal.normalized() * G*mass1*mass2/distanceFactor;
}

double PhysObj::getSurfaceVelocity() {
    return radius * rotationalVelocity.asRadians();
}

double PhysObj::getOrbitalPeriod(PhysObj& referenceObject) {
    double a = getSemiMajorAxis(referenceObject);
    if (a < 0) return 0.0;
    double M = referenceObject.mass;
    return 2.0*PI*sqrt(pow(a, 3.0)/(G*M));
}

double PhysObj::getSemiMajorAxis(PhysObj& referenceObject) {
    // v^2 = GM(2/r - 1/a)
    // v^2 = 2GM/r - GM/a
    // GM/a = 2GM/r - v^2
    // 1/a = 2/r - v^2/GM
    // a = 1/(2/r - v^2/GM)
    // a = r/2 - GM/v^2

    double v2 = (velocity - referenceObject.velocity).lengthSquared();
    double r = (position - referenceObject.position).length();
    double M = referenceObject.mass;
    double semiMajorAxis = 1.0/(2.0/r - v2/(G*M));
    return semiMajorAxis;
}

#pragma endregion

// Drawing -----------------------------------------------------------------------------------------
#pragma region Visualization

std::vector<Vector2d> PhysicsObject::getOrbitalPath(double duration, int resolutionScale) {
    int resolution = duration*resolutionScale;
    double timeDelta = 1.0/resolutionScale;
    Vector2d initialPosition = position;
    Vector2d initialVelocity = velocity;
    std::vector<Vector2d> orbitalPath;
    if (parentObject) {
        velocity -= parentObject->velocity;
    }
    
    for (int i=0; i<resolution; i++) {  
        if (parentObject) {
            Vector2d gravityVector = getGravityVector(*parentObject)*timeDelta;
            velocity += gravityVector;
            if (checkCollision(*parentObject, timeDelta)) {
                velocity += parentObject->velocity;
                velocity += getCollisionImpulse(*parentObject)/mass - gravityVector;
                velocity -= parentObject->velocity;
            }
        }      

        position += velocity*timeDelta;
        orbitalPath.push_back(position - initialPosition);
    }
    position = initialPosition;
    velocity = initialVelocity;
    return orbitalPath;
}

void PhysObj::drawOrbitalPath(sf::RenderWindow& window, int resolutionScale) {
    if (!parentObject) return;
    double period = getOrbitalPeriod(*parentObject);
    if (period <= 0.0) {
        period = 5.0; // Arbitrary path length for objects not in orbit
    }
    period = std::min(period, 360.0);
    resolutionScale /= (sqrt(period)/2.0);
    drawOrbitalPath(window, period, resolutionScale);
}

void PhysObj::drawOrbitalPath(sf::RenderWindow& window, double duration, int resolutionScale) {
    std::vector<Vector2d> orbitalPath = getOrbitalPath(duration, resolutionScale);
    int pathStart = 0;
    int pathEnd = orbitalPath.size();
    // if (!isGrounded) {
    //     for (auto& point : orbitalPath) {
    //         if (point.length() < radius) {
    //             pathStart += 1;
    //         } else break;
    //     }
    //     for (int i=pathEnd; i>pathStart;i--) {
    //         if (orbitalPath[i].length() >= radius) {
    //             pathEnd = i;
    //             break;
    //         }
    //     }
    // }
    int vertexCount = pathEnd - pathStart;
    bool isFullOrbit = false;
    if (orbitalPath[pathEnd-1].length() < radius) {
        vertexCount += 1;
        isFullOrbit = true;
    }
    if (vertexCount < 2) return;
    sf::VertexArray pathLine = sf::VertexArray(sf::PrimitiveType::LineStrip, vertexCount);
    Vector2d center = static_cast<Vector2d>(window.getSize())/2.0;

    sf::Color lineColor = sf::Color(150,150,255);

    for (int i=pathStart; i<pathEnd; i++) {
        pathLine[i - pathStart].position = static_cast<sf::Vector2f>(position + orbitalPath[i] + center);
        pathLine[i - pathStart].color = lineColor;
    }
    if (isFullOrbit) {
        pathLine[vertexCount-1].position = static_cast<sf::Vector2f>(position + orbitalPath[pathStart] + center);
        pathLine[vertexCount-1].color = lineColor;
    }


    // if (!isGrounded) {
    //     pathLine[0].position = static_cast<sf::Vector2f>(position + (orbitalPath[0].normalized()*radius) + center);
    //     pathLine[0].color = lineColor;
    //     if (pathEnd == orbitalPath.size()-1) {
    //         pathLine[vertexCount-1].position = static_cast<sf::Vector2f>(position + orbitalPath[pathEnd-1] + center);            
    //     } else {
    //         pathLine[vertexCount-1].position = static_cast<sf::Vector2f>(position + (orbitalPath[pathEnd-1].normalized()*radius) + center);
    //     }
    //     pathLine[vertexCount-1].color = lineColor;
        
    // }

    window.draw(pathLine);
}

#pragma endregion

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
