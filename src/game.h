#ifndef GAME_H
#define GAME_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "game_level.h"
#include "colision.h"
#include "power_up.h"

// Represents the current state of the game
enum GameState {
    GAME_ACTIVE,
    GAME_MENU,
    GAME_WIN,
    GAME_LOSE,
    GAME_PAUSE,
    GAME_ATTRIBUTES
};

// Initial size
const glm::vec2 PLAYER_SIZE(100.0f, 20.0f);
// Initial velocity 
const float PLAYER_VELOCITY(500.0f);


class Game
{
public:
    GameState State;  
    bool Keys[1024];
    bool CursorEntered; 
    bool MouseButtons[2];
    double xPos;
    double yPos;
    float PaddleVelocity = 0;
    unsigned int Width, Height;
    std::vector<GameLevel> Levels;
    unsigned int Level;
    unsigned int Lives = 3;
    std::vector<PowerUp>  PowerUps;

    Game(unsigned int width, unsigned int height);
    ~Game();

     
    void Init();

    void LoadShaders();
    void LoadTextures();
    void LoadLevels();

    void ConfigureGameObjects();
    
    // game loop    
    void ProcessInput(float dt);
    void Update(float dt);
    void CheckDeath();
    void CheckWin();
    void Render();
    void DoCollisions();
    void HorizontalCollision(Direction dir, glm::vec2 diff_vector);
    void VerticalCollision(Direction dir, glm::vec2 diff_vector);
    void PaddleCollision();
    
    void ResetLevel();
    void ResetPlayer();

    void SpawnPowerUps(GameObject &block);
    void UpdatePowerUps(float dt);
    
    bool KeysProcessed[1024];
};

#endif

