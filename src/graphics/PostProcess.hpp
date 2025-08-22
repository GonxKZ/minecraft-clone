/**
 * @file PostProcess.hpp
 * @brief VoxelCraft Advanced Post-Processing Effects System
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file defines the PostProcess class that provides comprehensive post-processing
 * effects for the VoxelCraft game engine, including HDR rendering, bloom, motion blur,
 * depth of field, chromatic aberration, color grading, and advanced visual effects
 * with GPU acceleration and performance optimization.
 */

#ifndef VOXELCRAFT_GRAPHICS_POST_PROCESS_HPP
#define VOXELCRAFT_GRAPHICS_POST_PROCESS_HPP

#include <memory>
#include <vector>
#include <unordered_map>
#include <string>
#include <functional>
#include <mutex>
#include <shared_mutex>
#include <atomic>
#include <chrono>
#include <optional>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../core/Config.hpp"

namespace VoxelCraft {

    // Forward declarations
    class Renderer;
    class ShaderManager;
    class RenderTarget;

    /**
     * @enum PostProcessEffect
     * @brief Available post-processing effects
     */
    enum class PostProcessEffect {
        Bloom,                   ///< Bloom effect
        HDR,                     ///< HDR tone mapping
        MotionBlur,              ///< Motion blur
        DepthOfField,            ///< Depth of field
        ChromaticAberration,     ///< Chromatic aberration
        Vignette,                ///< Vignette effect
        ColorGrading,            ///< Color grading/LUT
        FilmGrain,               ///< Film grain/noise
        LensFlare,               ///< Lens flare
        ScreenSpaceReflections,  ///< Screen space reflections
        AmbientOcclusion,        ///< Screen space ambient occlusion
        AntiAliasing,            ///< Post-process anti-aliasing
        Sharpen,                 ///< Image sharpening
        Blur,                    ///< Gaussian blur
        Distortion,              ///< Screen distortion
        Custom                   ///< Custom effect
    };

    /**
     * @enum BloomQuality
     * @brief Bloom effect quality levels
     */
    enum class BloomQuality {
        Low,                     ///< Basic bloom
        Medium,                  ///< Multi-pass bloom
        High,                    ///< High quality bloom with lens dirt
        Ultra                    ///< Ultra quality with advanced features
    };

    /**
     * @enum MotionBlurType
     * @brief Motion blur types
     */
    enum class MotionBlurType {
        None,                    ///< No motion blur
        Camera,                  ///< Camera-based motion blur
        Object,                  ///< Object-based motion blur
        Hybrid                   ///< Combined camera and object blur
    };

    /**
     * @enum DepthOfFieldQuality
     * @brief Depth of field quality levels
     */
    enum class DepthOfFieldQuality {
        Low,                     ///< Basic depth of field
        Medium,                  ///< Multi-sample DoF
        High,                    ///< Physically based DoF
        Bokeh                    ///< Bokeh DoF with lens simulation
    };

    /**
     * @enum TonemappingOperator
     * @brief HDR tone mapping operators
     */
    enum class TonemappingOperator {
        Reinhard,                ///< Reinhard tone mapping
        ReinhardExtended,        ///< Extended Reinhard
        ACES,                    ///< ACES filmic tone mapping
        Uncharted2,              ///< Uncharted 2 tone mapping
        Filmic,                  ///< Generic filmic operator
        Custom                   ///< Custom tone mapping
    };

    /**
     * @struct BloomSettings
     * @brief Bloom effect configuration
     */
    struct BloomSettings {
        bool enabled;                                ///< Enable bloom
        BloomQuality quality;                        ///< Bloom quality
        float intensity;                             ///< Bloom intensity
        float threshold;                             ///< Bloom threshold
        float radius;                                ///< Bloom radius
        int iterations;                              ///< Blur iterations
        float lensDirtIntensity;                     ///< Lens dirt intensity
        uint32_t lensDirtTexture;                    ///< Lens dirt texture

        // Advanced settings
        float knee;                                  ///< Bloom knee (soft threshold)
        bool enableLensFlare;                        ///< Enable lens flare
        float lensFlareIntensity;                    ///< Lens flare intensity
        glm::vec3 flareColor;                        ///< Lens flare color

        BloomSettings()
            : enabled(true)
            , quality(BloomQuality::High)
            , intensity(0.1f)
            , threshold(0.8f)
            , radius(2.0f)
            , iterations(5)
            , lensDirtIntensity(0.1f)
            , lensDirtTexture(0)
            , knee(0.1f)
            , enableLensFlare(true)
            , lensFlareIntensity(0.5f)
            , flareColor(1.0f, 0.9f, 0.8f)
        {}
    };

    /**
     * @struct HDRSettings
     * @brief HDR and tone mapping configuration
     */
    struct HDRSettings {
        bool enabled;                                ///< Enable HDR
        TonemappingOperator tonemapper;              ///< Tone mapping operator
        float exposure;                              ///< Exposure value
        float gamma;                                 ///< Gamma correction
        float whitePoint;                            ///< White point
        bool enableAutoExposure;                     ///< Auto exposure
        float adaptationSpeed;                       ///< Eye adaptation speed

        // Advanced settings
        float minLuminance;                          ///< Minimum luminance
        float maxLuminance;                          ///< Maximum luminance
        float shoulderStrength;                      ///< Shoulder strength (for filmic)
        float linearStrength;                        ///< Linear strength
        float linearAngle;                           ///< Linear angle
        float toeStrength;                           ///< Toe strength
        float toeNumerator;                          ///< Toe numerator
        float toeDenominator;                        ///< Toe denominator

        HDRSettings()
            : enabled(true)
            , tonemapper(TonemappingOperator::ACES)
            , exposure(1.0f)
            , gamma(2.2f)
            , whitePoint(1.0f)
            , enableAutoExposure(true)
            , adaptationSpeed(0.5f)
            , minLuminance(0.001f)
            , maxLuminance(1000.0f)
            , shoulderStrength(0.22f)
            , linearStrength(0.30f)
            , linearAngle(0.10f)
            , toeStrength(0.20f)
            , toeNumerator(0.02f)
            , toeDenominator(0.30f)
        {}
    };

    /**
     * @struct MotionBlurSettings
     * @brief Motion blur configuration
     */
    struct MotionBlurSettings {
        bool enabled;                                ///< Enable motion blur
        MotionBlurType type;                         ///< Motion blur type
        float intensity;                             ///< Blur intensity
        int sampleCount;                             ///< Sample count
        float maxBlurLength;                         ///< Maximum blur length

        // Camera motion blur
        float cameraBlurScale;                       ///< Camera blur scale
        bool enableRollingShutter;                   ///< Rolling shutter effect

        // Object motion blur
        bool enableObjectBlur;                       ///< Enable object motion blur
        float objectBlurScale;                       ///< Object blur scale
        int objectSampleCount;                       ///< Object sample count

        MotionBlurSettings()
            : enabled(true)
            , type(MotionBlurType::Hybrid)
            , intensity(0.5f)
            , sampleCount(8)
            , maxBlurLength(1.0f)
            , cameraBlurScale(1.0f)
            , enableRollingShutter(false)
            , enableObjectBlur(true)
            , objectBlurScale(1.0f)
            , objectSampleCount(4)
        {}
    };

    /**
     * @struct DepthOfFieldSettings
     * @brief Depth of field configuration
     */
    struct DepthOfFieldSettings {
        bool enabled;                                ///< Enable depth of field
        DepthOfFieldQuality quality;                 ///< DoF quality
        float focusDistance;                         ///< Focus distance
        float focusRange;                            ///< Focus range
        float nearBlurRadius;                        ///< Near blur radius
        float farBlurRadius;                         ///< Far blur radius

        // Bokeh settings
        bool enableBokeh;                            ///< Enable bokeh effect
        int bokehShape;                              ///< Bokeh shape (sides)
        float bokehRotation;                         ///< Bokeh rotation
        float bokehBrightness;                       ///< Bokeh brightness threshold

        // Advanced settings
        bool enableChromaticAberration;              ///< Enable chromatic aberration in DoF
        float aberrationStrength;                    ///< Aberration strength
        bool enableLensDistortion;                   ///< Enable lens distortion
        float distortionStrength;                    ///< Distortion strength

        DepthOfFieldSettings()
            : enabled(true)
            , quality(DepthOfFieldQuality::High)
            , focusDistance(10.0f)
            , focusRange(5.0f)
            , nearBlurRadius(2.0f)
            , farBlurRadius(3.0f)
            , enableBokeh(true)
            , bokehShape(6)
            , bokehRotation(0.0f)
            , bokehBrightness(0.5f)
            , enableChromaticAberration(true)
            , aberrationStrength(0.5f)
            , enableLensDistortion(false)
            , distortionStrength(0.1f)
        {}
    };

    /**
     * @struct ColorGradingSettings
     * @brief Color grading configuration
     */
    struct ColorGradingSettings {
        bool enabled;                                ///< Enable color grading
        bool useLUT;                                 ///< Use color lookup table
        uint32_t lutTexture;                         ///< LUT texture
        float lutIntensity;                          ///< LUT intensity

        // Basic color adjustments
        float brightness;                            ///< Brightness adjustment
        float contrast;                              ///< Contrast adjustment
        float saturation;                            ///< Saturation adjustment
        glm::vec3 colorBalance;                      ///< Color balance (RGB)
        glm::vec3 lift;                              ///< Lift (shadows)
        glm::vec3 gamma;                             ///< Gamma (midtones)
        glm::vec3 gain;                              ///< Gain (highlights)

        // Advanced adjustments
        float hueShift;                              ///< Hue shift
        float temperature;                           ///< Color temperature
        float tint;                                  ///< Color tint
        float vibrance;                              ///< Vibrance
        float highlights;                            ///< Highlights adjustment
        float shadows;                               ///< Shadows adjustment
        float whites;                                ///< Whites adjustment
        float blacks;                                ///< Blacks adjustment

        ColorGradingSettings()
            : enabled(true)
            , useLUT(false)
            , lutTexture(0)
            , lutIntensity(1.0f)
            , brightness(0.0f)
            , contrast(1.0f)
            , saturation(1.0f)
            , colorBalance(1.0f, 1.0f, 1.0f)
            , lift(0.0f, 0.0f, 0.0f)
            , gamma(1.0f, 1.0f, 1.0f)
            , gain(1.0f, 1.0f, 1.0f)
            , hueShift(0.0f)
            , temperature(6500.0f)
            , tint(0.0f)
            , vibrance(0.0f)
            , highlights(0.0f)
            , shadows(0.0f)
            , whites(0.0f)
            , blacks(0.0f)
        {}
    };

    /**
     * @struct PostProcessStats
     * @brief Post-processing performance statistics
     */
    struct PostProcessStats {
        // Performance metrics
        uint64_t totalEffects;                       ///< Total effects processed
        uint64_t activeEffects;                      ///< Currently active effects
        double totalRenderTime;                      ///< Total render time (ms)
        double averageRenderTime;                    ///< Average render time (ms)

        // Memory usage
        size_t renderTargetMemory;                   ///< Render target memory usage
        size_t textureMemory;                        ///< Texture memory usage
        uint32_t renderTargetCount;                  ///< Number of render targets

        // Effect-specific stats
        uint32_t bloomPasses;                        ///< Bloom render passes
        uint32_t blurPasses;                         ///< Blur render passes
        uint32_t dofSamples;                         ///< DoF samples taken
        uint32_t motionBlurSamples;                  ///< Motion blur samples

        // Quality metrics
        float averageLuminance;                      ///< Average scene luminance
        float exposureValue;                         ///< Current exposure value
        float bloomCoverage;                         ///< Bloom coverage percentage
        float dofCoverage;                           ///< DoF coverage percentage
    };

    /**
     * @class PostProcess
     * @brief Advanced post-processing effects system
     *
     * The PostProcess class provides comprehensive post-processing effects for the
     * VoxelCraft game engine, featuring HDR rendering, bloom, motion blur, depth of
     * field, chromatic aberration, color grading, and advanced visual effects with
     * GPU acceleration and performance optimization for modern graphics pipelines.
     *
     * Key Features:
     * - HDR rendering with multiple tone mapping operators
     * - Advanced bloom with lens dirt and lens flare
     * - Motion blur (camera and object-based)
     * - Depth of field with bokeh simulation
     * - Chromatic aberration and lens distortion
     * - Color grading with LUT support
     * - Film grain and noise effects
     * - Screen space reflections and ambient occlusion
     * - Post-process anti-aliasing (FXAA, TAA)
     * - Image sharpening and blur effects
     * - Performance monitoring and optimization
     * - Quality scaling for different hardware
     * - Real-time parameter adjustment
     *
     * The post-processing system is designed to be highly configurable and efficient,
     * supporting both cinematic-quality effects and performance-optimized modes for
     * a wide range of hardware configurations.
     */
    class PostProcess {
    public:
        /**
         * @brief Constructor
         * @param renderer Renderer instance
         */
        explicit PostProcess(Renderer* renderer);

        /**
         * @brief Destructor
         */
        ~PostProcess();

        /**
         * @brief Deleted copy constructor
         */
        PostProcess(const PostProcess&) = delete;

        /**
         * @brief Deleted copy assignment operator
         */
        PostProcess& operator=(const PostProcess&) = delete;

        // Post-processing lifecycle

        /**
         * @brief Initialize post-processing system
         * @return true if successful, false otherwise
         */
        bool Initialize();

        /**
         * @brief Shutdown post-processing system
         */
        void Shutdown();

        /**
         * @brief Update post-processing system
         * @param deltaTime Time elapsed since last update
         */
        void Update(double deltaTime);

        /**
         * @brief Render post-processing effects
         * @param inputTexture Input texture
         * @param outputTarget Output render target
         * @return true if successful, false otherwise
         */
        bool Render(uint32_t inputTexture, uint32_t outputTarget);

        // Effect configuration

        /**
         * @brief Enable post-processing effect
         * @param effect Effect to enable/disable
         * @param enabled Enable state
         * @return true if successful, false otherwise
         */
        bool EnableEffect(PostProcessEffect effect, bool enabled);

        /**
         * @brief Check if effect is enabled
         * @param effect Effect to check
         * @return true if enabled, false otherwise
         */
        bool IsEffectEnabled(PostProcessEffect effect) const;

        /**
         * @brief Get effect settings
         * @param effect Effect to get settings for
         * @return Effect settings or nullopt if not found
         */
        std::optional<std::any> GetEffectSettings(PostProcessEffect effect) const;

        /**
         * @brief Set effect settings
         * @param effect Effect to configure
         * @param settings Effect settings
         * @return true if successful, false otherwise
         */
        bool SetEffectSettings(PostProcessEffect effect, const std::any& settings);

        // Bloom effect

        /**
         * @brief Configure bloom effect
         * @param settings Bloom settings
         * @return true if successful, false otherwise
         */
        bool ConfigureBloom(const BloomSettings& settings);

        /**
         * @brief Get current bloom settings
         * @return Bloom settings
         */
        const BloomSettings& GetBloomSettings() const { return m_bloomSettings; }

        /**
         * @brief Set bloom intensity
         * @param intensity Bloom intensity
         * @return true if successful, false otherwise
         */
        bool SetBloomIntensity(float intensity);

        // HDR and tone mapping

        /**
         * @brief Configure HDR settings
         * @param settings HDR settings
         * @return true if successful, false otherwise
         */
        bool ConfigureHDR(const HDRSettings& settings);

        /**
         * @brief Get current HDR settings
         * @return HDR settings
         */
        const HDRSettings& GetHDRSettings() const { return m_hdrSettings; }

        /**
         * @brief Set exposure value
         * @param exposure Exposure value
         * @return true if successful, false otherwise
         */
        bool SetExposure(float exposure);

        /**
         * @brief Enable auto exposure
         * @param enabled Enable state
         * @return true if successful, false otherwise
         */
        bool EnableAutoExposure(bool enabled);

        // Motion blur

        /**
         * @brief Configure motion blur
         * @param settings Motion blur settings
         * @return true if successful, false otherwise
         */
        bool ConfigureMotionBlur(const MotionBlurSettings& settings);

        /**
         * @brief Get current motion blur settings
         * @return Motion blur settings
         */
        const MotionBlurSettings& GetMotionBlurSettings() const { return m_motionBlurSettings; }

        /**
         * @brief Set motion blur intensity
         * @param intensity Motion blur intensity
         * @return true if successful, false otherwise
         */
        bool SetMotionBlurIntensity(float intensity);

        // Depth of field

        /**
         * @brief Configure depth of field
         * @param settings Depth of field settings
         * @return true if successful, false otherwise
         */
        bool ConfigureDepthOfField(const DepthOfFieldSettings& settings);

        /**
         * @brief Get current depth of field settings
         * @return Depth of field settings
         */
        const DepthOfFieldSettings& GetDepthOfFieldSettings() const { return m_dofSettings; }

        /**
         * @brief Set focus distance
         * @param distance Focus distance
         * @return true if successful, false otherwise
         */
        bool SetFocusDistance(float distance);

        // Color grading

        /**
         * @brief Configure color grading
         * @param settings Color grading settings
         * @return true if successful, false otherwise
         */
        bool ConfigureColorGrading(const ColorGradingSettings& settings);

        /**
         * @brief Get current color grading settings
         * @return Color grading settings
         */
        const ColorGradingSettings& GetColorGradingSettings() const { return m_colorGradingSettings; }

        /**
         * @brief Set color grading LUT
         * @param lutTexture LUT texture ID
         * @param intensity LUT intensity
         * @return true if successful, false otherwise
         */
        bool SetColorGradingLUT(uint32_t lutTexture, float intensity = 1.0f);

        // Render target management

        /**
         * @brief Create render target for post-processing
         * @param width Target width
         * @param height Target height
         * @param format Color format
         * @return Render target ID or 0 if failed
         */
        uint32_t CreateRenderTarget(int width, int height, uint32_t format = 0x1908);

        /**
         * @brief Get render target texture
         * @param targetId Render target ID
         * @return Texture ID or 0 if not found
         */
        uint32_t GetRenderTargetTexture(uint32_t targetId) const;

        /**
         * @brief Resize render targets
         * @param width New width
         * @param height New height
         * @return true if successful, false otherwise
         */
        bool ResizeRenderTargets(int width, int height);

        // Effect chain management

        /**
         * @brief Add effect to render chain
         * @param effect Effect to add
         * @param order Render order (lower = earlier)
         * @return true if successful, false otherwise
         */
        bool AddEffectToChain(PostProcessEffect effect, int order = 1000);

        /**
         * @brief Remove effect from render chain
         * @param effect Effect to remove
         * @return true if successful, false otherwise
         */
        bool RemoveEffectFromChain(PostProcessEffect effect);

        /**
         * @brief Get current effect chain
         * @return Vector of effects in render order
         */
        std::vector<PostProcessEffect> GetEffectChain() const;

        /**
         * @brief Clear effect chain
         */
        void ClearEffectChain();

        // Performance optimization

        /**
         * @brief Optimize post-processing for current hardware
         * @return true if successful, false otherwise
         */
        bool OptimizeForHardware();

        /**
         * @brief Set quality level
         * @param quality Quality level (0.0 - 1.0)
         * @return true if successful, false otherwise
         */
        bool SetQualityLevel(float quality);

        /**
         * @brief Get current quality level
         * @return Quality level (0.0 - 1.0)
         */
        float GetQualityLevel() const { return m_qualityLevel; }

        // Performance monitoring

        /**
         * @brief Get post-processing statistics
         * @return Performance statistics
         */
        const PostProcessStats& GetStats() const { return m_stats; }

        /**
         * @brief Reset statistics
         */
        void ResetStats();

        /**
         * @brief Get performance report
         * @return Detailed performance report
         */
        std::string GetPerformanceReport() const;

        // Utility functions

        /**
         * @brief Get supported effects
         * @return Vector of supported effects
         */
        std::vector<PostProcessEffect> GetSupportedEffects() const;

        /**
         * @brief Check if effect is supported
         * @param effect Effect to check
         * @return true if supported, false otherwise
         */
        bool IsEffectSupported(PostProcessEffect effect) const;

        /**
         * @brief Get effect name
         * @param effect Effect
         * @return Effect name string
         */
        std::string GetEffectName(PostProcessEffect effect) const;

        /**
         * @brief Validate post-processing state
         * @return true if valid, false otherwise
         */
        bool Validate() const;

        /**
         * @brief Get status report
         * @return Detailed status report
         */
        std::string GetStatusReport() const;

        /**
         * @brief Optimize post-processing system
         * @return Optimization results
         */
        std::unordered_map<std::string, size_t> Optimize();

    private:
        /**
         * @brief Initialize post-processing system
         * @return true if successful, false otherwise
         */
        bool InitializeSystem();

        /**
         * @brief Create default effect chain
         */
        void CreateDefaultEffectChain();

        /**
         * @brief Render bloom effect
         * @param inputTexture Input texture
         * @param outputTarget Output target
         * @return true if successful, false otherwise
         */
        bool RenderBloom(uint32_t inputTexture, uint32_t outputTarget);

        /**
         * @brief Render HDR tone mapping
         * @param inputTexture Input texture
         * @param outputTarget Output target
         * @return true if successful, false otherwise
         */
        bool RenderHDR(uint32_t inputTexture, uint32_t outputTarget);

        /**
         * @brief Render motion blur
         * @param inputTexture Input texture
         * @param outputTarget Output target
         * @return true if successful, false otherwise
         */
        bool RenderMotionBlur(uint32_t inputTexture, uint32_t outputTarget);

        /**
         * @brief Render depth of field
         * @param inputTexture Input texture
         * @param outputTarget Output target
         * @return true if successful, false otherwise
         */
        bool RenderDepthOfField(uint32_t inputTexture, uint32_t outputTarget);

        /**
         * @brief Render color grading
         * @param inputTexture Input texture
         * @param outputTarget Output target
         * @return true if successful, false otherwise
         */
        bool RenderColorGrading(uint32_t inputTexture, uint32_t outputTarget);

        /**
         * @brief Render vignette effect
         * @param inputTexture Input texture
         * @param outputTarget Output target
         * @return true if successful, false otherwise
         */
        bool RenderVignette(uint32_t inputTexture, uint32_t outputTarget);

        /**
         * @brief Render chromatic aberration
         * @param inputTexture Input texture
         * @param outputTarget Output target
         * @return true if successful, false otherwise
         */
        bool RenderChromaticAberration(uint32_t inputTexture, uint32_t outputTarget);

        /**
         * @brief Render film grain
         * @param inputTexture Input texture
         * @param outputTarget Output target
         * @return true if successful, false otherwise
         */
        bool RenderFilmGrain(uint32_t inputTexture, uint32_t outputTarget);

        /**
         * @brief Update bloom parameters
         */
        void UpdateBloomParameters();

        /**
         * @brief Update HDR parameters
         */
        void UpdateHDRParameters();

        /**
         * @brief Update motion blur parameters
         */
        void UpdateMotionBlurParameters();

        /**
         * @brief Update depth of field parameters
         */
        void UpdateDepthOfFieldParameters();

        /**
         * @brief Update color grading parameters
         */
        void UpdateColorGradingParameters();

        /**
         * @brief Calculate auto exposure
         * @param inputTexture Input texture
         */
        void CalculateAutoExposure(uint32_t inputTexture);

        /**
         * @brief Update performance statistics
         * @param deltaTime Time elapsed
         */
        void UpdateStats(double deltaTime);

        /**
         * @brief Handle post-processing errors
         * @param error Error message
         */
        void HandleError(const std::string& error);

        // Post-processing data
        Renderer* m_renderer;                         ///< Renderer instance
        ShaderManager* m_shaderManager;               ///< Shader manager

        // Effect settings
        BloomSettings m_bloomSettings;                ///< Bloom effect settings
        HDRSettings m_hdrSettings;                    ///< HDR settings
        MotionBlurSettings m_motionBlurSettings;      ///< Motion blur settings
        DepthOfFieldSettings m_dofSettings;           ///< Depth of field settings
        ColorGradingSettings m_colorGradingSettings;  ///< Color grading settings

        // Effect state
        std::unordered_map<PostProcessEffect, bool> m_enabledEffects; ///< Enabled effects
        std::vector<std::pair<PostProcessEffect, int>> m_effectChain; ///< Effect render chain
        mutable std::shared_mutex m_effectsMutex;     ///< Effects synchronization

        // Render targets
        std::unordered_map<uint32_t, std::shared_ptr<RenderTarget>> m_renderTargets; ///< Render targets
        uint32_t m_temporaryTarget1;                  ///< Temporary render target 1
        uint32_t m_temporaryTarget2;                  ///< Temporary render target 2
        mutable std::shared_mutex m_targetsMutex;     ///< Targets synchronization

        // Performance data
        PostProcessStats m_stats;                     ///< Performance statistics
        float m_qualityLevel;                         ///< Current quality level

        // Auto exposure
        float m_currentLuminance;                     ///< Current scene luminance
        float m_targetExposure;                       ///< Target exposure value
        float m_currentExposure;                      ///< Current exposure value

        // State tracking
        bool m_isInitialized;                          ///< System is initialized
        double m_lastUpdateTime;                       ///< Last update time
        std::string m_lastError;                       ///< Last error message

        static std::atomic<uint32_t> s_nextTargetId;   ///< Next render target ID counter
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_GRAPHICS_POST_PROCESS_HPP
