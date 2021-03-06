#include "game_level.h"

#include <fstream>
#include <sstream>


void GameLevel::Load(const char *file, unsigned int levelWidth, unsigned int levelHeight)
{
    // clear old data
    this->Bricks.clear();
   
    // load from file
    unsigned int tileCode;
    GameLevel level;
    std::string line;
    std::ifstream fstream(file);
    std::vector<std::vector<unsigned int>> tileData;
    if (fstream)
    {
        while (std::getline(fstream, line)) 
        {
            std::istringstream sstream(line);
            std::vector<unsigned int> row;
            while (sstream >> tileCode)
                row.push_back(tileCode);
            tileData.push_back(row);
        }
        if (tileData.size() > 0)
            this->init(tileData, levelWidth, levelHeight);
    }
}

void GameLevel::Draw(SpriteRenderer &renderer)
{
    for (GameObject &tile : this->Bricks)
        if (!tile.Destroyed)
            tile.Draw(renderer);
}

bool GameLevel::IsCompleted()
{
    for (GameObject &tile : this->Bricks)
        if (!tile.IsSolid && !tile.Destroyed)
            return false;
    return true;
}

void GameLevel::CheckBlockType(float unit_width, float unit_height, unsigned int x, unsigned int y)
{
    glm::vec2 pos(unit_width * x, unit_height * y);
    glm::vec2 size(unit_width, unit_height);
    GameObject obj(pos, size, ResourceManager::GetTexture("brick_solid"), glm::vec3(0.8f, 0.8f, 0.7f));
    obj.IsSolid = true;
    this->Bricks.push_back(obj);
}

void GameLevel::BlockColoring(std::vector<std::vector<unsigned int>> tileData, float unit_width, 
                                float unit_height, unsigned int x, unsigned int y)
{
    glm::vec3 color = glm::vec3(1.0f); // original: white
    if (tileData[y][x] == 2)
        color = glm::vec3(0.11f, 0.2f, 0.804f);// dark blue 
    else if (tileData[y][x] == 3)
        color = glm::vec3(0.2f, 0.649f, 0.9f); // light blue
    else if (tileData[y][x] == 4)
        color = glm::vec3(0.581f, 0.169f, 0.827f); // purple
    else if (tileData[y][x] == 5)
        color = glm::vec3(0.350f, 0.0f, 0.610f); // dark purple
    glm::vec2 pos(unit_width * x, unit_height * y);
    glm::vec2 size(unit_width, unit_height);
    this->Bricks.push_back(GameObject(pos, size, ResourceManager::GetTexture("brick"), color));
}

void GameLevel::init(std::vector<std::vector<unsigned int>> tileData, unsigned int levelWidth, unsigned int levelHeight)
{
    //dimensions
    unsigned int width = tileData[0].size(); //we can index vector at [0], function only called if height > 0
    unsigned int height = tileData.size();
    float unit_width = levelWidth / static_cast<float>(width);
    float unit_height = levelHeight / height; 
   
    // initialize tiles using tileData		
    for (unsigned int y = 0; y < height; ++y)
    {
        for (unsigned int x = 0; x < width; ++x)
        {
            // check block type from level data (2D level array)
            if (tileData[y][x] == 1) // solid
            {
               this->CheckBlockType(unit_width, unit_height, x, y);
            }

            else if (tileData[y][x] > 1)// non-solid, determine its color based on level data
            {
                this->BlockColoring(tileData, unit_width, unit_height, x, y);
            }
        }
    }
}

