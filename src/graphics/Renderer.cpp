/**
 * @file Renderer.cpp
 * @brief VoxelCraft Graphics System - OpenGL Renderer Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#include "Renderer.hpp"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <algorithm>
#include <cmath>

// Simple math structures (we'll expand these later)
struct Vec3 {
    float x, y, z;
    Vec3(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z) {}
};

struct Vec4 {
    float x, y, z, w;
    Vec4(float x = 0, float y = 0, float z = 0, float w = 1) : x(x), y(y), z(z), w(w) {}
};

struct Mat4 {
    float data[16];

    Mat4() {
        // Identity matrix
        for (int i = 0; i < 16; ++i) data[i] = (i % 5 == 0) ? 1.0f : 0.0f;
    }

    // Simple translation matrix
    static Mat4 Translate(const Vec3& v) {
        Mat4 m;
        m.data[12] = v.x;
        m.data[13] = v.y;
        m.data[14] = v.z;
        return m;
    }

    // Simple scale matrix
    static Mat4 Scale(const Vec3& v) {
        Mat4 m;
        m.data[0] = v.x;
        m.data[5] = v.y;
        m.data[10] = v.z;
        return m;
    }
};

namespace VoxelCraft {

// Forward declarations for internal classes
class Shader {
public:
    unsigned int program;

    Shader() : program(0) {}
    ~Shader() {
        if (program) glDeleteProgram(program);
    }

    bool Compile(const std::string& vertexSource, const std::string& fragmentSource) {
        // Create vertex shader
        unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
        const char* vsrc = vertexSource.c_str();
        glShaderSource(vertexShader, 1, &vsrc, nullptr);
        glCompileShader(vertexShader);

        // Check vertex shader compilation
        int success;
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
            std::cerr << "Vertex shader compilation failed: " << infoLog << std::endl;
            return false;
        }

        // Create fragment shader
        unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        const char* fsrc = fragmentSource.c_str();
        glShaderSource(fragmentShader, 1, &fsrc, nullptr);
        glCompileShader(fragmentShader);

        // Check fragment shader compilation
        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
            std::cerr << "Fragment shader compilation failed: " << infoLog << std::endl;
            return false;
        }

        // Create program
        program = glCreateProgram();
        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);
        glLinkProgram(program);

        // Check program linking
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetProgramInfoLog(program, 512, nullptr, infoLog);
            std::cerr << "Shader program linking failed: " << infoLog << std::endl;
            return false;
        }

        // Clean up shaders
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        return true;
    }

    void Use() {
        if (program) glUseProgram(program);
    }

    void SetMat4(const std::string& name, const Mat4& mat) {
        if (program) {
            unsigned int loc = glGetUniformLocation(program, name.c_str());
            glUniformMatrix4fv(loc, 1, GL_FALSE, mat.data);
        }
    }

    void SetVec3(const std::string& name, const Vec3& vec) {
        if (program) {
            unsigned int loc = glGetUniformLocation(program, name.c_str());
            glUniform3f(loc, vec.x, vec.y, vec.z);
        }
    }

    void SetVec4(const std::string& name, const Vec4& vec) {
        if (program) {
            unsigned int loc = glGetUniformLocation(program, name.c_str());
            glUniform4f(loc, vec.x, vec.y, vec.z, vec.w);
        }
    }
};

/**
 * @enum CameraMode
 * @brief Different camera movement modes
 */
enum class CameraMode {
    FIRST_PERSON,    // First-person camera (Minecraft-style)
    THIRD_PERSON,    // Third-person camera
    FREE,           // Free movement camera
    ORBITAL         // Orbital camera around a point
};

/**
 * @enum CameraMovement
 * @brief Camera movement directions
 */
enum class CameraMovement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

/**
 * @class Camera
 * @brief Advanced 3D camera for first-person and other camera modes
 */
class Camera {
public:
    // Position and orientation
    Vec3 position;
    Vec3 rotation; // pitch (x), yaw (y), roll (z)
    Vec3 target;   // Look-at target for orbital mode

    // Camera properties
    float fov;
    float aspectRatio;
    float nearPlane;
    float farPlane;
    CameraMode mode;

    // Movement properties
    float movementSpeed;
    float mouseSensitivity;
    float zoom;              // For orbital mode
    float minZoom;          // Minimum zoom distance
    float maxZoom;          // Maximum zoom distance

    // Constraints
    float maxPitch;         // Maximum pitch angle (radians)
    float minPitch;         // Minimum pitch angle (radians)

    // Physics/interpolation
    Vec3 velocity;
    Vec3 targetPosition;
    Vec3 targetRotation;
    float interpolationSpeed; // 0-1, how fast to interpolate
    bool enableInterpolation;

    // Collision
    bool enableCollision;
    float collisionRadius;
    float heightOffset;      // Camera height above ground

    Camera()
        : position(0, 2, 5)    // Start above ground looking at origin
        , rotation(0, 0, 0)   // No rotation initially
        , target(0, 0, 0)
        , fov(70.0f)
        , aspectRatio(16.0f/9.0f)
        , nearPlane(0.1f)
        , farPlane(1000.0f)
        , mode(CameraMode::FIRST_PERSON)
        , movementSpeed(5.0f)
        , mouseSensitivity(0.1f)
        , zoom(5.0f)
        , minZoom(1.0f)
        , maxZoom(50.0f)
        , maxPitch(1.5f)      // ~85 degrees
        , minPitch(-1.5f)     // ~-85 degrees
        , velocity(0, 0, 0)
        , targetPosition(0, 2, 5)
        , targetRotation(0, 0, 0)
        , interpolationSpeed(0.1f)
        , enableInterpolation(false)
        , enableCollision(true)
        , collisionRadius(0.5f)
        , heightOffset(1.8f)  // Player height
        , forwardVector(0, 0, -1)
        , rightVector(1, 0, 0)
        , upVector(0, 1, 0)
        , vectorsDirty(true)
    {
        UpdateDirectionVectors();
    }

    /**
     * @brief Update camera (handle interpolation, physics)
     * @param deltaTime Time since last update
     */
    void Update(float deltaTime) {
        if (enableInterpolation) {
            Interpolate(deltaTime);
        }

        if (enableCollision) {
            HandleCollision();
        }

        if (vectorsDirty) {
            UpdateDirectionVectors();
        }
    }

    /**
     * @brief Process keyboard input for movement
     * @param direction Movement direction
     * @param deltaTime Time since last update
     */
    void ProcessKeyboard(CameraMovement direction, float deltaTime) {
        float velocity = movementSpeed * deltaTime;

        switch (direction) {
            case CameraMovement::FORWARD:
                position.x += forwardVector.x * velocity;
                position.z += forwardVector.z * velocity;
                if (mode == CameraMode::FIRST_PERSON) {
                    position.y += forwardVector.y * velocity;
                }
                break;
            case CameraMovement::BACKWARD:
                position.x -= forwardVector.x * velocity;
                position.z -= forwardVector.z * velocity;
                if (mode == CameraMode::FIRST_PERSON) {
                    position.y -= forwardVector.y * velocity;
                }
                break;
            case CameraMovement::LEFT:
                position.x -= rightVector.x * velocity;
                position.z -= rightVector.z * velocity;
                break;
            case CameraMovement::RIGHT:
                position.x += rightVector.x * velocity;
                position.z += rightVector.z * velocity;
                break;
            case CameraMovement::UP:
                position.y += velocity;
                break;
            case CameraMovement::DOWN:
                position.y -= velocity;
                break;
        }

        // Keep camera above ground in first-person mode
        if (mode == CameraMode::FIRST_PERSON) {
            position.y = std::max(position.y, heightOffset);
        }
    }

    /**
     * @brief Process mouse movement for rotation
     * @param xoffset Mouse X movement
     * @param yoffset Mouse Y movement
     * @param constrainPitch Whether to constrain pitch angle
     */
    void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true) {
        xoffset *= mouseSensitivity;
        yoffset *= mouseSensitivity;

        rotation.y += xoffset; // Yaw
        rotation.x -= yoffset; // Pitch

        if (constrainPitch) {
            rotation.x = std::max(minPitch, std::min(maxPitch, rotation.x));
        }

        // Wrap yaw around
        if (rotation.y > 2 * 3.14159f) rotation.y -= 2 * 3.14159f;
        if (rotation.y < -2 * 3.14159f) rotation.y += 2 * 3.14159f;

        vectorsDirty = true;
    }

    /**
     * @brief Process mouse scroll for zoom
     * @param yoffset Scroll amount
     */
    void ProcessMouseScroll(float yoffset) {
        if (mode == CameraMode::ORBITAL) {
            zoom -= yoffset;
            zoom = std::max(minZoom, std::min(maxZoom, zoom));
        } else {
            // Change FOV in other modes
            fov -= yoffset;
            fov = std::max(1.0f, std::min(120.0f, fov));
        }
    }

    /**
     * @brief Get the view matrix
     * @return View matrix
     */
    Mat4 GetViewMatrix() const {
        // Create rotation matrices
        Mat4 pitch = Mat4::Translate(Vec3(0, 0, 0)); // TODO: Implement rotation matrices
        Mat4 yaw = Mat4::Translate(Vec3(0, 0, 0));   // TODO: Implement rotation matrices

        // For now, use simple translation (camera at origin, world moves)
        Mat4 view = Mat4::Translate(Vec3(-position.x, -position.y, -position.z));

        return view;
    }

    /**
     * @brief Get the projection matrix
     * @return Projection matrix
     */
    Mat4 GetProjectionMatrix() const {
        // Simple perspective projection matrix
        float fovRad = fov * 3.14159f / 180.0f;
        float tanHalfFov = std::tan(fovRad / 2.0f);

        Mat4 proj;
        proj.data[0] = 1.0f / (aspectRatio * tanHalfFov);
        proj.data[5] = 1.0f / tanHalfFov;
        proj.data[10] = -(farPlane + nearPlane) / (farPlane - nearPlane);
        proj.data[11] = -1.0f;
        proj.data[14] = -(2.0f * farPlane * nearPlane) / (farPlane - nearPlane);
        proj.data[15] = 0.0f;

        return proj;
    }

    /**
     * @brief Get the camera's forward vector
     * @return Forward direction
     */
    Vec3 GetForwardVector() const {
        return forwardVector;
    }

    /**
     * @brief Get the camera's right vector
     * @return Right direction
     */
    Vec3 GetRightVector() const {
        return rightVector;
    }

    /**
     * @brief Get the camera's up vector
     * @return Up direction
     */
    Vec3 GetUpVector() const {
        return upVector;
    }

private:
    /**
     * @brief Calculate direction vectors from rotation
     */
    void UpdateDirectionVectors() {
        // Calculate forward vector
        forwardVector.x = std::sin(rotation.y) * std::cos(rotation.x);
        forwardVector.y = std::sin(rotation.x);
        forwardVector.z = std::cos(rotation.y) * std::cos(rotation.x);

        // Normalize forward vector
        float length = std::sqrt(forwardVector.x * forwardVector.x +
                                forwardVector.y * forwardVector.y +
                                forwardVector.z * forwardVector.z);
        if (length > 0) {
            forwardVector.x /= length;
            forwardVector.y /= length;
            forwardVector.z /= length;
        }

        // Calculate right vector (cross product of forward and world up)
        rightVector.x = forwardVector.z;
        rightVector.y = 0;
        rightVector.z = -forwardVector.x;

        // Normalize right vector
        length = std::sqrt(rightVector.x * rightVector.x + rightVector.z * rightVector.z);
        if (length > 0) {
            rightVector.x /= length;
            rightVector.z /= length;
        }

        // Calculate up vector (cross product of right and forward)
        upVector.x = rightVector.y * forwardVector.z - rightVector.z * forwardVector.y;
        upVector.y = rightVector.z * forwardVector.x - rightVector.x * forwardVector.z;
        upVector.z = rightVector.x * forwardVector.y - rightVector.y * forwardVector.x;

        vectorsDirty = false;
    }

    /**
     * @brief Handle camera collision and correction
     */
    void HandleCollision() {
        // Basic ground collision
        if (position.y < heightOffset) {
            position.y = heightOffset;
        }
    }

    /**
     * @brief Smooth interpolation between current and target position/rotation
     * @param deltaTime Time since last update
     */
    void Interpolate(float deltaTime) {
        // Interpolate position
        Vec3 posDiff = Vec3(targetPosition.x - position.x,
                           targetPosition.y - position.y,
                           targetPosition.z - position.z);

        position.x += posDiff.x * interpolationSpeed;
        position.y += posDiff.y * interpolationSpeed;
        position.z += posDiff.z * interpolationSpeed;

        // Interpolate rotation
        Vec3 rotDiff = Vec3(targetRotation.x - rotation.x,
                           targetRotation.y - rotation.y,
                           targetRotation.z - rotation.z);

        rotation.x += rotDiff.x * interpolationSpeed;
        rotation.y += rotDiff.y * interpolationSpeed;
        rotation.z += rotDiff.z * interpolationSpeed;

        vectorsDirty = true;
    }

    // Cached direction vectors
    mutable Vec3 forwardVector;
    mutable Vec3 rightVector;
    mutable Vec3 upVector;
    mutable bool vectorsDirty;
};

class Mesh {
public:
    unsigned int vao;
    unsigned int vbo;
    unsigned int ebo;
    unsigned int indexCount;
    bool hasIndices;

    Mesh() : vao(0), vbo(0), ebo(0), indexCount(0), hasIndices(false) {}
    ~Mesh() {
        if (vao) glDeleteVertexArrays(1, &vao);
        if (vbo) glDeleteBuffers(1, &vbo);
        if (ebo) glDeleteBuffers(1, &ebo);
    }

    void Render() {
        if (vao) {
            glBindVertexArray(vao);
            if (hasIndices) {
                glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
            } else {
                glDrawArrays(GL_TRIANGLES, 0, indexCount);
            }
            glBindVertexArray(0);
        }
    }
};

// Renderer implementation
Renderer::Renderer(std::shared_ptr<Window> window)
    : m_window(window)
    , m_currentMode(RenderMode::SOLID)
    , m_initialized(false)
    , m_wireframeMode(false)
    , m_vao(0)
    , m_vbo(0)
    , m_ebo(0)
{
    // Create default camera
    m_camera = std::make_shared<Camera>();
    m_camera->position = Vec3(0, 2, 5); // Position camera above ground looking at origin
}

Renderer::~Renderer() {
    Shutdown();
}

bool Renderer::Initialize() {
    if (m_initialized) {
        return true;
    }

    if (!m_window) {
        std::cerr << "No window provided to renderer" << std::endl;
        return false;
    }

    // Make window context current
    m_window->MakeCurrent();

    // Initialize GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return false;
    }

    // Initialize OpenGL
    InitializeOpenGL();

    // Initialize shaders
    InitializeShaders();

    // Setup buffers
    SetupBuffers();

    // Set camera aspect ratio
    if (m_window) {
        m_camera->aspectRatio = m_window->GetAspectRatio();
    }

    m_initialized = true;
    std::cout << "Renderer initialized successfully" << std::endl;
    return true;
}

void Renderer::Shutdown() {
    if (!m_initialized) {
        return;
    }

    // Clean up OpenGL resources
    if (m_vao) glDeleteVertexArrays(1, &m_vao);
    if (m_vbo) glDeleteBuffers(1, &m_vbo);
    if (m_ebo) glDeleteBuffers(1, &m_ebo);

    // Clear caches
    m_blockMeshes.clear();
    m_meshCache.clear();

    m_initialized = false;
}

void Renderer::BeginFrame() {
    if (!m_initialized) return;

    // Clear render queue
    m_renderQueue.clear();

    // Reset stats
    m_stats.drawCalls = 0;
    m_stats.vertices = 0;
    m_stats.triangles = 0;
}

void Renderer::EndFrame() {
    if (!m_initialized) return;

    // Update stats
    UpdateStats();
}

void Renderer::Render() {
    if (!m_initialized) return;

    // Sort render commands
    SortRenderCommands();

    // Render different passes
    RenderSolidPass();
    RenderTransparentPass();
    RenderUIPass();
}

void Renderer::Clear(const Vec4& color) {
    if (!m_initialized) return;

    glClearColor(color.x, color.y, color.z, color.w);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::SubmitCommand(const RenderCommand& command) {
    m_renderQueue.push_back(command);
}

void Renderer::SetCamera(std::shared_ptr<Camera> camera) {
    m_camera = camera;
    if (m_window) {
        m_camera->aspectRatio = m_window->GetAspectRatio();
    }
}

void Renderer::SetWireframeMode(bool enabled) {
    m_wireframeMode = enabled;
    if (enabled) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
}

void Renderer::SetVSync(bool enabled) {
    if (m_window) {
        m_window->SetVSync(enabled);
    }
}

std::shared_ptr<Mesh> Renderer::CreateCubeMesh(float size) {
    std::string key = "cube_" + std::to_string(size);

    // Check cache first
    auto it = m_meshCache.find(key);
    if (it != m_meshCache.end()) {
        return it->second;
    }

    // Create new cube mesh
    auto mesh = std::make_shared<Mesh>();

    // Cube vertices (position, normal, texCoord)
    float vertices[] = {
        // Front face
        -size, -size,  size,  0, 0, 1,  0, 0,
         size, -size,  size,  0, 0, 1,  1, 0,
         size,  size,  size,  0, 0, 1,  1, 1,
        -size,  size,  size,  0, 0, 1,  0, 1,

        // Back face
        -size, -size, -size,  0, 0, -1, 1, 0,
        -size,  size, -size,  0, 0, -1, 1, 1,
         size,  size, -size,  0, 0, -1, 0, 1,
         size, -size, -size,  0, 0, -1, 0, 0,

        // Left face
        -size,  size,  size, -1, 0, 0,  0, 1,
        -size,  size, -size, -1, 0, 0,  1, 1,
        -size, -size, -size, -1, 0, 0,  1, 0,
        -size, -size,  size, -1, 0, 0,  0, 0,

        // Right face
         size,  size,  size,  1, 0, 0,  1, 1,
         size, -size,  size,  1, 0, 0,  1, 0,
         size, -size, -size,  1, 0, 0,  0, 0,
         size,  size, -size,  1, 0, 0,  0, 1,

        // Top face
        -size,  size, -size,  0, 1, 0,  0, 1,
        -size,  size,  size,  0, 1, 0,  0, 0,
         size,  size,  size,  0, 1, 0,  1, 0,
         size,  size, -size,  0, 1, 0,  1, 1,

        // Bottom face
        -size, -size, -size,  0, -1, 0, 1, 1,
         size, -size, -size,  0, -1, 0, 0, 1,
         size, -size,  size,  0, -1, 0, 0, 0,
        -size, -size,  size,  0, -1, 0, 1, 0
    };

    // Cube indices
    unsigned int indices[] = {
        0, 1, 2,  0, 2, 3,    // front
        4, 5, 6,  4, 6, 7,    // back
        8, 9, 10, 8, 10, 11,  // left
        12, 13, 14, 12, 14, 15, // right
        16, 17, 18, 16, 18, 19, // top
        20, 21, 22, 20, 22, 23  // bottom
    };

    // Create VAO
    glGenVertexArrays(1, &mesh->vao);
    glBindVertexArray(mesh->vao);

    // Create VBO
    glGenBuffers(1, &mesh->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Create EBO
    glGenBuffers(1, &mesh->ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Set vertex attributes
    // Position (3 floats)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Normal (3 floats)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // TexCoord (2 floats)
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    mesh->indexCount = 36; // 36 indices for cube
    mesh->hasIndices = true;

    // Cache the mesh
    m_meshCache[key] = mesh;

    return mesh;
}

std::shared_ptr<Mesh> Renderer::CreateBlockMesh(BlockType blockType) {
    // For now, all blocks are cubes
    return CreateCubeMesh(0.5f); // Half size for blocks
}

std::shared_ptr<Shader> Renderer::LoadShader(const std::string& name,
                                           const std::string& vertexSource,
                                           const std::string& fragmentSource) {
    auto shader = std::make_shared<Shader>();
    if (shader->Compile(vertexSource, fragmentSource)) {
        return shader;
    }
    return nullptr;
}

void Renderer::InitializeOpenGL() {
    // Set viewport
    if (m_window) {
        glViewport(0, 0, m_window->GetWidth(), m_window->GetHeight());
    }

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Enable backface culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    // Enable MSAA if available
    glEnable(GL_MULTISAMPLE);

    // Set clear color
    glClearColor(0.2f, 0.3f, 0.8f, 1.0f);

    std::cout << "OpenGL initialized: " << glGetString(GL_VERSION) << std::endl;
}

void Renderer::InitializeShaders() {
    // Create default shader
    m_defaultShader = LoadShader("default",
                                GetDefaultVertexShader(),
                                GetDefaultFragmentShader());

    // Create block shader
    m_blockShader = LoadShader("block",
                              GetBlockVertexShader(),
                              GetBlockFragmentShader());

    if (!m_defaultShader) {
        std::cerr << "Failed to create default shader" << std::endl;
    }
}

void Renderer::SetupBuffers() {
    // Nothing to do here for now
}

void Renderer::RenderSolidPass() {
    if (!m_defaultShader) return;

    // Use default shader
    m_defaultShader->Use();

    // Set view and projection matrices
    if (m_camera) {
        m_defaultShader->SetMat4("view", m_camera->GetViewMatrix());
        m_defaultShader->SetMat4("projection", m_camera->GetProjectionMatrix());
    }

    // Render solid objects
    for (const auto& command : m_renderQueue) {
        if (command.mode != RenderMode::SOLID) continue;

        // Set model matrix
        m_defaultShader->SetMat4("model", command.transform);

        // Set color
        m_defaultShader->SetVec4("color", command.color);

        // Render mesh
        if (command.mesh) {
            command.mesh->Render();
            m_stats.drawCalls++;
        }
    }
}

void Renderer::RenderTransparentPass() {
    // TODO: Implement transparent rendering
}

void Renderer::RenderUIPass() {
    // TODO: Implement UI rendering
}

void Renderer::SortRenderCommands() {
    // Sort by render mode and distance
    std::sort(m_renderQueue.begin(), m_renderQueue.end(),
              [](const RenderCommand& a, const RenderCommand& b) {
                  if (a.mode != b.mode) return static_cast<int>(a.mode) < static_cast<int>(b.mode);
                  return a.distance > b.distance; // Back to front for transparency
              });
}

void Renderer::UpdateStats() {
    m_stats.vertices = 0;
    m_stats.triangles = m_stats.vertices / 3;

    // Calculate FPS (this is simplified)
    static float lastTime = 0.0f;
    float currentTime = static_cast<float>(glfwGetTime());
    float deltaTime = currentTime - lastTime;
    lastTime = currentTime;

    if (deltaTime > 0) {
        m_stats.fps = static_cast<int>(1.0f / deltaTime);
    }

    m_stats.renderTime = deltaTime * 1000.0f; // Convert to ms
}

std::string Renderer::GetDefaultVertexShader() {
    return R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec3 aNormal;
        layout (location = 2) in vec2 aTexCoord;

        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;

        out vec3 Normal;
        out vec2 TexCoord;
        out vec3 FragPos;

        void main() {
            FragPos = vec3(model * vec4(aPos, 1.0));
            Normal = mat3(transpose(inverse(model))) * aNormal;
            TexCoord = aTexCoord;

            gl_Position = projection * view * vec4(FragPos, 1.0);
        }
    )";
}

std::string Renderer::GetDefaultFragmentShader() {
    return R"(
        #version 330 core
        out vec4 FragColor;

        in vec3 Normal;
        in vec2 TexCoord;
        in vec3 FragPos;

        uniform vec4 color;

        void main() {
            // Simple lighting
            vec3 lightDir = normalize(vec3(1.0, 1.0, 0.5));
            float diff = max(dot(Normal, lightDir), 0.0);
            vec3 diffuse = diff * vec3(1.0, 1.0, 1.0);

            vec3 ambient = vec3(0.3, 0.3, 0.3);

            vec3 result = (ambient + diffuse) * color.rgb;
            FragColor = vec4(result, color.a);
        }
    )";
}

std::string Renderer::GetBlockVertexShader() {
    return GetDefaultVertexShader(); // Same as default for now
}

std::string Renderer::GetBlockFragmentShader() {
    return R"(
        #version 330 core
        out vec4 FragColor;

        in vec3 Normal;
        in vec2 TexCoord;
        in vec3 FragPos;

        uniform vec4 color;

        void main() {
            // Block-specific shading
            vec3 lightDir = normalize(vec3(1.0, 1.0, 0.5));
            float diff = max(dot(Normal, lightDir), 0.0);
            vec3 diffuse = diff * vec3(0.8, 0.8, 0.8);

            vec3 ambient = vec3(0.4, 0.4, 0.4);

            // Add some texture-like variation based on position
            float variation = fract(sin(dot(floor(FragPos.xz * 2.0), vec2(12.9898, 78.233))) * 43758.5453);
            vec3 texColor = color.rgb * (0.8 + 0.4 * variation);

            vec3 result = (ambient + diffuse) * texColor;
            FragColor = vec4(result, color.a);
        }
    )";
}

} // namespace VoxelCraft
