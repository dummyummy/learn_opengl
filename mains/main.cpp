#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "shader.h"
#include "config.h"
#include "camera.h"
#include "model.h"
#include "default_textures.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <iostream>
#include <cmath>
#include <map>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
void init_imgui(GLFWwindow *window);
unsigned int loadTexture(char const * path);
unsigned int loadCubemap(std::vector<std::string> faces);

// settings
const unsigned int SCR_WIDTH = 1000;
const unsigned int SCR_HEIGHT = 750;
const unsigned int msaa = 8;

// camera
Camera camera(glm::vec3(0.0f, 2.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
float near = 0.1f;
float far = 100.0f;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// mouse control
bool interactWithUI = false;

std::map<DefaultTextures::TextureType, unsigned int> DefaultTextures::textures;

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW!" << std::endl;
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // glfwWindowHint(GLFW_DECORATED, GL_FALSE);
#ifdef __APPLE
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    glfwWindowHint(GLFW_SAMPLES, 8);

    // glfw window creation
    // --------------------
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", nullptr, nullptr);
    if (window == nullptr)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        exit(-1);
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // imgui initialization
    // --------------------
    init_imgui(window);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_MULTISAMPLE);

    /***** create viewport *****/
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

    // build and compile shader
    // ------------------------
    Shader blinnShader(CMAKE_SOURCE_DIR"/shaders/vert.glsl", CMAKE_SOURCE_DIR"/shaders/frag.glsl");
    Shader screenShader(CMAKE_SOURCE_DIR"/shaders/post_processing/screen_vert.glsl", CMAKE_SOURCE_DIR"/shaders/post_processing/screen_frag.glsl");
    Shader dirDepthShader(CMAKE_SOURCE_DIR"/shaders/shadow_mapping/light_depth_vert.glsl",
                          CMAKE_SOURCE_DIR"/shaders/shadow_mapping/light_depth_frag.glsl");
    Shader pointDepthShader(CMAKE_SOURCE_DIR"/shaders/shadow_mapping/depth_cubemap_vert.glsl",
                            CMAKE_SOURCE_DIR"/shaders/shadow_mapping/depth_cubemap_frag.glsl",
                            CMAKE_SOURCE_DIR"/shaders/shadow_mapping/depth_cubemap_geo.glsl");
    // Shader secondDepthShader(CMAKE_SOURCE_DIR"/shaders/shadow_mapping/depth_vert.glsl", CMAKE_SOURCE_DIR"/shaders/shadow_mapping/depth_frag.glsl");
    blinnShader.use();

    DefaultTextures::init();

    float quadVertices[] = {   // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };

    Model sponza = Model(CMAKE_SOURCE_DIR"/resources/objects/sponza/sponza.obj");

    // setup screen VAO
    unsigned int quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    
    unsigned int framebuffer;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    unsigned int textureColorBufferMultiSampled;
    glGenTextures(1, &textureColorBufferMultiSampled);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, textureColorBufferMultiSampled);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, msaa, GL_RGB, SCR_WIDTH, SCR_HEIGHT, GL_TRUE);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, textureColorBufferMultiSampled, 0);
    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, msaa, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    // configure second post-processing framebuffer
    unsigned int intermediateFBO;
    glGenFramebuffers(1, &intermediateFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, intermediateFBO);
    // create a color attachment texture
    unsigned int screenTexture;
    glGenTextures(1, &screenTexture);
    glBindTexture(GL_TEXTURE_2D, screenTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screenTexture, 0);	// we only need a color buffer

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "ERROR::FRAMEBUFFER:: Intermediate framebuffer is not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    unsigned int depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);

    const unsigned int SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;
    unsigned int depthMap;
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    unsigned int depthCubeMapFBO;
    glGenFramebuffers(1, &depthCubeMapFBO);

    unsigned int depthCubeMap;
    glGenTextures(1, &depthCubeMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubeMap);
    for (unsigned int i = 0; i < 6; ++i)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, 
                     SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
    glTexParameterfv(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BORDER_COLOR, borderColor);
    glBindFramebuffer(GL_FRAMEBUFFER, depthCubeMapFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubeMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // transform properties
    float imgui_background_alpha = 0.5f;

    // material properties
    float ratio = 1.52f;
    float bumpScale = 1.0f;
    float heightScale = 0.2f;
    // model transformation
    float scale = 0.05f;
    // lighting properties
    float shininess = 128.0f;
    // light properties
    glm::vec3 lightDir(0.0f, -1.0f, -1.0f);
    glm::vec3 lightAmbient(0.2f, 0.2f, 0.2f);
    glm::vec3 lightDiffuse(0.8, 0.8f, 0.8f);
    glm::vec3 lightSpecular(1.0f, 1.0f, 1.0f);
    glm::vec3 clearColor(0.1f, 0.1f, 0.1f);
    glm::vec3 lightAttenuation(1.0f, 0.09f, 0.032f);
    float innerTheta = 12.5f, thetaTransition = 5.0f;
    glm::vec3 pointLightPositions[] = {
        glm::vec3(0.0f, 2.0f, 0.0f),
        glm::vec3(2.3f, -3.3f, -4.0f),
    };
    // post processing
    bool postProcessing = false;
    float offsetScale = 0.005f;
    float offsetFreq = 20.0f;
    float gamma = 2.2f;

    
    /***** render loop *****/
    while(!glfwWindowShouldClose(window))
    {
        // inputs
        // ------
        float currentTime = glfwGetTime();
        deltaTime = currentTime - lastFrame;
        lastFrame = currentTime;
        processInput(window); // read input

        // imgui loop start
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGuiStyle &style = ImGui::GetStyle();
        style.Colors[ImGuiCol_WindowBg].w = imgui_background_alpha;

        // imgui draw guis
        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Appearing);
        ImGui::SetNextWindowSize(ImVec2(250, SCR_HEIGHT), ImGuiCond_Appearing);
        ImGui::Begin("Properties"); // Create a window and append into it.
        ImGui::Text("Material");
        ImGui::SliderFloat("ratio", &ratio, 1.0f, 2.0f);
        ImGui::Text("Model");
        ImGui::DragFloat("scale", &scale, 0.01f, 0.0f, 5.0f);
        ImGui::DragFloat("bumpScale", &bumpScale, 0.01f, -5.0f, 5.0f);
        ImGui::DragFloat("heightScale", &heightScale, 0.01f, -5.0f, 5.0f);
        ImGui::Text("Camera");
        ImGui::SliderFloat3("cameraPos", glm::value_ptr(camera.Position), -10.0f, 10.0f, "%.1f");
        if (ImGui::SliderFloat3("cameraFront", glm::value_ptr(camera.Front), -1.0f, 1.0f, "%.2f"))
            camera.updateCameraVectors();
        ImGui::SliderFloat("fov", &camera.Zoom, 1.0f, 89.0f, "%.1f");
        ImGui::InputFloat("cameraSpeed", &camera.MovementSpeed);
        ImGui::InputFloat("sensitivity", &camera.MouseSensitivity);
        ImGui::SliderFloat("near", &near, 0.1f, 1.0f);
        ImGui::SliderFloat("far", &far, 10.0f, 200.0f);
        ImGui::Text("Lighting");
        ImGui::SliderFloat("shininess", &shininess, 1.0f, 256.0f);
        ImGui::Text("Light");
        ImGui::SliderFloat3("lightDir", glm::value_ptr(lightDir), -1.0f, 1.0f, "%.2f");
        ImGui::ColorEdit3("lightAmbient", glm::value_ptr(lightAmbient));
        ImGui::ColorEdit3("lightDiffuse", glm::value_ptr(lightDiffuse));
        ImGui::ColorEdit3("lightSpecular", glm::value_ptr(lightSpecular));
        ImGui::DragFloat3("lightAttenuation", glm::value_ptr(lightAttenuation), 0.01f, 0.0f, 1.0f);
        ImGui::SliderFloat("innerTheta", &innerTheta, 0.1f, 90.0f);
        ImGui::SliderFloat("thetaTransition", &thetaTransition, 0.0f, 30.0f);
        for (int i = 0; i < 1; i++)
        {
            ImGui::Text("Point Light %d", i);
            ImGui::SliderFloat3(("position" + std::to_string(i)).c_str(), glm::value_ptr(pointLightPositions[i]), -10.0f, 10.0f, "%.2f");
        }
        ImGui::Text("Misc");
        ImGui::SliderFloat("gui_alpha", &imgui_background_alpha, 0.0f, 1.0f, "%.2f");
        ImGui::ColorEdit3("clearColor", glm::value_ptr(clearColor));
        ImGui::Text("Misc");
        ImGui::Checkbox("postProcessing", &postProcessing);
        ImGui::DragFloat("offsetScale", &offsetScale, 0.001f);
        ImGui::DragFloat("offsetFreq", &offsetFreq, 0.1f);
        ImGui::SliderFloat("gamma", &gamma, 1.0f, 3.0f);
        ImGui::End();

        glm::mat4 model = glm::mat4(1.0f), normalMatrix;
        // shadow mapping settings
        float point_near_plane = 1.0f, point_far_plane = 100.0f;
        float aspect = (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT;
        glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), aspect, point_near_plane, point_far_plane);
        std::vector<glm::mat4> shadowTransforms;
        auto lightPos = pointLightPositions[0];
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 1.0,  0.0,  0.0), glm::vec3(0.0, -1.0,  0.0)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(-1.0,  0.0,  0.0), glm::vec3(0.0, -1.0,  0.0)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 0.0,  1.0,  0.0), glm::vec3(0.0,  0.0,  1.0)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 0.0, -1.0,  0.0), glm::vec3(0.0,  0.0, -1.0)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 0.0,  0.0,  1.0), glm::vec3(0.0, -1.0,  0.0)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 0.0,  0.0, -1.0), glm::vec3(0.0, -1.0, 0.0)));

        pointDepthShader.use();
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthCubeMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        pointDepthShader.setFloat("far_plane", point_far_plane);
        pointDepthShader.setVec3("lightPos", glm::value_ptr(lightPos));
        pointDepthShader.setMat4("shadowMatrices[0]", glm::value_ptr(shadowTransforms[0]));
        pointDepthShader.setMat4("shadowMatrices[1]", glm::value_ptr(shadowTransforms[1]));
        pointDepthShader.setMat4("shadowMatrices[2]", glm::value_ptr(shadowTransforms[2]));
        pointDepthShader.setMat4("shadowMatrices[3]", glm::value_ptr(shadowTransforms[3]));
        pointDepthShader.setMat4("shadowMatrices[4]", glm::value_ptr(shadowTransforms[4]));
        pointDepthShader.setMat4("shadowMatrices[5]", glm::value_ptr(shadowTransforms[5]));
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
            model = glm::scale(model, glm::vec3(scale));
            pointDepthShader.setMat4("model", glm::value_ptr(model));
            sponza.Draw(pointDepthShader);
        }
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

        float dir_near_plane = 1.0f, dir_far_plane = 100.0f;
        glm::mat4 lightProjection = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, dir_near_plane, dir_far_plane);
        glm::mat4 lightView = glm::lookAt(-20.0f * glm::normalize(lightDir), glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
        glm::mat4 lightSpaceMatrix = lightProjection * lightView;

        dirDepthShader.use();
        dirDepthShader.setMat4("lightSpaceMatrix", glm::value_ptr(lightSpaceMatrix));
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
            model = glm::scale(model, glm::vec3(scale));
            dirDepthShader.setMat4("model", glm::value_ptr(model));
            sponza.Draw(dirDepthShader);
        }
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);


        // render
        // ------
        // pass 1
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glClearColor(clearColor.x, clearColor.y, clearColor.z, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        // create transformations
        glm::mat4 view          = glm::mat4(1.0f);
        glm::mat4 projection    = glm::mat4(1.0f);
        view = camera.GetViewMatrix();
        projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / SCR_HEIGHT, near, far);

        blinnShader.use();

        blinnShader.setMat4("lightSpaceMatrix", glm::value_ptr(lightSpaceMatrix));
        blinnShader.setFloat("far_plane", point_far_plane);
        blinnShader.setFloat("bumpScale", bumpScale);
        blinnShader.setFloat("heightScale", heightScale);

        blinnShader.setFloat("gamma", gamma);
        blinnShader.setMat4("view", glm::value_ptr(view));
        blinnShader.setMat4("projection", glm::value_ptr(projection));
        blinnShader.setFloat("material.shininess", shininess);
        blinnShader.setVec3("viewPos",  glm::value_ptr(camera.Position));

        // directional light
        blinnShader.setVec3("dirLight.direction", glm::value_ptr(lightDir));
        blinnShader.setVec3("dirLight.ambient", glm::value_ptr(lightAmbient));
        blinnShader.setVec3("dirLight.diffuse", glm::value_ptr(lightDiffuse));
        blinnShader.setVec3("dirLight.specular", glm::value_ptr(lightSpecular));
        // point light 1
        blinnShader.setVec3("pointLights[0].position", glm::value_ptr(pointLightPositions[0]));
        blinnShader.setVec3("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
        blinnShader.setVec3("pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
        blinnShader.setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
        blinnShader.setFloat("pointLights[0].constant", lightAttenuation.x);
        blinnShader.setFloat("pointLights[0].linear", lightAttenuation.y);
        blinnShader.setFloat("pointLights[0].quadratic", lightAttenuation.z);

        blinnShader.setInt("material.texture_diffuse1", 0);
        blinnShader.setInt("material.texture_specular1", 1);
        blinnShader.setInt("material.texture_reflect1", 2);
        blinnShader.setInt("material.texture_normal1", 3);
        blinnShader.setInt("material.texture_height1", 4);
        blinnShader.setInt("dirShadowMap", 5);
        blinnShader.setInt("pointShadowMap", 6);
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubeMap);
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
            model = glm::scale(model, glm::vec3(scale));
            normalMatrix = glm::transpose(glm::inverse(model));
            blinnShader.setMat4("model", glm::value_ptr(model));
            blinnShader.setMat4("normalMatrix", glm::value_ptr(normalMatrix));
            sponza.Draw(blinnShader);
        }

        glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, intermediateFBO);
        glBlitFramebuffer(0, 0, SCR_WIDTH, SCR_HEIGHT, 0, 0, SCR_WIDTH, SCR_HEIGHT, GL_COLOR_BUFFER_BIT, GL_NEAREST);

        glDisable(GL_DEPTH_TEST);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, screenTexture);
        screenShader.use();
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // secondDepthShader.use();
        // model = glm::mat4(1.0f);
        // model = glm::translate(model, glm::vec3(0.7f, 0.7f, 0.0f));
        // model = glm::scale(model, glm::vec3(0.3f));
        // secondDepthShader.setMat4("model", glm::value_ptr(model));
        // glActiveTexture(GL_TEXTURE0);
        // glBindTexture(GL_TEXTURE_2D, depthMap);
        // glBindVertexArray(quadVAO);
        // glDrawArrays(GL_TRIANGLES, 0, 6);

        glEnable(GL_DEPTH_TEST);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents(); // poll IO events
    }

    /***** clean *****/
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();

    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{

    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && !interactWithUI)
        camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    if (!interactWithUI)
        camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    ImGuiIO &io = ImGui::GetIO();
    interactWithUI = io.WantCaptureMouse;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(Camera_Movement::FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(Camera_Movement::BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(Camera_Movement::LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(Camera_Movement::RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        camera.ProcessKeyboard(Camera_Movement::UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        camera.ProcessKeyboard(Camera_Movement::DOWN, deltaTime);
}

void init_imgui(GLFWwindow *window)
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);          // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
    ImGui_ImplOpenGL3_Init();
}

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(char const * path)
{
    stbi_set_flip_vertically_on_load(false);
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT); // for this tutorial: use GL_CLAMP_TO_EDGE to prevent semi-transparent borders. Due to interpolation it takes texels from next repeat 
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

unsigned int loadCubemap(std::vector<std::string> faces)
{
    stbi_set_flip_vertically_on_load(false);
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 
                         0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
            );
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}