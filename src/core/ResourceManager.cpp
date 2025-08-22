/**
 * @file ResourceManager.cpp
 * @brief VoxelCraft Engine Resource Manager Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file implements the resource management system for the VoxelCraft engine.
 */

#include "ResourceManager.hpp"
#include "Logger.hpp"

#include <fstream>
#include <sstream>
#include <algorithm>

namespace VoxelCraft {

    // Global resource manager instance
    static std::unique_ptr<ResourceManager> s_instance;
    static std::mutex s_instanceMutex;

    // Resource implementation

    Resource::Resource(ResourceID id, const std::string& name, ResourceType type)
        : m_id(id)
        , m_name(name)
        , m_type(type)
        , m_state(ResourceState::Unloaded)
        , m_refCount(0)
        , m_memoryUsage(0)
        , m_loadTime(0.0)
        , m_lastAccess(0.0)
    {
    }

    Resource::~Resource() {
        VOXELCRAFT_TRACE("Resource '{}' destroyed", m_name);
    }

    void Resource::AddRef() {
        m_refCount++;
        m_lastAccess = GetCurrentTime();
    }

    bool Resource::Release() {
        if (m_refCount > 0) {
            m_refCount--;
            if (m_refCount == 0) {
                return true; // Can be unloaded
            }
        }
        return false;
    }

    double Resource::GetCurrentTime() {
        return std::chrono::duration_cast<std::chrono::duration<double>>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();
    }

    // ResourceManager implementation

    ResourceManager::ResourceManager()
        : m_nextResourceId(1)
        , m_maxCacheSize(100 * 1024 * 1024) // 100MB default
        , m_currentCacheSize(0)
        , m_autoUnloadEnabled(true)
        , m_asyncLoadingEnabled(true)
        , m_compressionEnabled(false)
        , m_hotReloadEnabled(false)
        , m_loadingThreadCount(2)
        , m_running(false)
    {
        VOXELCRAFT_TRACE("Resource manager instance created");
    }

    ResourceManager::~ResourceManager() {
        VOXELCRAFT_TRACE("Resource manager instance destroyed");

        StopAsyncLoading();
        ClearCache();
    }

    bool ResourceManager::Initialize() {
        VOXELCRAFT_INFO("Initializing resource manager");

        if (m_asyncLoadingEnabled) {
            StartAsyncLoading();
        }

        VOXELCRAFT_INFO("Resource manager initialized with cache size: {}MB", m_maxCacheSize / (1024 * 1024));
        return true;
    }

    void ResourceManager::Shutdown() {
        VOXELCRAFT_INFO("Shutting down resource manager");

        StopAsyncLoading();
        ClearCache();
    }

    ResourceHandle ResourceManager::LoadResource(const std::string& path, ResourceType type) {
        std::lock_guard<std::mutex> lock(m_resourceMutex);

        // Check if already loaded
        auto it = m_resourceMap.find(path);
        if (it != m_resourceMap.end()) {
            Resource* resource = it->second;
            resource->AddRef();
            return std::shared_ptr<void>(resource->GetData(), [this, resource](void*) {
                if (resource->Release()) {
                    UnloadResource(resource->GetID());
                }
            });
        }

        // Create new resource
        ResourceID id = m_nextResourceId++;
        auto resource = std::make_unique<Resource>(id, path, type);
        resource->SetState(ResourceState::Loading);

        // Load resource data (simplified)
        if (LoadResourceData(resource.get())) {
            resource->SetState(ResourceState::Loaded);
            resource->AddRef();

            Resource* resourcePtr = resource.get();
            m_resources[id] = std::move(resource);
            m_resourceMap[path] = resourcePtr;

            return std::shared_ptr<void>(resourcePtr->GetData(), [this, resourcePtr](void*) {
                if (resourcePtr->Release()) {
                    UnloadResource(resourcePtr->GetID());
                }
            });
        }

        VOXELCRAFT_ERROR("Failed to load resource: {}", path);
        return nullptr;
    }

    bool ResourceManager::UnloadResource(ResourceID id) {
        std::lock_guard<std::mutex> lock(m_resourceMutex);

        auto it = m_resources.find(id);
        if (it != m_resources.end()) {
            Resource* resource = it->second.get();
            m_resourceMap.erase(resource->GetName());
            m_resources.erase(it);

            VOXELCRAFT_INFO("Unloaded resource '{}' (ID: {})", resource->GetName(), id);
            return true;
        }

        return false;
    }

    ResourceHandle ResourceManager::GetResource(const std::string& path) {
        std::lock_guard<std::mutex> lock(m_resourceMutex);

        auto it = m_resourceMap.find(path);
        if (it != m_resourceMap.end()) {
            Resource* resource = it->second;
            resource->AddRef();
            return std::shared_ptr<void>(resource->GetData(), [this, resource](void*) {
                if (resource->Release()) {
                    UnloadResource(resource->GetID());
                }
            });
        }

        return nullptr;
    }

    bool ResourceManager::IsResourceLoaded(const std::string& path) const {
        std::lock_guard<std::mutex> lock(m_resourceMutex);
        return m_resourceMap.count(path) > 0;
    }

    std::vector<ResourceInfo> ResourceManager::GetLoadedResources() const {
        std::lock_guard<std::mutex> lock(m_resourceMutex);
        std::vector<ResourceInfo> info;

        for (const auto& pair : m_resources) {
            const Resource* resource = pair.second.get();
            ResourceInfo resInfo;
            resInfo.id = resource->GetID();
            resInfo.name = resource->GetName();
            resInfo.type = resource->GetType();
            resInfo.state = resource->GetState();
            resInfo.refCount = resource->GetRefCount();
            resInfo.memoryUsage = resource->GetMemoryUsage();
            info.push_back(resInfo);
        }

        return info;
    }

    size_t ResourceManager::GetCacheSize() const {
        return m_currentCacheSize;
    }

    size_t ResourceManager::GetMaxCacheSize() const {
        return m_maxCacheSize;
    }

    void ResourceManager::SetMaxCacheSize(size_t size) {
        m_maxCacheSize = size;
        VOXELCRAFT_INFO("Max cache size set to {} bytes", size);
    }

    bool ResourceManager::ClearCache() {
        std::lock_guard<std::mutex> lock(m_resourceMutex);

        size_t resourceCount = m_resources.size();
        m_resources.clear();
        m_resourceMap.clear();
        m_currentCacheSize = 0;

        VOXELCRAFT_INFO("Cleared resource cache, removed {} resources", resourceCount);
        return true;
    }

    void ResourceManager::StartAsyncLoading() {
        if (m_running) {
            return;
        }

        m_running = true;
        for (uint32_t i = 0; i < m_loadingThreadCount; ++i) {
            m_loadingThreads.emplace_back(&ResourceManager::LoadingThreadFunction, this, i);
        }

        VOXELCRAFT_INFO("Started {} async loading threads", m_loadingThreadCount);
    }

    void ResourceManager::StopAsyncLoading() {
        if (!m_running) {
            return;
        }

        m_running = false;
        m_loadingCondition.notify_all();

        for (auto& thread : m_loadingThreads) {
            if (thread.joinable()) {
                thread.join();
            }
        }
        m_loadingThreads.clear();

        VOXELCRAFT_INFO("Stopped async loading threads");
    }

    void ResourceManager::LoadingThreadFunction(int threadId) {
        VOXELCRAFT_INFO("Resource loading thread {} started", threadId);

        while (m_running) {
            std::unique_lock<std::mutex> lock(m_loadingMutex);

            m_loadingCondition.wait(lock, [this]() {
                return !m_loadingQueue.empty() || !m_running;
            });

            if (!m_running) {
                break;
            }

            if (!m_loadingQueue.empty()) {
                LoadingTask task = std::move(m_loadingQueue.front());
                m_loadingQueue.pop();
                lock.unlock();

                // Execute loading task
                try {
                    // Load resource (simplified)
                    VOXELCRAFT_TRACE("Loading resource '{}' in thread {}", task.path, threadId);
                    std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Simulate loading

                    if (task.callback) {
                        task.callback(task.path, true); // Success
                    }
                } catch (const std::exception& e) {
                    VOXELCRAFT_ERROR("Exception in loading thread {}: {}", threadId, e.what());
                    if (task.callback) {
                        task.callback(task.path, false); // Failure
                    }
                }
            }
        }

        VOXELCRAFT_INFO("Resource loading thread {} stopped", threadId);
    }

    bool ResourceManager::LoadResourceData(Resource* resource) {
        // Simplified resource loading - in a real implementation this would
        // load different file formats based on resource type

        const std::string& path = resource->GetName();

        try {
            // Simulate loading different resource types
            if (resource->GetType() == ResourceType::Texture) {
                // Simulate texture loading
                size_t dataSize = 1024 * 1024; // 1MB texture
                char* data = new char[dataSize];
                resource->SetData(data, dataSize);
                resource->SetMemoryUsage(dataSize);

            } else if (resource->GetType() == ResourceType::Model) {
                // Simulate model loading
                size_t dataSize = 512 * 1024; // 512KB model
                char* data = new char[dataSize];
                resource->SetData(data, dataSize);
                resource->SetMemoryUsage(dataSize);

            } else {
                // Generic resource
                size_t dataSize = 64 * 1024; // 64KB generic
                char* data = new char[dataSize];
                resource->SetData(data, dataSize);
                resource->SetMemoryUsage(dataSize);
            }

            m_currentCacheSize += resource->GetMemoryUsage();
            return true;

        } catch (const std::exception& e) {
            VOXELCRAFT_ERROR("Exception loading resource '{}': {}", path, e.what());
            return false;
        }
    }

    // Resource implementation continued

    void Resource::SetData(void* data, size_t size) {
        m_data = data;
        m_memoryUsage = size;
        m_loadTime = GetCurrentTime();
    }

    void* Resource::GetData() const {
        return m_data;
    }

    // Global accessor

    ResourceManager& GetResourceManager() {
        std::lock_guard<std::mutex> lock(s_instanceMutex);
        if (!s_instance) {
            s_instance = std::make_unique<ResourceManager>();
        }
        return *s_instance;
    }

} // namespace VoxelCraft
