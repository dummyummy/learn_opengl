#ifndef DEFAULT_TEXTURES_H
#define DEFAULT_TEXTURES_H

#include <glad/glad.h>

#include <string>
#include <map>

class DefaultTextures
{
public:
    enum class TextureType
    {
        BLACK,
        WHITE,
        RED,
        GREEN,
        BLUE
    };
    static std::map<TextureType, unsigned int> textures;

    static void init()
    {
        unsigned int blackTexture;
        glGenTextures(1, &blackTexture);
        glBindTexture(GL_TEXTURE_2D, blackTexture);
        float black[] = {0.0f, 0.0f, 0.0f, 1.0f};
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_FLOAT, black);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        textures[TextureType::BLACK] = blackTexture;

        unsigned int whiteTexture;
        glGenTextures(1, &whiteTexture);
        glBindTexture(GL_TEXTURE_2D, whiteTexture);
        float white[] = {1.0f, 1.0f, 1.0f, 1.0f};
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_FLOAT, white);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        textures[TextureType::WHITE] = whiteTexture;

        unsigned int redTexture;
        glGenTextures(1, &redTexture);
        glBindTexture(GL_TEXTURE_2D, redTexture);
        float red[] = {1.0f, 0.0f, 0.0f, 1.0f};
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_FLOAT, red);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        textures[TextureType::RED] = redTexture;

        unsigned int greenTexture;
        glGenTextures(1, &greenTexture);
        glBindTexture(GL_TEXTURE_2D, greenTexture);
        float green[] = {0.0f, 1.0f, 0.0f, 1.0f};
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_FLOAT, green);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        textures[TextureType::GREEN] = greenTexture;

        unsigned int blueTexture;
        glGenTextures(1, &blueTexture);
        glBindTexture(GL_TEXTURE_2D, blueTexture);
        float blue[] = {0.0f, 0.0f, 1.0f, 1.0f};
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_FLOAT, blue);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        textures[TextureType::BLUE] = blueTexture;
    }
};

#endif // DEFAULT_TEXTURES_H