#ifndef MESH_H
#define MESH_H

#include <glad/glad.h> // holds all OpenGL type declarations

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "shader.h"
#include "default_textures.h"

#include <string>
#include <vector>
#include <iostream>

#define MAX_BONE_INFLUENCE 4

struct Vertex
{
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    glm::vec3 Tangent;
    glm::vec3 Bitangent;
};

struct Texture
{
    unsigned int id;
    std::string type;
    std::string path;
};

class Mesh
{
public:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;
    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);
    void Draw(Shader &shader);
    void DrawInstanced(Shader &shader, int amount);
    unsigned int getVAO() const { return VAO; }
private:
    unsigned int VAO, VBO, EBO;
    void setupMesh();
};

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures)
{
    this->vertices = vertices;
    this->indices = indices;
    this->textures = textures;

    DefaultTextures::init();

    setupMesh();
}

void Mesh::setupMesh()
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
    // enable the first attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0); // pos
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal)); // normal
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords)); // uv
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent)); // T
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent)); // B

    glBindVertexArray(0);
}

void Mesh::Draw(Shader &shader)
{
    using namespace std;
    unsigned int diffuseNr = 1;
    unsigned int specularNr = 1;
    unsigned int reflectNr = 1;
    unsigned int normalNr = 1;
    unsigned int heightNr = 1;
    for (unsigned int i = 0; i < textures.size(); i++)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        string number;
        string name = textures[i].type;
        if (name == "texture_diffuse")
            number = to_string(diffuseNr++);
        else if (name == "texture_specular")
            number = to_string(specularNr++);
        else if (name == "texture_reflect")
            number = to_string(reflectNr++);
        else if (name == "texture_normal")
            number = to_string(normalNr++);
        else if (name == "texture_height")
            number = to_string(heightNr++);
        
        shader.setInt(("material." + name + number).c_str(), i);
        glBindTexture(GL_TEXTURE_2D, textures[i].id);
    }
    unsigned int noneNr = textures.size();
    if (diffuseNr == 1)
    {
        glActiveTexture(GL_TEXTURE0 + noneNr);
        shader.setInt("material.texture_diffuse1", noneNr);
        glBindTexture(GL_TEXTURE_2D, DefaultTextures::textures[DefaultTextures::TextureType::WHITE]);
        noneNr++;
    }
    if (specularNr == 1)
    {
        glActiveTexture(GL_TEXTURE0 + noneNr);
        shader.setInt("material.texture_specular1", noneNr);
        glBindTexture(GL_TEXTURE_2D, DefaultTextures::textures[DefaultTextures::TextureType::BLACK]);
        noneNr++;
    }
    if (reflectNr == 1)
    {
        glActiveTexture(GL_TEXTURE0 + noneNr);
        shader.setInt("material.texture_reflect1", noneNr);
        glBindTexture(GL_TEXTURE_2D, DefaultTextures::textures[DefaultTextures::TextureType::BLACK]);
        noneNr++;
    }
    if (normalNr == 1)
    {
        glActiveTexture(GL_TEXTURE0 + noneNr);
        shader.setInt("material.texture_normal1", noneNr);
        glBindTexture(GL_TEXTURE_2D, DefaultTextures::textures[DefaultTextures::TextureType::BLUE]);
        noneNr++;
    }
    if (heightNr == 1)
    {
        glActiveTexture(GL_TEXTURE0 + noneNr);
        shader.setInt("material.texture_height1", noneNr);
        glBindTexture(GL_TEXTURE_2D, DefaultTextures::textures[DefaultTextures::TextureType::BLACK]);
        noneNr++;
    }
    glActiveTexture(GL_TEXTURE0);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Mesh::DrawInstanced(Shader &shader, int amount)
{
    using namespace std;
    unsigned int diffuseNr = 1;
    unsigned int specularNr = 1;
    unsigned int reflectNr = 1;
    unsigned int normalNr = 1;
    for (unsigned int i = 0; i < textures.size(); i++)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        string number;
        string name = textures[i].type;
        if (name == "texture_diffuse")
            number = to_string(diffuseNr++);
        else if (name == "texture_specular")
            number = to_string(specularNr++);
        else if (name == "texture_reflect")
            number = to_string(reflectNr++);
        else if (name == "texture_normal")
            number = to_string(normalNr++);
        
        shader.setInt(("material." + name + number).c_str(), i);
        glBindTexture(GL_TEXTURE_2D, textures[i].id);
    }
    glActiveTexture(GL_TEXTURE0);

    glBindVertexArray(VAO);
    glDrawElementsInstanced(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0, amount);
    glBindVertexArray(0);
}

#endif // MESH_H