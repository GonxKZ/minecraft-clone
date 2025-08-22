/**
 * @file Profiler.hpp
 * @brief VoxelCraft Performance Profiler
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#ifndef VOXELCRAFT_CORE_PROFILER_HPP
#define VOXELCRAFT_CORE_PROFILER_HPP

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <chrono>
#include <mutex>

namespace VoxelCraft {

class Profiler {
public:
    Profiler();
    ~Profiler();

    bool Initialize();
    void Shutdown();
    void Update(double deltaTime);

private:
    bool m_initialized;
};

} // namespace VoxelCraft

#endif // VOXELCRAFT_CORE_PROFILER_HPP
