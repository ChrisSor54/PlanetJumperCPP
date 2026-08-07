#include "PhysicsObject.h"

//--------------------------------------------------------------------------------------------------
// PhysicsObject
//--------------------------------------------------------------------------------------------------


// Constructors
PhysObj::PhysicsObject(Vector2d position, Vector2d velocity, double mass, double radius, sf::Angle rotationalVelocity, float surfaceFriction, MatterState state)
    : position(position), velocity(velocity), mass(mass), radius(radius), rotationalVelocity(rotationalVelocity), surfaceFriction(surfaceFriction), matterState(state) {
    
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
    if (surfaceDist <= GROUNDED_MARGIN) {
        //std::cout << "Gravity" << std::endl;
        hasCollided = true;
        if (other.mass >= mass && other.matterState == MatterState::SOLID) {
            parentObject = &other;
            isGrounded = true;
        }
    }
    
    if (other.mass >= 2*mass) {
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
        if (matterState == MatterState::SOLID && other.matterState == MatterState::SOLID && collisionNormal.lengthSquared() <= restThreshold) {
            double surfaceDistance = getSurfaceDistance(other);
            if (other.mass >= 2*mass) {
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
    if (matterState != MatterState::SOLID || other.matterState != MatterState::SOLID && !force) return;
    double surfaceDist = getSurfaceDistance(other);
    if (surfaceDist <= GROUNDED_MARGIN || force) {
        //std::cout << "Overlap" << std::endl;
        if ((position - other.position).lengthSquared() == 0) {
            position.y -= 1.0;
        }
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


Vector2d PhysObj::calculateGravityVector(Vector2d position1, double mass1, Vector2d position2, double mass2) {
    Vector2d normal = (position2 - position1).normalized();
    double distanceFactor = normal.lengthSquared();
    return normal.normalized() * G*mass1*mass2/distanceFactor;
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
    if (vNormal < 0) return Vector2d(0,0);
    double avgElasticity = sqrt(elasticity*other.elasticity);
    Vector2d collisionImpulse = -(vNormal*(1 + avgElasticity)/(1/mass + 1/other.mass)) * normal;
    if (matterState != MatterState::SOLID || other.matterState != MatterState::SOLID || collisionImpulse.lengthSquared() <= sqrRestThreshold) {
        collisionImpulse = -(vNormal)/(1/mass + 1/other.mass) * normal; // Nullify soft collisions
    }
    double jNormal = collisionImpulse.dot(normal);
    double totalSurfaceVelocity = getSurfaceVelocity() + other.getSurfaceVelocity();
    relativeVelocity -= normal.rotatedBy(sf::degrees(-90)) * totalSurfaceVelocity;
    Vector2d vTangentVel = relativeVelocity - normal * relativeVelocity.dot(normal);
    double tangentSpeed = vTangentVel.length();

    Vector2d frictionImpulse(0, 0);
    if (matterState != MatterState::SOLID || other.matterState != MatterState::SOLID) {
        Vector2d dir = relativeVelocity.normalized();
        // double reducedMass = 1.0 / (1.0/mass + 1.0/other.mass);
        // double jTangentNeeded = tangentSpeed * reducedMass; // impulse to fully stop sliding
        // double combinedFriction = sqrt(surfaceFriction*other.surfaceFriction);
        // double jTangentMax = avgFriction * std::abs(jNormal); // Coulomb's law
        // double jFriction = std::min(jTangentNeeded, jTangentMax);
        collisionImpulse = Vector2d(0,0);
        frictionImpulse = -relativeVelocity * (double)other.surfaceFriction;
    } else if (tangentSpeed > 0) {
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
    Vector2d gravityVector;
    Vector2d normal = other.position - position;
    double distanceFactor = normal.lengthSquared();
    double sqrRadius = pow(other.radius, 2);
    if (distanceFactor >= sqrRadius) {
        if (distancePower != 2) {
            distanceFactor = pow(normal.length(), distancePower);
        }
        gravityVector = normal.normalized() * G*other.mass/distanceFactor;
    } else if (distanceFactor < sqrRadius) {
        gravityVector = normal.normalized() * G*other.mass*normal.length()/pow(other.radius, 3);
    } else {
        gravityVector = Vector2d(0, 0);

    }
    return gravityVector;
}

double PhysObj::getSurfaceVelocity() {
    return radius * rotationalVelocity.asRadians();
}

double PhysObj::getOrbitalPeriod(PhysObj& referenceObject) {
    double a = getSemiMajorAxis(referenceObject);
    if (a <= 0.0) return 0.0;
    double M = referenceObject.mass + mass;
    return 2.0*PI*sqrt((a*a*a)/(G*M));
}

double PhysObj::getSemiMajorAxis(PhysObj& referenceObject) {
    double v2 = (velocity - referenceObject.velocity).lengthSquared();
    double r = (position - referenceObject.position).length();
    double M = referenceObject.mass;
    double semiMajorAxis = 1.0/(2.0/r - v2/(G*M));
    return semiMajorAxis;
}

double PhysObj::getEscapeVelocity(double distance) {
    return sqrt((2*G*mass)/distance);
}

bool PhysObj::isOnEscapeTrajectory(PhysObj& other) {
    bool isEscaping = false;
    Vector2d rV = velocity - other.velocity;
    if (rV.lengthSquared() == 0) return isEscaping;
    Vector2d normal = (other.position - position);
    Vector2d tangent = normal.rotatedBy(sf::degrees(90));
    double tComponent = abs(rV.dot(tangent));
    double nComponent = rV.dot(normal);
    if (tComponent < nComponent && tComponent*nComponent > radius + other.radius) return isEscaping; // Colliision course
    isEscaping = rV.lengthSquared() >= (pow(other.getEscapeVelocity(normal.length()), 2))*.9;
    return isEscaping;
}

#pragma endregion

// Drawing -----------------------------------------------------------------------------------------
#pragma region Visualization

void PhysicsObject::updateOrbitalPath(int resolutionScale) {
    if (!parentObject) return;
    double period = getOrbitalPeriod(*parentObject);
    if (period <= 0.0 || isOnEscapeTrajectory(*parentObject)) {
        if (parentObject->parentObject) {
            period = getOrbitalPeriod(*(parentObject->parentObject));
        } else {
            period = 5.0; // Arbitrary path length for objects not in orbit
        }
    }
    updateOrbitalPath(period, resolutionScale);
}

void PhysicsObject::updateOrbitalPath(double duration, int resolutionScale) {
    if (parentObject) updateOrbitalPath(*parentObject, duration, resolutionScale);
    else orbitalPath.clear();
}

void PhysicsObject::updateOrbitalPath(PhysObj& referenceObject, double duration, int resolutionScale) {
    int resolution = resolutionScale;
    double timeDelta = duration/resolutionScale;
    Vector2d initialPosition = position;
    Vector2d initialVelocity = velocity;
    orbitalPath.clear();

    int numCollisions = 0;
    for (int i=0; i<resolution; i++) {  
        Vector2d gravityVector = getGravityVector(referenceObject)*timeDelta;
        velocity += gravityVector;
        if (checkCollision(referenceObject, timeDelta)) {
            velocity += getCollisionImpulse(referenceObject)/mass - gravityVector;
            numCollisions += 1;
        }

        position += (velocity - referenceObject.velocity)*timeDelta;
        Vector2d positionOffset = position - initialPosition; 
        //if (positionOffset.lengthSquared() < velocity.lengthSquared()*timeDelta) break;
        orbitalPath.push_back(positionOffset);
        if (numCollisions > 3) break;
    }
    position = initialPosition;
    velocity = initialVelocity;
}

void PhysObj::drawOrbitalPath(sf::RenderWindow& window) {
    int vertexCount = orbitalPath.size() + 1;;
    bool isFullOrbit = false;
    // if (orbitalPath.back().length() < radius) {
    //     //vertexCount += 1;
    //     isFullOrbit = true;
    // }
    if (vertexCount < 2) return;
    sf::VertexArray pathLine = sf::VertexArray(sf::PrimitiveType::LineStrip, vertexCount);

    sf::Color lineColor = sf::Color(150,150,255);

    pathLine[0].position = static_cast<sf::Vector2f>(position);
    pathLine[0].color = lineColor;
    for (int i=0; i<orbitalPath.size(); i++) {
        double completionPercent = (double)i/(double)orbitalPath.size();
        pathLine[i+1].position = static_cast<sf::Vector2f>(position + orbitalPath[i]);
        lineColor.a = std::min(255.0, 255*(1.0 - completionPercent) + 50.0);        
        pathLine[i+1].color = lineColor;
    }
    if (isFullOrbit) {
        //pathLine[vertexCount-1].position = static_cast<sf::Vector2f>(position + center);
        //pathLine[vertexCount-1].color = lineColor;
    }

    window.draw(pathLine);
}

void PhysObj::drawVelocity(sf::RenderWindow& window, Vector2d referenceVelocity, double scale) {
    Vector2d relativeVelocity = velocity - referenceVelocity;

    sf::Color lineColor = sf::Color(255,0,0);
    
    sf::VertexArray velocityLine(sf::PrimitiveType::Lines, 2); 
    velocityLine[0].position = static_cast<sf::Vector2f>(position);
    velocityLine[0].color = lineColor;
    velocityLine[1].position = static_cast<sf::Vector2f>(position + relativeVelocity*scale);
    velocityLine[1].color = lineColor;

    window.draw(velocityLine);
}


#pragma endregion

//--------------------------------------------------------------------------------------------------
// Body
//--------------------------------------------------------------------------------------------------

#pragma region Body Class

sf::Image Body::textureSheet;
bool Body::textureSheetLoaded = false;

Body::Body(Vector2d position, Vector2d velocity,
    double mass, double radius, sf::Angle rotationalVelocity, float surfaceFriction, MatterState state,
    sf::Color color, BodyTexture bodyTexture) 
    : Body(position, velocity, mass, radius, rotationalVelocity, surfaceFriction, state, color) {
        
    if (!Body::textureSheetLoaded) {
        if (!textureSheet.loadFromFile("assets/body_texture.png")) {
            throw std::invalid_argument("Bad body texture");
        }
        Body::textureSheetLoaded = true;
    }
    int xOrigin = bodyTexture.xOrigin;
    int yOrigin = bodyTexture.yOrigin;
    int width = bodyTexture.width;
    int height = bodyTexture.height;
    sf::Image tileTextureImage(sf::Vector2u(width, height));
    if (!tileTextureImage.copy(textureSheet, {0, 0}, sf::IntRect({xOrigin, yOrigin}, {width,height}))) {
        throw std::invalid_argument("Failed to build tile texture");
    }

    if (!texture.loadFromImage(tileTextureImage)) {
        throw std::invalid_argument("Failed to build tile texture");
    }
        
        
    int textureWidth = (int)(bodyTexture.width);
    if (!bodyTexture.repeat) {
        
    }
    texture.setRepeated(bodyTexture.repeat);
    shape.setTexture(&texture);
    shape.setTextureRect(sf::IntRect({0,0}, {textureWidth, textureWidth}));
}   

Body::Body(Vector2d position, Vector2d velocity,
    double mass, double radius, sf::Angle rotationalVelocity, float surfaceFriction, MatterState state,
    sf::Color color) 
    : PhysicsObject(position, velocity, mass, radius, rotationalVelocity, surfaceFriction, state), color(color)  {
    shape.setRadius(radius);
    shape.setOrigin(sf::Vector2f(radius, radius));
    shape.setPointCount(30 + (int) (radius/5));
    shape.setFillColor(color);
}

void Body::draw(sf::RenderWindow& window) {
    shape.setPosition(static_cast<sf::Vector2f>(position));
    shape.setRotation(rotation);
    window.draw(shape);
}

void Body::draw(sf::RenderWindow& window, Vector2f scale) {
    Vector2f currentScale = shape.getScale();
    shape.setScale(scale);
    draw(window);
    shape.setScale(currentScale);
}


#pragma endregion
