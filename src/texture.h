#ifndef TEXTURE_H
#define TEXTURE_H

#include <glad/glad.h>

class Texture2D
{
public:
    unsigned int ID;

    unsigned int Width;
    unsigned int Height;

    unsigned int Internal_Format;
    unsigned int Image_Format;

    // config
    unsigned int Wrap_S;
    unsigned int Wrap_T;
    unsigned int Filter_Min; // if texture pixels < screen pixels
    unsigned int Filter_Max; // if texture pixels > screen pixels

    Texture2D();

    void Generate(unsigned int width, unsigned int height, unsigned char* data);

    // binds texture as current active GL_TEXTURE_2D texture object
    void Bind() const;
};

#endif

