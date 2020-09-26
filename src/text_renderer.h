#ifndef TEXT_RENDERER_H
#define TEXT_RENDERER_H

#include <map>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "texture.h"
#include "shader.h"


// state information relevant to a character as loaded using FreeType
struct Character {
    unsigned int TextureID; 
    glm::ivec2 Size;      
    glm::ivec2 Bearing;     // offset from baseline to left/top of glyph
    long Advance;   // horizontal offset to advance to next glyph
};


class TextRenderer
{
public:
    // pre-compiled Characters
    std::map<char, Character> Characters; 

    Shader TextShader;

    TextRenderer(unsigned int width, unsigned int height);

    // pre-compiles characters from the given font
    void Load(std::string font, unsigned int fontSize);

    void RenderText(std::string text, float x, float y, float scale, 
                        glm::vec3 color = glm::vec3(1.0f));
private:
    unsigned int VAO, VBO;
};

#endif 

