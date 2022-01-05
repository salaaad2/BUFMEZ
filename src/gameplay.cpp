/*********************************/
/*   THRESHOLD        (  //      */
/*   gameplay          ( )/      */
/*   by salade         )(/       */
/*  ________________  ( /)       */
/* ()__)____________)))))   :^}  */
/*********************************/

#include "gameplay.hpp"

#include "raymath.h"
#include <fstream>

#include "weapon.hpp"

Game::Game(std::string const & path)
{
    std::ifstream ifs(path);
    std::string tok;
    auto radius = 10;

    std::cout << "Init: reading map file [" << path << "]" << std::endl;
    while (ifs >> tok)
    {
        if (tok == "ENEMIES")
        {
            ifs >> tok;
            std::cout << "will spawn " << tok << " enemies";
            nEnemies = std::atoi(tok.c_str());
            ifs >> tok;
            radius = std::atoi(tok.c_str());
        }
        if (tok == "NEXT")
        {
            ifs >> tok;
            next = tok;
            std::cout << "next level is " << next;
        }
    }
    ifs.close();
    enemies = new std::vector<Entity>(nEnemies);
    for (auto en = enemies->begin(); en != enemies->end(); en++) {
        en->radius = radius;
    }
    player = new Entity;
    player->posX = 0;
    player->posY = SCREENHEIGHT / 2;
    player->direction.x = 100;
    player->direction.y = 100;
    player->radius = 10;
    player->victims = 0;
    player->fury = 0;
    player->wp = new Weapon(10, 10,
            "../meta/media/mp3/shotty_shoot.mp3",
            "../meta/media/mp3/shotty_reload.mp3");
}

Game::~Game()
{
    delete enemies;
    delete player;
}

void Game::start()
{
    player->tex = LoadTexture("../meta/media/sprites/cowboy_idle.png");
    std::cout << "----- Gameplay: Start -----" << std::endl;
    std::cout << "Gameplay: " << nEnemies << "enemies need to be spawned" << std::endl;
    frameWidth = player->tex.width;
    frameHeight = player->tex.height;

    sourceRec = { 0.0f, 0.0f, (float)frameWidth, (float)frameHeight };

    origin = { (float)frameWidth, (float)frameHeight };
}

// draw bad boys and player
void Game::draw() const
{
    auto left = std::to_string(enemies->size());
    ClearBackground(COOLPURPLE);
    for (auto & en : *enemies)
    {
        DrawCircleV((Vector2){en.posX, en.posY}, en.radius, DARKBLUE);
    }
    // Destination rectangle (screen rectangle where drawing part of texture)
    Rectangle destRec = { player->posX, player->posY, frameWidth * 1.4f, frameHeight * 1.4f };

    // Origin of the texture (rotation/scale point), it's relative to destination rectangle size

    DrawTexturePro(player->tex, sourceRec, destRec, origin, Vector2Angle((Vector2){0.0f, 0.0f}, player->direction), WHITE);


    DrawText("Enemies left : ", 10, 10, 20, GREEN);
    DrawText(left.c_str(), 150, 10, 20, RED);
    if (player->fury >= 5) {
        DrawText("[E] FURY", SCREENWIDTH - 300, 10, 50, RED);
    }
}


// progress the game & check for player death
// NEW: go towards player NEXT: spawn at different times
int Game::tick() const
{
    for (auto  en = enemies->begin(); en != enemies->end(); en++)
    {
        if (en->hp != 0)
      {
        if (en->posX >= SCREENWIDTH || en->posX <= 0) {
          en->direction.x = -en->direction.x;
        }
        if (en->posY >= SCREENHEIGHT || en->posY <= 0) {
          en->direction.y = -en->direction.y;
        }
        if (en->posX >= player->posX) {
          en->direction.x -= 0.1f;
        }
        if (en->posY >= player->posY) {
          en->direction.y -= 0.1f;
        }
        if (en->posX <= player->posX) {
          en->direction.x += 0.1f;
        }
        if (en->posY <= player->posY) {
          en->direction.y += 0.1f;
        }
      } else {
            if (en->posX >= SCREENWIDTH || en->posX <= 0 || en->posY >= SCREENHEIGHT ) {
              enemies->erase(en);
              return (0);
            }
        }

      en->posX += en->direction.x;
      en->posY += en->direction.y;
      if (en->hp != 0 && // check for player death (one shot one kill)
          CheckCollisionCircles((Vector2){player->posX, player->posY}, 10,
                                (Vector2){en->posX, en->posY}, 10)) {
        return (1);
        }
    }
    return (0);
}

int Game::getKeys() const
{
    auto oldX = 0, oldY = 0; // get position before processing keys to check for player movement
                             // in threshold mode
    auto aimer = player->direction; // duplicate player direction before making changes to it. this becomes the reticle

    oldX = player->posX;
    oldY = player->posY;
    if (IsKeyDown(KEY_W)) {
        player->posX += 0;
        player->posY += -4;
    }
    if (IsKeyDown(KEY_S)) {
        player->posX += 0;
        player->posY += 4;
    }
    if (IsKeyDown(KEY_A)) {
        player->posX += -4;
        player->posY += 0;
    }
    if (IsKeyDown(KEY_D)) {
        player->posX += 4;
        player->posY += 0;
    }
    if (player->fury >= 5 &&
        IsKeyDown(KEY_E)) {
        player->time = GetTime();

        player->threshold = true;
        player->fury = 0;
    }
    if (IsKeyDown(KEY_LEFT)) {
        player->direction = Vector2Rotate(player->direction, -0.1f); // left
    }
    if (IsKeyDown(KEY_RIGHT)) {
        player->direction = Vector2Rotate(player->direction, 0.1f);
    }
    if (IsKeyPressed(KEY_SPACE)) {
        if (shoot()) {
            return (0);
        }
        if (player->victims == nEnemies) {
            return (2);
        }
        std::cout << player->victims << "|" << nEnemies << std::endl;
    }
    if (player->threshold)
    {
        if (GetTime() >= (player->time + 5))
        {
            player->fury = 0;
            player->threshold = false;
        }
        if (oldX != player->posX ||
            oldY != player->posY)
        {
            if (this->tick()) {
                return (1);
            }
        }
    } else {
        if (this->tick()) {
            return (1);
        }
    }
    aimer.x = (player->direction.x / 3);
    aimer.y = (player->direction.y / 3);
    DrawLineEx((Vector2){player->posX, player->posY}, Vector2Add((Vector2){player->posX, player->posY}, aimer), 5, GREEN);
    return (0);
}


int
Game::shoot() const
{
        auto rot1 = Vector2Rotate(player->direction, -0.2f);
        auto rot2 = Vector2Rotate(player->direction, 0.2f);

        auto add1 = Vector2Add((Vector2){player->posX, player->posY}, rot1);
        auto add2 = Vector2Add((Vector2){player->posX, player->posY}, rot2);

        if (player->wp->bang() == 1) {
            return (0);
        } else {
            player->wp->bang();
        }
        for (auto en = enemies->begin(); en != enemies->end(); en++)
        {
            if (CheckCollisionPointLine((Vector2){en->posX, en->posY}, (Vector2){player->posX, player->posY}, add1, (en->radius * 2)) ||
                CheckCollisionPointLine((Vector2){en->posX, en->posY}, (Vector2){player->posX, player->posY}, Vector2Add((Vector2){player->posX, player->posY}, player->direction), (en->radius * 2)) ||
                CheckCollisionPointLine((Vector2){en->posX, en->posY}, (Vector2){player->posX, player->posY}, add2, (en->radius * 2)))
            {
              std::cout << "hit enemy at " << en->posX << "|" << en->posY
                        << std::endl;
              en->hp = 0;
              en->direction.x = (player->direction.x / 2);
              en->direction.y = (player->direction.y / 2);
              // enemies->erase(en);
              player->victims++;
              player->fury++;
              DrawLineEx((Vector2){player->posX, player->posY}, add1, 10,
                         ORANGE);
              DrawLineEx((Vector2){player->posX, player->posY},
                         Vector2Add((Vector2){player->posX, player->posY},
                                    player->direction),
                         10, ORANGE);
              DrawLineEx((Vector2){player->posX, player->posY}, add2, 10,
                         ORANGE);
              return (1);
            }
        }
        DrawLineEx((Vector2){player->posX, player->posY}, add1, 10, ORANGE);
        DrawLineEx((Vector2){player->posX, player->posY},
                   Vector2Add((Vector2){player->posX, player->posY},
                              player->direction),
                   10, ORANGE);
        DrawLineEx((Vector2){player->posX, player->posY}, add2, 10, ORANGE);
        return (0);
}

std::string const &
Game::getNext() const
{return next;}
