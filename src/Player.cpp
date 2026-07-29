#include "includes.h"

#include "Player.h"

using pl = Player;

pl::Player() 
    : PhysObj(Vector2d(0, 0), Vector2d(0,0), PLAYER_MASS, COLLISION_RADIUS), sprite(sf::Sprite(spriteTexture)) {
    elasticity = 0;
    animSpeed = 0.25;
    if (!spriteTexture.loadFromFile("assets/astronaut.png")) {
        throw std::invalid_argument("Bad player texture");
    }
    sprite.setTexture(spriteTexture);
    playAnimation(Anim::IDLE);
    //sprite.setPosition(static_cast<sf::Vector2f>(position));
    sprite.setOrigin(sf::Vector2f(SPRITE_WIDTH/2.f, SPRITE_WIDTH/2.f));


};


void pl::update(double dt) {
    if (parentObject) {
        rotation = (position-parentObject->position).angle();
    }
    std::cout << std::to_string(rotation.asDegrees()) << std::endl;
};

void pl::playAnimation(Anim anim) {
    int row = animations[anim].row;
    int column = animations[anim].column;

    sprite.setTextureRect(sf::IntRect(
        sf::Vector2i(column*SPRITE_WIDTH, row*SPRITE_WIDTH),
        sf::Vector2i(SPRITE_WIDTH, SPRITE_WIDTH)
    ));
};


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