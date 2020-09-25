#include "game.h"
#include "resource_manager.h"
#include "sprite_renderer.h"
#include "game_object.h"
#include "ball_object.h"
#include "game_level.h"
#include "colision.h"
#include "particle_generator.h"
#include "text_renderer.h"

#include <sstream>
#include <iostream>


SpriteRenderer *Renderer;
GameObject *Player;
ParticleGenerator *Particles;
TextRenderer *Text;

const glm::vec2 INITIAL_BALL_VELOCITY(100.0f, -350.0f);
const float BALL_RADIUS = 12.5f;

BallObject *Ball;

Game::Game(unsigned int width, unsigned int height) 
    : State(GAME_MENU), Keys(), Width(width), Height(height)
{ 

}

Game::~Game()
{
    delete Renderer;
    delete Player;
}

void Game::Init()
{
    this->LoadShaders();
    this->LoadTextures(); 
    this->LoadLevels();
    this->ConfigureGameObjects();
}

void Game::LoadShaders()
{
    ResourceManager::LoadShader("shaders/sprite.vs", "shaders/sprite.fs", nullptr, "sprite");
    ResourceManager::LoadShader("shaders/particle.vs", "shaders/particle.fs", nullptr, "particle");
    
    // configure shaders
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(this->Width), 
        static_cast<float>(this->Height), 0.0f, -1.0f, 1.0f);
    ResourceManager::GetShader("sprite").Use().SetInteger("image", 0);
    ResourceManager::GetShader("sprite").SetMatrix4("projection", projection);
    ResourceManager::GetShader("particle").Use().SetMatrix4("projection", projection);
    
    // set render-specific controls
    Shader mySprite = ResourceManager::GetShader("sprite");
    Renderer = new SpriteRenderer(mySprite);
}

void Game::LoadTextures()
{
    ResourceManager::LoadTexture("textures/starry_background.jpg", false, "background");
    ResourceManager::LoadTexture("textures/ball.png", true, "face");
    ResourceManager::LoadTexture("textures/brick.png", false, "block");
    ResourceManager::LoadTexture("textures/brick_solid.png", false, "block_solid");
    ResourceManager::LoadTexture("textures/player_paddle.png", true, "paddle");
    ResourceManager::LoadTexture("textures/circle_particle.png", true, "particle");
}

void Game::LoadLevels()
{
    GameLevel one; one.Load("levels/one.lvl", this->Width, this->Height / 2);
    GameLevel two; two.Load("levels/two.lvl", this->Width, this->Height / 2);
    GameLevel three; three.Load("levels/three.lvl", this->Width, this->Height / 2);
    GameLevel four; four.Load("levels/four.lvl", this->Width, this->Height / 2);
    this->Levels.push_back(one);
    this->Levels.push_back(two);
    this->Levels.push_back(three);
    this->Levels.push_back(four);
    this->Level = 0;
}

void Game::ConfigureGameObjects()
{
    // Player
    glm::vec2 playerPos = glm::vec2(this->Width / 2.0f - PLAYER_SIZE.x / 2.0f, this->Height - PLAYER_SIZE.y);
    Texture2D myPaddle = ResourceManager::GetTexture("paddle");
    Player = new GameObject(playerPos, PLAYER_SIZE, myPaddle);
    
    // Ball
    glm::vec2 ballPos = playerPos + glm::vec2(PLAYER_SIZE.x / 2.0f - BALL_RADIUS, -BALL_RADIUS * 2.0f);
    Texture2D myFace = ResourceManager::GetTexture("face");
    Ball = new BallObject(ballPos, BALL_RADIUS, INITIAL_BALL_VELOCITY, myFace);
    
    //Particle Effect
    Particles = new ParticleGenerator(
        ResourceManager::GetShader("particle"), 
        ResourceManager::GetTexture("particle"), 
        500
    );

    // Text
    Text = new TextRenderer(this->Width, this->Height);
    Text->Load("fonts/VCR_OSD_MONO.ttf", 24);

}

void Game::Update(float dt)
{
    Ball->Move(dt, this->Width);
    
    this->DoCollisions();
    
    Particles->Update(dt, *Ball, 2, glm::vec2(Ball->Radius / 2.0f));    

    this->CheckDeath();
    this->CheckWin();
}  

void Game::CheckDeath()
{
    if (Ball->Position.y >= this->Height) // did ball reach bottom edge?
    {
        --this->Lives;
        if(this->Lives <= 0){
            this->ResetLevel();
            this->State = GAME_LOSE;
        }
        this->ResetPlayer();
    }
}

void Game::CheckWin()
{
  if(this->State == GAME_ACTIVE && this->Levels[this->Level].IsCompleted())
    {
        this->ResetLevel();
        this->ResetPlayer();
        this->State = GAME_WIN;
    }
}

void Game::ProcessInput(float dt)
{
    if (this->State == GAME_ACTIVE)
    {
        this->PaddleVelocity = 0;

        if(this->xPos >= 0.0f && this->xPos <= this->Width/2 
            && Player->Position.x >= 0.0f) 
        {
            this->PaddleVelocity = (this->Width/2 - this->xPos)/(this->Width/15);
            Player->Position.x -=  this->PaddleVelocity;
            if (Ball->Stuck)
                Ball->Position.x -= this->PaddleVelocity + dt;
        }
        else if(this->xPos <= this->Width && this->xPos > this->Width/2 
                && Player->Position.x <= this->Width - Player->Size.x)
        {
            this->PaddleVelocity = (this->xPos - this->Width/2)/(this->Width/15);
            Player->Position.x += this->PaddleVelocity;
            if (Ball->Stuck)
                Ball->Position.x += this->PaddleVelocity + dt;
        }

        if (this->Keys[GLFW_KEY_SPACE])
            Ball->Stuck = false;
        if (this->Keys[GLFW_KEY_R])
            this->ResetLevel();
        if(this->MouseButtons[GLFW_MOUSE_BUTTON_LEFT])
            this->State = GAME_PAUSE;
        if(this->MouseButtons[GLFW_MOUSE_BUTTON_RIGHT])
            this->State = GAME_ATTRIBUTES;
    }
    
    if (this->State == GAME_MENU)
    {
        if (this->Keys[GLFW_KEY_ENTER] && !this->KeysProcessed[GLFW_KEY_ENTER])
        {
            this->State = GAME_ACTIVE;
            this->KeysProcessed[GLFW_KEY_ENTER] = true;
        }
        if (this->Keys[GLFW_KEY_W] && !this->KeysProcessed[GLFW_KEY_W])
        {
            this->Level = (this->Level + 1) % 4;
            this->KeysProcessed[GLFW_KEY_W] = true;
        }
        if (this->Keys[GLFW_KEY_S] && !this->KeysProcessed[GLFW_KEY_S])
        {
            if (this->Level > 0)
                --this->Level;
            else
                this->Level = 3;
            this->KeysProcessed[GLFW_KEY_S] = true;
        }
    }
    
    if(this->State == GAME_WIN || this->State == GAME_LOSE)
    {
        if(this->Keys[GLFW_KEY_ENTER])
            this->State = GAME_MENU;
    }

    if(this->State == GAME_PAUSE)
    {   
        Ball->Stuck = true;
        if(!this->MouseButtons[GLFW_MOUSE_BUTTON_LEFT])
        {   
            this->State = GAME_ACTIVE;
            Ball->Stuck = false;
        }

    }
    if(this->State == GAME_ATTRIBUTES)
    {   
        Ball->Stuck = true;
        if(!this->MouseButtons[GLFW_MOUSE_BUTTON_RIGHT])
        {
            this->State = GAME_ACTIVE;
            Ball->Stuck = false;
        }
    }
}

void Game::Render()
{
    if(this->State == GAME_ACTIVE || this->State == GAME_MENU || this->State == GAME_PAUSE || this->State == GAME_WIN || this->State == GAME_LOSE)
    {
        Texture2D myBackground = ResourceManager::GetTexture("background");
        
        Renderer->DrawSprite(myBackground, glm::vec2(0.0f, 0.0f), glm::vec2(this->Width, this->Height), 0.0f);
        
        this->Levels[this->Level].Draw(*Renderer);
        
        Player->Draw(*Renderer);
        	
        Particles->Draw();
        
        Ball->Draw(*Renderer);

        int bricksDestroyed = 0;

        for (GameObject &tile : this->Levels[this->Level].Bricks)
            if(tile.Destroyed)
                bricksDestroyed += 1;

        std::stringstream balls; balls << this->Lives;
        std::stringstream bricks; bricks << bricksDestroyed;

        Text->RenderText("Balls:" + balls.str(), 5.0f, 5.0f, 1.0f);
        Text->RenderText("Bricks:" + bricks.str(), 150.0f, 5.0f, 1.0f);
    }
    if(this->State == GAME_MENU)
    {   
        Text->RenderText("Press ENTER to Start", 250.0f, Height/2+60.0f, 1.0f);
        Text->RenderText("Press W or S to select level", 245.0f, Height / 2 + 85.0f, 0.75f);
        Text->RenderText("Press SPACE to throw ball", 260.0f, Height / 2 + 105.0f, 0.75f);
    }   
    if(this->State == GAME_WIN)
    {
        Text->RenderText("You Won the game!", 300.0f, Height/2+60.0f, 1.0f);
        Text->RenderText("Press ENTER to retry or Q to quit", 250.0f, Height / 2 + 85.0f, 0.75f);
    }
    if(this->State == GAME_LOSE)
    {
        Text->RenderText("You Lose!", 350.0f, Height/2+60.0f, 1.0f);
        Text->RenderText("Press ENTER to retry or Q to quit", 250.0f, Height / 2 + 85.0f, 0.75f);
    }
    if(this->State == GAME_PAUSE)
    {
        Text->RenderText("PAUSE", 365.0f, Height/2, 1.0f);
    }
    if(this->State == GAME_ATTRIBUTES)
    {   
        Texture2D myBackground = ResourceManager::GetTexture("background");
        
        Renderer->DrawSprite(myBackground, glm::vec2(0.0f, 0.0f), glm::vec2(this->Width, this->Height), 0.0f);
        
        std::stringstream playerX; playerX << Player->Position.x;
        std::stringstream playerY; playerY << Player->Position.y;
        std::stringstream playerV; playerV << this->PaddleVelocity;

        std::stringstream ballX; ballX << Ball->Position.x;
        std::stringstream ballY; ballY << Ball->Position.y;
        std::stringstream ballVx; ballVx << Ball->Velocity.x;
        std::stringstream ballVy; ballVy << Ball->Velocity.y;

        Text->RenderText("ATTRIBUTES:", 5.0f, 5.0f, 1.0f);
        Text->RenderText("Paddle: (" + playerX.str() + "," + playerY.str() + ")  V: " + playerV.str(), 5.0f, 50.0f, 1.0f);
        Text->RenderText("Ball: (" + ballX.str() + "," + ballY.str() + ")  V: (" + ballVx.str() + "," + ballVy.str() + ")", 5.0f, 100.0f, 1.0f);
    }
}

void Game::DoCollisions()
{
    for (GameObject &box : this->Levels[this->Level].Bricks)
    {
        if (!box.Destroyed)
        {
            Collision collision = CheckCollision(*Ball, box);
            if (std::get<0>(collision)) // if collision is true
            {
                if (!box.IsSolid)
                    box.Destroyed = true;

                Direction dir = std::get<1>(collision);
                glm::vec2 diff_vector = std::get<2>(collision);
                
                if (dir == LEFT || dir == RIGHT) 
                {
                    this->HorizontalCollision(dir, diff_vector);
                }
                else 
                {
                    this->VerticalCollision(dir, diff_vector);
                }
            }
        }
    }
    Collision result = CheckCollision(*Ball, *Player);
    if (!Ball->Stuck && std::get<0>(result))
    {
        this->PaddleCollision();
    } 

}  

void Game::HorizontalCollision(Direction dir, glm::vec2 diff_vector)
{
    Ball->Velocity.x = -Ball->Velocity.x; // reverse horizontal velocity
    
    // relocate
    float penetration = Ball->Radius - std::abs(diff_vector.x);
    if (dir == LEFT)
        Ball->Position.x += penetration; // move ball right
    else
        Ball->Position.x -= penetration; // move ball left;
}

void Game::VerticalCollision(Direction dir, glm::vec2 diff_vector)
{
    Ball->Velocity.y = -Ball->Velocity.y; // reverse vertical velocity
    
    // relocate
    float penetration = Ball->Radius - std::abs(diff_vector.y);
    if (dir == UP)
        Ball->Position.y -= penetration; // move ball up
    else
        Ball->Position.y += penetration; // move ball down
}

void Game::PaddleCollision()
{
    // check where it hit the board, and change velocity based on where it hit the board
    float centerBoard = Player->Position.x + Player->Size.x / 2.0f;
    float distance = (Ball->Position.x + Ball->Radius) - centerBoard;
    float percentage = distance / (Player->Size.x / 2.0f);
    // then move accordingly
    float strength = 2.0f;
    glm::vec2 oldVelocity = Ball->Velocity;
    Ball->Velocity.x = INITIAL_BALL_VELOCITY.x * percentage * strength; 
    Ball->Velocity.y = -1.0f * abs(Ball->Velocity.y);
    Ball->Velocity = glm::normalize(Ball->Velocity) * glm::length(oldVelocity);
}

void Game::ResetLevel()
{   
    this->ResetPlayer();
    Ball->Stuck = true;
    this->Lives = 3;
    this->State = GAME_MENU;
    if (this->Level == 0)
        this->Levels[0].Load("levels/one.lvl", this->Width, this->Height / 2);
    else if (this->Level == 1)
        this->Levels[1].Load("levels/two.lvl", this->Width, this->Height / 2);
    else if (this->Level == 2)
        this->Levels[2].Load("levels/three.lvl", this->Width, this->Height / 2);
    else if (this->Level == 3)
        this->Levels[3].Load("levels/four.lvl", this->Width, this->Height / 2);
}

void Game::ResetPlayer()
{
    // reset player/ball stats
    Player->Size = PLAYER_SIZE;
    Player->Position = glm::vec2(this->Width / 2.0f - PLAYER_SIZE.x / 2.0f, this->Height - PLAYER_SIZE.y);
    Ball->Reset(Player->Position + glm::vec2(PLAYER_SIZE.x / 2.0f - BALL_RADIUS, -(BALL_RADIUS * 2.0f)), INITIAL_BALL_VELOCITY);
}
