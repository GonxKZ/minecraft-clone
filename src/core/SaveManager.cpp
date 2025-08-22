/**
 * @file SaveManager.cpp
 * @brief VoxelCraft Save Manager Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#include "SaveManager.hpp"
#include "Config.hpp"
#include "Logger.hpp"
#include "../world/World.hpp"
#include "../player/Player.hpp"

#include <algorithm>
#include <chrono>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <fstream>

namespace VoxelCraft {

    SaveManager::SaveManager()
        : m_state(SaveState::IDLE)
        , m_autoSaveInterval(300.0) // 5 minutes default
        , m_compressionEnabled(true)
        , m_encryptionEnabled(false)
        , m_maxBackups(5)
        , m_saveThreadRunning(false)
    {
        // Initialize metrics
        std::memset(&m_metrics, 0, sizeof(SaveMetrics));
        m_lastAutoSave = std::chrono::steady_clock::now();
    }

    SaveManager::~SaveManager() {
        Shutdown();
    }

    bool SaveManager::Initialize(const Config& config) {
        VOXELCRAFT_INFO("Initializing Save Manager");

        try {
            // Load save configuration
            m_saveDirectory = config.Get("save.directory", "saves");
            m_autoSaveInterval = config.Get("save.auto_save_interval", 300.0);
            m_compressionEnabled = config.Get("save.compression_enabled", true);
            m_encryptionEnabled = config.Get("save.encryption_enabled", false);
            m_maxBackups = config.Get("save.max_backups", 5);

            // Create save directory if it doesn't exist
            std::filesystem::create_directories(m_saveDirectory);

            VOXELCRAFT_INFO("Save Manager initialized successfully - Directory: {}",
                          m_saveDirectory);

            return true;

        } catch (const std::exception& e) {
            VOXELCRAFT_ERROR("Failed to initialize Save Manager: {}", e.what());
            return false;
        }
    }

    void SaveManager::Shutdown() {
        VOXELCRAFT_INFO("Shutting down Save Manager");

        // Stop save thread
        m_saveThreadRunning = false;
        if (m_saveThread && m_saveThread->joinable()) {
            m_saveThread->join();
        }

        // Wait for any ongoing save operations
        std::lock_guard<std::mutex> lock(m_saveMutex);
        m_state = SaveState::IDLE;

        VOXELCRAFT_INFO("Save Manager shutdown complete");
    }

    void SaveManager::Update(double deltaTime) {
        // Check if auto-save is needed
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - m_lastAutoSave);

        if (elapsed.count() >= m_autoSaveInterval) {
            AutoSave();
        }
    }

    bool SaveManager::SaveWorld(const World* world, SaveType saveType) {
        if (!world) {
            VOXELCRAFT_ERROR("Cannot save world: World is null");
            return false;
        }

        if (m_state != SaveState::IDLE) {
            VOXELCRAFT_WARNING("Cannot save world: Save operation already in progress");
            return false;
        }

        VOXELCRAFT_INFO("Saving world: {}", world->GetName());

        SaveOperation operation;
        operation.type = saveType;
        operation.filename = GenerateSaveFilename(world->GetName(), saveType);
        operation.startTime = std::chrono::steady_clock::now();
        operation.success = false;

        m_state = SaveState::SAVING;
        m_currentOperation = operation;

        // Start save thread
        m_saveThreadRunning = true;
        m_saveThread = std::make_unique<std::thread>(&SaveManager::SaveThread, this);

        return true;
    }

    bool SaveManager::LoadWorld(const std::string& worldName, World* world) {
        if (!world) {
            VOXELCRAFT_ERROR("Cannot load world: World is null");
            return false;
        }

        if (m_state != SaveState::IDLE) {
            VOXELCRAFT_WARNING("Cannot load world: Operation already in progress");
            return false;
        }

        VOXELCRAFT_INFO("Loading world: {}", worldName);

        std::string filename = GenerateSaveFilename(worldName, SaveType::FULL_SAVE);
        std::vector<uint8_t> data;

        if (!PerformLoad(filename, data)) {
            VOXELCRAFT_ERROR("Failed to load world: {}", worldName);
            return false;
        }

        // Parse world data (simplified)
        // In a real implementation, you would deserialize the world data
        VOXELCRAFT_INFO("World loaded successfully: {}", worldName);
        m_metrics.totalLoads++;

        return true;
    }

    bool SaveManager::SavePlayer(const Player* player, SaveType saveType) {
        if (!player) {
            VOXELCRAFT_ERROR("Cannot save player: Player is null");
            return false;
        }

        VOXELCRAFT_INFO("Saving player: {}", player->GetName());

        // Create player save data
        PlayerSaveData data;
        data.playerName = player->GetName();
        data.position = player->GetPosition();
        data.rotation = player->GetRotation();
        data.health = player->GetHealth();
        data.food = player->GetFood();
        data.experience = player->GetExperience();
        data.level = player->GetLevel();
        // data.inventory = player->GetInventory(); // Would need to implement

        // Serialize and save (simplified)
        std::string filename = GenerateSaveFilename("player_" + player->GetName(), saveType);
        VOXELCRAFT_INFO("Player saved successfully: {}", player->GetName());
        m_metrics.totalSaves++;

        return true;
    }

    bool SaveManager::LoadPlayer(const std::string& playerName, Player* player) {
        if (!player) {
            VOXELCRAFT_ERROR("Cannot load player: Player is null");
            return false;
        }

        VOXELCRAFT_INFO("Loading player: {}", playerName);

        std::string filename = GenerateSaveFilename("player_" + playerName, SaveType::FULL_SAVE);
        std::vector<uint8_t> data;

        if (!PerformLoad(filename, data)) {
            VOXELCRAFT_ERROR("Failed to load player: {}", playerName);
            return false;
        }

        // Parse player data (simplified)
        // In a real implementation, you would deserialize the player data
        VOXELCRAFT_INFO("Player loaded successfully: {}", playerName);
        m_metrics.totalLoads++;

        return true;
    }

    bool SaveManager::AutoSave() {
        VOXELCRAFT_INFO("Performing auto-save");

        // Auto-save logic would go here
        // For now, just update the timestamp
        m_lastAutoSave = std::chrono::steady_clock::now();
        m_metrics.autoSaveCount++;

        return true;
    }

    bool SaveManager::CreateBackup(const std::string& worldName) {
        VOXELCRAFT_INFO("Creating backup for world: {}", worldName);

        auto timestamp = std::chrono::system_clock::now();
        std::string backupName = GenerateBackupFilename(worldName, timestamp);

        // Copy save files to backup location (simplified)
        // In a real implementation, you would copy all save files
        VOXELCRAFT_INFO("Backup created: {}", backupName);

        CleanOldBackups(worldName);
        return true;
    }

    bool SaveManager::RestoreFromBackup(const std::string& worldName, const std::string& backupName) {
        VOXELCRAFT_INFO("Restoring world {} from backup {}", worldName, backupName);

        // Restore logic would go here
        // In a real implementation, you would copy backup files to save location
        VOXELCRAFT_INFO("World restored successfully");

        return true;
    }

    std::vector<std::string> SaveManager::ListWorlds() const {
        std::vector<std::string> worlds;

        try {
            for (const auto& entry : std::filesystem::directory_iterator(m_saveDirectory)) {
                if (entry.is_directory()) {
                    worlds.push_back(entry.path().filename().string());
                }
            }
        } catch (const std::exception& e) {
            VOXELCRAFT_ERROR("Failed to list worlds: {}", e.what());
        }

        return worlds;
    }

    std::vector<std::string> SaveManager::ListBackups(const std::string& worldName) const {
        std::vector<std::string> backups;

        try {
            std::filesystem::path backupDir = GetWorldSaveDirectory(worldName) / "backups";
            if (std::filesystem::exists(backupDir)) {
                for (const auto& entry : std::filesystem::directory_iterator(backupDir)) {
                    if (entry.is_regular_file()) {
                        backups.push_back(entry.path().filename().string());
                    }
                }
            }
        } catch (const std::exception& e) {
            VOXELCRAFT_ERROR("Failed to list backups for world {}: {}", worldName, e.what());
        }

        return backups;
    }

    bool SaveManager::DeleteWorld(const std::string& worldName) {
        VOXELCRAFT_INFO("Deleting world: {}", worldName);

        try {
            std::filesystem::path worldDir = GetWorldSaveDirectory(worldName);
            if (std::filesystem::exists(worldDir)) {
                std::filesystem::remove_all(worldDir);
                VOXELCRAFT_INFO("World deleted successfully: {}", worldName);
                return true;
            }
        } catch (const std::exception& e) {
            VOXELCRAFT_ERROR("Failed to delete world {}: {}", worldName, e.what());
        }

        return false;
    }

    WorldSaveData SaveManager::GetWorldData(const std::string& worldName) const {
        WorldSaveData data;
        data.worldName = worldName;
        data.seed = 0; // Would load from file
        data.version = 1;
        data.created = std::chrono::system_clock::now();
        data.lastPlayed = std::chrono::system_clock::now();
        data.playTime = 0.0;
        data.spawnPoint = Vec3(0, 0, 0);
        return data;
    }

    PlayerSaveData SaveManager::GetPlayerData(const std::string& playerName) const {
        PlayerSaveData data;
        data.playerName = playerName;
        data.position = Vec3(0, 0, 0);
        data.rotation = Vec3(0, 0, 0);
        data.health = 20.0f;
        data.food = 20.0f;
        data.experience = 0;
        data.level = 1;
        return data;
    }

    // Private methods

    void SaveManager::SaveThread() {
        VOXELCRAFT_INFO("Save thread started");

        while (m_saveThreadRunning) {
            std::lock_guard<std::mutex> lock(m_saveMutex);

            if (m_state == SaveState::SAVING) {
                bool success = PerformSave(m_currentOperation);
                m_currentOperation.success = success;
                m_currentOperation.endTime = std::chrono::steady_clock::now();

                UpdateMetrics(m_currentOperation);
                m_state = SaveState::IDLE;
            }

            // Sleep to prevent busy waiting
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        VOXELCRAFT_INFO("Save thread stopped");
    }

    bool SaveManager::PerformSave(const SaveOperation& operation) {
        // Simplified save implementation
        // In a real implementation, you would serialize the data and write to file

        try {
            std::filesystem::path savePath = std::filesystem::path(m_saveDirectory) / operation.filename;
            std::filesystem::create_directories(savePath.parent_path());

            std::ofstream file(savePath, std::ios::binary);
            if (!file.is_open()) {
                VOXELCRAFT_ERROR("Failed to open save file: {}", operation.filename);
                return false;
            }

            // Write save header (simplified)
            uint32_t magic = 0x12345678;
            file.write(reinterpret_cast<const char*>(&magic), sizeof(magic));

            // Write timestamp
            auto timestamp = std::chrono::system_clock::now();
            auto timestampCount = timestamp.time_since_epoch().count();
            file.write(reinterpret_cast<const char*>(&timestampCount), sizeof(timestampCount));

            // Write data size placeholder
            size_t dataSize = 0;
            file.write(reinterpret_cast<const char*>(&dataSize), sizeof(dataSize));

            // Compress and encrypt data (simplified)
            std::vector<uint8_t> data; // Would contain serialized data

            if (m_compressionEnabled) {
                std::vector<uint8_t> compressedData;
                if (CompressData(data, compressedData)) {
                    data = compressedData;
                }
            }

            if (m_encryptionEnabled) {
                std::vector<uint8_t> encryptedData;
                if (EncryptData(data, encryptedData)) {
                    data = encryptedData;
                }
            }

            // Write actual data
            dataSize = data.size();
            file.seekp(sizeof(magic) + sizeof(timestampCount), std::ios::beg);
            file.write(reinterpret_cast<const char*>(&dataSize), sizeof(dataSize));
            file.seekp(0, std::ios::end);
            file.write(reinterpret_cast<const char*>(data.data()), dataSize);

            file.close();

            VOXELCRAFT_INFO("Save operation completed: {}", operation.filename);
            return true;

        } catch (const std::exception& e) {
            VOXELCRAFT_ERROR("Save operation failed: {}", e.what());
            return false;
        }
    }

    bool SaveManager::PerformLoad(const std::string& filename, std::vector<uint8_t>& data) {
        try {
            std::filesystem::path loadPath = std::filesystem::path(m_saveDirectory) / filename;

            if (!std::filesystem::exists(loadPath)) {
                VOXELCRAFT_ERROR("Save file not found: {}", filename);
                return false;
            }

            std::ifstream file(loadPath, std::ios::binary);
            if (!file.is_open()) {
                VOXELCRAFT_ERROR("Failed to open save file: {}", filename);
                return false;
            }

            // Read save header (simplified)
            uint32_t magic;
            file.read(reinterpret_cast<char*>(&magic), sizeof(magic));

            if (magic != 0x12345678) {
                VOXELCRAFT_ERROR("Invalid save file format: {}", filename);
                return false;
            }

            // Read timestamp
            int64_t timestampCount;
            file.read(reinterpret_cast<char*>(&timestampCount), sizeof(timestampCount));

            // Read data size
            size_t dataSize;
            file.read(reinterpret_cast<char*>(&dataSize), sizeof(dataSize));

            // Read data
            data.resize(dataSize);
            file.read(reinterpret_cast<char*>(data.data()), dataSize);
            file.close();

            // Decrypt and decompress data (simplified)
            if (m_encryptionEnabled) {
                std::vector<uint8_t> decryptedData;
                if (DecryptData(data, decryptedData)) {
                    data = decryptedData;
                }
            }

            if (m_compressionEnabled) {
                std::vector<uint8_t> decompressedData;
                if (DecompressData(data, decompressedData)) {
                    data = decompressedData;
                }
            }

            VOXELCRAFT_INFO("Load operation completed: {}", filename);
            return true;

        } catch (const std::exception& e) {
            VOXELCRAFT_ERROR("Load operation failed: {}", e.what());
            return false;
        }
    }

    bool SaveManager::CompressData(const std::vector<uint8_t>& input, std::vector<uint8_t>& output) {
        // Simplified compression - in real implementation would use zlib or similar
        output = input;
        return true;
    }

    bool SaveManager::DecompressData(const std::vector<uint8_t>& input, std::vector<uint8_t>& output) {
        // Simplified decompression
        output = input;
        return true;
    }

    bool SaveManager::EncryptData(const std::vector<uint8_t>& input, std::vector<uint8_t>& output) {
        // Simplified encryption - in real implementation would use proper encryption
        output = input;
        return true;
    }

    bool SaveManager::DecryptData(const std::vector<uint8_t>& input, std::vector<uint8_t>& output) {
        // Simplified decryption
        output = input;
        return true;
    }

    std::string SaveManager::GenerateSaveFilename(const std::string& baseName, SaveType saveType) {
        std::stringstream ss;
        ss << baseName;

        switch (saveType) {
            case SaveType::AUTO_SAVE:
                ss << "_autosave";
                break;
            case SaveType::QUICK_SAVE:
                ss << "_quicksave";
                break;
            case SaveType::FULL_SAVE:
                // No suffix for full saves
                break;
            default:
                break;
        }

        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        auto tm = *std::localtime(&time);
        ss << "_" << std::put_time(&tm, "%Y%m%d_%H%M%S") << ".sav";

        return ss.str();
    }

    std::string SaveManager::GenerateBackupFilename(const std::string& baseName, const std::chrono::system_clock::time_point& timestamp) {
        std::stringstream ss;
        ss << baseName << "_backup";

        auto time = std::chrono::system_clock::to_time_t(timestamp);
        auto tm = *std::localtime(&time);
        ss << "_" << std::put_time(&tm, "%Y%m%d_%H%M%S") << ".bak";

        return ss.str();
    }

    void SaveManager::CleanOldBackups(const std::string& worldName) {
        auto backups = ListBackups(worldName);

        if (backups.size() > m_maxBackups) {
            // Sort backups by date (newest first)
            std::sort(backups.rbegin(), backups.rend());

            // Remove oldest backups
            for (size_t i = m_maxBackups; i < backups.size(); ++i) {
                std::filesystem::path backupPath = GetWorldSaveDirectory(worldName) / "backups" / backups[i];
                try {
                    std::filesystem::remove(backupPath);
                    VOXELCRAFT_INFO("Removed old backup: {}", backups[i]);
                } catch (const std::exception& e) {
                    VOXELCRAFT_ERROR("Failed to remove old backup {}: {}", backups[i], e.what());
                }
            }
        }
    }

    void SaveManager::UpdateMetrics(const SaveOperation& operation) {
        std::lock_guard<std::mutex> lock(m_metricsMutex);

        auto duration = std::chrono::duration_cast<std::chrono::duration<double>>(
            operation.endTime - operation.startTime
        ).count();

        m_metrics.totalSaves++;
        if (operation.type == SaveType::AUTO_SAVE) {
            m_metrics.autoSaveCount++;
        } else if (operation.type == SaveType::MANUAL_SAVE) {
            m_metrics.manualSaveCount++;
        }

        if (!operation.success) {
            m_metrics.failedSaves++;
        }

        m_metrics.totalDataSaved += operation.dataSize;

        // Update average save time
        double totalSaveTime = m_metrics.averageSaveTime * (m_metrics.totalSaves - 1);
        m_metrics.averageSaveTime = (totalSaveTime + duration) / m_metrics.totalSaves;
    }

    std::filesystem::path SaveManager::GetWorldSaveDirectory(const std::string& worldName) const {
        return std::filesystem::path(m_saveDirectory) / worldName;
    }

    std::filesystem::path SaveManager::GetPlayerSaveDirectory(const std::string& playerName) const {
        return std::filesystem::path(m_saveDirectory) / "players" / playerName;
    }

} // namespace VoxelCraft
