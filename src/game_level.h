#ifndef GAMELEVEL_H
#define GAMELEVEL_H
#include <vector>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "game_object.h"
#include "sprite_renderer.h"
#include "resource_manager.h"

class GameLevel
{
public:
    // State
    std::vector<GameObject> Bricks;
    
    GameLevel() { }

    void Load(const char *file, unsigned int levelWidth, unsigned int levelHeight);
   
    void Draw(SpriteRenderer &renderer);
   
    bool IsCompleted();

private:
    // initialize from tile data
    void init(std::vector<std::vector<unsigned int>> tileData, unsigned int levelWidth, 
                unsigned int levelHeight);
    void CheckBlockType(float unit_width, float unit_height, unsigned int x, unsigned int y);
    void BlockColoring(std::vector<std::vector<unsigned int>> tileData, 
                        float unit_width, float unit_height, unsigned int x, unsigned int y);
};

#endif

