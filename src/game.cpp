#include "game.h"
#include "resource_manager.h"
#include "sprite_renderer.h"
#include "game_object.h"
#include "ball_object.h"
#include "game_level.h"
#include "colision.h"
#include "particle_generator.h"
#include "text_renderer.h"
#include "post_process.h"

#include <sstream>
#include <iostream>
#include <algorithm>


SpriteRenderer *Renderer;
GameObject *Player;
ParticleGenerator *Particles;
TextRenderer *Text;
PostProcessor *Effects;

//Effect time
float ShakeTime = 0.0f;

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
    delete Ball;
    delete Particles;
    delete Effects;
    delete Text;
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
    ResourceManager::LoadShader("shaders/post_process.vs", "shaders/post_process.fs", nullptr, "postprocessing");
    
    // configure shaders
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(this->Width), 
        static_cast<float>(this->Height), 0.0f, -1.0f, 1.0f);
    ResourceManager::GetShader("sprite").Use().SetInteger("image", 0);
    ResourceManager::GetShader("sprite").SetMatrix4("projection", projection);
    ResourceManager::GetShader("particle").Use().SetMatrix4("projection", projection);
    
}

void Game::LoadTextures()
{
    ResourceManager::LoadTexture("textures/starry_background.jpg", false, "background");
    ResourceManager::LoadTexture("textures/ball.png", true, "ball");
    ResourceManager::LoadTexture("textures/brick.png", false, "brick");
    ResourceManager::LoadTexture("textures/brick_solid.png", false, "brick_solid");
    ResourceManager::LoadTexture("textures/player_paddle.png", true, "paddle");
    ResourceManager::LoadTexture("textures/star_particle.png", true, "particle");
    ResourceManager::LoadTexture("textures/powerup_speed.png", true, "powerup_speed");
    ResourceManager::LoadTexture("textures/powerup_sticky.png", true, "powerup_sticky");
    ResourceManager::LoadTexture("textures/powerup_increase.png", true, "powerup_increase");
    ResourceManager::LoadTexture("textures/powerup_confuse.png", true, "powerup_confuse");
    ResourceManager::LoadTexture("textures/powerup_chaos.png", true, "powerup_chaos");
    ResourceManager::LoadTexture("textures/powerup_passthrough.png", true, "powerup_passthrough");

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
    Texture2D myFace = ResourceManager::GetTexture("ball");
    Ball = new BallObject(ballPos, BALL_RADIUS, INITIAL_BALL_VELOCITY, myFace);
    

    // set render-specific controls
    Shader mySprite = ResourceManager::GetShader("sprite");
    Renderer = new SpriteRenderer(mySprite);
    Effects = new PostProcessor(ResourceManager::GetShader("postprocessing"), 
                                this->Width, this->Height);
    Particles = new ParticleGenerator(
        ResourceManager::GetShader("particle"), 
        ResourceManager::GetTexture("particle"), 
        500
    );

    Text = new TextRenderer(this->Width, this->Height);
    Text->Load("fonts/VCR_OSD_MONO.ttf", 24);

}

void Game::Update(float dt)
{
    Ball->Move(dt, this->Width);
    
    this->DoCollisions();
    
    Particles->Update(dt, *Ball, 2, glm::vec2(Ball->Radius / 2.0f));    
    
    this->UpdatePowerUps(dt);

    this->CheckDeath();
    this->CheckWin();

    if (ShakeTime > 0.0f)
    {
        ShakeTime -= dt;
        if (ShakeTime <= 0.0f)
            Effects->Shake = false;
    }
}  

void Game::CheckDeath()
{
    if (Ball->Position.y >= this->Height) // did ball reach bottom edge?
    {
        --this->Lives;
        if(this->Lives <= 0){
            //this->ResetLevel();
            this->State = GAME_LOSE;
        }
        this->ResetPlayer();
    }
}

void Game::CheckWin()
{
  if(this->State == GAME_ACTIVE && this->Levels[this->Level].IsCompleted())
    {
        //this->ResetLevel();
        //this->ResetPlayer();
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
        if (this->Keys[GLFW_KEY_SPACE] && !this->KeysProcessed[GLFW_KEY_SPACE])
        {
            this->State = GAME_ACTIVE;
            this->KeysProcessed[GLFW_KEY_SPACE] = true;
        }
        if (this->Keys[GLFW_KEY_D] && !this->KeysProcessed[GLFW_KEY_D])
        {
            this->Level = (this->Level + 1) % 4;
            this->KeysProcessed[GLFW_KEY_D] = true;
        }
        if (this->Keys[GLFW_KEY_A] && !this->KeysProcessed[GLFW_KEY_A])
        {
            if (this->Level > 0)
                --this->Level;
            else
                this->Level = 3;
            this->KeysProcessed[GLFW_KEY_A] = true;
        }
    }
    
    if(this->State == GAME_WIN || this->State == GAME_LOSE)
    {
        if(this->Keys[GLFW_KEY_R])
        {
            this->ResetLevel();
            this->ResetPlayer();
            this->State = GAME_MENU;
        }
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
    if(this->State == GAME_ACTIVE || this->State == GAME_MENU || this->State == GAME_PAUSE || this->State == GAME_WIN || this->State == GAME_LOSE || this->State == GAME_ATTRIBUTES)
    {
        Texture2D myBackground = ResourceManager::GetTexture("background");
        
        Effects->BeginRender();

        Renderer->DrawSprite(myBackground, glm::vec2(0.0f, 0.0f), glm::vec2(this->Width, this->Height), 0.0f);
        
        this->Levels[this->Level].Draw(*Renderer);
        
        Player->Draw(*Renderer);

        for (PowerUp &powerUp : this->PowerUps)
                if (!powerUp.Destroyed)
                    powerUp.Draw(*Renderer);
        	
        Particles->Draw();
        
        Ball->Draw(*Renderer);

        Effects->EndRender();
        Effects->Render(glfwGetTime());

        if(this->State == GAME_ACTIVE || this->State == GAME_PAUSE)
        {
            int bricksDestroyed = 0;

            for (GameObject &tile : this->Levels[this->Level].Bricks)
                if(tile.Destroyed)
                    bricksDestroyed += 1;

            std::stringstream balls; balls << this->Lives;
            std::stringstream bricks; bricks << bricksDestroyed;

            Text->RenderText("Balls:" + balls.str(), 5.0f, 5.0f, 1.0f);
            Text->RenderText("Bricks:" + bricks.str(), 150.0f, 5.0f, 1.0f);
        }
    }
    if(this->State == GAME_MENU)
    {   
        Text->RenderText("Press SPACE to Start", 250.0f, Height/2+60.0f, 1.0f);
        Text->RenderText("Press A or D to select level", 245.0f, Height / 2 + 85.0f, 0.75f);
    }   
    if(this->State == GAME_WIN)
    {
        Text->RenderText("You Won the game!", 300.0f, Height/2+60.0f, 1.0f);
        Text->RenderText("Press R to retry or Q to quit", 250.0f, Height / 2 + 85.0f, 0.75f);
    }
    if(this->State == GAME_LOSE)
    {
        Text->RenderText("You Lose!", 350.0f, Height/2+60.0f, 1.0f);
        Text->RenderText("Press R to retry or Q to quit", 250.0f, Height / 2 + 85.0f, 0.75f);
    }
    if(this->State == GAME_PAUSE)
    {
        Text->RenderText("PAUSE", 360.0f, Height/2, 1.0f);
    }
    
    if(this->State == GAME_ATTRIBUTES)
    {   
        std::stringstream playerX; playerX << Player->Position.x;
        std::stringstream playerY; playerY << Player->Position.y;
        std::stringstream playerV; playerV << this->PaddleVelocity;

        std::stringstream ballX; ballX << Ball->Position.x;
        std::stringstream ballY; ballY << Ball->Position.y;
        std::stringstream ballVx; ballVx << Ball->Velocity.x;
        std::stringstream ballVy; ballVy << Ball->Velocity.y;

        //Text->RenderText("ATTRIBUTES:", 5.0f, 5.0f, 1.0f);
        Text->RenderText("X:" + playerX.str() + ", Y:" + playerY.str(), Player->Position.x+5.0f, Player->Position.y-20.0f, 0.4f);
        Text->RenderText("V: " + playerV.str(), Player->Position.x+5.0f, Player->Position.y-10.0f, 0.4f);
        Text->RenderText("X: " + ballX.str() + ",Y: " + ballY.str(), Ball->Position.x+35.0f, Ball->Position.y+5.0f, 0.4f);
        Text->RenderText("V: (" + ballVx.str() + "," + ballVy.str() + ")", Ball->Position.x+35.0f, Ball->Position.y+15.0f, 0.4f);

        for (GameObject &box : this->Levels[this->Level].Bricks)
        {
            if(!box.Destroyed)
            {
                std::stringstream brickX; brickX << round(box.Position.x);
                std::stringstream brickY; brickY << box.Position.y;
                Text->RenderText("X:" + brickX.str(), box.Position.x+12.0f, box.Position.y+10.0f, 0.40f);
                Text->RenderText("Y:" + brickY.str(), box.Position.x+12.0f, box.Position.y+20.0f, 0.40f);
            }
        }
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
                {
                    box.Destroyed = true;
                    this->SpawnPowerUps(box);
                }

                else
                {   // Solid block, enable shake effect on impact
                    ShakeTime = 0.05f;
                    Effects->Shake = true;
                }

                Direction dir = std::get<1>(collision);
                glm::vec2 diff_vector = std::get<2>(collision);
                
                if(!(Ball->PassThrough && !box.IsSolid))
                {
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
    }
    Collision result = CheckCollision(*Ball, *Player);
    if (!Ball->Stuck && std::get<0>(result))
    {
        this->PaddleCollision();
    } 

    for (PowerUp &powerUp : this->PowerUps)
    {
        if (!powerUp.Destroyed)
        {
            if (powerUp.Position.y >= this->Height)
                powerUp.Destroyed = true;
            if (CheckCollision(*Player, powerUp))
            {
                ActivatePowerUp(powerUp, Effects, Player, Ball);
                powerUp.Destroyed = true;
                powerUp.Activated = true;
            }
        }
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
    Ball->Stuck = Ball->Sticky;
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

void Game::SpawnPowerUps(GameObject &block)
{
    //Positives
    if (ShouldSpawn(75))
        this->PowerUps.push_back(PowerUp("speed", glm::vec3(0.5f, 0.5f, 1.0f), 0.0f, 
                                        block.Position, ResourceManager::GetTexture("powerup_speed")));
    if (ShouldSpawn(75))
        this->PowerUps.push_back(PowerUp("sticky", glm::vec3(1.0f, 0.5f, 1.0f), 20.0f, 
                                        block.Position, ResourceManager::GetTexture("powerup_sticky")));
    if (ShouldSpawn(75))
        this->PowerUps.push_back(PowerUp("pass-through", glm::vec3(0.5f, 1.0f, 0.5f), 10.0f, 
                                        block.Position, ResourceManager::GetTexture("powerup_passthrough")));
    if (ShouldSpawn(75))
        this->PowerUps.push_back(PowerUp("pad-size-increase", glm::vec3(1.0f, 0.6f, 0.4), 0.0f, 
                                        block.Position, ResourceManager::GetTexture("powerup_increase")));
    //Negatives
    if (ShouldSpawn(15)) 
        this->PowerUps.push_back(PowerUp("confuse", glm::vec3(1.0f, 0.3f, 0.3f), 15.0f, 
                                        block.Position, ResourceManager::GetTexture("powerup_confuse")));
    if (ShouldSpawn(15))
        this->PowerUps.push_back(PowerUp("chaos", glm::vec3(0.9f, 0.25f, 0.25f), 15.0f, 
                                        block.Position, ResourceManager::GetTexture("powerup_chaos")));
}  

void Game::UpdatePowerUps(float dt)
{
    for (PowerUp &powerUp : this->PowerUps)
    {
        powerUp.Position += powerUp.Velocity * dt;
        if (powerUp.Activated)
        {
            powerUp.Duration -= dt;

            if (powerUp.Duration <= 0.0f)
            {
                // remove powerup from list (will later be removed)
                powerUp.Activated = false;
                // deactivate effects
                if (powerUp.Type == "sticky")
                {
                    if (!IsOtherPowerUpActive(this->PowerUps, "sticky"))
                    {	// only reset if no other PowerUp of type sticky is active
                        Ball->Sticky = false;
                        Player->Color = glm::vec3(1.0f);
                    }
                }
                else if (powerUp.Type == "pass-through")
                {
                    if (!IsOtherPowerUpActive(this->PowerUps, "pass-through"))
                    {	// only reset if no other PowerUp of type pass-through is active
                        Ball->PassThrough = false;
                        Ball->Color = glm::vec3(1.0f);
                    }
                }
                else if (powerUp.Type == "confuse")
                {
                    if (!IsOtherPowerUpActive(this->PowerUps, "confuse"))
                    {	// only reset if no other PowerUp of type confuse is active
                        Effects->Confuse = false;
                    }
                }
                else if (powerUp.Type == "chaos")
                {
                    if (!IsOtherPowerUpActive(this->PowerUps, "chaos"))
                    {	// only reset if no other PowerUp of type chaos is active
                        Effects->Chaos = false;
                    }
                }                
            }
        }
    }
    this->PowerUps.erase(std::remove_if(this->PowerUps.begin(), this->PowerUps.end(),
        [](const PowerUp &powerUp) { return powerUp.Destroyed && !powerUp.Activated; }
    ), this->PowerUps.end());
}