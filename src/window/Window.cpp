/**
 * @file Window.cpp
 * @brief VoxelCraft Window System - Window Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#include "Window.hpp"
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <iostream>

namespace VoxelCraft {

    // Static callback storage for GLFW
    static Window* s_currentWindow = nullptr;

    Window::Window(const WindowProperties& properties)
        : m_window(nullptr)
        , m_properties(properties)
        , m_initialized(false)
    {
    }

    Window::~Window() {
        Shutdown();
    }

    bool Window::Initialize() {
        if (m_initialized) {
            return true;
        }

        if (!InitializeGLFW()) {
            return false;
        }

        if (!CreateWindow()) {
            return false;
        }

        if (!SetupOpenGL()) {
            return false;
        }

        SetupCallbacks();
        m_initialized = true;

        std::cout << "Window initialized successfully: " << m_properties.width << "x" << m_properties.height << std::endl;
        return true;
    }

    void Window::Shutdown() {
        if (!m_initialized) {
            return;
        }

        if (m_window) {
            glfwDestroyWindow(m_window);
            m_window = nullptr;
        }

        glfwTerminate();
        m_initialized = false;
    }

    void Window::Update() {
        if (!m_initialized) {
            return;
        }

        glfwPollEvents();
    }

    void Window::Present() {
        if (!m_initialized) {
            return;
        }

        glfwSwapBuffers(m_window);
    }

    bool Window::ShouldClose() const {
        if (!m_initialized || !m_window) {
            return true;
        }

        return glfwWindowShouldClose(m_window);
    }

    void Window::SetTitle(const std::string& title) {
        if (!m_initialized || !m_window) {
            return;
        }

        m_properties.title = title;
        glfwSetWindowTitle(m_window, title.c_str());
    }

    void Window::SetSize(int width, int height) {
        if (!m_initialized || !m_window) {
            return;
        }

        m_properties.width = width;
        m_properties.height = height;
        glfwSetWindowSize(m_window, width, height);
    }

    void Window::SetMode(WindowMode mode) {
        if (!m_initialized || !m_window) {
            return;
        }

        m_properties.mode = mode;

        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* videoMode = glfwGetVideoMode(monitor);

        switch (mode) {
            case WindowMode::WINDOWED:
                glfwSetWindowMonitor(m_window, nullptr, 100, 100, m_properties.width, m_properties.height, GLFW_DONT_CARE);
                break;

            case WindowMode::FULLSCREEN:
                glfwSetWindowMonitor(m_window, monitor, 0, 0, videoMode->width, videoMode->height, videoMode->refreshRate);
                break;

            case WindowMode::BORDERLESS:
                glfwSetWindowMonitor(m_window, nullptr, 0, 0, videoMode->width, videoMode->height, GLFW_DONT_CARE);
                break;
        }
    }

    void Window::SetVSync(bool enabled) {
        if (!m_initialized) {
            return;
        }

        m_properties.vsync = enabled;
        glfwSwapInterval(enabled ? 1 : 0);
    }

    void Window::MakeCurrent() {
        if (!m_initialized || !m_window) {
            return;
        }

        glfwMakeContextCurrent(m_window);
    }

    bool Window::InitializeGLFW() {
        if (!glfwInit()) {
            std::cerr << "Failed to initialize GLFW" << std::endl;
            return false;
        }

        // Set OpenGL version
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        // Window hints
        glfwWindowHint(GLFW_RESIZABLE, m_properties.resizable ? GLFW_TRUE : GLFW_FALSE);
        glfwWindowHint(GLFW_SAMPLES, m_properties.samples);

        return true;
    }

    bool Window::CreateWindow() {
        // Store this window as current for callbacks
        s_currentWindow = this;

        // Create window
        if (m_properties.mode == WindowMode::FULLSCREEN) {
            GLFWmonitor* monitor = glfwGetPrimaryMonitor();
            const GLFWvidmode* videoMode = glfwGetVideoMode(monitor);
            m_window = glfwCreateWindow(videoMode->width, videoMode->height,
                                      m_properties.title.c_str(), monitor, nullptr);
        } else {
            m_window = glfwCreateWindow(m_properties.width, m_properties.height,
                                      m_properties.title.c_str(), nullptr, nullptr);
        }

        if (!m_window) {
            std::cerr << "Failed to create GLFW window" << std::endl;
            glfwTerminate();
            return false;
        }

        // Make context current
        glfwMakeContextCurrent(m_window);

        // Set VSync
        glfwSwapInterval(m_properties.vsync ? 1 : 0);

        return true;
    }

    bool Window::SetupOpenGL() {
        // Load OpenGL functions using GLAD
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            std::cerr << "Failed to initialize GLAD" << std::endl;
            return false;
        }

        // Set viewport
        glViewport(0, 0, m_properties.width, m_properties.height);

        // Enable depth test
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

        // Enable backface culling
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        // Enable MSAA if requested
        if (m_properties.samples > 1) {
            glEnable(GL_MULTISAMPLE);
        }

        // Set clear color
        glClearColor(0.2f, 0.3f, 0.8f, 1.0f);

        std::cout << "OpenGL initialized successfully" << std::endl;
        std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
        std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

        return true;
    }

    void Window::SetupCallbacks() {
        if (!m_window) {
            return;
        }

        glfwSetWindowUserPointer(m_window, this);

        glfwSetKeyCallback(m_window, GLFWKeyCallback);
        glfwSetCursorPosCallback(m_window, GLFWMousePosCallback);
        glfwSetMouseButtonCallback(m_window, GLFWMouseButtonCallback);
        glfwSetScrollCallback(m_window, GLFWScrollCallback);
        glfwSetFramebufferSizeCallback(m_window, GLFWResizeCallback);
        glfwSetWindowCloseCallback(m_window, GLFWCloseCallback);
    }

    // GLFW Callback implementations
    void Window::GLFWKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
        Window* wnd = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if (wnd && wnd->m_callbacks.keyCallback) {
            wnd->m_callbacks.keyCallback(key, scancode, action, mods);
        }
    }

    void Window::GLFWMousePosCallback(GLFWwindow* window, double xpos, double ypos) {
        Window* wnd = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if (wnd && wnd->m_callbacks.mousePosCallback) {
            wnd->m_callbacks.mousePosCallback(xpos, ypos);
        }
    }

    void Window::GLFWMouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
        Window* wnd = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if (wnd && wnd->m_callbacks.mouseButtonCallback) {
            wnd->m_callbacks.mouseButtonCallback(button, action, mods);
        }
    }

    void Window::GLFWScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
        Window* wnd = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if (wnd && wnd->m_callbacks.scrollCallback) {
            wnd->m_callbacks.scrollCallback(xoffset, yoffset);
        }
    }

    void Window::GLFWResizeCallback(GLFWwindow* window, int width, int height) {
        Window* wnd = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if (wnd) {
            wnd->m_properties.width = width;
            wnd->m_properties.height = height;

            // Update viewport
            glViewport(0, 0, width, height);

            if (wnd->m_callbacks.resizeCallback) {
                wnd->m_callbacks.resizeCallback(width, height);
            }
        }
    }

    void Window::GLFWCloseCallback(GLFWwindow* window) {
        Window* wnd = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if (wnd && wnd->m_callbacks.closeCallback) {
            wnd->m_callbacks.closeCallback();
        }
    }

} // namespace VoxelCraft
