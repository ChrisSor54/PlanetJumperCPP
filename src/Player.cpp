#include "includes.h"

#include "Player.h"

using pl = Player;


pl::Player() 
    : PhysObj(Vector2d(0, 0), Vector2d(0,0), PLAYER_MASS, COLLISION_RADIUS, 0), sprite(sf::Sprite(spriteTexture.getTexture())) {
    elasticity = 0;
    animSpeed = 0.25;

    sf::Color playerColor(255, 0, 150);

    // Initialize textures and sprite
    sf::Texture baseTexture, maskTexture;
    if (!baseTexture.loadFromFile("assets/astronaut.png")) {
        throw std::invalid_argument("Bad player texture");
    }
    
    if (!maskTexture.loadFromFile("assets/astronaut_body_mask.png")) {
        throw std::invalid_argument("Bad player mask");
    }
    spriteTexture = sf::RenderTexture(baseTexture.getSize());
    spriteTexture.clear(sf::Color::Transparent);

    sprite.setTexture(baseTexture, true);
    sprite.setColor(sf::Color::White);
    spriteTexture.draw(sprite);

    sprite.setTexture(maskTexture, true);
    sprite.setColor(playerColor);
    spriteTexture.draw(sprite);
    spriteTexture.display();

    sprite.setColor(sf::Color::White);
    sprite.setTexture(spriteTexture.getTexture(), true);

    playAnimation(Anim::WALKING);
    //sprite.setPosition(static_cast<sf::Vector2f>(position));
    sprite.setOrigin(sf::Vector2f(SPRITE_WIDTH/2.f, SPRITE_WIDTH/2.f));
    //sprite.setColor(sf::Color(255, 0, 180));
    

}


void pl::update(double dt) {
    switch (state) {

    }



    if (parentObject) {
        rotation = (position-parentObject->position).angle();
    }
    updateAnimation(dt);
}

void pl::playAnimation(Anim anim) {
    int row = animations[anim].row;
    int column = animations[anim].column;

    sprite.setTextureRect(sf::IntRect(
        sf::Vector2i(column*SPRITE_WIDTH, row*SPRITE_WIDTH),
        sf::Vector2i(SPRITE_WIDTH, SPRITE_WIDTH)
    ));
    currentAnimation = anim;
    animationTimer = 0;
}

void pl::updateAnimation(float dt) {
    float previousTimer = animationTimer;
    animationTimer += animations[currentAnimation].animSpeed*dt;
    int animLength = animations[currentAnimation].numSprites;

    if (floor(previousTimer) != floor(animationTimer)) {
        animationTimer = fmod(animationTimer, animLength);
        int spriteIndex = floor(animationTimer);
        int row = animations[currentAnimation].row;
        int column = animations[currentAnimation].column;

        sprite.setTextureRect(sf::IntRect(
            sf::Vector2i((column + spriteIndex)*SPRITE_WIDTH, row*SPRITE_WIDTH),
            sf::Vector2i(SPRITE_WIDTH, SPRITE_WIDTH)
        ));
    }

}


void pl::draw(sf::RenderWindow& window, float distanceScale) {
    sf::Vector2f scaledPosition = static_cast<sf::Vector2f>(position)/distanceScale;
    sf::Vector2f center = static_cast<sf::Vector2f>(window.getSize())/2.f;
    sprite.setScale(sf::Vector2f(1.f/distanceScale, 1.f/distanceScale));
    // sf::CircleShape shape = sf::CircleShape();
    // shape.setRadius(COLLISION_RADIUS/distanceScale);
    // shape.setOrigin(sf::Vector2f(COLLISION_RADIUS/distanceScale, COLLISION_RADIUS/distanceScale));
    // shape.setPosition(scaledPosition+center);
    // window.draw(shape);
    sprite.setPosition(scaledPosition + center);
    sprite.setRotation(rotation.wrapUnsigned() + sf::degrees(90.f));
    window.draw(sprite);
}
