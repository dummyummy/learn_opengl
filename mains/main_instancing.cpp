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

// camera
Camera camera(glm::vec3(0.0f, 5.0f, 20.0f));
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
    glDepthFunc(GL_LESS);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    /***** create viewport *****/
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

    // build and compile shader
    // ------------------------
    Shader instancingShader(CMAKE_SOURCE_DIR"/shaders/instancing/instancing_vert.glsl",
                            CMAKE_SOURCE_DIR"/shaders/instancing/instancing_frag.glsl");
    Shader phongShader(CMAKE_SOURCE_DIR"/shaders/instancing/phong_vert.glsl",
                       CMAKE_SOURCE_DIR"/shaders/instancing/phong_frag.glsl");

    float cubeVertices[] = {
        // positions          // normals           // texture coords
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
    };
    
    // cube VAO
    unsigned int cubeVAO, cubeVBO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);

    Model planet(CMAKE_SOURCE_DIR"/resources/objects/planet/planet.obj");
    Model rock(CMAKE_SOURCE_DIR"/resources/objects/rock/rock.obj");

    constexpr unsigned int amount = 10000;
    glm::mat4 *modelMatrices, *normalMatrices;
    modelMatrices = new glm::mat4[amount];
    normalMatrices = new glm::mat4[amount];
    srand(glfwGetTime()); // 初始化随机种子    
    float radius = 20.0;
    float offset = 2.5f;
    for(unsigned int i = 0; i < amount; i++)
    {
        glm::mat4 model;
        // 1. 位移：分布在半径为 'radius' 的圆形上，偏移的范围是 [-offset, offset]
        float angle = (float)i / (float)amount * 360.0f;
        float displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
        float x = sin(angle) * radius + displacement;
        displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
        float y = displacement * 0.4f; // 让行星带的高度比x和z的宽度要小
        displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
        float z = cos(angle) * radius + displacement;
        model = glm::translate(model, glm::vec3(x, y, z));

        // 2. 缩放：在 0.05 和 0.25f 之间缩放
        float scale = (rand() % 20) / 100.0f + 0.05;
        model = glm::scale(model, glm::vec3(scale));

        // 3. 旋转：绕着一个（半）随机选择的旋转轴向量进行随机的旋转
        float rotAngle = (rand() % 360);
        model = glm::rotate(model, rotAngle, glm::vec3(0.4f, 0.6f, 0.8f));

        // 4. 添加到矩阵的数组中
        modelMatrices[i] = model;
        normalMatrices[i] = glm::transpose(glm::inverse(model));
    }

    unsigned int modelBuffer, normalBuffer;
    glGenBuffers(1, &modelBuffer);
    glGenBuffers(1, &normalBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, modelBuffer);
    glBufferData(GL_ARRAY_BUFFER, amount * sizeof(glm::mat4), &modelMatrices[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
    glBufferData(GL_ARRAY_BUFFER, amount * sizeof(glm::mat4), &normalMatrices[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    for (const auto &mesh : rock.meshes)
    {
        unsigned int VAO = mesh.getVAO();
        glBindVertexArray(VAO);
        // set attribute pointers for matrix (4 times vec4)
        glBindBuffer(GL_ARRAY_BUFFER, modelBuffer);
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
        glEnableVertexAttribArray(5);
        glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
        glEnableVertexAttribArray(6);
        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));
        // set attribute pointers for normal matrix (3 times vec4)
        glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
        glEnableVertexAttribArray(7);
        glVertexAttribPointer(7, 3, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
        glEnableVertexAttribArray(8);
        glVertexAttribPointer(8, 3, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
        glEnableVertexAttribArray(9);
        glVertexAttribPointer(9, 3, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glVertexAttribDivisor(3, 1);
        glVertexAttribDivisor(4, 1);
        glVertexAttribDivisor(5, 1);
        glVertexAttribDivisor(6, 1);
        glVertexAttribDivisor(7, 1);
        glVertexAttribDivisor(8, 1);
        glVertexAttribDivisor(9, 1);
        glBindVertexArray(0);
    }
    
    // transform properties
    float imgui_background_alpha = 0.5f;

    // material properties
    float ratio = 1.52f;
    // model transformation
    float scale = 1.0f;
    float magnitude = 0.1f;
    // lighting properties
    float shininess = 32.0f;
    // light properties
    glm::vec3 lightDir(0.0f, -1.0f, 0.0f);
    glm::vec3 lightAmbient(0.2f, 0.2f, 0.2f);
    glm::vec3 lightDiffuse(1.0f, 1.0f, 1.0f);
    glm::vec3 lightSpecular(1.0f, 1.0f, 1.0f);
    glm::vec3 clearColor(0.1f, 0.1f, 0.1f);
    float innerTheta = 12.5f, thetaTransition = 5.0f;
    glm::vec3 pointLightPositions[] = {
        glm::vec3(0.7f, 0.2f, 2.0f),
        glm::vec3(2.3f, -3.3f, -4.0f),
    };
    // post processing
    bool postProcessing = false;
    float offsetScale = 0.005f;
    float offsetFreq = 20.0f;
    
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
        ImGui::SliderFloat("scale", &scale, 0.1f, 2.0f);
        ImGui::SliderFloat("magnitude", &magnitude, 0.0f, 0.2f);
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
        ImGui::End();

        // render
        // ------
        // planet
        glClearColor(clearColor.x, clearColor.y, clearColor.z, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        // create transformations
        glm::mat4 model = glm::mat4(1.0f), normalMatrix;
        glm::mat4 view          = glm::mat4(1.0f);
        glm::mat4 projection    = glm::mat4(1.0f);
        view = camera.GetViewMatrix();
        projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / SCR_HEIGHT, near, far);

        phongShader.use();
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(scale));
        normalMatrix = glm::transpose(glm::inverse(model));
        phongShader.setMat4("view", glm::value_ptr(view));
        phongShader.setMat4("projection", glm::value_ptr(projection));
        phongShader.setMat4("model", glm::value_ptr(model));
        phongShader.setMat4("normalMatrix", glm::value_ptr(normalMatrix));
        phongShader.setFloat("material.shininess", shininess);
        phongShader.setVec3("viewPos",  glm::value_ptr(camera.Position));

        // directional light
        phongShader.setVec3("dirLight.direction", glm::value_ptr(lightDir));
        phongShader.setVec3("dirLight.ambient", glm::value_ptr(lightAmbient));
        phongShader.setVec3("dirLight.diffuse", glm::value_ptr(lightDiffuse));
        phongShader.setVec3("dirLight.specular", glm::value_ptr(lightSpecular));
        // point light 1
        phongShader.setVec3("pointLights[0].position", glm::value_ptr(pointLightPositions[0]));
        phongShader.setVec3("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
        phongShader.setVec3("pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
        phongShader.setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
        phongShader.setFloat("pointLights[0].constant", 1.0f);
        phongShader.setFloat("pointLights[0].linear", 0.09f);
        phongShader.setFloat("pointLights[0].quadratic", 0.032f);

        planet.Draw(phongShader);

        // draw rocks
        instancingShader.use();
        instancingShader.setMat4("view", glm::value_ptr(view));
        instancingShader.setMat4("projection", glm::value_ptr(projection));
        instancingShader.setFloat("material.shininess", shininess);
        instancingShader.setVec3("viewPos",  glm::value_ptr(camera.Position));

        // directional light
        instancingShader.setVec3("dirLight.direction", glm::value_ptr(lightDir));
        instancingShader.setVec3("dirLight.ambient", glm::value_ptr(lightAmbient));
        instancingShader.setVec3("dirLight.diffuse", glm::value_ptr(lightDiffuse));
        instancingShader.setVec3("dirLight.specular", glm::value_ptr(lightSpecular));
        // point light 1
        instancingShader.setVec3("pointLights[0].position", glm::value_ptr(pointLightPositions[0]));
        instancingShader.setVec3("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
        instancingShader.setVec3("pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
        instancingShader.setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
        instancingShader.setFloat("pointLights[0].constant", 1.0f);
        instancingShader.setFloat("pointLights[0].linear", 0.09f);
        instancingShader.setFloat("pointLights[0].quadratic", 0.032f);

        // planet.Draw(instancingShader);

        for (const auto &mesh : rock.meshes)
        {
            glBindVertexArray(mesh.getVAO());
            glDrawElementsInstanced(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0, amount);
        }

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
    stbi_set_flip_vertically_on_load(true);
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