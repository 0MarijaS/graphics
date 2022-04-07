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
const unsigned int SCR_WIDTH = 1100;
const unsigned int SCR_HEIGHT = 850;

// camera
Camera camera(glm::vec3(0.0f, 1.4f, 4.95f));
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

int main() {
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
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL) {
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
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);


    // build and compile shaders
    // -------------------------
    Shader shader("resources/shaders/shader.vs", "resources/shaders/shader.fs");
    Shader normalMappingShader("resources/shaders/normalMappingShader.vs", "resources/shaders/normalMappingShader.fs");
    Shader screenShader("resources/shaders/framebufferScreenShader.vs", "resources/shaders/framebufferScreenShader.fs");

    Model ourModel(FileSystem::getPath("resources/objects/rust_gas/Gasoline_barrel.obj"));
    ourModel.SetShaderTextureNamePrefix("material.");

    pointLight.position = glm::vec3(0.0f, 3.5, 0.0);
    pointLight.ambient = glm::vec3(0.2, 0.2, 0.2);
    pointLight.diffuse = glm::vec3(0.7, 0.7, 0.7);
    pointLight.specular = glm::vec3(1.0, 1.0, 1.0);

    pointLight.constant = 1.0f;
    pointLight.linear = 0.09f;
    pointLight.quadratic = 0.032f;

    DirLight dirLight;
    dirLight.direction = glm::vec3(-0.2f, -1.0f, -0.3f);
    dirLight.ambient = glm::vec3(0.05f, 0.05f, 0.05f);
    dirLight.diffuse = glm::vec3(0.4f, 0.4f, 0.4f);
    dirLight.specular = glm::vec3(0.5f, 0.5f, 0.5f);

    SpotLight spotLight;
    spotLight.direction = glm::vec3(camera.Front);
    spotLight.position = glm::vec3(camera.Position);

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
            0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, // bottom-right
            0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f, // top-right
            0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f, // top-right
            -0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, // top-left
            -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
            // front face
            -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom-left
            0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, // top-right
            0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, // bottom-right
            0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, // top-right
            -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom-left
            -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, // top-left
            // left face
            -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-right
            -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-left
            -0.5f, 0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top-left
            -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-left
            -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-right
            -0.5f, -0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // bottom-right
            // right face
            0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-left
            0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top-right
            0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-right
            0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-right
            0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // bottom-left
            0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-left
            // bottom face
            -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // top-right
            0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, // bottom-left
            0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f, // top-left
            0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, // bottom-left
            -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // top-right
            -0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, // bottom-right
            // top face
            -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // top-left
            0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, // top-right
            0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom-right
            0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom-right
            -0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, // bottom-left
            -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f  // top-left
    };

//    float floorVertices[]{
//            5.0f, -0.5f, 5.0f, 0.0f, 1.0f, 0.0f, 5.0f, 0.0f,
//            -5.0f, -0.5f, -5.0f, 0.0f, 1.0f, 0.0f, 0.0f, 5.0f,
//            -5.0f, -0.5f, 5.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
//
//            5.0f, -0.5f, 5.0f, 0.0f, 1.0f, 0.0f, 5.0f, 0.0f,
//            5.0f, -0.5f, -5.0f, 0.0f, 1.0f, 0.0f, 5.0f, 5.0f,
//            -5.0f, -0.5f, -5.0f, 0.0f, 1.0f, 0.0f, 0.0f, 5.0f
//    };
//
//    float ceilingVertices[] = {
//            5.0f, 5.0f, 5.0f, 0.0f, -1.0f, 0.0f, 5.0f, 0.0f,
//            -5.0f, 5.0f, -5.0f, 0.0f, -1.0f, 0.0f, 0.0f, 5.0f,
//            -5.0f, 5.0f, 5.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f,
//
//            5.0f, 5.0f, 5.0f, 0.0f, -1.0f, 0.0f, 5.0f, 0.0f,
//            5.0f, 5.0f, -5.0f, 0.0f, -1.0f, 0.0f, 5.0f, 5.0f,
//            -5.0f, 5.0f, -5.0f, 0.0f, -1.0f, 0.0f, 0.0f, 5.0f
//    };


//    float leftWallVertices[] = {
//            -5.0f, -0.5f,  5.0f,  1.0f, 0.0f, 0.0f, 0.0f,  0.0f,
//            -5.0f, 5.0f, 5.0f, 1.0f, 0.0f, 0.0f,   0.0f, 5.0f,
//            -5.0f, -0.5f,  -5.0f, 1.0f, 0.0f, 0.0f,   5.0f,  0.0f,
//
//            -5.0f, -0.5f,  -5.0f,  1.0f, 0.0f, 0.0f, 5.0f,  0.0f,
//            -5.0f, 5.0f, -5.0f,  1.0f, 0.0f, 0.0f, 5.0f, 5.0f,
//            -5.0f, 5.0f, 5.0f, 1.0f, 0.0f, 0.0f,   0.0f, 5.0f
//    };
//    float rightWallVertices[] = {
//            5.0f, -0.5f, 5.0f, -1.0f, 0.0f, 0.0f, 5.0f, 0.0f,
//            5.0f, 5.0f, 5.0f, -1.0f, 0.0f, 0.0f, 5.0f, 5.0f,
//            5.0f, -0.5f, -5.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
//
//            5.0f, -0.5f, -5.0f, -1.0f, 0.0f, 0.0f, 5.0f, 0.0f,
//            5.0f, 5.0f, -5.0f, -1.0f, 0.0f, 0.0f, 5.0f, 5.0f,
//            5.0f, 5.0f, 5.0f, -1.0f, 0.0f, 0.0f, 0.0f, 5.0f
//    };
//    float backWallVertices[] = {
//            -5.0f, -0.5f, -5.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
//            -5.0f, 5.0f, -5.0f, 0.0f, 0.0f, 1.0f, 0.0f, 5.0f,
//            5.0f, -0.5f, -5.0f, 0.0f, 0.0f, 1.0f, 5.0f, 0.0f,
//
//            -5.0f, 5.0f, -5.0f, 0.0f, 0.0f, 1.0f, 0.0f, 5.0f,
//            5.0f, 5.0f, -5.0f, 0.0f, 0.0f, 1.0f, 5.0f, 5.0f,
//            5.0f, -0.5f, -5.0f, 0.0f, 0.0f, 1.0f, 5.0f, 0.0f
//    };
//
//    float frontWallVertices[] = {
//            -5.0f, -0.5f, 5.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
//            -5.0f, 5.0f, 5.0f, 0.0f, 0.0f, -1.0f, 0.0f, 5.0f,
//            5.0f, -0.5f, 5.0f, 0.0f, 0.0f, -1.0f, 5.0f, 0.0f,
//
//            -5.0f, 5.0f, 5.0f, 0.0f, 0.0f, -1.0f, 0.0f, 5.0f,
//            5.0f, 5.0f, 5.0f, 0.0f, 0.0f, -1.0f, 5.0f, 5.0f,
//            5.0f, -0.5f, 5.0f, 0.0f, 0.0f, -1.0f, 5.0f, 0.0f
//    };

    float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
            // positions   // texCoords
            -1.0f, 1.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f,
            1.0f, -1.0f, 1.0f, 0.0f,

            -1.0f, 1.0f, 0.0f, 1.0f,
            1.0f, -1.0f, 1.0f, 0.0f,
            1.0f, 1.0f, 1.0f, 1.0f
    };

    float transparentVertices[] = {
            // positions         //normals                                  // texture Coords (swapped y coordinates because texture is flipped upside down)
            0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
            1.0f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,

            0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
            1.0f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
            1.0f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f
    };


    // cube VAO
    unsigned int cubeVAO, cubeVBO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) (3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) (6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    //normalQuad VAO
    unsigned int quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void *) (3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void *) (6 * sizeof(float)));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void *) (8 * sizeof(float)));
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void *) (11 * sizeof(float)));

    // screen quad VAO
    unsigned int screenQuadVAO, screenQuadVBO;
    glGenVertexArrays(1, &screenQuadVAO);
    glGenBuffers(1, &screenQuadVBO);
    glBindVertexArray(screenQuadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, screenQuadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *) (2 * sizeof(float)));

    // transparent VAO
    unsigned int transparentVAO, transparentVBO;
    glGenVertexArrays(1, &transparentVAO);
    glGenBuffers(1, &transparentVBO);
    glBindVertexArray(transparentVAO);
    glBindBuffer(GL_ARRAY_BUFFER, transparentVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(transparentVertices), transparentVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) (3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) (6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    // load textures
    // -------------
    unsigned int cubeTextureDiffuse = loadTexture(FileSystem::getPath("resources/textures/rust_diffuse.jpg").c_str());
    unsigned int cubeTextureSpecular = loadTexture(FileSystem::getPath("resources/textures/rust_specular.jpg").c_str());
    unsigned int ceilingTextureDiffuse = loadTexture(FileSystem::getPath("resources/textures/ceiling_diffuse.jpg").c_str());
    unsigned int ceilingTextureSpecular = loadTexture(FileSystem::getPath("resources/textures/ceiling_specular.jpg").c_str());
    unsigned int ceilingTextureNormal = loadTexture(FileSystem::getPath("resources/textures/ceiling_normal.jpg").c_str());
    unsigned int floorTextureDiffuse = loadTexture(FileSystem::getPath("resources/textures/concrete_wall.jpg").c_str());
    unsigned int floorTextureSpecular = loadTexture(FileSystem::getPath("resources/textures/concrete_wall_specular.jpg").c_str());
    unsigned int floorTextureNormal = loadTexture(FileSystem::getPath("resources/textures/concrete_wall_normal.jpg").c_str());
    unsigned int transparentTextureDiffuse = loadTexture(FileSystem::getPath("resources/textures/caution_diffuse.png").c_str());
    unsigned int transparentTextureSpecular = loadTexture(FileSystem::getPath("resources/textures/caution_specular.png").c_str());
    unsigned int wallTextureDiffuse = loadTexture(FileSystem::getPath("resources/textures/BRICKS.jpg").c_str());
    unsigned int wallTextureSpecular = loadTexture(FileSystem::getPath("resources/textures/BRICKS_SPEC.jpg").c_str());
    unsigned int wallTextureNormal = loadTexture(FileSystem::getPath("resources/textures/BRICKS_NORM.jpg").c_str());
    unsigned int wallTextureDisplacement = loadTexture(FileSystem::getPath("resources/textures/BRICKS_DISP.jpg").c_str());

    screenShader.use();
    screenShader.setInt("screenTexture", 0);

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
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH,
                          SCR_HEIGHT); // use a single renderbuffer object for both a depth AND stencil buffer.
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                              rbo); // now actually attach it
    // now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    // render loop
    // -----------
    while (!glfwWindowShouldClose(window)) {
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
        shader.setBool("blending", false);
        shader.setFloat("material.shininess", 32.0f);
        shader.setVec3("viewPosition", camera.Position);
        //dirLight
        shader.setVec3("dirLight.direction", dirLight.direction);
        shader.setVec3("dirLight.ambient", dirLight.ambient);
        shader.setVec3("dirLight.diffuse", dirLight.diffuse);
        shader.setVec3("dirLight.specular", dirLight.specular);
        //pointLight
        if (greenLight) {
            shader.setVec3("pointLight.ambient", glm::vec3(0.0f, 0.2f, 0.0f));
            shader.setVec3("pointLight.diffuse", glm::vec3(0.0f, 0.8, 0.0f));
            shader.setVec3("pointLight.specular", glm::vec3(0.0f, 1.0f, 0.0f));
        } else {

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
        shader.setVec3("spotLight.diffuse", spotLight.diffuse);
        shader.setVec3("spotLight.specular", spotLight.specular);
        shader.setFloat("spotLight.constant", spotLight.constant);
        shader.setFloat("spotLight.linear", spotLight.linear);
        shader.setFloat("spotLight.quadratic", spotLight.quadratic);
        shader.setFloat("spotLight.cutOff", spotLight.cutOff);
        shader.setFloat("spotLight.outerCutOff", spotLight.outerCutOff);

        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float) SCR_WIDTH / (float) SCR_HEIGHT, 0.1f,100.0f);
        shader.setMat4("view", view);
        shader.setMat4("projection", projection);

        // cubes
        glEnable(GL_CULL_FACE);
        glFrontFace(GL_CW);
        glBindVertexArray(cubeVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, cubeTextureDiffuse);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, cubeTextureSpecular);
        model = glm::translate(model, glm::vec3(-2.0f, 0.0f, -1.5f));
        model = glm::scale(model, glm::vec3(1.3, 1.3, 1.3));
        shader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(2.0f, 0.0f, -1.5f));
        model = glm::scale(model, glm::vec3(1.3, 1.3, 1.3));
        shader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glDisable(GL_CULL_FACE);
        // blending
        shader.setBool("blending", true);
        glBindVertexArray(transparentVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, transparentTextureDiffuse);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, transparentTextureSpecular);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(1.54f,0.1f,-0.83));
        model = glm::scale(model, glm::vec3(0.9, 0.9, 0.9));
        shader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-2.44f,0.1f,-0.83));
        model = glm::scale(model, glm::vec3(0.9, 0.9, 0.9));
        shader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        shader.setBool("blending", false);

        //normalMapping
        normalMappingShader.use();
        normalMappingShader.setVec3("viewPos", camera.Position);
        normalMappingShader.setVec3("lightPos", pointLight.position);
        normalMappingShader.setVec3(" lightPosSpotLight", camera.Position);
        normalMappingShader.setFloat("material.shininess", 32.0f);
        //dirLight
        normalMappingShader.setVec3("dirLight.direction", dirLight.direction);
        normalMappingShader.setVec3("dirLight.ambient", dirLight.ambient);
        normalMappingShader.setVec3("dirLight.diffuse", dirLight.diffuse);
        normalMappingShader.setVec3("dirLight.specular", dirLight.specular);
        //pointLight
        if (greenLight) {
            normalMappingShader.setVec3("pointLight.ambient", glm::vec3(0.0f, 0.2f, 0.0f));
            normalMappingShader.setVec3("pointLight.diffuse", glm::vec3(0.0f, 0.8, 0.0f));
            normalMappingShader.setVec3("pointLight.specular", glm::vec3(0.0f, 1.0f, 0.0f));
        } else {

            normalMappingShader.setVec3("pointLight.ambient", pointLight.ambient);
            normalMappingShader.setVec3("pointLight.diffuse", pointLight.diffuse);
            normalMappingShader.setVec3("pointLight.specular", pointLight.specular);
        }
        normalMappingShader.setVec3("pointLight.position", pointLight.position);
        normalMappingShader.setFloat("pointLight.constant", pointLight.constant);
        normalMappingShader.setFloat("pointLight.linear", pointLight.linear);
        normalMappingShader.setFloat("pointLight.quadratic", pointLight.quadratic);
        //spotLight
        normalMappingShader.setBool("spotLightOn", spotLightOn);
        normalMappingShader.setVec3("spotLight.position", camera.Position);
        normalMappingShader.setVec3("spotLight.direction", camera.Front);
        normalMappingShader.setVec3("spotLight.ambient", spotLight.ambient);
        normalMappingShader.setVec3("spotLight.diffuse", spotLight.diffuse);
        normalMappingShader.setVec3("spotLight.specular", spotLight.specular);
        normalMappingShader.setFloat("spotLight.constant", spotLight.constant);
        normalMappingShader.setFloat("spotLight.linear", spotLight.linear);
        normalMappingShader.setFloat("spotLight.quadratic", spotLight.quadratic);
        normalMappingShader.setFloat("spotLight.cutOff", spotLight.cutOff);
        normalMappingShader.setFloat("spotLight.outerCutOff", spotLight.outerCutOff);

        normalMappingShader.setMat4("projection", projection);
        normalMappingShader.setMat4("view", view);
        normalMappingShader.setMat4("model", glm::mat4(1.0f));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, wallTextureDiffuse);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, wallTextureSpecular);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, wallTextureNormal);


        glm::vec3 pos1(-5.0f, 5.0f, 5.0f);
        glm::vec3 pos2(-5.0f, -0.5f, 5.0f);
        glm::vec3 pos3( -5.0f, -0.5f, -5.0f);
        glm::vec3 pos4( -5.0f, 5.0f, -5.0f);
        // texture coordinates
        glm::vec2 uv1(0.0f, 5.5f);
        glm::vec2 uv2(0.0f, 0.0f);
        glm::vec2 uv3(10.0f, 0.0f);
        glm::vec2 uv4(10.0f, 5.5f);
        // normal vector
        glm::vec3 nm(1.0f, 0.0f, 0.0f);

        // calculate tangent/bitangent vectors of both triangles
        glm::vec3 tangent1, bitangent1;
        glm::vec3 tangent2, bitangent2;
        // triangle 1
        // ----------
        glm::vec3 edge1 = pos2 - pos1;
        glm::vec3 edge2 = pos3 - pos1;
        glm::vec2 deltaUV1 = uv2 - uv1;
        glm::vec2 deltaUV2 = uv3 - uv1;

        float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        tangent1.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent1.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent1.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

        bitangent1.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        bitangent1.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        bitangent1.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

        // triangle 2
        // ----------
        edge1 = pos3 - pos1;
        edge2 = pos4 - pos1;
        deltaUV1 = uv3 - uv1;
        deltaUV2 = uv4 - uv1;

        f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        tangent2.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent2.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent2.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);


        bitangent2.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        bitangent2.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        bitangent2.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);


        float vertices[] = {
                // positions            // normal         // texcoords  // tangent                          // bitangent
                pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
                pos2.x, pos2.y, pos2.z, nm.x, nm.y, nm.z, uv2.x, uv2.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
                pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,

                pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
                pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
                pos4.x, pos4.y, pos4.z, nm.x, nm.y, nm.z, uv4.x, uv4.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z
        };

        //leftWall
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        //rightWall
        model = glm::mat4(1.0f);
       // model=glm::translate(model, glm::vec3(10.0f, 0.0f, 0.0f));
        model=glm::rotate(model, glm::radians(180.0f), glm::normalize(glm::vec3(0.0, 1.0, 0.0)));
        normalMappingShader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        //frontWall
        model = glm::mat4(1.0f);
        //model=glm::translate(model, glm::vec3(5.0f, 0.0f, 5.0f));
        model=glm::rotate(model, glm::radians(90.0f), glm::normalize(glm::vec3(0.0, 1.0, 0.0)));
        normalMappingShader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        //backWall
        model = glm::mat4(1.0f);
        //model=glm::translate(model, glm::vec3(5.0f, 0.0f, -5.0f));
        model=glm::rotate(model, glm::radians(270.0f), glm::normalize(glm::vec3(0.0, 1.0, 0.0)));
        normalMappingShader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 6);


        //floor
        pos1 = glm::vec3 (-5.0f, -0.5f, -5.0f);
        pos2 = glm::vec3 (-5.0f, -0.5f, 5.0f);
        pos3 = glm::vec3 ( 5.0f, -0.5f, 5.0f);
        pos4 = glm::vec3 ( 5.0f, -0.5f, -5.0f);
        // texture coordinates
        uv1 = glm::vec2 (0.0f, 10.0f);
        uv2 = glm::vec2 (0.0f, 0.0f);
        uv3 = glm::vec2 (10.0f, 0.0f);
        uv4 = glm::vec2(10.0f, 10.f);
        // normal vector
        nm = glm::vec3 (0.0f, 1.0f, 0.0f);

        // calculate tangent/bitangent vectors of both triangles
        // triangle 1
        // ----------
        edge1 = pos2 - pos1;
        edge2 = pos3 - pos1;
        deltaUV1 = uv2 - uv1;
        deltaUV2 = uv3 - uv1;

        f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        tangent1.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent1.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent1.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

        bitangent1.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        bitangent1.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        bitangent1.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

        // triangle 2
        // ----------
        edge1 = pos3 - pos1;
        edge2 = pos4 - pos1;
        deltaUV1 = uv3 - uv1;
        deltaUV2 = uv4 - uv1;

        f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        tangent2.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent2.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent2.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);


        bitangent2.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        bitangent2.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        bitangent2.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);


        float vertices2[] = {
                // positions            // normal         // texcoords  // tangent                          // bitangent
                pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
                pos2.x, pos2.y, pos2.z, nm.x, nm.y, nm.z, uv2.x, uv2.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
                pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,

                pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
                pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
                pos4.x, pos4.y, pos4.z, nm.x, nm.y, nm.z, uv4.x, uv4.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z
        };

        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices2), &vertices2, GL_STATIC_DRAW);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, floorTextureDiffuse);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, floorTextureSpecular);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, floorTextureNormal);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        //ceiling
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, ceilingTextureDiffuse);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, ceilingTextureSpecular);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, ceilingTextureNormal);
        model = glm::mat4(1.0f);
        model=glm::translate(model, glm::vec3(0.0f, 4.5f, 0.0f));
        model=glm::rotate(model, glm::radians(180.0f), glm::normalize(glm::vec3(1.0, 0.0, 0.0)));
        normalMappingShader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 6);


        //model
        glEnable(GL_CULL_FACE);
        glFrontFace(GL_CCW);
        model = glm::mat4(1.0f);
        model = glm::translate(model,glm::vec3(0.0f, 0.25f, -3.0f)); // translate it down so it's at the center of the scene
        model = glm::rotate(model, glm::radians(172.0f), glm::normalize(glm::vec3(0.0, 1.0, 0.0)));
        model = glm::scale(model,glm::vec3(0.2f, 0.25f, 0.2f));    // it's a bit too big for our scene, so scale it down
        normalMappingShader.setMat4("model", model);
        ourModel.Draw(normalMappingShader);
        glDisable(GL_CULL_FACE);

        // now bind back to default framebuffer and draw a quad plane with the attached framebuffer color texture
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDisable(GL_DEPTH_TEST); // disable depth test so screen-space quad isn't discarded due to depth test.
        // clear all relevant buffers
        glClearColor(1.0f, 1.0f, 1.0f,1.0f); // set clear color to white (not really necessary actually, since we won't be able to see behind the quad anyways)
        glClear(GL_COLOR_BUFFER_BIT);

        screenShader.use();
        screenShader.setBool("blur", blur);
        glBindVertexArray(screenQuadVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D,textureColorbuffer);    // use the color attachment texture as the texture of the quad plane
        glDrawArrays(GL_TRIANGLES, 0, 6);


        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteVertexArrays(1, &quadVAO);
    glDeleteVertexArrays(1, &screenQuadVAO);
    glDeleteBuffers(1, &cubeVBO);
    glDeleteBuffers(1, &quadVBO);
    glDeleteBuffers(1, &screenQuadVBO);

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
