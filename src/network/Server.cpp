/**
 * @file Server.cpp
 * @brief VoxelCraft Dedicated Server Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#include "Server.hpp"
#include "Logger.hpp"

#include <iostream>
#include <chrono>

namespace VoxelCraft {

Server::Server()
    : m_port(25565)
    , m_maxPlayers(10)
    , m_serverName("VoxelCraft Server")
    , m_serverMotd("Welcome to VoxelCraft!")
    , m_running(false)
    , m_initialized(false)
{
    std::memset(&m_metrics, 0, sizeof(ServerMetrics));
}

Server::~Server() {
    Shutdown();
}

bool Server::Initialize(const Config& config) {
    if (m_initialized) {
        return true;
    }

    VOXELCRAFT_INFO("Initializing VoxelCraft Dedicated Server v1.0.0");

    try {
        // Load server configuration
        m_port = config.Get("server.port", 25565);
        m_maxPlayers = config.Get("server.max_players", 10);
        m_serverName = config.Get("server.name", "VoxelCraft Server");
        m_serverMotd = config.Get("server.motd", "Welcome to VoxelCraft!");

        // Create network manager
        m_networkManager = std::make_unique<NetworkManager>();
        if (!m_networkManager->Initialize(config)) {
            VOXELCRAFT_ERROR("Failed to initialize network manager");
            return false;
        }

        // Create world
        WorldSettings worldSettings;
        worldSettings.worldName = "Server World";
        worldSettings.worldType = WorldType::INFINITE;
        worldSettings.renderDistance = 8;
        worldSettings.simulationDistance = 6;

        m_world = std::make_unique<World>(worldSettings);
        if (!m_world->Initialize()) {
            VOXELCRAFT_ERROR("Failed to initialize world");
            return false;
        }

        // Set up network callbacks
        m_networkManager->SetPlayerConnectedCallback(
            [this](uint32_t playerId) { OnPlayerConnected(playerId); }
        );

        m_networkManager->SetPlayerDisconnectedCallback(
            [this](uint32_t playerId) { OnPlayerDisconnected(playerId); }
        );

        m_networkManager->SetPacketReceivedCallback(
            [this](const NetworkPacket& packet) {
                // Handle packet based on type
                if (packet.type == PacketType::PLAYER_POSITION) {
                    // Handle player position update
                } else if (packet.type == PacketType::CHAT_MESSAGE) {
                    // Handle chat message
                }
            }
        );

        m_initialized = true;
        VOXELCRAFT_INFO("Server initialized successfully - {} (Port: {})", m_serverName, m_port);

        return true;

    } catch (const std::exception& e) {
        VOXELCRAFT_ERROR("Failed to initialize server: {}", e.what());
        return false;
    }
}

void Server::Shutdown() {
    if (!m_initialized) {
        return;
    }

    VOXELCRAFT_INFO("Shutting down VoxelCraft Dedicated Server");

    // Stop server
    Stop();

    // Clean up resources
    m_world.reset();
    m_networkManager.reset();

    m_initialized = false;
    VOXELCRAFT_INFO("Server shutdown complete");
}

void Server::Run() {
    if (!m_initialized) {
        VOXELCRAFT_ERROR("Server not initialized");
        return;
    }

    VOXELCRAFT_INFO("Starting VoxelCraft Dedicated Server on port {}", m_port);

    if (!Start(m_port, m_maxPlayers)) {
        VOXELCRAFT_ERROR("Failed to start server");
        return;
    }

    // Main server loop
    auto lastMetricsUpdate = std::chrono::steady_clock::now();

    while (m_running) {
        auto currentTime = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
            currentTime - lastMetricsUpdate
        );

        // Update metrics every second
        if (elapsed.count() >= 1) {
            UpdateServerMetrics();
            lastMetricsUpdate = currentTime;
        }

        // Process server commands from console
        std::string command;
        if (std::getline(std::cin, command)) {
            ProcessServerCommand(command);
        }

        // Small sleep to prevent busy waiting
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    VOXELCRAFT_INFO("Server main loop ended");
}

bool Server::Start(uint16_t port, uint32_t maxPlayers) {
    if (!m_initialized) {
        VOXELCRAFT_ERROR("Cannot start server: not initialized");
        return false;
    }

    if (m_running) {
        VOXELCRAFT_WARNING("Server already running");
        return true;
    }

    VOXELCRAFT_INFO("Starting server on port {} with max {} players", port, maxPlayers);

    m_port = port;
    m_maxPlayers = maxPlayers;
    m_running = true;

    // Start server
    if (!m_networkManager->StartServer(port, maxPlayers)) {
        VOXELCRAFT_ERROR("Failed to start network server");
        m_running = false;
        return false;
    }

    // Start server thread
    m_serverThread = std::make_unique<std::thread>(&Server::ServerLoop, this);

    // Start world update thread
    m_worldUpdateThread = std::make_unique<std::thread>(&Server::WorldUpdateLoop, this);

    VOXELCRAFT_INFO("Server started successfully");
    return true;
}

void Server::Stop() {
    if (!m_running) {
        return;
    }

    VOXELCRAFT_INFO("Stopping server");

    m_running = false;

    // Stop network manager
    if (m_networkManager) {
        m_networkManager->Disconnect();
    }

    // Wait for threads to finish
    if (m_serverThread && m_serverThread->joinable()) {
        m_serverThread->join();
    }

    if (m_worldUpdateThread && m_worldUpdateThread->joinable()) {
        m_worldUpdateThread->join();
    }

    VOXELCRAFT_INFO("Server stopped");
}

uint32_t Server::GetConnectedPlayerCount() const {
    if (!m_networkManager) {
        return 0;
    }

    return m_networkManager->GetConnectedPlayers().size();
}

const std::vector<std::string> Server::GetConnectedPlayerNames() const {
    std::vector<std::string> names;

    if (m_networkManager) {
        auto players = m_networkManager->GetConnectedPlayers();
        for (const auto& player : players) {
            names.push_back(player.playerName);
        }
    }

    return names;
}

void Server::ServerLoop() {
    VOXELCRAFT_INFO("Server loop started");

    auto lastTick = std::chrono::steady_clock::now();
    const double tickRate = 20.0; // 20 TPS (ticks per second)
    const double tickInterval = 1.0 / tickRate;

    while (m_running) {
        auto currentTime = std::chrono::steady_clock::now();
        double elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(
            currentTime - lastTick
        ).count();

        if (elapsed >= tickInterval) {
            // Process one server tick
            ProcessNetworkEvents();
            ProcessPlayerActions();

            lastTick = currentTime;
        }

        // Sleep to maintain tick rate
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    VOXELCRAFT_INFO("Server loop ended");
}

void Server::WorldUpdateLoop() {
    VOXELCRAFT_INFO("World update loop started");

    auto lastUpdate = std::chrono::steady_clock::now();
    const double updateRate = 20.0; // 20 UPS (updates per second)
    const double updateInterval = 1.0 / updateRate;

    while (m_running) {
        auto currentTime = std::chrono::steady_clock::now();
        double elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(
            currentTime - lastUpdate
        ).count();

        if (elapsed >= updateInterval) {
            // Update world
            std::lock_guard<std::mutex> lock(m_worldMutex);
            if (m_world) {
                Vec3 defaultPos(0, 0, 0); // Server doesn't have a camera position
                m_world->Update(elapsed, defaultPos);
            }

            // Broadcast world state to players
            BroadcastWorldState();

            lastUpdate = currentTime;
        }

        // Sleep to maintain update rate
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    VOXELCRAFT_INFO("World update loop ended");
}

void Server::ProcessNetworkEvents() {
    if (m_networkManager) {
        m_networkManager->Update(0.05); // 20 TPS = 0.05 seconds per tick
    }
}

void Server::UpdateServerMetrics() {
    auto currentTime = std::chrono::steady_clock::now();
    static auto startTime = currentTime;

    m_metrics.uptime = std::chrono::duration_cast<std::chrono::duration<double>>(
        currentTime - startTime
    ).count();

    uint32_t currentPlayers = GetConnectedPlayerCount();
    if (currentPlayers > m_metrics.peakPlayers) {
        m_metrics.peakPlayers = currentPlayers;
    }

    if (m_networkManager) {
        auto networkMetrics = m_networkManager->GetMetrics();
        m_metrics.totalPacketsSent = networkMetrics.bytesSent;
        m_metrics.totalPacketsReceived = networkMetrics.bytesReceived;
    }

    // Log metrics every 60 seconds
    static int metricsCounter = 0;
    if (++metricsCounter >= 60) {
        VOXELCRAFT_INFO("Server Metrics - Players: {}/{}, Uptime: {:.1f}h, Packets: {}/{}",
                       currentPlayers, m_maxPlayers, m_metrics.uptime / 3600.0,
                       m_metrics.totalPacketsSent, m_metrics.totalPacketsReceived);
        metricsCounter = 0;
    }
}

void Server::OnPlayerConnected(uint32_t playerId) {
    m_metrics.totalConnections++;

    auto player = m_networkManager->GetPlayerConnection(playerId);
    if (player) {
        VOXELCRAFT_INFO("Player {} connected from {}", player->playerName, player->address);

        // Send welcome message
        auto welcomePacket = NetworkManager::CreateChatMessagePacket(
            0, "Welcome to " + m_serverName + "! " + m_serverMotd
        );
        m_networkManager->SendPacket(playerId, welcomePacket);

        // Send initial world state
        SendWorldStateToPlayer(playerId);
    }
}

void Server::OnPlayerDisconnected(uint32_t playerId) {
    m_metrics.totalDisconnections++;

    auto player = m_networkManager->GetPlayerConnection(playerId);
    if (player) {
        VOXELCRAFT_INFO("Player {} disconnected", player->playerName);
    }
}

void Server::OnPlayerInput(uint32_t playerId, const std::vector<uint8_t>& inputData) {
    // Process player input (movement, actions, etc.)
    // This would update the player's entity in the world
}

void Server::BroadcastWorldState() {
    // Create world state packet
    // This would contain chunk updates, entity positions, etc.
    // For now, just broadcast a simple packet
    NetworkPacket worldPacket;
    worldPacket.type = PacketType::WORLD_TIME;
    worldPacket.senderId = 0;
    worldPacket.sequenceNumber = 0;

    m_networkManager->BroadcastPacket(worldPacket);
}

void Server::SendWorldStateToPlayer(uint32_t playerId) {
    // Send initial world state to newly connected player
    // This would include spawn position, initial chunks, etc.
    VOXELCRAFT_INFO("Sending initial world state to player {}", playerId);
}

void Server::ProcessPlayerActions() {
    // Process queued player actions
    // This would handle movement, block placement, etc.
}

void Server::ProcessServerCommand(const std::string& command) {
    VOXELCRAFT_INFO("Processing server command: {}", command);

    if (command == "stop") {
        VOXELCRAFT_INFO("Shutdown command received");
        Stop();
    } else if (command == "status") {
        VOXELCRAFT_INFO("Server Status:");
        VOXELCRAFT_INFO("  Players: {}/{}", GetConnectedPlayerCount(), m_maxPlayers);
        VOXELCRAFT_INFO("  Uptime: {:.1f} hours", m_metrics.uptime / 3600.0);
        VOXELCRAFT_INFO("  Peak Players: {}", m_metrics.peakPlayers);
    } else if (command.substr(0, 4) == "kick") {
        // Extract player name and kick
        if (command.length() > 5) {
            std::string playerName = command.substr(5);
            VOXELCRAFT_INFO("Kicking player: {}", playerName);
            // Implement player kicking logic
        }
    } else if (command == "help") {
        VOXELCRAFT_INFO("Available commands:");
        VOXELCRAFT_INFO("  stop - Stop the server");
        VOXELCRAFT_INFO("  status - Show server status");
        VOXELCRAFT_INFO("  kick <player> - Kick a player");
        VOXELCRAFT_INFO("  help - Show this help");
    } else {
        VOXELCRAFT_INFO("Unknown command: {}", command);
        VOXELCRAFT_INFO("Type 'help' for available commands");
    }
}

void Server::LogServerEvent(const std::string& event) {
    VOXELCRAFT_INFO("[SERVER] {}", event);
}

} // namespace VoxelCraft
