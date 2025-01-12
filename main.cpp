#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "shader.h"
#include "config.h"
#include "camera.h"
#include "model.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <iostream>
#include <cmath>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
void init_imgui(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 1000;
const unsigned int SCR_HEIGHT = 750;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
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

    /***** create viewport *****/
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

    // build and compile shader
    // ------------------------
    // Shader lightingShader(CMAKE_SOURCE_DIR"/shaders/multiple_lights_vert.glsl", CMAKE_SOURCE_DIR"/shaders/multiple_lights_frag.glsl");
    // Shader lightingShader(CMAKE_SOURCE_DIR"/shaders/model_loading_vert.glsl", CMAKE_SOURCE_DIR"/shaders/model_loading_frag.glsl");
    Shader depthShader(CMAKE_SOURCE_DIR"/shaders/depth_buffer_vert.glsl", CMAKE_SOURCE_DIR"/shaders/depth_buffer_frag.glsl");

    stbi_set_flip_vertically_on_load(false);

    // load models
    // -----------
    Model nanosuit(CMAKE_SOURCE_DIR"/resources/objects/nanosuit/nanosuit.obj");
    // Model mars(CMAKE_SOURCE_DIR"/resources/objects/planet/planet.obj");
    // Model rock(CMAKE_SOURCE_DIR"/resources/objects/rock/rock.obj");

    // transform properties
    float imgui_background_alpha = 0.5f;

    // model transformation
    float scale = 0.2f;
    // lighting properties
    float shininess = 32.0f;
    // light properties
    glm::vec3 lightDir(0.0f, -1.0f, 0.0f);
    glm::vec3 lightAmbient(0.2f, 0.2f, 0.2f);
    glm::vec3 lightDiffuse(0.5f, 0.5f, 0.5f);
    glm::vec3 lightSpecular(1.0f, 1.0f, 1.0f);
    float innerTheta = 12.5f, thetaTransition = 5.0f;
    glm::vec3 pointLightPositions[] = {
        glm::vec3(0.7f, 0.2f, 2.0f),
        glm::vec3(2.3f, -3.3f, -4.0f),
    };
    
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
        ImGui::Text("Model");
        ImGui::SliderFloat("scale", &scale, 0.1f, 2.0f);
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
        ImGui::SliderFloat("shininess", &shininess, 32.0f, 256.0f);
        ImGui::Text("Light");
        ImGui::SliderFloat3("lightDir", glm::value_ptr(lightDir), -1.0f, 1.0f, "%.2f");
        ImGui::ColorEdit3("lightAmbient", glm::value_ptr(lightAmbient));
        ImGui::ColorEdit3("lightDiffuse", glm::value_ptr(lightDiffuse));
        ImGui::ColorEdit3("lightSpecular", glm::value_ptr(lightSpecular));
        ImGui::SliderFloat("innerTheta", &innerTheta, 0.1f, 90.0f);
        ImGui::SliderFloat("thetaTransition", &thetaTransition, 0.0f, 30.0f);
        for (int i = 0; i < 2; i++)
        {
            ImGui::Text("Point Light %d", i);
            ImGui::SliderFloat3(("position" + std::to_string(i)).c_str(), glm::value_ptr(pointLightPositions[i]), -10.0f, 10.0f, "%.2f");
        }
        ImGui::Text("Misc");
        ImGui::SliderFloat("gui_alpha", &imgui_background_alpha, 0.0f, 1.0f, "%.2f");
        ImGui::End();

        // render
        // ------
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // create transformations
        glm::mat4 model = glm::mat4(1.0f), normalMatrix;
        glm::mat4 view          = glm::mat4(1.0f);
        glm::mat4 projection    = glm::mat4(1.0f);
        view = camera.GetViewMatrix();
        projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / SCR_HEIGHT, near, far);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(scale));
        normalMatrix = glm::transpose(glm::inverse(model));

        // lightingShader.use();
        // lightingShader.setMat4("view", glm::value_ptr(view));
        // lightingShader.setMat4("projection", glm::value_ptr(projection));
        // lightingShader.setMat4("model", glm::value_ptr(model));
        // lightingShader.setMat4("normalMatrix", glm::value_ptr(normalMatrix));
        // lightingShader.setFloat("material.shininess", shininess);
        // lightingShader.setVec3("viewPos",  glm::value_ptr(camera.Position));

        // // directional light
        // lightingShader.setVec3("dirLight.direction", glm::value_ptr(lightDir));
        // lightingShader.setVec3("dirLight.ambient", glm::value_ptr(lightAmbient));
        // lightingShader.setVec3("dirLight.diffuse", glm::value_ptr(lightDiffuse));
        // lightingShader.setVec3("dirLight.specular", glm::value_ptr(lightSpecular));
        // // point light 1
        // lightingShader.setVec3("pointLights[0].position", glm::value_ptr(pointLightPositions[0]));
        // lightingShader.setVec3("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
        // lightingShader.setVec3("pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
        // lightingShader.setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
        // lightingShader.setFloat("pointLights[0].constant", 1.0f);
        // lightingShader.setFloat("pointLights[0].linear", 0.09f);
        // lightingShader.setFloat("pointLights[0].quadratic", 0.032f);
        // // point light 2
        // lightingShader.setVec3("pointLights[1].position", glm::value_ptr(pointLightPositions[1]));
        // lightingShader.setVec3("pointLights[1].ambient", 0.05f, 0.05f, 0.05f);
        // lightingShader.setVec3("pointLights[1].diffuse", 0.8f, 0.8f, 0.8f);
        // lightingShader.setVec3("pointLights[1].specular", 1.0f, 1.0f, 1.0f);
        // lightingShader.setFloat("pointLights[1].constant", 1.0f);
        // lightingShader.setFloat("pointLights[1].linear", 0.09f);
        // lightingShader.setFloat("pointLights[1].quadratic", 0.032f);
        // // spotLight
        // lightingShader.setVec3("spotLight.position", glm::value_ptr(camera.Position));
        // lightingShader.setVec3("spotLight.direction", glm::value_ptr(camera.Front));
        // lightingShader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
        // lightingShader.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
        // lightingShader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
        // lightingShader.setFloat("spotLight.constant", 1.0f);
        // lightingShader.setFloat("spotLight.linear", 0.09f);
        // lightingShader.setFloat("spotLight.quadratic", 0.032f);
        // lightingShader.setFloat("spotLight.innerCutOff", glm::cos(glm::radians(innerTheta)));
        // lightingShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(innerTheta + thetaTransition)));  

        depthShader.use();
        depthShader.setMat4("view", glm::value_ptr(view));
        depthShader.setMat4("projection", glm::value_ptr(projection));
        depthShader.setMat4("model", glm::value_ptr(model));
        depthShader.setMat4("normalMatrix", glm::value_ptr(normalMatrix));
        depthShader.setFloat("near", near);
        depthShader.setFloat("far", far);

        // nanosuit.Draw(lightingShader);
        nanosuit.Draw(depthShader);
        // mars.Draw(lightingShader);
        // rock.Draw(lightingShader);
        // rock.Draw(depthShader);

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