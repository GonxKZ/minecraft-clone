#pragma once
#include <memory>
#include <vector>
#include <unordered_map>
#include <string>
#include <functional>
#include <mutex>
#include <atomic>
#include <cstdint>
#include "core/Logger.hpp"

namespace VoxelCraft {

	class Window;
	class Camera;
	class Shader;
	class Mesh;
	class Texture;
	class Material;
	class RenderTarget;
	class VertexBuffer;
	class IndexBuffer;
	class VertexArray;
	class UniformBuffer;
	class FrameBuffer;
	class RenderPass;
	class Pipeline;
	class CommandBuffer;

	/**
	 * @brief Graphics API enumeration
	 */
	enum class GraphicsAPI
	{
		OPENGL,
		VULKAN,
		DIRECTX12,
		METAL
	};

	/**
	 * @brief Render mode enumeration
	 */
	enum class RenderMode
	{
		FORWARD,
		DEFERRED,
		TILED,
		CLUSTERED,
		RAY_TRACING
	};

	/**
	 * @brief Rendering quality enumeration
	 */
	enum class RenderQuality
	{
		LOW,
		MEDIUM,
		HIGH,
		ULTRA,
		CUSTOM
	};

	/**
	 * @brief Anti-aliasing mode enumeration
	 */
	enum class AntiAliasing
	{
		NONE,
		MSAA_2X,
		MSAA_4X,
		MSAA_8X,
		MSAA_16X,
		FXAA,
		TAA,
		SSAA
	};

	/**
	 * @brief Texture filtering mode enumeration
	 */
	enum class TextureFilter
	{
		NEAREST,
		BILINEAR,
		TRILINEAR,
		ANISOTROPIC_2X,
		ANISOTROPIC_4X,
		ANISOTROPIC_8X,
		ANISOTROPIC_16X
	};

	/**
	 * @brief Shadow quality enumeration
	 */
	enum class ShadowQuality
	{
		NONE,
		LOW,
		MEDIUM,
		HIGH,
		ULTRA,
		RAY_TRACED
	};

	/**
	 * @brief Renderer configuration structure
	 */
	struct RendererConfig
	{
		GraphicsAPI api = GraphicsAPI::OPENGL;
		RenderMode mode = RenderMode::DEFERRED;
		RenderQuality quality = RenderQuality::HIGH;
		AntiAliasing antiAliasing = AntiAliasing::MSAA_4X;
		TextureFilter textureFilter = TextureFilter::ANISOTROPIC_8X;
		ShadowQuality shadowQuality = ShadowQuality::HIGH;

		// Resolution and display
		uint32_t width = 1920;
		uint32_t height = 1080;
		bool fullscreen = false;
		bool vsync = true;
		float refreshRate = 60.0f;

		// Performance settings
		uint32_t maxFramesInFlight = 2;
		uint32_t maxTextureUnits = 32;
		uint32_t maxVertexAttributes = 16;
		size_t maxMemoryUsage = 1024 * 1024 * 1024; // 1GB

		// Advanced features
		bool enableRayTracing = false;
		bool enableDLSS = false;
		bool enableFSR = false;
		bool enableRTX = false;
		bool enableVR = false;
		bool enableAR = false;

		// Debug features
		bool enableWireframe = false;
		bool enableNormals = false;
		bool enableBounds = false;
		bool enableStats = true;
		bool enableProfiling = true;
	};

	/**
	 * @brief Renderer statistics structure
	 */
	struct RendererStats
	{
		// Frame timing
		float frameTime;
		float renderTime;
		float gpuTime;
		uint32_t fps;
		uint32_t frameCount;

		// Resource counts
		uint32_t drawCalls;
		uint32_t triangles;
		uint32_t vertices;
		uint32_t shaders;
		uint32_t textures;
		uint32_t materials;
		uint32_t meshes;

		// Memory usage
		size_t memoryUsed;
		size_t memoryAvailable;
		size_t textureMemory;
		size_t meshMemory;
		size_t bufferMemory;

		// GPU stats
		float gpuTemperature;
		float gpuUtilization;
		uint32_t gpuMemoryUsed;
		uint32_t gpuMemoryTotal;

		// Performance counters
		uint32_t cullingTests;
		uint32_t cullingPassed;
		uint32_t occlusionTests;
		uint32_t occlusionPassed;
		uint32_t shadowDrawCalls;
		uint32_t lightDrawCalls;
		uint32_t particleDrawCalls;
	};

	/**
	 * @brief Render command structure
	 */
	struct RenderCommand
	{
		uint32_t sortKey;
		std::function<void()> execute;
	};

	/**
	 * @brief Main Renderer class
	 *
	 * The Renderer is responsible for:
	 * - Managing graphics API abstraction
	 * - Handling rendering pipeline
	 * - Managing resources (textures, shaders, meshes)
	 * - Implementing rendering techniques (deferred, forward, etc.)
	 * - Performance optimization and profiling
	 * - Multi-threading support
	 */
	class Renderer
	{
	public:
		/**
		 * @brief Constructor
		 */
		Renderer(const RendererConfig& config = RendererConfig());

		/**
		 * @brief Destructor
		 */
		~Renderer();

		/**
		 * @brief Initialize renderer
		 */
		bool Initialize(Window* window);

		/**
		 * @brief Shutdown renderer
		 */
		void Shutdown();

		/**
		 * @brief Begin frame rendering
		 */
		bool BeginFrame();

		/**
		 * @brief End frame rendering
		 */
		void EndFrame();

		/**
		 * @brief Submit render command
		 */
		void SubmitCommand(const RenderCommand& command);

		/**
		 * @brief Render frame
		 */
		void Render();

		/**
		 * @brief Present frame to screen
		 */
		void Present();

		/**
		 * @brief Resize viewport
		 */
		void Resize(uint32_t width, uint32_t height);

		/**
		 * @brief Set camera for rendering
		 */
		void SetCamera(Camera* camera);

		/**
		 * @brief Get current camera
		 */
		Camera* GetCamera() const { return m_camera; }

		/**
		 * @brief Create shader
		 */
		std::shared_ptr<Shader> CreateShader(const std::string& vertexSource,
			const std::string& fragmentSource,
			const std::string& geometrySource = "",
			const std::string& tessControlSource = "",
			const std::string& tessEvalSource = "");

		/**
		 * @brief Create texture
		 */
		std::shared_ptr<Texture> CreateTexture(const std::string& path);

		/**
		 * @brief Create texture from data
		 */
		std::shared_ptr<Texture> CreateTexture(const void* data, uint32_t width,
			uint32_t height, uint32_t channels);

		/**
		 * @brief Create material
		 */
		std::shared_ptr<Material> CreateMaterial(const std::string& name);

		/**
		 * @brief Create mesh
		 */
		std::shared_ptr<Mesh> CreateMesh();

		/**
		 * @brief Create render target
		 */
		std::shared_ptr<RenderTarget> CreateRenderTarget(uint32_t width, uint32_t height);

		/**
		 * @brief Get renderer statistics
		 */
		const RendererStats& GetStats() const { return m_stats; }

		/**
		 * @brief Get renderer configuration
		 */
		const RendererConfig& GetConfig() const { return m_config; }

		/**
		 * @brief Set renderer configuration
		 */
		void SetConfig(const RendererConfig& config);

		/**
		 * @brief Enable/disable wireframe mode
		 */
		void SetWireframeMode(bool enabled);

		/**
		 * @brief Enable/disable VSync
		 */
		void SetVSync(bool enabled);

		/**
		 * @brief Set render quality
		 */
		void SetRenderQuality(RenderQuality quality);

		/**
		 * @brief Take screenshot
		 */
		std::vector<uint8_t> TakeScreenshot();

		/**
		 * @brief Get memory usage
		 */
		size_t GetMemoryUsage() const;

		/**
		 * @brief Check if renderer is initialized
		 */
		bool IsInitialized() const { return m_initialized; }

		/**
		 * @brief Get graphics API
		 */
		GraphicsAPI GetAPI() const { return m_config.api; }

		/**
		 * @brief Get render mode
		 */
		RenderMode GetMode() const { return m_config.mode; }

		/**
		 * @brief Get supported features
		 */
		std::vector<std::string> GetSupportedFeatures() const;

		/**
		 * @brief Check if feature is supported
		 */
		bool IsFeatureSupported(const std::string& feature) const;

	private:
		RendererConfig m_config;
		RendererStats m_stats;
		Window* m_window;
		Camera* m_camera;
		bool m_initialized;

		// Resource management
		std::unordered_map<std::string, std::shared_ptr<Shader>> m_shaders;
		std::unordered_map<std::string, std::shared_ptr<Texture>> m_textures;
		std::unordered_map<std::string, std::shared_ptr<Material>> m_materials;
		std::unordered_map<std::string, std::shared_ptr<Mesh>> m_meshes;

		// Render targets
		std::shared_ptr<RenderTarget> m_mainRenderTarget;
		std::shared_ptr<RenderTarget> m_shadowRenderTarget;
		std::shared_ptr<RenderTarget> m_gBufferRenderTarget;
		std::shared_ptr<RenderTarget> m_postProcessRenderTarget;

		// Render commands
		std::vector<RenderCommand> m_renderCommands;
		std::mutex m_commandMutex;

		// Synchronization
		std::atomic<bool> m_frameActive;
		std::mutex m_renderMutex;

		/**
		 * @brief Initialize graphics API
		 */
		bool InitializeAPI();

		/**
		 * @brief Initialize render targets
		 */
		bool InitializeRenderTargets();

		/**
		 * @brief Initialize default shaders
		 */
		bool InitializeDefaultShaders();

		/**
		 * @brief Initialize default materials
		 */
		bool InitializeDefaultMaterials();

		/**
		 * @brief Render shadow maps
		 */
		void RenderShadows();

		/**
		 * @brief Render G-Buffer
		 */
		void RenderGBuffer();

		/**
		 * @brief Render lighting
		 */
		void RenderLighting();

		/**
		 * @brief Render transparent objects
		 */
		void RenderTransparent();

		/**
		 * @brief Render post-processing effects
		 */
		void RenderPostProcessing();

		/**
		 * @brief Render UI elements
		 */
		void RenderUI();

		/**
		 * @brief Sort render commands
		 */
		void SortRenderCommands();

		/**
		 * @brief Execute render commands
		 */
		void ExecuteRenderCommands();

		/**
		 * @brief Update renderer statistics
		 */
		void UpdateStats();

		/**
		 * @brief Cleanup resources
		 */
		void CleanupResources();

		/**
		 * @brief Handle device lost
		 */
		void OnDeviceLost();

		/**
		 * @brief Handle device restored
		 */
		void OnDeviceRestored();
	};

} // namespace VoxelCraft