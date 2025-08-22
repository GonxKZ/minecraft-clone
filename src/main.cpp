/**
 * @file main.cpp
 * @brief VoxelCraft Engine Main Entry Point
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file contains the main entry point for the VoxelCraft engine.
 * It initializes the application, handles command-line arguments,
 * and manages the main game loop.
 */

#include "core/Application.hpp"
#include "core/Logger.hpp"
#include "core/Config.hpp"

#include <unordered_map>

#include <iostream>
#include <string>
#include <vector>
#include <exception>
#include <memory>

/**
 * @brief Parse command line arguments
 * @param argc Number of arguments
 * @param argv Array of argument strings
 * @return Parsed arguments as key-value pairs
 */
std::unordered_map<std::string, std::string> ParseCommandLine(int argc, char* argv[]) {
    std::unordered_map<std::string, std::string> args;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        // Handle flags (starting with --)
        if (arg.substr(0, 2) == "--") {
            std::string key = arg.substr(2);

            // Handle key-value pairs (--key=value)
            size_t equalsPos = key.find('=');
            if (equalsPos != std::string::npos) {
                std::string value = key.substr(equalsPos + 1);
                key = key.substr(0, equalsPos);
                args[key] = value;
            } else {
                // Handle boolean flags (--flag)
                args[key] = "true";
            }
        }
        // Handle short flags (starting with -)
        else if (arg.substr(0, 1) == "-") {
            std::string key = arg.substr(1);

            // Look for next argument as value
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                args[key] = argv[++i];
            } else {
                args[key] = "true";
            }
        }
    }

    return args;
}

/**
 * @brief Print help information
 * @param programName Name of the executable
 */
void PrintHelp(const std::string& programName) {
    std::cout << "VoxelCraft Engine v1.0.0" << std::endl;
    std::cout << "Advanced Minecraft Clone Engine" << std::endl;
    std::cout << std::endl;
    std::cout << "Usage: " << programName << " [options]" << std::endl;
    std::cout << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  --help, -h              Show this help message" << std::endl;
    std::cout << "  --version, -v           Show version information" << std::endl;
    std::cout << "  --dev-mode              Enable development mode" << std::endl;
    std::cout << "  --debug                 Enable debug features" << std::endl;
    std::cout << "  --config=<file>         Specify configuration file" << std::endl;
    std::cout << "  --world=<name>          Load specific world" << std::endl;
    std::cout << "  --seed=<number>         Set world seed" << std::endl;
    std::cout << "  --port=<number>         Set server port" << std::endl;
    std::cout << "  --server                Run as dedicated server" << std::endl;
    std::cout << "  --client=<address>      Connect to server" << std::endl;
    std::cout << "  --profiling             Enable performance profiling" << std::endl;
    std::cout << "  --no-vsync              Disable vertical synchronization" << std::endl;
    std::cout << "  --fullscreen            Start in fullscreen mode" << std::endl;
    std::cout << "  --windowed              Start in windowed mode" << std::endl;
    std::cout << "  --resolution=<WxH>      Set window resolution" << std::endl;
    std::cout << "  --log-level=<level>     Set logging level (trace, debug, info, warn, error)" << std::endl;
    std::cout << std::endl;
    std::cout << "Examples:" << std::endl;
    std::cout << "  " << programName << " --dev-mode --debug" << std::endl;
    std::cout << "  " << programName << " --world=my_world --seed=12345" << std::endl;
    std::cout << "  " << programName << " --server --port=25565" << std::endl;
    std::cout << "  " << programName << " --client=localhost:25565" << std::endl;
}

/**
 * @brief Print version information
 */
void PrintVersion() {
    std::cout << "VoxelCraft Engine v1.0.0" << std::endl;
    std::cout << "Built with C++20" << std::endl;
    std::cout << "OpenGL/Vulkan Graphics Backend" << std::endl;
    std::cout << "Multi-threaded Architecture" << std::endl;
    std::cout << "Advanced ECS System" << std::endl;
}

/**
 * @brief Main entry point
 * @param argc Number of command line arguments
 * @param argv Array of command line arguments
 * @return Exit code
 */
int main(int argc, char* argv[]) {
    try {
        // Initialize logger first
        VoxelCraft::LogManager::Initialize();

        // Parse command line arguments
        auto args = ParseCommandLine(argc, argv);

        // Handle special commands
        if (args.count("help") || args.count("h")) {
            PrintHelp(argv[0]);
            return 0;
        }

        if (args.count("version") || args.count("v")) {
            PrintVersion();
            return 0;
        }

        VOXELCRAFT_INFO("Starting VoxelCraft Engine v1.0.0");

        // Create and configure application
        auto application = std::make_unique<VoxelCraft::Application>();

        // Apply command line configurations
        VoxelCraft::Config& config = application->GetConfig();

        // Development mode
        if (args.count("dev-mode")) {
            config.Set("engine.debug_mode", true);
            config.Set("debug.enable_debug_renderer", true);
            config.Set("debug.show_fps", true);
            VOXELCRAFT_INFO("Development mode enabled");
        }

        // Debug features
        if (args.count("debug")) {
            config.Set("engine.debug_mode", true);
            config.Set("debug.enable_debug_renderer", true);
            VOXELCRAFT_INFO("Debug features enabled");
        }

        // Configuration file
        if (args.count("config")) {
            config.LoadFromFile(args["config"]);
            VOXELCRAFT_INFO("Loaded configuration from: {}", args["config"]);
        }

        // World settings
        if (args.count("world")) {
            config.Set("world.name", args["world"]);
            VOXELCRAFT_INFO("Loading world: {}", args["world"]);
        }

        if (args.count("seed")) {
            config.Set("world.seed", std::stoi(args["seed"]));
            VOXELCRAFT_INFO("Using world seed: {}", args["seed"]);
        }

        // Network settings
        if (args.count("port")) {
            config.Set("network.server_port", std::stoi(args["port"]));
            VOXELCRAFT_INFO("Server port set to: {}", args["port"]);
        }

        // Server mode
        if (args.count("server")) {
            config.Set("network.server_mode", true);
            VOXELCRAFT_INFO("Running in server mode");
        }

        // Client mode
        if (args.count("client")) {
            config.Set("network.client_mode", true);
            config.Set("network.server_address", args["client"]);
            VOXELCRAFT_INFO("Connecting to server: {}", args["client"]);
        }

        // Profiling
        if (args.count("profiling")) {
            config.Set("profiling.enable_profiler", true);
            VOXELCRAFT_INFO("Performance profiling enabled");
        }

        // Graphics settings
        if (args.count("no-vsync")) {
            config.Set("engine.vsync", false);
            VOXELCRAFT_INFO("VSync disabled");
        }

        if (args.count("fullscreen")) {
            config.Set("graphics.fullscreen", true);
            VOXELCRAFT_INFO("Fullscreen mode enabled");
        }

        if (args.count("windowed")) {
            config.Set("graphics.fullscreen", false);
            VOXELCRAFT_INFO("Windowed mode enabled");
        }

        if (args.count("resolution")) {
            std::string resolution = args["resolution"];
            size_t xPos = resolution.find('x');
            if (xPos != std::string::npos) {
                int width = std::stoi(resolution.substr(0, xPos));
                int height = std::stoi(resolution.substr(xPos + 1));
                config.Set("graphics.width", width);
                config.Set("graphics.height", height);
                VOXELCRAFT_INFO("Resolution set to: {}x{}", width, height);
            }
        }

        // Logging level
        if (args.count("log-level")) {
            std::string level = args["log-level"];
            config.Set("logging.level", level);

            // Convert string level to LogLevel enum
            VoxelCraft::LogLevel logLevel = VoxelCraft::LogLevel::Info;
            if (level == "trace") logLevel = VoxelCraft::LogLevel::Trace;
            else if (level == "debug") logLevel = VoxelCraft::LogLevel::Debug;
            else if (level == "info") logLevel = VoxelCraft::LogLevel::Info;
            else if (level == "warning") logLevel = VoxelCraft::LogLevel::Warning;
            else if (level == "error") logLevel = VoxelCraft::LogLevel::Error;
            else if (level == "fatal") logLevel = VoxelCraft::LogLevel::Fatal;

            VoxelCraft::LogManager::SetGlobalLevel(logLevel);
            VOXELCRAFT_INFO("Log level set to: {}", level);
        }

        // Initialize and run application
        VOXELCRAFT_INFO("Initializing application...");
        if (!application->Initialize()) {
            VOXELCRAFT_ERROR("Failed to initialize application");
            return 1;
        }

        VOXELCRAFT_INFO("Starting main game loop...");
        int exitCode = application->Run();

        VOXELCRAFT_INFO("Shutting down application...");
        application->Shutdown();

        VOXELCRAFT_INFO("VoxelCraft Engine shutdown complete");
        return exitCode;

    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        VOXELCRAFT_ERROR("Fatal error: {}", e.what());
        return 1;
    } catch (...) {
        std::cerr << "Unknown fatal error occurred" << std::endl;
        VOXELCRAFT_ERROR("Unknown fatal error occurred");
        return 1;
    }
}
