#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
unsigned int loadTexture(const char *path);


// settings
const unsigned int SCR_WIDTH = 1200;
const unsigned int SCR_HEIGHT = 900;

// camera
Camera camera(glm::vec3(0.0f, 1.2f, 4.95f));
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

bool blur = false;
bool spotLightOn = false;
bool greenLight = false;

struct PointLight {
    glm::vec3 position;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

struct DirLight {
    glm::vec3 direction;

    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
};

struct SpotLight {
    glm::vec3 position;
    glm::vec3 direction;
    float cutOff;
    float outerCutOff;

    float constant;
    float linear;
    float quadratic;

    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
};

PointLight pointLight;

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CW);
    glCullFace(GL_BACK);


    // build and compile shaders
    // -------------------------
    Shader shader("resources/shaders/shader.vs", "resources/shaders/shader.fs");
    Shader screenShader("resources/shaders/framebufferScreenShader.vs", "resources/shaders/framebufferScreenShader.fs");

    Model ourModel(FileSystem::getPath("resources/objects/rust_gas/Gasoline_barrel.obj"));
    ourModel.SetShaderTextureNamePrefix("material.");

    pointLight.position = glm::vec3(0.0f, 4.0, 4.0);
    pointLight.ambient = glm::vec3(0.2, 0.2, 0.2);
    pointLight.diffuse = glm::vec3(0.7, 0.7, 0.7);
    pointLight.specular = glm::vec3(1.0, 1.0, 1.0);

    pointLight.constant = 1.0f;
    pointLight.linear = 0.09f;
    pointLight.quadratic = 0.032f;

    DirLight dirLight;
    dirLight.direction= glm::vec3(-0.2f, -1.0f, -0.3f);
    dirLight.ambient= glm::vec3(0.05f, 0.05f, 0.05f);
    dirLight.diffuse = glm::vec3(0.4f, 0.4f, 0.4f);
    dirLight.specular= glm::vec3(0.5f, 0.5f, 0.5f);

    SpotLight spotLight;
    spotLight.direction = glm::vec3 (camera.Front);
    spotLight.position = glm::vec3 (camera.Position);

    spotLight.ambient = glm::vec3(0.2, 0.2, 0.2);
    spotLight.diffuse = glm::vec3(0.8, 0.8, 0.8);
    spotLight.specular = glm::vec3(1.0, 1.0, 1.0);

    spotLight.constant = 1.0f;
    spotLight.linear = 0.09f;
    spotLight.quadratic = 0.032f;

    spotLight.cutOff = glm::cos(glm::radians(12.5f));
    spotLight.outerCutOff = glm::cos(glm::radians(15.0f));


    // set up vertex data (and buffer(s)) and configure vertex attributes

    float cubeVertices[] = {
            // back face
            -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
            0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f,  1.0f, 0.0f, // bottom-right
            0.5f,  0.5f, -0.5f, 0.0f, 0.0f, -1.0f,  1.0f, 1.0f, // top-right
            0.5f,  0.5f, -0.5f, 0.0f, 0.0f, -1.0f,  1.0f, 1.0f, // top-right
            -0.5f,  0.5f, -0.5f,0.0f, 0.0f, -1.0f,   0.0f, 1.0f, // top-left
            -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f,  0.0f, 0.0f, // bottom-left
            // front face
            -0.5f, -0.5f,  0.5f, 0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // bottom-left
            0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 1.0f,  1.0f, 1.0f, // top-right
            0.5f, -0.5f,  0.5f, 0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // bottom-right
            0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 1.0f,  1.0f, 1.0f, // top-right
            -0.5f, -0.5f,  0.5f,0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // bottom-left
            -0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 1.0f,  0.0f, 1.0f, // top-left
            // left face
            -0.5f,  0.5f,  0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-right
            -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-left
            -0.5f,  0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top-left
            -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-left
            -0.5f,  0.5f,  0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-right
            -0.5f, -0.5f,  0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // bottom-right
            // right face
            0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,1.0f, 0.0f, // top-left
            0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top-right
            0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,0.0f, 1.0f, // bottom-right
            0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-right
            0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // bottom-left
            0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-left
            // bottom face
            -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // top-right
            0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, // bottom-left
            0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f, // top-left
            0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, // bottom-left
            -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // top-right
            -0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, // bottom-right
            // top face
            -0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // top-left
            0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, // top-right
            0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom-right
            0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom-right
            -0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, // bottom-left
            -0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f  // top-left
    };

    float floorVertices[] {
            5.0f, -0.5f,  5.0f,  0.0f, 1.0f, 0.0f, 5.0f,  0.0f,
            -5.0f, -0.5f, -5.0f, 0.0f, 1.0f, 0.0f,   0.0f, 5.0f,
            -5.0f, -0.5f,  5.0f, 0.0f, 1.0f, 0.0f,   0.0f,  0.0f,

            5.0f, -0.5f,  5.0f,  0.0f, 1.0f, 0.0f, 5.0f,  0.0f,
            5.0f, -0.5f, -5.0f,  0.0f, 1.0f, 0.0f, 5.0f, 5.0f,
            -5.0f, -0.5f, -5.0f, 0.0f, 1.0f, 0.0f ,   0.0f, 5.0f
    };

    float ceilingVertices[] = {
            5.0f, 5.0f,  5.0f,  0.0f, -1.0f, 0.0f, 5.0f,  0.0f,
            -5.0f, 5.0f, -5.0f, 0.0f, -1.0f, 0.0f,   0.0f, 5.0f,
            -5.0f, 5.0f,  5.0f, 0.0f, -1.0f, 0.0f,   0.0f,  0.0f,

            5.0f, 5.0f,  5.0f,  0.0f, -1.0f, 0.0f, 5.0f,  0.0f,
            5.0f, 5.0f, -5.0f,  0.0f, -1.0f, 0.0f, 5.0f, 5.0f,
            -5.0f, 5.0f, -5.0f, 0.0f, -1.0f, 0.0f ,   0.0f, 5.0f
    };


    float leftWallVertices[] = {
            -5.0f, -0.5f,  5.0f,  1.0f, 0.0f, 0.0f, 0.0f,  0.0f,
            -5.0f, 5.0f, 5.0f, 1.0f, 0.0f, 0.0f,   0.0f, 5.0f,
            -5.0f, -0.5f,  -5.0f, 1.0f, 0.0f, 0.0f,   5.0f,  0.0f,

            -5.0f, -0.5f,  -5.0f,  1.0f, 0.0f, 0.0f, 5.0f,  0.0f,
            -5.0f, 5.0f, -5.0f,  1.0f, 0.0f, 0.0f, 5.0f, 5.0f,
            -5.0f, 5.0f, 5.0f, 1.0f, 0.0f, 0.0f,   0.0f, 5.0f
    };
    float rightWallVertices[] = {
            5.0f, -0.5f,  5.0f,  -1.0f, 0.0f, 0.0f, 5.0f,  0.0f,
            5.0f, 5.0f, 5.0f, -1.0f, 0.0f, 0.0f,   5.0f, 5.0f,
            5.0f, -0.5f,  -5.0f, -1.0f, 0.0f, 0.0f,   0.0f,  0.0f,

            5.0f, -0.5f,  -5.0f,  -1.0f, 0.0f, 0.0f, 5.0f,  0.0f,
            5.0f, 5.0f, -5.0f,  -1.0f, 0.0f, 0.0f, 5.0f, 5.0f,
            5.0f, 5.0f, 5.0f, -1.0f, 0.0f, 0.0f,   0.0f, 5.0f
    };
    float backWallVertices[] = {
            -5.0f, -0.5f,  -5.0f,  0.0f, 0.0f, 1.0f, 0.0f,  0.0f,
            -5.0f, 5.0f, -5.0f, 0.0f, 0.0f, 1.0f,   0.0f, 5.0f,
            5.0f, -0.5f,  -5.0f, 0.0f, 0.0f, 1.0f,   5.0f,  0.0f,

            -5.0f, 5.0f,  -5.0f,  0.0f, 0.0f, 1.0f, 0.0f,  5.0f,
            5.0f, 5.0f, -5.0f,  0.0f, 0.0f, 1.0f, 5.0f, 5.0f,
            5.0f, -0.5f, -5.0f, 0.0f, 0.0f, 1.0f,   5.0f, 0.0f
    };

    float frontWallVertices[] = {
            -5.0f, -0.5f,  5.0f,  0.0f, 0.0f, -1.0f, 0.0f,  0.0f,
            -5.0f, 5.0f, 5.0f, 0.0f, 0.0f, -1.0f,   0.0f, 5.0f,
            5.0f, -0.5f,  5.0f, 0.0f, 0.0f, -1.0f,   5.0f,  0.0f,

            -5.0f, 5.0f,  5.0f,  0.0f, 0.0f, -1.0f, 0.0f,  5.0f,
            5.0f, 5.0f, 5.0f,  0.0f, 0.0f, -1.0f, 5.0f, 5.0f,
            5.0f, -0.5f, 5.0f, 0.0f, 0.0f, -1.0f,   5.0f, 0.0f
    };

    float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
            // positions   // texCoords
            -1.0f,  1.0f,  0.0f, 1.0f,
            -1.0f, -1.0f,  0.0f, 0.0f,
            1.0f, -1.0f,  1.0f, 0.0f,

            -1.0f,  1.0f,  0.0f, 1.0f,
            1.0f, -1.0f,  1.0f, 0.0f,
            1.0f,  1.0f,  1.0f, 1.0f
    };

    float transparentVertices[] = {
            // positions         //normals                                  // texture Coords (swapped y coordinates because texture is flipped upside down)
            0.0f,  0.5f,  0.0f,  0.0f, 0.0f, 1.0f,0.0f,  0.0f,
            0.0f, -0.5f,  0.0f,0.0f, 0.0f, 1.0f,  0.0f,  1.0f,
            1.0f, -0.5f,  0.0f, 0.0f, 0.0f, 1.0f, 1.0f,  1.0f,

            0.0f,  0.5f,  0.0f, 0.0f, 0.0f, 1.0f, 0.0f,  0.0f,
            1.0f, -0.5f,  0.0f, 0.0f, 0.0f, 1.0f, 1.0f,  1.0f,
            1.0f,  0.5f,  0.0f, 0.0f, 0.0f, 1.0f, 1.0f,  0.0f
    };


    // cube VAO
    unsigned int cubeVAO, cubeVBO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    // plane VAO
    unsigned int planeVAO, planeVBO;
    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);
    glBindVertexArray(planeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE, 8 * sizeof(float),(void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE, 8 * sizeof(float),(void*)(3* sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float),(void*)(6* sizeof(float)));
    glEnableVertexAttribArray(2);

    // Wall VAO
    unsigned int WallVAO, WallVBO;
    glGenVertexArrays(1, &WallVAO);
    glGenBuffers(1, &WallVBO);
    glBindVertexArray(WallVAO);
    glBindBuffer(GL_ARRAY_BUFFER, WallVBO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE, 8 * sizeof(float),(void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE, 8 * sizeof(float),(void*)(3* sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float),(void*)(6* sizeof(float)));
    glEnableVertexAttribArray(2);
    // screen quad VAO
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
    // transparent VAO
    unsigned int transparentVAO, transparentVBO;
    glGenVertexArrays(1, &transparentVAO);
    glGenBuffers(1, &transparentVBO);
    glBindVertexArray(transparentVAO);
    glBindBuffer(GL_ARRAY_BUFFER, transparentVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(transparentVertices), transparentVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE, 8 * sizeof(float),(void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE, 8 * sizeof(float),(void*)(3* sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float),(void*)(6* sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    // load textures
    // -------------
    unsigned int cubeTextureDiffuse = loadTexture(FileSystem::getPath("resources/textures/rust_diffuse.jpg").c_str());
    unsigned int cubeTextureSpecular = loadTexture(FileSystem::getPath("resources/textures/rust_specular.jpg").c_str());
    unsigned int floorTextureDiffuse = loadTexture(FileSystem::getPath("resources/textures/concrete_diffuse.jpg").c_str());
    unsigned int floorTextureSpecular = loadTexture(FileSystem::getPath("resources/textures/concrete_specular.jpg").c_str());
    unsigned int ceilingTextureDiffuse = loadTexture(FileSystem::getPath("resources/textures/ceiling_diffuse.jpg").c_str());
    unsigned int ceilingTextureSpecular = loadTexture(FileSystem::getPath("resources/textures/ceiling_specular.jpg").c_str());
    unsigned int wallTextureDiffuse = loadTexture(FileSystem::getPath("resources/textures/stonebrick_diffuse.jpg").c_str());
    unsigned int wallTextureSpecular = loadTexture(FileSystem::getPath("resources/textures/stonebrick_specular.jpg").c_str());
    unsigned int transparentTextureDiffuse = loadTexture(FileSystem::getPath("resources/textures/caution_diffuse.png").c_str());
    unsigned int transparentTextureSpecular = loadTexture(FileSystem::getPath("resources/textures/caution_specular.png").c_str());

    // shader configuration
    // --------------------
   // shader.use();
    //shader.setInt("material.texture_diffuse1",0);
    //shader.setInt("material.texture_specular1", 1);

    screenShader.use();
    screenShader.setInt("screenTexture", 0);

    vector<glm::vec3> blending
            {
                    glm::vec3(-2.32f, 0.0f, -0.48f),
                    glm::vec3( 1.68f, 0.0f, -0.48f),

            };

    // framebuffer configuration
    // -------------------------
    unsigned int framebuffer;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    // create a color attachment texture
    unsigned int textureColorbuffer;
    glGenTextures(1, &textureColorbuffer);
    glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);
    // create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT); // use a single renderbuffer object for both a depth AND stencil buffer.
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); // now actually attach it
    // now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // draw as wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);


        // render
        // ------
        // bind to framebuffer and draw scene as we normally would to color texture
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glEnable(GL_DEPTH_TEST); // enable depth testing (is disabled for rendering screen-space quad)

        // make sure we clear the framebuffer's content
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();
        shader.setBool("blending",false);
        shader.setFloat("material.shininess", 32.0f);
        shader.setVec3("viewPosition", camera.Position);
        //dirLight
        shader.setVec3("dirLight.direction", dirLight.direction);
        shader.setVec3("dirLight.ambient", dirLight.ambient);
        shader.setVec3("dirLight.diffuse", dirLight.diffuse);
        shader.setVec3("dirLight.specular", dirLight.specular);
        //pointLight
        if(greenLight) {
            shader.setVec3("pointLight.ambient", glm::vec3(0.0f,0.2f,0.0f));
            shader.setVec3("pointLight.diffuse",  glm::vec3(0.0f,0.8,0.0f));
            shader.setVec3("pointLight.specular",  glm::vec3(0.0f,1.0f,0.0f));
        }
        else{

            shader.setVec3("pointLight.ambient", pointLight.ambient);
            shader.setVec3("pointLight.diffuse", pointLight.diffuse);
            shader.setVec3("pointLight.specular", pointLight.specular);
        }
        shader.setVec3("pointLight.position", pointLight.position);
        shader.setFloat("pointLight.constant", pointLight.constant);
        shader.setFloat("pointLight.linear", pointLight.linear);
        shader.setFloat("pointLight.quadratic", pointLight.quadratic);
        //spotLight
        shader.setBool("spotLightOn", spotLightOn);
        shader.setVec3("spotLight.position", camera.Position);
        shader.setVec3("spotLight.direction", camera.Front);
        shader.setVec3("spotLight.ambient", spotLight.ambient);
        shader.setVec3("spotLight.diffuse",spotLight.diffuse);
        shader.setVec3("spotLight.specular", spotLight.specular);
        shader.setFloat("spotLight.constant", spotLight.constant);
        shader.setFloat("spotLight.linear", spotLight.linear);
        shader.setFloat("spotLight.quadratic", spotLight.quadratic);
        shader.setFloat("spotLight.cutOff", spotLight.cutOff);
        shader.setFloat("spotLight.outerCutOff", spotLight.outerCutOff);

        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        shader.setMat4("view", view);
        shader.setMat4("projection", projection);
        // cubes
        glEnable(GL_CULL_FACE);
        glBindVertexArray(cubeVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, cubeTextureDiffuse);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, cubeTextureSpecular);
        model = glm::translate(model, glm::vec3(-2.0f, 0.0f, -1.0f));
        shader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(2.0f, 0.0f, -1.0f));
        shader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glDisable(GL_CULL_FACE);
        // blending
        shader.setBool("blending",true);
        glBindVertexArray(transparentVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, transparentTextureDiffuse);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, transparentTextureSpecular);
        for (unsigned int i = 0; i < blending.size(); i++)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, blending[i]);
            model = glm::scale(model, glm::vec3(0.7f, 0.7f, 0.7f));
            shader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
        shader.setBool("blending",false);
        // floor
        glBindVertexArray(planeVAO);
        glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(floorVertices), &floorVertices, GL_STATIC_DRAW);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, floorTextureDiffuse);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, floorTextureSpecular);
        shader.setMat4("model", glm::mat4(1.0f));
        glDrawArrays(GL_TRIANGLES, 0, 6);
        //ceiling
        glBufferData(GL_ARRAY_BUFFER, sizeof(ceilingVertices), &ceilingVertices, GL_STATIC_DRAW);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, ceilingTextureDiffuse);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, ceilingTextureSpecular);
        shader.setMat4("model", glm::mat4(1.0f));
        glDrawArrays(GL_TRIANGLES, 0, 6);
        //leftWall
        glBufferData(GL_ARRAY_BUFFER, sizeof(leftWallVertices), &leftWallVertices, GL_STATIC_DRAW);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, wallTextureDiffuse);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, wallTextureSpecular);
        shader.setMat4("model", glm::mat4(1.0f));
        glDrawArrays(GL_TRIANGLES, 0, 6);
        //rightWall
        glBufferData(GL_ARRAY_BUFFER, sizeof(rightWallVertices), &rightWallVertices, GL_STATIC_DRAW);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, wallTextureDiffuse);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, wallTextureSpecular);
        shader.setMat4("model", glm::mat4(1.0f));
        glDrawArrays(GL_TRIANGLES, 0, 6);
        //frontWall
        glBufferData(GL_ARRAY_BUFFER, sizeof(frontWallVertices), &frontWallVertices, GL_STATIC_DRAW);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, wallTextureDiffuse);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, wallTextureSpecular);
        shader.setMat4("model", glm::mat4(1.0f));
        glDrawArrays(GL_TRIANGLES, 0, 6);
        //backWall
        glBufferData(GL_ARRAY_BUFFER, sizeof(backWallVertices), &backWallVertices, GL_STATIC_DRAW);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, wallTextureDiffuse);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, wallTextureSpecular);
        shader.setMat4("model", glm::mat4(1.0f));
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        //model
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.25f, 0.5f)); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(0.15f, 0.15f, 0.15f));	// it's a bit too big for our scene, so scale it down
        shader.setMat4("model", model);
        ourModel.Draw(shader);

        // now bind back to default framebuffer and draw a quad plane with the attached framebuffer color texture
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDisable(GL_DEPTH_TEST); // disable depth test so screen-space quad isn't discarded due to depth test.
        // clear all relevant buffers
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // set clear color to white (not really necessary actually, since we won't be able to see behind the quad anyways)
        glClear(GL_COLOR_BUFFER_BIT);

        screenShader.use();
        screenShader.setBool("blur", blur);
        glBindVertexArray(quadVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureColorbuffer);	// use the color attachment texture as the texture of the quad plane
        glDrawArrays(GL_TRIANGLES, 0, 6);


        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteVertexArrays(1, &planeVAO);
    glDeleteVertexArrays(1, &quadVAO);
    glDeleteBuffers(1, &cubeVBO);
    glDeleteBuffers(1, &planeVBO);
    glDeleteBuffers(1, &quadVBO);

    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);

    if(glfwGetKey(window,GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS){
        blur = true;
    }
    if(glfwGetKey(window,GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE){
        blur = false;
    }
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {

    if (key == GLFW_KEY_X && action == GLFW_PRESS) {
        spotLightOn = !spotLightOn;

    }

    if(key == GLFW_KEY_G && action == GLFW_PRESS) {
        greenLight = !greenLight;
    }


}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
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

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(char const * path)
{
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

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
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