#include "Renderer.hpp"

namespace VoxelCraft {

	Renderer::Renderer(const RendererConfig& config)
		: m_config(config)
		, m_window(nullptr)
		, m_camera(nullptr)
		, m_initialized(false)
		, m_frameActive(false)
	{
		m_stats = {};
		VOXEL_LOG_INFO("Renderer created");
	}

	Renderer::~Renderer()
	{
		if (m_initialized) {
			Shutdown();
		}
	}

	bool Renderer::Initialize(Window* window)
	{
		m_window = window;
		m_initialized = true;
		VOXEL_LOG_INFO("Renderer initialized");
		return true;
	}

	void Renderer::Shutdown()
	{
		m_initialized = false;
		VOXEL_LOG_INFO("Renderer shutdown");
	}

	bool Renderer::BeginFrame()
	{
		if (!m_initialized || m_frameActive) {
			return false;
		}

		m_frameActive = true;
		return true;
	}

	void Renderer::EndFrame()
	{
		if (!m_initialized || !m_frameActive) {
			return;
		}

		m_frameActive = false;
		m_stats.frameCount++;
	}

	void Renderer::Render()
	{
		if (!m_initialized || !m_frameActive) {
			return;
		}

		// Basic rendering implementation
		SortRenderCommands();
		ExecuteRenderCommands();
	}

	size_t Renderer::GetMemoryUsage() const
	{
		return sizeof(Renderer);
	}

	void Renderer::SortRenderCommands()
	{
		std::lock_guard<std::mutex> lock(m_commandMutex);

		std::sort(m_renderCommands.begin(), m_renderCommands.end(),
			[](const RenderCommand& a, const RenderCommand& b) {
				return a.sortKey < b.sortKey;
			});
	}

	void Renderer::ExecuteRenderCommands()
	{
		std::lock_guard<std::mutex> lock(m_commandMutex);

		for (const auto& command : m_renderCommands) {
			command.execute();
		}
	}

} // namespace VoxelCraft
