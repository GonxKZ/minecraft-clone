/**
 * @file Window.hpp
 * @brief VoxelCraft Window System - Game Window Management
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#ifndef VOXELCRAFT_WINDOW_WINDOW_HPP
#define VOXELCRAFT_WINDOW_WINDOW_HPP

#include <string>
#include <functional>
#include <memory>

struct GLFWwindow;

namespace VoxelCraft {

    /**
     * @enum WindowMode
     * @brief Window display modes
     */
    enum class WindowMode {
        WINDOWED,      // Windowed mode
        FULLSCREEN,    // Fullscreen mode
        BORDERLESS     // Borderless windowed mode
    };

    /**
     * @struct WindowProperties
     * @brief Window creation properties
     */
    struct WindowProperties {
        std::string title;
        int width;
        int height;
        WindowMode mode;
        bool resizable;
        bool vsync;
        int samples; // MSAA samples

        WindowProperties()
            : title("VoxelCraft")
            , width(1280)
            , height(720)
            , mode(WindowMode::WINDOWED)
            , resizable(true)
            , vsync(true)
            , samples(4)
        {}
    };

    /**
     * @struct WindowEventCallbacks
     * @brief Callback functions for window events
     */
    struct WindowEventCallbacks {
        std::function<void(int key, int scancode, int action, int mods)> keyCallback;
        std::function<void(double xpos, double ypos)> mousePosCallback;
        std::function<void(int button, int action, int mods)> mouseButtonCallback;
        std::function<void(double xoffset, double yoffset)> scrollCallback;
        std::function<void(int width, int height)> resizeCallback;
        std::function<void()> closeCallback;
    };

    /**
     * @class Window
     * @brief Main game window class
     */
    class Window {
    public:
        /**
         * @brief Constructor
         * @param properties Window properties
         */
        Window(const WindowProperties& properties);

        /**
         * @brief Destructor
         */
        ~Window();

        /**
         * @brief Initialize the window
         * @return true if successful
         */
        bool Initialize();

        /**
         * @brief Shutdown the window
         */
        void Shutdown();

        /**
         * @brief Update window (process events)
         */
        void Update();

        /**
         * @brief Present the window (swap buffers)
         */
        void Present();

        /**
         * @brief Check if window should close
         * @return true if window should close
         */
        bool ShouldClose() const;

        /**
         * @brief Get window width
         * @return Window width
         */
        int GetWidth() const { return m_properties.width; }

        /**
         * @brief Get window height
         * @return Window height
         */
        int GetHeight() const { return m_properties.height; }

        /**
         * @brief Get window aspect ratio
         * @return Aspect ratio
         */
        float GetAspectRatio() const {
            return static_cast<float>(m_properties.width) / m_properties.height;
        }

        /**
         * @brief Set window title
         * @param title New title
         */
        void SetTitle(const std::string& title);

        /**
         * @brief Set window size
         * @param width New width
         * @param height New height
         */
        void SetSize(int width, int height);

        /**
         * @brief Set window mode
         * @param mode New window mode
         */
        void SetMode(WindowMode mode);

        /**
         * @brief Set VSync
         * @param enabled Enable VSync
         */
        void SetVSync(bool enabled);

        /**
         * @brief Get native GLFW window handle
         * @return GLFW window pointer
         */
        GLFWwindow* GetNativeWindow() const { return m_window; }

        /**
         * @brief Set event callbacks
         * @param callbacks Event callbacks
         */
        void SetEventCallbacks(const WindowEventCallbacks& callbacks) {
            m_callbacks = callbacks;
        }

        /**
         * @brief Get window properties
         * @return Window properties
         */
        const WindowProperties& GetProperties() const { return m_properties; }

        /**
         * @brief Make window current context
         */
        void MakeCurrent();

        /**
         * @brief Check if window is initialized
         * @return true if initialized
         */
        bool IsInitialized() const { return m_initialized; }

    private:
        GLFWwindow* m_window;
        WindowProperties m_properties;
        WindowEventCallbacks m_callbacks;
        bool m_initialized;

        /**
         * @brief Initialize GLFW
         * @return true if successful
         */
        bool InitializeGLFW();

        /**
         * @brief Create GLFW window
         * @return true if successful
         */
        bool CreateWindow();

        /**
         * @brief Setup OpenGL context
         * @return true if successful
         */
        bool SetupOpenGL();

        /**
         * @brief Setup event callbacks
         */
        void SetupCallbacks();

        /**
         * @brief GLFW key callback
         */
        static void GLFWKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

        /**
         * @brief GLFW mouse position callback
         */
        static void GLFWMousePosCallback(GLFWwindow* window, double xpos, double ypos);

        /**
         * @brief GLFW mouse button callback
         */
        static void GLFWMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

        /**
         * @brief GLFW scroll callback
         */
        static void GLFWScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

        /**
         * @brief GLFW resize callback
         */
        static void GLFWResizeCallback(GLFWwindow* window, int width, int height);

        /**
         * @brief GLFW close callback
         */
        static void GLFWCloseCallback(GLFWwindow* window);
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_WINDOW_WINDOW_HPP
