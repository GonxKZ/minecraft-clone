/**
 * @file ResourceSystem.cpp
 * @brief VoxelCraft Advanced Resource Management System Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#include "ResourceSystem.hpp"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <functional>
#include <queue>
#include <future>
#include <memory>
#include <filesystem>
#include <fstream>
#include <atomic>

#include "../utils/Random.hpp"
#include "../logging/Logger.hpp"
#include "../memory/MemorySystem.hpp"

namespace VoxelCraft {

    // Resource implementations
    Resource::Resource(const std::string& id, ResourceType type)
        : m_id(id), m_type(type), m_state(ResourceState::UNLOADED),
          m_refCount(0), m_loadProgress(0.0f) {
    }

    void Resource::AddReference() {
        m_refCount++;
    }

    void Resource::RemoveReference() {
        if (m_refCount > 0) {
            m_refCount--;
        }
    }

    // TextureResource implementation
    TextureResource::TextureResource(const std::string& id)
        : Resource(id, ResourceType::TEXTURE), m_size(0, 0), m_channels(0),
          m_mipLevels(0), m_hasAlpha(false), m_textureHandle(nullptr) {
    }

    TextureResource::~TextureResource() {
        Unload();
    }

    size_t TextureResource::GetMemoryUsage() const {
        return m_pixelData.size();
    }

    bool TextureResource::Validate() const {
        return m_textureHandle != nullptr && m_size.x > 0 && m_size.y > 0;
    }

    bool TextureResource::Load(const std::vector<uint8_t>& data) {
        std::unique_lock<std::mutex> lock(m_resourceMutex);

        try {
            // This would implement actual texture loading
            // For now, just store the data
            m_pixelData = data;

            // Parse basic image information (simplified)
            if (data.size() >= 8) {
                // Assume first 8 bytes contain width and height
                m_size.x = *reinterpret_cast<const int*>(data.data());
                m_size.y = *reinterpret_cast<const int*>(data.data() + 4);
                m_channels = 4; // Assume RGBA
                m_hasAlpha = true;
                m_mipLevels = 1;
            }

            m_state = ResourceState::READY;
            m_loadProgress = 1.0f;
            return true;

        } catch (const std::exception& e) {
            m_errorMessage = e.what();
            m_state = ResourceState::ERROR;
            return false;
        }
    }

    bool TextureResource::Unload() {
        std::unique_lock<std::mutex> lock(m_resourceMutex);

        m_pixelData.clear();
        m_textureHandle = nullptr;
        m_state = ResourceState::UNLOADED;
        return true;
    }

    bool TextureResource::Reload() {
        if (m_pixelData.empty()) {
            return false;
        }

        std::vector<uint8_t> data = m_pixelData;
        return Load(data);
    }

    // ModelResource implementation
    ModelResource::ModelResource(const std::string& id)
        : Resource(id, ResourceType::MODEL), m_vertexCount(0), m_indexCount(0),
          m_hasAnimations(false), m_hasSkeleton(false), m_modelHandle(nullptr) {
    }

    ModelResource::~ModelResource() {
        Unload();
    }

    size_t ModelResource::GetMemoryUsage() const {
        return m_vertexData.size() + m_indexData.size();
    }

    bool ModelResource::Validate() const {
        return m_modelHandle != nullptr && m_vertexCount > 0;
    }

    bool ModelResource::Load(const std::vector<uint8_t>& data) {
        std::unique_lock<std::mutex> lock(m_resourceMutex);

        try {
            // This would implement actual model loading
            // For now, just store the data
            m_vertexData = data;
            m_meshes.push_back(nullptr); // Placeholder

            // Parse basic model information (simplified)
            if (data.size() >= 12) {
                m_vertexCount = *reinterpret_cast<const uint32_t*>(data.data());
                m_indexCount = *reinterpret_cast<const uint32_t*>(data.data() + 4);
                m_hasAnimations = *reinterpret_cast<const bool*>(data.data() + 8);
                m_hasSkeleton = *reinterpret_cast<const bool*>(data.data() + 9);
            }

            m_state = ResourceState::READY;
            m_loadProgress = 1.0f;
            return true;

        } catch (const std::exception& e) {
            m_errorMessage = e.what();
            m_state = ResourceState::ERROR;
            return false;
        }
    }

    bool ModelResource::Unload() {
        std::unique_lock<std::mutex> lock(m_resourceMutex);

        m_vertexData.clear();
        m_indexData.clear();
        m_meshes.clear();
        m_modelHandle = nullptr;
        m_state = ResourceState::UNLOADED;
        return true;
    }

    bool ModelResource::Reload() {
        if (m_vertexData.empty()) {
            return false;
        }

        std::vector<uint8_t> data = m_vertexData;
        return Load(data);
    }

    // AudioResource implementation
    AudioResource::AudioResource(const std::string& id)
        : Resource(id, ResourceType::AUDIO), m_duration(0.0f), m_sampleRate(0),
          m_channels(0), m_bitsPerSample(0), m_audioHandle(nullptr) {
    }

    AudioResource::~AudioResource() {
        Unload();
    }

    size_t AudioResource::GetMemoryUsage() const {
        return m_pcmData.size() * sizeof(float);
    }

    bool AudioResource::Validate() const {
        return m_audioHandle != nullptr && m_duration > 0.0f;
    }

    bool AudioResource::Load(const std::vector<uint8_t>& data) {
        std::unique_lock<std::mutex> lock(m_resourceMutex);

        try {
            // This would implement actual audio loading
            // For now, just convert to PCM (simplified)
            m_pcmData.resize(data.size() / sizeof(float));
            std::memcpy(m_pcmData.data(), data.data(), data.size());

            // Parse basic audio information (simplified)
            if (data.size() >= 16) {
                m_duration = *reinterpret_cast<const float*>(data.data());
                m_sampleRate = *reinterpret_cast<const uint32_t*>(data.data() + 4);
                m_channels = *reinterpret_cast<const uint32_t*>(data.data() + 8);
                m_bitsPerSample = *reinterpret_cast<const uint32_t*>(data.data() + 12);
            }

            m_state = ResourceState::READY;
            m_loadProgress = 1.0f;
            return true;

        } catch (const std::exception& e) {
            m_errorMessage = e.what();
            m_state = ResourceState::ERROR;
            return false;
        }
    }

    bool AudioResource::Unload() {
        std::unique_lock<std::mutex> lock(m_resourceMutex);

        m_pcmData.clear();
        m_audioHandle = nullptr;
        m_state = ResourceState::UNLOADED;
        return true;
    }

    bool AudioResource::Reload() {
        if (m_pcmData.empty()) {
            return false;
        }

        std::vector<uint8_t> data(m_pcmData.size() * sizeof(float));
        std::memcpy(data.data(), m_pcmData.data(), data.size());
        return Load(data);
    }

    // ResourceLoader implementation
    ResourceLoader::ResourceLoader(ResourceType type)
        : m_type(type) {
    }

    // ResourceCache implementation
    ResourceCache::ResourceCache(size_t maxMemorySize, size_t maxDiskSize)
        : m_maxMemorySize(maxMemorySize), m_maxDiskSize(maxDiskSize),
          m_currentMemorySize(0), m_currentDiskSize(0), m_compressionEnabled(false) {
    }

    bool ResourceCache::AddResource(const std::shared_ptr<Resource>& resource, CachePolicy policy) {
        if (!resource) return false;

        std::unique_lock<std::shared_mutex> lock(m_cacheMutex);

        // Check if we need to evict resources
        size_t resourceSize = resource->GetMemoryUsage();
        if (m_currentMemorySize + resourceSize > m_maxMemorySize) {
            EvictOldResources();
        }

        switch (policy) {
            case CachePolicy::MEMORY_ONLY:
            case CachePolicy::MEMORY_AND_DISK:
                m_memoryCache[resource->GetId()] = resource;
                m_currentMemorySize += resourceSize;
                break;
            case CachePolicy::DISK_ONLY:
                // Would save to disk
                break;
            case CachePolicy::STREAMING:
                // Handle streaming cache
                break;
            case CachePolicy::NEVER_CACHE:
                return false;
        }

        return true;
    }

    std::shared_ptr<Resource> ResourceCache::GetResource(const std::string& id) const {
        std::shared_lock<std::shared_mutex> lock(m_cacheMutex);

        auto it = m_memoryCache.find(id);
        if (it != m_memoryCache.end()) {
            return it->second;
        }

        return nullptr;
    }

    bool ResourceCache::RemoveResource(const std::string& id) {
        std::unique_lock<std::shared_mutex> lock(m_cacheMutex);

        auto it = m_memoryCache.find(id);
        if (it != m_memoryCache.end()) {
            m_currentMemorySize -= it->second->GetMemoryUsage();
            m_memoryCache.erase(it);
            return true;
        }

        return false;
    }

    void ResourceCache::Clear() {
        std::unique_lock<std::shared_mutex> lock(m_cacheMutex);
        m_memoryCache.clear();
        m_diskCache.clear();
        m_currentMemorySize = 0;
        m_currentDiskSize = 0;
    }

    size_t ResourceCache::GetMemoryUsage() const {
        std::shared_lock<std::shared_mutex> lock(m_cacheMutex);
        return m_currentMemorySize;
    }

    size_t ResourceCache::GetDiskUsage() const {
        std::shared_lock<std::shared_mutex> lock(m_cacheMutex);
        return m_currentDiskSize;
    }

    float ResourceCache::GetMemoryUtilization() const {
        if (m_maxMemorySize == 0) return 0.0f;
        return static_cast<float>(GetMemoryUsage()) / m_maxMemorySize;
    }

    float ResourceCache::GetDiskUtilization() const {
        if (m_maxDiskSize == 0) return 0.0f;
        return static_cast<float>(GetDiskUsage()) / m_maxDiskSize;
    }

    void ResourceCache::Update(float deltaTime) {
        // Periodic cache maintenance
        if (GetMemoryUtilization() > 0.9f) {
            EvictOldResources();
        }
    }

    void ResourceCache::EvictOldResources() {
        std::unique_lock<std::shared_mutex> lock(m_cacheMutex);

        auto candidates = GetEvictionCandidates();
        for (const auto& id : candidates) {
            auto it = m_memoryCache.find(id);
            if (it != m_memoryCache.end()) {
                m_currentMemorySize -= it->second->GetMemoryUsage();
                m_memoryCache.erase(it);
            }
        }
    }

    void ResourceCache::Optimize() {
        // Implement cache optimization
        EvictOldResources();
    }

    bool ResourceCache::ShouldEvict(const std::shared_ptr<Resource>& resource) const {
        // Implement LRU eviction policy
        return resource->GetReferenceCount() == 0;
    }

    std::vector<std::string> ResourceCache::GetEvictionCandidates() const {
        std::vector<std::string> candidates;

        for (const auto& pair : m_memoryCache) {
            if (ShouldEvict(pair.second)) {
                candidates.push_back(pair.first);
            }
        }

        return candidates;
    }

    // ResourceStreamer implementation
    ResourceStreamer::ResourceStreamer(size_t bufferSize)
        : m_bufferSize(bufferSize) {
    }

    bool ResourceStreamer::StartStreaming(const std::string& id, const std::string& path) {
        std::unique_lock<std::shared_mutex> lock(m_streamMutex);

        if (m_activeStreams.find(id) != m_activeStreams.end()) {
            return false; // Already streaming
        }

        StreamInfo info;
        info.path = path;
        info.file.open(path, std::ios::binary);
        if (!info.file.is_open()) {
            return false;
        }

        info.file.seekg(0, std::ios::end);
        info.fileSize = info.file.tellg();
        info.file.seekg(0, std::ios::beg);
        info.currentPosition = 0;
        info.progress = 0.0f;
        info.active = true;
        info.buffer.resize(m_bufferSize);

        m_activeStreams[id] = info;
        return true;
    }

    bool ResourceStreamer::StopStreaming(const std::string& id) {
        std::unique_lock<std::shared_mutex> lock(m_streamMutex);

        auto it = m_activeStreams.find(id);
        if (it == m_activeStreams.end()) {
            return false;
        }

        it->second.file.close();
        m_activeStreams.erase(it);
        return true;
    }

    bool ResourceStreamer::IsStreaming(const std::string& id) const {
        std::shared_lock<std::shared_mutex> lock(m_streamMutex);
        return m_activeStreams.find(id) != m_activeStreams.end();
    }

    size_t ResourceStreamer::GetStreamedData(const std::string& id, std::vector<uint8_t>& buffer,
                                           size_t offset, size_t size) {
        std::unique_lock<std::shared_mutex> lock(m_streamMutex);

        auto it = m_activeStreams.find(id);
        if (it == m_activeStreams.end()) {
            return 0;
        }

        StreamInfo& info = it->second;
        if (offset >= info.fileSize) {
            return 0;
        }

        size = std::min(size, info.fileSize - offset);
        info.file.seekg(offset);
        info.file.read(reinterpret_cast<char*>(buffer.data()), size);

        size_t bytesRead = info.file.gcount();
        info.currentPosition = offset + bytesRead;
        info.progress = static_cast<float>(info.currentPosition) / info.fileSize;

        return bytesRead;
    }

    float ResourceStreamer::GetStreamingProgress(const std::string& id) const {
        std::shared_lock<std::shared_mutex> lock(m_streamMutex);

        auto it = m_activeStreams.find(id);
        if (it == m_activeStreams.end()) {
            return 0.0f;
        }

        return it->second.progress;
    }

    void ResourceStreamer::Update(float deltaTime) {
        // Update streaming progress
        std::shared_lock<std::shared_mutex> lock(m_streamMutex);

        for (auto& pair : m_activeStreams) {
            StreamInfo& info = pair.second;
            if (info.active && info.currentPosition < info.fileSize) {
                info.progress = static_cast<float>(info.currentPosition) / info.fileSize;
            }
        }
    }

    // Main ResourceSystem implementation
    ResourceSystem& ResourceSystem::GetInstance() {
        static ResourceSystem instance;
        return instance;
    }

    ResourceSystem::ResourceSystem()
        : m_cache(1024 * 1024 * 512, 1024 * 1024 * 1024), // 512MB memory, 1GB disk
          m_streamer(64 * 1024), // 64KB buffer
          m_maxConcurrentLoads(8),
          m_cacheMemoryLimit(1024 * 1024 * 512),
          m_cacheDiskLimit(1024 * 1024 * 1024) {
    }

    ResourceSystem::~ResourceSystem() {
        Shutdown();
    }

    bool ResourceSystem::Initialize() {
        if (!System::Initialize()) {
            return false;
        }

        Logger::GetInstance().Info("ResourceSystem initializing", "ResourceSystem");

        // Start loader threads
        StartLoaderThreads();

        Logger::GetInstance().Info("ResourceSystem initialized successfully", "ResourceSystem");
        return true;
    }

    void ResourceSystem::Update(float deltaTime) {
        System::Update(deltaTime);

        // Update cache
        m_cache.Update(deltaTime);

        // Update streamer
        m_streamer.Update(deltaTime);

        // Process load queue
        ProcessLoadQueue();

        // Update stats
        {
            std::unique_lock<std::shared_mutex> lock(m_statsMutex);
            // Update rolling averages, etc.
        }
    }

    void ResourceSystem::Shutdown() {
        if (!System::Shutdown()) {
            return;
        }

        Logger::GetInstance().Info("ResourceSystem shutting down", "ResourceSystem");

        // Stop loader threads
        StopLoaderThreads();

        // Clear all resources
        {
            std::unique_lock<std::shared_mutex> lock(m_resourcesMutex);
            m_resources.clear();
        }

        // Clear cache
        m_cache.Clear();

        // Stop all streaming
        {
            std::unique_lock<std::shared_mutex> lock(m_loadMutex);
            for (auto& pair : m_activeLoads) {
                if (m_streamer.IsStreaming(pair.first)) {
                    m_streamer.StopStreaming(pair.first);
                }
            }
            m_activeLoads.clear();
        }

        Logger::GetInstance().Info("ResourceSystem shutdown complete", "ResourceSystem");
    }

    std::shared_ptr<Resource> ResourceSystem::LoadResource(const std::string& id,
                                                         const std::string& path,
                                                         ResourceType type,
                                                         const ResourceLoadParams& params) {
        // Check if already loaded
        {
            std::shared_lock<std::shared_mutex> lock(m_resourcesMutex);
            auto it = m_resources.find(id);
            if (it != m_resources.end()) {
                return it->second;
            }
        }

        // Check cache first
        std::shared_ptr<Resource> resource;
        if (LoadFromCache(id, resource)) {
            std::unique_lock<std::shared_mutex> lock(m_resourcesMutex);
            m_resources[id] = resource;
            return resource;
        }

        // Create load task
        ResourceLoadTask task;
        task.resourceId = id;
        task.path = path;
        task.type = type;
        task.params = params;
        task.priority = params.priority;
        task.startTime = std::chrono::steady_clock::now();

        // Load resource
        resource = LoadResourceInternal(task);
        if (resource) {
            std::unique_lock<std::shared_mutex> lock(m_resourcesMutex);
            m_resources[id] = resource;

            // Add to cache if requested
            if (params.useCache) {
                m_cache.AddResource(resource, CachePolicy::MEMORY_ONLY);
            }
        }

        UpdateStats(id, resource != nullptr, false);
        return resource;
    }

    std::future<std::shared_ptr<Resource>> ResourceSystem::LoadResourceAsync(const std::string& id,
                                                                           const std::string& path,
                                                                           ResourceType type,
                                                                           const ResourceLoadParams& params) {
        return std::async(std::launch::async, [this, id, path, type, params]() {
            return LoadResource(id, path, type, params);
        });
    }

    bool ResourceSystem::UnloadResource(const std::string& id) {
        std::unique_lock<std::shared_mutex> lock(m_resourcesMutex);

        auto it = m_resources.find(id);
        if (it == m_resources.end()) {
            return false;
        }

        auto resource = it->second;
        if (resource->GetReferenceCount() > 0) {
            return false; // Still referenced
        }

        resource->Unload();
        m_resources.erase(it);

        // Remove from cache
        m_cache.RemoveResource(id);

        // Stop streaming if active
        if (m_streamer.IsStreaming(id)) {
            m_streamer.StopStreaming(id);
        }

        return true;
    }

    bool ResourceSystem::ReloadResource(const std::string& id) {
        std::shared_lock<std::shared_mutex> lock(m_resourcesMutex);

        auto it = m_resources.find(id);
        if (it == m_resources.end()) {
            return false;
        }

        return it->second->Reload();
    }

    std::shared_ptr<Resource> ResourceSystem::GetResource(const std::string& id) const {
        std::shared_lock<std::shared_mutex> lock(m_resourcesMutex);

        auto it = m_resources.find(id);
        return it != m_resources.end() ? it->second : nullptr;
    }

    bool ResourceSystem::IsResourceLoaded(const std::string& id) const {
        auto resource = GetResource(id);
        return resource && resource->IsReady();
    }

    bool ResourceSystem::IsResourceLoading(const std::string& id) const {
        std::shared_lock<std::shared_mutex> lock(m_loadMutex);
        return m_activeLoads.find(id) != m_activeLoads.end();
    }

    void ResourceSystem::RegisterLoader(std::shared_ptr<ResourceLoader> loader) {
        if (!loader) return;

        std::unique_lock<std::shared_mutex> lock(m_loadersMutex);
        m_loaders[loader->GetType()] = loader;
    }

    void ResourceSystem::UnregisterLoader(ResourceType type) {
        std::unique_lock<std::shared_mutex> lock(m_loadersMutex);
        m_loaders.erase(type);
    }

    std::shared_ptr<ResourceLoader> ResourceSystem::GetLoader(ResourceType type) const {
        std::shared_lock<std::shared_mutex> lock(m_loadersMutex);

        auto it = m_loaders.find(type);
        return it != m_loaders.end() ? it->second : nullptr;
    }

    bool ResourceSystem::AddToCache(const std::shared_ptr<Resource>& resource, CachePolicy policy) {
        return m_cache.AddResource(resource, policy);
    }

    bool ResourceSystem::RemoveFromCache(const std::string& id) {
        return m_cache.RemoveResource(id);
    }

    void ResourceSystem::ClearCache() {
        m_cache.Clear();
    }

    void ResourceSystem::OptimizeCache() {
        m_cache.Optimize();
    }

    bool ResourceSystem::StartStreaming(const std::string& id, const std::string& path) {
        return m_streamer.StartStreaming(id, path);
    }

    bool ResourceSystem::StopStreaming(const std::string& id) {
        return m_streamer.StopStreaming(id);
    }

    size_t ResourceSystem::GetStreamedData(const std::string& id, std::vector<uint8_t>& buffer,
                                          size_t offset, size_t size) {
        return m_streamer.GetStreamedData(id, buffer, offset, size);
    }

    std::vector<std::string> ResourceSystem::GetLoadedResources(ResourceType type) const {
        std::shared_lock<std::shared_mutex> lock(m_resourcesMutex);

        std::vector<std::string> resources;
        for (const auto& pair : m_resources) {
            if (type == ResourceType::MAX_TYPES || pair.second->GetType() == type) {
                if (pair.second->IsReady()) {
                    resources.push_back(pair.first);
                }
            }
        }

        return resources;
    }

    std::vector<std::string> ResourceSystem::GetLoadingResources() const {
        std::shared_lock<std::shared_mutex> lock(m_loadMutex);

        std::vector<std::string> loading;
        for (const auto& pair : m_activeLoads) {
            loading.push_back(pair.first);
        }

        return loading;
    }

    std::vector<std::string> ResourceSystem::GetFailedResources() const {
        std::shared_lock<std::shared_mutex> lock(m_resourcesMutex);

        std::vector<std::string> failed;
        for (const auto& pair : m_resources) {
            if (pair.second->HasError()) {
                failed.push_back(pair.first);
            }
        }

        return failed;
    }

    ResourceSystem::ResourceStats ResourceSystem::GetStats() const {
        std::shared_lock<std::shared_mutex> lock(m_statsMutex);
        return m_stats;
    }

    void ResourceSystem::SetCacheMemoryLimit(size_t limit) {
        m_cacheMemoryLimit = limit;
        m_cache.SetMaxMemorySize(limit);
    }

    size_t ResourceSystem::GetCacheMemoryLimit() const {
        return m_cacheMemoryLimit;
    }

    void ResourceSystem::SetCacheDiskLimit(size_t limit) {
        m_cacheDiskLimit = limit;
        m_cache.SetMaxDiskSize(limit);
    }

    size_t ResourceSystem::GetCacheDiskLimit() const {
        return m_cacheDiskLimit;
    }

    std::string ResourceSystem::GetResourceInfo() const {
        std::stringstream ss;
        ss << "Resource System Info:\n";
        ss << "Loaded Resources: " << GetLoadedResources().size() << "\n";
        ss << "Loading Resources: " << GetLoadingResources().size() << "\n";
        ss << "Failed Resources: " << GetFailedResources().size() << "\n";
        ss << "Cache Memory: " << m_cache.GetMemoryUsage() << " bytes\n";
        ss << "Cache Disk: " << m_cache.GetDiskUsage() << " bytes\n";
        ss << "Active Streams: " << m_activeLoads.size() << "\n";
        return ss.str();
    }

    void ResourceSystem::PrintResourceReport() const {
        auto stats = GetStats();
        std::stringstream report;

        report << "=== Resource System Report ===\n";
        report << "Total Resources: " << stats.totalResources << "\n";
        report << "Loaded Resources: " << stats.loadedResources << "\n";
        report << "Loading Resources: " << stats.loadingResources << "\n";
        report << "Failed Resources: " << stats.failedResources << "\n";
        report << "Total Memory Usage: " << stats.totalMemoryUsage << " bytes\n";
        report << "Cache Memory Usage: " << stats.cacheMemoryUsage << " bytes\n";
        report << "Cache Disk Usage: " << stats.cacheDiskUsage << " bytes\n";
        report << "Active Streams: " << stats.activeStreams << "\n";
        report << "Average Load Time: " << stats.averageLoadTime << " ms\n";
        report << "Cache Hit Rate: " << (stats.cacheHits + stats.cacheMisses > 0 ?
            100.0f * stats.cacheHits / (stats.cacheHits + stats.cacheMisses) : 0.0f) << "%\n";

        Logger::GetInstance().Info(report.str(), "ResourceSystem");
    }

    bool ResourceSystem::ValidateAllResources() const {
        std::shared_lock<std::shared_mutex> lock(m_resourcesMutex);

        for (const auto& pair : m_resources) {
            if (!pair.second->Validate()) {
                return false;
            }
        }

        return true;
    }

    void ResourceSystem::CleanupUnusedResources() {
        std::unique_lock<std::shared_mutex> lock(m_resourcesMutex);

        std::vector<std::string> toRemove;
        for (const auto& pair : m_resources) {
            if (pair.second->GetReferenceCount() == 0 && pair.second->IsReady()) {
                toRemove.push_back(pair.first);
            }
        }

        for (const auto& id : toRemove) {
            m_resources.erase(id);
        }
    }

    bool ResourceSystem::AddResourceDependency(const std::string& resourceId, const std::string& dependencyId) {
        auto resource = GetResource(resourceId);
        if (!resource) return false;

        resource->GetMetadata().dependencies.push_back(dependencyId);
        return true;
    }

    std::vector<std::string> ResourceSystem::GetResourceDependencies(const std::string& resourceId) const {
        auto resource = GetResource(resourceId);
        if (!resource) return {};

        return resource->GetMetadata().dependencies;
    }

    bool ResourceSystem::CheckResourceDependencies(const std::string& resourceId) const {
        auto resource = GetResource(resourceId);
        if (!resource) return false;

        for (const auto& depId : resource->GetMetadata().dependencies) {
            if (!IsResourceLoaded(depId)) {
                return false;
            }
        }

        return true;
    }

    bool ResourceSystem::OptimizeResource(const std::string& id, const std::unordered_map<std::string, std::any>& options) {
        auto resource = GetResource(id);
        if (!resource) return false;

        // Implement resource optimization based on options
        return true;
    }

    bool ResourceSystem::CompressResource(const std::string& id, CompressionType type) {
        auto resource = GetResource(id);
        if (!resource) return false;

        // Implement resource compression
        return true;
    }

    bool ResourceSystem::DecompressResource(const std::string& id) {
        auto resource = GetResource(id);
        if (!resource) return false;

        // Implement resource decompression
        return true;
    }

    void ResourceSystem::StartLoaderThreads() {
        for (uint32_t i = 0; i < m_maxConcurrentLoads; ++i) {
            m_loaderThreads.emplace_back(&ResourceSystem::LoaderThreadFunction, this);
        }

        Logger::GetInstance().Info("Started " + std::to_string(m_maxConcurrentLoads) + " resource loader threads", "ResourceSystem");
    }

    void ResourceSystem::StopLoaderThreads() {
        {
            std::unique_lock<std::shared_mutex> lock(m_loadMutex);
            // Signal threads to stop
        }
        m_loadCondition.notify_all();

        for (auto& thread : m_loaderThreads) {
            if (thread.joinable()) {
                thread.join();
            }
        }

        m_loaderThreads.clear();

        Logger::GetInstance().Info("Stopped resource loader threads", "ResourceSystem");
    }

    void ResourceSystem::LoaderThreadFunction() {
        while (true) {
            ResourceLoadTask task;

            {
                std::unique_lock<std::shared_mutex> lock(m_loadMutex);
                m_loadCondition.wait(lock, [this]() {
                    return !m_loadQueue.empty();
                });

                if (m_loadQueue.empty()) {
                    break; // Shutdown signal
                }

                task = m_loadQueue.top();
                m_loadQueue.pop();
                m_activeLoads[task.resourceId] = task;
            }

            // Load the resource
            auto resource = LoadResourceInternal(task);

            {
                std::unique_lock<std::shared_mutex> lock(m_loadMutex);
                m_activeLoads.erase(task.resourceId);

                if (resource) {
                    std::unique_lock<std::shared_mutex> resourceLock(m_resourcesMutex);
                    m_resources[task.resourceId] = resource;
                }
            }

            UpdateStats(task.resourceId, resource != nullptr, false);
        }
    }

    std::shared_ptr<Resource> ResourceSystem::LoadResourceInternal(const ResourceLoadTask& task) {
        // Find appropriate loader
        auto loader = FindLoaderForPath(task.path);
        if (!loader) {
            loader = GetLoader(task.type);
        }

        if (!loader) {
            Logger::GetInstance().Error("No loader found for resource: " + task.resourceId, "ResourceSystem");
            return nullptr;
        }

        // Load resource data
        std::vector<uint8_t> data;
        if (!std::filesystem::exists(task.path)) {
            Logger::GetInstance().Error("Resource file not found: " + task.path, "ResourceSystem");
            return nullptr;
        }

        std::ifstream file(task.path, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            Logger::GetInstance().Error("Failed to open resource file: " + task.path, "ResourceSystem");
            return nullptr;
        }

        size_t fileSize = file.tellg();
        data.resize(fileSize);
        file.seekg(0);
        file.read(reinterpret_cast<char*>(data.data()), fileSize);
        file.close();

        // Create resource instance
        std::shared_ptr<Resource> resource;
        switch (task.type) {
            case ResourceType::TEXTURE:
                resource = std::make_shared<TextureResource>(task.resourceId);
                break;
            case ResourceType::MODEL:
                resource = std::make_shared<ModelResource>(task.resourceId);
                break;
            case ResourceType::AUDIO:
                resource = std::make_shared<AudioResource>(task.resourceId);
                break;
            default:
                Logger::GetInstance().Error("Unsupported resource type: " + std::to_string(static_cast<int>(task.type)), "ResourceSystem");
                return nullptr;
        }

        // Load resource
        if (!resource->Load(data)) {
            Logger::GetInstance().Error("Failed to load resource: " + task.resourceId, "ResourceSystem");
            return nullptr;
        }

        return resource;
    }

    bool ResourceSystem::ProcessLoadQueue() {
        // This would process the async load queue
        return true;
    }

    void ResourceSystem::UpdateResourceState(const std::string& id, ResourceState state) {
        auto resource = GetResource(id);
        if (resource) {
            // Update resource state (would need to be thread-safe)
        }
    }

    void ResourceSystem::UpdateStats(const std::string& id, bool loaded, bool failed) {
        std::unique_lock<std::shared_mutex> lock(m_statsMutex);

        if (loaded) {
            m_stats.loadedResources++;
        } else if (failed) {
            m_stats.failedResources++;
        }

        m_stats.totalResources = m_resources.size();
        m_stats.loadingResources = m_activeLoads.size();
    }

    bool ResourceSystem::LoadFromCache(const std::string& id, std::shared_ptr<Resource>& resource) {
        resource = m_cache.GetResource(id);
        return resource != nullptr;
    }

    bool ResourceSystem::SaveToCache(const std::shared_ptr<Resource>& resource) {
        return m_cache.AddResource(resource, CachePolicy::MEMORY_ONLY);
    }

    ResourceType ResourceSystem::DetectResourceType(const std::string& path) const {
        std::string ext = std::filesystem::path(path).extension().string();

        if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".tga" || ext == ".bmp") {
            return ResourceType::TEXTURE;
        } else if (ext == ".obj" || ext == ".fbx" || ext == ".dae" || ext == ".gltf" || ext == ".glb") {
            return ResourceType::MODEL;
        } else if (ext == ".wav" || ext == ".mp3" || ext == ".ogg" || ext == ".flac") {
            return ResourceType::AUDIO;
        } else if (ext == ".json" || ext == ".xml" || ext == ".yaml") {
            return ResourceType::CONFIG;
        }

        return ResourceType::CUSTOM_1;
    }

    std::shared_ptr<ResourceLoader> ResourceSystem::FindLoaderForPath(const std::string& path) const {
        ResourceType type = DetectResourceType(path);
        return GetLoader(type);
    }

} // namespace VoxelCraft
