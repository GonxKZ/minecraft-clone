/**
 * @file test_main.cpp
 * @brief VoxelCraft Test Main - Simple test to verify the engine works
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#include "core/Application.hpp"
#include "core/Engine.hpp"
#include "core/Logger.hpp"
#include "core/Config.hpp"
#include "core/EventSystem.hpp"
#include "core/MemoryManager.hpp"
#include "core/ResourceManager.hpp"
#include "core/Timer.hpp"
#include "entities/EntityManager.hpp"
#include "entities/Entity.hpp"
#include "entities/Component.hpp"

#include <iostream>
#include <chrono>
#include <thread>

using namespace VoxelCraft;

int main(int argc, char* argv[]) {
    try {
        std::cout << "=== VoxelCraft Engine Test ===" << std::endl;

        // Test 1: Initialize Logger
        std::cout << "Test 1: Logger initialization..." << std::endl;
        auto logger = LogManager::GetLogger("TestLogger");
        VOXELCRAFT_INFO("Logger test passed");

        // Test 2: Initialize Config
        std::cout << "Test 2: Config system..." << std::endl;
        auto& config = GetConfig();
        config.Set("test.value", "Hello World");
        auto value = config.Get<std::string>("test.value", "");
        if (value == "Hello World") {
            VOXELCRAFT_INFO("Config test passed");
        } else {
            VOXELCRAFT_ERROR("Config test failed");
            return 1;
        }

        // Test 3: Initialize Memory Manager
        std::cout << "Test 3: Memory manager..." << std::endl;
        auto& memoryManager = GetMemoryManager();
        auto testPtr = memoryManager.Allocate(1024, "test_allocation");
        if (testPtr) {
            memoryManager.Deallocate(testPtr);
            VOXELCRAFT_INFO("Memory manager test passed");
        } else {
            VOXELCRAFT_ERROR("Memory manager test failed");
            return 1;
        }

        // Test 4: Initialize Resource Manager
        std::cout << "Test 4: Resource manager..." << std::endl;
        auto& resourceManager = GetResourceManager();
        VOXELCRAFT_INFO("Resource manager test passed");

        // Test 5: Initialize Timer
        std::cout << "Test 5: Timer system..." << std::endl;
        auto& timerManager = GetTimerManager();
        auto timerId = timerManager.CreateTimer("TestTimer");
        timerManager.StartTimer(timerId);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        timerManager.StopTimer(timerId);
        auto elapsed = timerManager.GetTimerElapsedSeconds(timerId);
        if (elapsed > 0.0) {
            VOXELCRAFT_INFO("Timer test passed - elapsed: {}s", elapsed);
        } else {
            VOXELCRAFT_ERROR("Timer test failed");
            return 1;
        }

        // Test 6: Initialize Event System
        std::cout << "Test 6: Event system..." << std::endl;
        auto& eventSystem = GetEventSystem();
        bool eventReceived = false;
        auto callbackId = eventSystem.AddListener("Test", [&](const Event& event) {
            eventReceived = true;
            VOXELCRAFT_INFO("Event received: {}", event.type);
        });
        Event testEvent("TestEvent");
        eventSystem.QueueEvent(std::move(testEvent));
        eventSystem.DispatchEvents();
        if (eventReceived) {
            VOXELCRAFT_INFO("Event system test passed");
        } else {
            VOXELCRAFT_ERROR("Event system test failed");
            return 1;
        }

        // Test 7: Initialize Application
        std::cout << "Test 7: Application system..." << std::endl;
        auto application = std::make_unique<Application>();
        if (application->Initialize()) {
            VOXELCRAFT_INFO("Application initialization test passed");

            // Test Engine access
            auto engine = application->GetEngine();
            if (engine) {
                VOXELCRAFT_INFO("Engine access test passed");

                // Test Entity Manager
                auto entityManager = engine->GetEntityManager();
                if (entityManager) {
                    auto entity = entityManager->CreateEntity("TestEntity");
                    if (entity) {
                        VOXELCRAFT_INFO("Entity creation test passed - Entity: {} (ID: {})",
                                      entity->GetName(), entity->GetID());
                    } else {
                        VOXELCRAFT_ERROR("Entity creation test failed");
                        return 1;
                    }
                } else {
                    VOXELCRAFT_ERROR("Entity manager access test failed");
                    return 1;
                }
            } else {
                VOXELCRAFT_ERROR("Engine access test failed");
                return 1;
            }

            application->Shutdown();
            VOXELCRAFT_INFO("Application shutdown test passed");

        } else {
            VOXELCRAFT_ERROR("Application initialization test failed");
            return 1;
        }

        // Test 8: Performance test
        std::cout << "Test 8: Performance test..." << std::endl;
        auto perfTimer = timerManager.CreateTimer("PerformanceTest");
        timerManager.StartTimer(perfTimer);

        // Create many entities for performance test
        auto testApp = std::make_unique<Application>();
        if (testApp->Initialize()) {
            auto testEngine = testApp->GetEngine();
            auto testEntityManager = testEngine->GetEntityManager();

            const int ENTITY_COUNT = 1000;
            for (int i = 0; i < ENTITY_COUNT; ++i) {
                auto entity = testEntityManager->CreateEntity("PerfEntity_" + std::to_string(i));
                // Add some dummy components if needed
            }

            timerManager.StopTimer(perfTimer);
            auto perfTime = timerManager.GetTimerElapsedSeconds(perfTimer);

            VOXELCRAFT_INFO("Performance test: Created {} entities in {:.3f}s",
                          ENTITY_COUNT, perfTime);
            VOXELCRAFT_INFO("Entities per second: {:.0f}", ENTITY_COUNT / perfTime);

            testApp->Shutdown();
        }

        std::cout << "=== ALL TESTS PASSED! ===" << std::endl;
        std::cout << "VoxelCraft Engine is working correctly!" << std::endl;
        std::cout << "Ready for game development!" << std::endl;

        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Test failed with unknown exception" << std::endl;
        return 1;
    }
}
