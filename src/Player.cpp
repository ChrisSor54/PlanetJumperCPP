#include "Player.h"

using pl = Player;

sf::Texture pl::spritesheet;
sf::Texture pl::spriteMask;
bool pl::spritesheetLoaded = false;

pl::Player(int ID, sf::Color playerColor) 
    : PhysObj(Vector2d(0, 0), Vector2d(0,0), PLAYER_MASS, COLLISION_RADIUS, sf::degrees(0), PLAYER_FRICTION, MatterState::SOLID), sprite(sf::Sprite(spriteTexture.getTexture())) {
    playerID = ID;
    elasticity = PLAYER_ELASTICITY;
    id -= 4;

    position += Vector2d(playerID*2*COLLISION_RADIUS, 0);
    // Initialize textures and sprite
    if (!spritesheetLoaded) {
        if (!spritesheet.loadFromFile("assets/astronaut.png")) {
            throw std::invalid_argument("Bad player texture");
        }
        
        if (!spriteMask.loadFromFile("assets/astronaut_body_mask.png")) {
            throw std::invalid_argument("Bad player mask");
        }
        pl::spritesheetLoaded = true;
    }
    spriteTexture = sf::RenderTexture(spritesheet.getSize());
    spriteTexture.clear(sf::Color::Transparent);

    sprite.setTexture(spritesheet, true);
    sprite.setColor(sf::Color::White);
    spriteTexture.draw(sprite);

    sprite.setTexture(spriteMask, true);
    sprite.setColor(playerColor);
    spriteTexture.draw(sprite);
    spriteTexture.display();

    sprite.setColor(sf::Color::White);
    sprite.setTexture(spriteTexture.getTexture(), true);
    sprite.setOrigin(sf::Vector2f(SPRITE_WIDTH/2.f, SPRITE_WIDTH/2.f));

    sprite.setPosition(Vector2f(0,0));
    setState(State::FLYING);
    playAnimation(Anim::FLYING);
}

// Getters & Setters -------------------------------------------------------------------------------

State pl::getState() {
    return state;
}

void pl::setState(State newState) {
    if (state != newState) {
        state = newState;
        switch (state) {
            case (State::FLYING):
                playAnimation(Anim::FLYING);
                break;
        }
    }
}

// Updates -----------------------------------------------------------------------------------------
#pragma region Updates

void pl::update(double dt) {
    smokeSpawnCooldown -= dt;
    if (isGrounded) {
        setState(State::GROUNDED);
        rotationalVelocity = sf::degrees(0);
    } else {
        setState(State::FLYING);
        jumpCharge == 0.0;
    }

    switch (state) {

    }

    if (state == State::GROUNDED && parentObject) {
        rotation = (position-parentObject->position).angle();
    }
    updateAnimation(dt);
    //std::cout << std::to_string(static_cast<int>(state)) << std::endl;
}

void pl::updateGravity(PhysObj& other, double dt) {
    PhysObj::updateGravity(other, dt);
    // Instead of calculating gravity for each particle, just apply the player's gravity
    Vector2d gravityVector = getGravityVector(other)*dt;
    for (auto& particle : smokeArray) {
        if (particle.lifespan <= 0) continue;
        particle.vel += (sf::Vector2f) gravityVector;
    }
}

void pl::updateCollision(PhysObj& other, double dt) {
    if (checkCollision(other, dt)) {
        Vector2d normal = (other.position - position).normalized();
        double restThreshold = (COLLISION_REST_COEFFICIENT*getGravityVector(other)*mass).lengthSquared();
        Vector2d collisionImpulse = getCollisionImpulse(other, restThreshold);        
        Vector2d collisionNormal = normal * collisionImpulse.dot(normal);
        if (collisionNormal.lengthSquared() <= restThreshold) {
            double surfaceDistance = getSurfaceDistance(other);
            if (surfaceDistance > 0) {
                fixOverlap(other, true);
            }
            if (other.mass >= mass  && other.matterState == MatterState::SOLID) {
                parentObject = &other;
                isGrounded = true;
            }
        }
        applyImpulse(collisionImpulse);
        hasCollided = true;
    }
}

#pragma endregion

// Physics Overloads -------------------------------------------------------------------------------
#pragma region Physics

Vector2d pl::getCollisionImpulse(PhysObj& other) {
    double sqrRestThreshold = (COLLISION_REST_COEFFICIENT*getGravityVector(other)*mass).lengthSquared();
    return getCollisionImpulse(other, sqrRestThreshold);
}

Vector2d pl::getCollisionImpulse(PhysObj& other, double sqrRestThreshold) {
    sf::Angle rotVel = rotationalVelocity;
    rotationalVelocity += other.rotationalVelocity;
    Vector2d collisionImpulse = PhysObj::getCollisionImpulse(other, sqrRestThreshold);
    rotationalVelocity = rotVel;
    return collisionImpulse;
}


#pragma endregion

// Input & Actions ---------------------------------------------------------------------------------
#pragma region Input & Actions

void pl::handleInput(InputMap& input, double dt) {
    Vector2d dirInput = static_cast<Vector2d>(input.directionalInput);
    int xInput = input.inputStates[InputAction::Right].pressed - input.inputStates[InputAction::Left].pressed;
    if (xInput != 0)  {
        flipSprite = xInput < 0;
    }
    if (state == State::GROUNDED && parentObject) {
        if (input.inputStates[InputAction::Jump].pressed) {
            chargeJump(dt);
        } else if (jumpCharge > 0) {  
            jump();
        } else {
            Vector2d normal = (position - parentObject->position).normalized();
            Vector2d tangent = normal.rotatedBy(sf::degrees(90.f));
            double parentSurfaceVelocity = parentObject->rotationalVelocity.asRadians()*(radius+parentObject->radius);
            Vector2d relVelocity = velocity - (parentObject->velocity + tangent*(parentSurfaceVelocity));
            double moveSpeed = speed;
            double maxMoveSpeed = MOVE_SPEED;
            if (input.inputStates[InputAction::Walk].pressed) {
                moveSpeed /= 2.5;
                maxMoveSpeed /= 2.5;
            }
            Vector2d movementVector = tangent * dirInput.dot(tangent) * moveSpeed;
            Vector2d rvTangent = tangent * relVelocity.dot(tangent);
            if (dirInput.x != 0) {
                if ((rvTangent + movementVector).length() > maxMoveSpeed) {
                    double clampedSpeed = std::max(0.0, static_cast<double>(maxMoveSpeed - rvTangent.length()));
                    movementVector = movementVector.normalized() * clampedSpeed;
                }
                velocity += movementVector;
                float moveAnimSpeed = animations[Anim::WALKING].animSpeed * moveSpeed;
                playAnimation(Anim::WALKING, false, moveAnimSpeed);
            } else {
                playAnimation(Anim::IDLE);
            }
        }
    } else if (!input.inputStates[InputAction::DEBUG].pressed) {
        if (input.inputStates[InputAction::Walk].pressed && parentObject) {
            Vector2d retrograde = parentObject->velocity - velocity;
            if (retrograde.lengthSquared() > 1.0) {
                fly(retrograde.normalized(), dt);
            }
        } else {
            if (dirInput.lengthSquared() > 0) { // Flying
                fly(dirInput, dt);
            }
            // if (input.inputStates[InputAction::RotateR].pressed || input.inputStates[InputAction::RotateL].pressed) {
            //     //sf::Angle rotationSpeed = (input.inputStates[InputAction::RotateR].pressed) ? ROTATION_SPEED : -ROTATION_SPEED;
            //     //rotate(rotationSpeed, dt);
            // }
        }
    }
}

void pl::fly(Vector2d direction, double dt) {
    Vector2d thrustVector = (direction.normalized()*THRUSTER_STRENGTH)*dt;
    velocity += thrustVector;
    double rotationDifference = (thrustVector.angle() - rotation).wrapSigned().asDegrees();
    rotation = sf::degrees(rotation.asDegrees() + std::lerp(0.0, rotationDifference, 0.2));
    playAnimation(Anim::FLYING);
    if (smokeSpawnCooldown <= 0) {
        int numParticles = 1;
        if (dt/SMOKE_SPAWN_COOLDOWN > 1) {
            numParticles = (int) floor(dt/SMOKE_SPAWN_COOLDOWN);
        }
        for (int i=0; i<numParticles; i++) {
            sf::Angle angleOffset = sf::degrees(((double) 2*rand()/(double)RAND_MAX - 1.0)*SMOKE_ANGLE_OFFSET);
            float smokeVelocity = ((double) rand()/(double)RAND_MAX )*(MAX_SMOKE_VELOCITY-MIN_SMOKE_VELOCITY) + MIN_SMOKE_VELOCITY;
            spawnSmokeParticle(smokeVelocity, angleOffset, SMOKE_LIFESPAN);
        }
        smokeSpawnCooldown = SMOKE_SPAWN_COOLDOWN;
    }
    rotationalVelocity = sf::degrees(0);
}

void pl::rotate(sf::Angle rotationSpeed, double dt) {
    rotationalVelocity += rotationSpeed*dt;
    sf::Angle angleOffset = sf::degrees(((double) 2*rand()/(double)RAND_MAX - 1.0)*2*SMOKE_ANGLE_OFFSET) - (rotationSpeed);
    float smokeVelocity = ((double) rand()/(double)RAND_MAX )*(MAX_SMOKE_VELOCITY-MIN_SMOKE_VELOCITY) + MIN_SMOKE_VELOCITY;
    spawnSmokeParticle(smokeVelocity, angleOffset, SMOKE_LIFESPAN);
}


void pl::chargeJump(double dt) {
    jumpCharge += JUMP_CHARGE_SPEED*dt;
    jumpCharge = std::min(jumpCharge, 1.f);
    playAnimation(Anim::CHARGING);
    float randX = ((double) 2*rand()/(double)RAND_MAX - 1.0)*CHARGE_SPRITE_OFFSET*jumpCharge;
    float randY = ((double) 2*rand()/(double)RAND_MAX - 1.0)*CHARGE_SPRITE_OFFSET*jumpCharge;
    // std::cout << std::to_string(jumpCharge) << " | " << std::to_string(randX) << " , " << std::to_string(randY) << std::endl;
    sf::Vector2f chargeSpriteOffset = sf::Vector2f(randX, randY);
    sprite.setOrigin(SPRITE_ORIGIN + chargeSpriteOffset);
    if (jumpCharge == 1.0) {
        for (int i=0; i<4; i++) {
            float angleOffset = ((double) rand()/(double)RAND_MAX)*15.f;
            float thrust = ((double) rand()/(double)RAND_MAX)*THRUSTER_STRENGTH/15;
            spawnSmokeParticle(thrust, sf::degrees(90 - angleOffset), SMOKE_LIFESPAN/4);
            spawnSmokeParticle(thrust, sf::degrees(-90 + angleOffset), SMOKE_LIFESPAN/4);
        }
    }
    // int colorOffset = (int) (255.0*jumpCharge/5);
    // sf::Color chargeColor = (jumpCharge == 100.0) ? sf::Color(255,100,100) : sf::Color(255, 255 - colorOffset, 255 - colorOffset);
    // sprite.setColor(chargeColor);
}


void pl::jump() {
    if (state !=State::GROUNDED || !parentObject) {
        return;
    }
    Vector2d normal = (position - parentObject->position).normalized();
    double jumpChargeValue = (MAX_JUMP_CHARGE-MIN_JUMP_CHARGE)*jumpCharge + MIN_JUMP_CHARGE;
    Vector2d jumpImpulse = normal*jumpChargeValue;
    applyImpulse(jumpImpulse);
    parentObject->applyImpulse(-jumpImpulse);
    for (int i=0; i<50; i++) {
        float angleOffset = ((double) rand()/(double)RAND_MAX)*10.f;
        float thrust = -((double) rand()/(double)RAND_MAX)*THRUSTER_STRENGTH/6*jumpCharge;
        spawnSmokeParticle(thrust, sf::degrees(angleOffset), SMOKE_LIFESPAN);
    }
    jumpCharge = 0.0;
}

#pragma endregion

// Animations & Drawing ----------------------------------------------------------------------------
#pragma region Animations & Drawing

void pl::playAnimation(Anim anim) {
    playAnimation(anim, false, animations[anim].animSpeed);
}

void pl::playAnimation(Anim anim, bool force) {
    playAnimation(anim, force, animations[anim].animSpeed);
}

void pl::playAnimation(Anim anim, bool force, float customSpeed) {
    if (currentAnimation == anim  && !force && customSpeed == animSpeed ) {
        return;
    }
    if (currentAnimation != anim || force) {
        animationTimer = 0;
    }
    currentAnimation = anim;
    animSpeed = customSpeed;
    
}

void pl::updateAnimation(float dt) {
    float previousTimer = animationTimer;
    animationTimer += animSpeed*dt;
    int animLength = animations[currentAnimation].numSprites;
    int spriteIndex = floor(animationTimer);
    int row = animations[currentAnimation].row;
    int column = animations[currentAnimation].column;

    if (floor(previousTimer) != floor(animationTimer)) {
        animationTimer = fmod(animationTimer, animLength);
        spriteIndex = floor(animationTimer);
        row = animations[currentAnimation].row;
        column = animations[currentAnimation].column;
    }

    sprite.setTextureRect(sf::IntRect(
        sf::Vector2i((column + spriteIndex)*SPRITE_WIDTH, row*SPRITE_WIDTH),
        sf::Vector2i(SPRITE_WIDTH, SPRITE_WIDTH)
    ));

}

void pl::draw(sf::RenderWindow& window) {
    // sf::Vector2f center = static_cast<sf::Vector2f>(window.getSize())/2.f;
    int flip = (flipSprite) ? -1.f : 1.f;
    if (jumpCharge == 0) {
        sprite.setOrigin(SPRITE_ORIGIN);
        sprite.setColor(sf::Color(255,255,255));
    }
    sprite.setScale(sf::Vector2f(flip, 1.f));
    sprite.setPosition(static_cast<Vector2f>(position));
    sprite.setRotation(rotation.wrapUnsigned() + sf::degrees(90.f));
    window.draw(sprite);
}

void pl::draw(sf::RenderWindow& window, Vector2f scale) {
    Vector2f currentScale = sprite.getScale();
    sprite.setScale(scale);
    draw(window);
    sprite.setScale(currentScale);
}

#pragma endregion

// Smoke -------------------------------------------------------------------------------------------
#pragma region Smoke

void pl::spawnSmokeParticle(double smokeVelocity, sf::Angle angleOffset, double lifespan) {
    Vector2d dir = Vector2d(-(COLLISION_RADIUS+SMOKE_RADIUS+1),0).rotatedBy(rotation + angleOffset);
    Vector2d velocityVector = dir*smokeVelocity + velocity;
    float flip = (flipSprite) ? -1 : 1;
    sf::Vector2f smokePosition(-COLLISION_RADIUS/1.75, -flip*COLLISION_RADIUS/2.5);
    smokePosition= (sf::Vector2f) position + smokePosition.rotatedBy(rotation);
    smokeIndex = (smokeIndex + 1) % MAX_SMOKE;
    smokeArray[smokeIndex] = { smokePosition , (sf::Vector2f) velocityVector, (float) lifespan, (float) lifespan};
}

void pl::updateSmoke(double dt) {
    for (int i=0;i<MAX_SMOKE;i++) {
        if (smokeArray[i].lifespan > 0) {
            smokeArray[i].pos += smokeArray[i].vel*(float)dt;
            smokeArray[i].lifespan -= dt;
        }
    }
}


void pl::drawSmoke(sf::RenderWindow& window) {
    sf::CircleShape smokeShape;
    smokeShape.setPointCount(10);
    float size = SMOKE_RADIUS;
    smokeShape.setRadius(size);
    
    smokeShape.setOrigin(sf::Vector2f(size, size));

    for (const auto& particle : smokeArray) {
        if (particle.lifespan <= 0) continue;

        float alpha = (particle.lifespan / particle.totalLifespan) * 255.f;
        alpha = std::min(alpha, 255.f);
        sf::Color color(255, 255, 255, static_cast<int>(alpha));
        smokeShape.setFillColor(color);
        smokeShape.setPosition(static_cast<sf::Vector2f>(particle.pos));
        window.draw(smokeShape);
    }
}

#pragma endregion
