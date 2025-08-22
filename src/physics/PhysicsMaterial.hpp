/**
 * @file PhysicsMaterial.hpp
 * @brief VoxelCraft Physics Material System
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file defines the PhysicsMaterial class that represents physical
 * properties of materials for collision response and friction calculations.
 */

#ifndef VOXELCRAFT_PHYSICS_PHYSICS_MATERIAL_HPP
#define VOXELCRAFT_PHYSICS_PHYSICS_MATERIAL_HPP

#include <memory>
#include <string>
#include <unordered_map>
#include <functional>
#include <mutex>
#include <shared_mutex>

#include "../core/Config.hpp"

namespace VoxelCraft {

    /**
     * @struct PhysicsMaterialProperties
     * @brief Physical properties of a material
     */
    struct PhysicsMaterialProperties {
        std::string name;                      ///< Material name
        float density;                         ///< Material density (kg/m³)
        float friction;                        ///< Friction coefficient
        float restitution;                     ///< Restitution (bounciness) coefficient
        float rollingFriction;                 ///< Rolling friction coefficient
        float spinningFriction;                ///< Spinning friction coefficient
        float damping;                         ///< Linear damping factor
        float angularDamping;                  ///< Angular damping factor
        float hardness;                        ///< Material hardness (0.0 - 1.0)
        float toughness;                       ///< Material toughness (resistance to breaking)
        float conductivity;                    ///< Thermal conductivity
        float heatCapacity;                    ///< Specific heat capacity

        // Surface properties
        float surfaceRoughness;                ///< Surface roughness (0.0 - 1.0)
        float surfaceEnergy;                   ///< Surface energy for adhesion
        bool isMagnetic;                       ///< Material is magnetic
        bool isConductive;                     ///< Material conducts electricity

        // Audio properties
        std::string collisionSound;            ///< Collision sound effect
        std::string frictionSound;             ///< Friction sound effect
        float collisionVolume;                 ///< Collision sound volume
        float frictionVolume;                  ///< Friction sound volume
    };

    /**
     * @enum MaterialType
     * @brief Types of physics materials
     */
    enum class MaterialType {
        Solid,              ///< Solid material (default)
        Liquid,             ///< Liquid material
        Gas,                ///< Gas material
        Plasma,             ///< Plasma material
        Custom              ///< Custom material type
    };

    /**
     * @enum MaterialPreset
     * @brief Predefined material presets
     */
    enum class MaterialPreset {
        Default,            ///< Default material properties
        Metal,              ///< Metal properties
        Wood,               ///< Wood properties
        Plastic,            ///< Plastic properties
        Rubber,             ///< Rubber properties
        Glass,              ///< Glass properties
        Stone,              ///< Stone properties
        Dirt,               ///< Dirt/soil properties
        Sand,               ///< Sand properties
        Water,              ///< Water properties
        Ice,                ///< Ice properties
        Snow,               ///< Snow properties
        Grass,              ///< Grass properties
        Flesh,              ///< Flesh/biological properties
        Custom              ///< Custom preset
    };

    /**
     * @class PhysicsMaterial
     * @brief Physics material for collision and friction calculations
     *
     * PhysicsMaterial defines the physical properties of materials used in
     * collision detection and response. It provides friction, restitution,
     * and other material-specific properties that affect how objects interact
     * with each other in the physics simulation.
     *
     * Key Features:
     * - Comprehensive material properties (friction, restitution, density)
     * - Material interaction matrix for different material combinations
     * - Sound effects integration for material-based audio
     * - Temperature and thermal properties
     * - Magnetic and electrical properties
     * - Preset materials for common substances
     * - Custom material creation and modification
     */
    class PhysicsMaterial {
    public:
        /**
         * @brief Constructor
         * @param name Material name
         * @param preset Optional material preset
         */
        explicit PhysicsMaterial(const std::string& name, MaterialPreset preset = MaterialPreset::Default);

        /**
         * @brief Destructor
         */
        ~PhysicsMaterial() = default;

        /**
         * @brief Deleted copy constructor
         */
        PhysicsMaterial(const PhysicsMaterial&) = delete;

        /**
         * @brief Deleted copy assignment operator
         */
        PhysicsMaterial& operator=(const PhysicsMaterial&) = delete;

        // Material identification

        /**
         * @brief Get material name
         * @return Material name
         */
        const std::string& GetName() const { return m_properties.name; }

        /**
         * @brief Set material name
         * @param name New material name
         */
        void SetName(const std::string& name) { m_properties.name = name; }

        /**
         * @brief Get material type
         * @return Material type
         */
        MaterialType GetType() const { return m_type; }

        /**
         * @brief Set material type
         * @param type New material type
         */
        void SetType(MaterialType type) { m_type = type; }

        /**
         * @brief Get material preset
         * @return Material preset
         */
        MaterialPreset GetPreset() const { return m_preset; }

        // Physical properties access

        /**
         * @brief Get material density
         * @return Density (kg/m³)
         */
        float GetDensity() const { return m_properties.density; }

        /**
         * @brief Set material density
         * @param density New density (kg/m³)
         */
        void SetDensity(float density) { m_properties.density = density; }

        /**
         * @brief Get friction coefficient
         * @return Friction coefficient
         */
        float GetFriction() const { return m_properties.friction; }

        /**
         * @brief Set friction coefficient
         * @param friction New friction coefficient
         */
        void SetFriction(float friction) { m_properties.friction = friction; }

        /**
         * @brief Get restitution coefficient
         * @return Restitution coefficient
         */
        float GetRestitution() const { return m_properties.restitution; }

        /**
         * @brief Set restitution coefficient
         * @param restitution New restitution coefficient
         */
        void SetRestitution(float restitution) { m_properties.restitution = restitution; }

        /**
         * @brief Get rolling friction coefficient
         * @return Rolling friction coefficient
         */
        float GetRollingFriction() const { return m_properties.rollingFriction; }

        /**
         * @brief Set rolling friction coefficient
         * @param friction New rolling friction coefficient
         */
        void SetRollingFriction(float friction) { m_properties.rollingFriction = friction; }

        /**
         * @brief Get spinning friction coefficient
         * @return Spinning friction coefficient
         */
        float GetSpinningFriction() const { return m_properties.spinningFriction; }

        /**
         * @brief Set spinning friction coefficient
         * @param friction New spinning friction coefficient
         */
        void SetSpinningFriction(float friction) { m_properties.spinningFriction = friction; }

        /**
         * @brief Get linear damping
         * @return Linear damping factor
         */
        float GetDamping() const { return m_properties.damping; }

        /**
         * @brief Set linear damping
         * @param damping New linear damping factor
         */
        void SetDamping(float damping) { m_properties.damping = damping; }

        /**
         * @brief Get angular damping
         * @return Angular damping factor
         */
        float GetAngularDamping() const { return m_properties.angularDamping; }

        /**
         * @brief Set angular damping
         * @param damping New angular damping factor
         */
        void SetAngularDamping(float damping) { m_properties.angularDamping = damping; }

        // Material interaction

        /**
         * @brief Calculate combined friction with another material
         * @param other Other material
         * @return Combined friction coefficient
         */
        float CalculateCombinedFriction(const PhysicsMaterial* other) const;

        /**
         * @brief Calculate combined restitution with another material
         * @param other Other material
         * @return Combined restitution coefficient
         */
        float CalculateCombinedRestitution(const PhysicsMaterial* other) const;

        /**
         * @brief Calculate rolling friction with another material
         * @param other Other material
         * @return Combined rolling friction coefficient
         */
        float CalculateCombinedRollingFriction(const PhysicsMaterial* other) const;

        /**
         * @brief Calculate interaction strength with another material
         * @param other Other material
         * @return Interaction strength (0.0 - 1.0)
         */
        float CalculateInteractionStrength(const PhysicsMaterial* other) const;

        // Material properties

        /**
         * @brief Get all material properties
         * @return Material properties
         */
        const PhysicsMaterialProperties& GetProperties() const { return m_properties; }

        /**
         * @brief Set all material properties
         * @param properties New material properties
         */
        void SetProperties(const PhysicsMaterialProperties& properties);

        /**
         * @brief Reset material to preset
         * @param preset Material preset
         */
        void ResetToPreset(MaterialPreset preset);

        /**
         * @brief Check if material is magnetic
         * @return true if magnetic, false otherwise
         */
        bool IsMagnetic() const { return m_properties.isMagnetic; }

        /**
         * @brief Set magnetic property
         * @param magnetic New magnetic state
         */
        void SetMagnetic(bool magnetic) { m_properties.isMagnetic = magnetic; }

        /**
         * @brief Check if material is conductive
         * @return true if conductive, false otherwise
         */
        bool IsConductive() const { return m_properties.isConductive; }

        /**
         * @brief Set conductive property
         * @param conductive New conductive state
         */
        void SetConductive(bool conductive) { m_properties.isConductive = conductive; }

        // Audio integration

        /**
         * @brief Get collision sound
         * @return Collision sound name
         */
        const std::string& GetCollisionSound() const { return m_properties.collisionSound; }

        /**
         * @brief Set collision sound
         * @param sound New collision sound name
         */
        void SetCollisionSound(const std::string& sound) { m_properties.collisionSound = sound; }

        /**
         * @brief Get friction sound
         * @return Friction sound name
         */
        const std::string& GetFrictionSound() const { return m_properties.frictionSound; }

        /**
         * @brief Set friction sound
         * @param sound New friction sound name
         */
        void SetFrictionSound(const std::string& sound) { m_properties.frictionSound = sound; }

        /**
         * @brief Get collision volume
         * @return Collision volume (0.0 - 1.0)
         */
        float GetCollisionVolume() const { return m_properties.collisionVolume; }

        /**
         * @brief Set collision volume
         * @param volume New collision volume (0.0 - 1.0)
         */
        void SetCollisionVolume(float volume) { m_properties.collisionVolume = volume; }

        /**
         * @brief Get friction volume
         * @return Friction volume (0.0 - 1.0)
         */
        float GetFrictionVolume() const { return m_properties.frictionVolume; }

        /**
         * @brief Set friction volume
         * @param volume New friction volume (0.0 - 1.0)
         */
        void SetFrictionVolume(float volume) { m_properties.frictionVolume = volume; }

        // Utility functions

        /**
         * @brief Get material description
         * @return Material description string
         */
        std::string GetDescription() const;

        /**
         * @brief Validate material properties
         * @return true if valid, false otherwise
         */
        bool Validate() const;

        /**
         * @brief Clone material
         * @param newName Optional new name for cloned material
         * @return Cloned material
         */
        std::unique_ptr<PhysicsMaterial> Clone(const std::string& newName = "") const;

        /**
         * @brief Interpolate between two materials
         * @param other Other material
         * @param factor Interpolation factor (0.0 - 1.0)
         * @return Interpolated material
         */
        std::unique_ptr<PhysicsMaterial> Interpolate(const PhysicsMaterial* other, float factor) const;

    private:
        /**
         * @brief Initialize default properties
         */
        void InitializeDefaults();

        /**
         * @brief Load preset properties
         * @param preset Material preset
         */
        void LoadPreset(MaterialPreset preset);

        /**
         * @brief Calculate combined property using geometric mean
         * @param prop1 First property value
         * @param prop2 Second property value
         * @return Combined property value
         */
        static float CalculateGeometricMean(float prop1, float prop2);

        /**
         * @brief Calculate combined property using arithmetic mean
         * @param prop1 First property value
         * @param prop2 Second property value
         * @return Combined property value
         */
        static float CalculateArithmeticMean(float prop1, float prop2);

        /**
         * @brief Calculate combined property using minimum value
         * @param prop1 First property value
         * @param prop2 Second property value
         * @return Combined property value
         */
        static float CalculateMinimum(float prop1, float prop2);

        PhysicsMaterialProperties m_properties;    ///< Material properties
        MaterialType m_type;                       ///< Material type
        MaterialPreset m_preset;                   ///< Material preset
    };

    /**
     * @class PhysicsMaterialManager
     * @brief Manager for physics materials
     *
     * PhysicsMaterialManager provides centralized management of physics materials
     * with registration, lookup, and material interaction matrix functionality.
     */
    class PhysicsMaterialManager {
    public:
        /**
         * @brief Constructor
         */
        PhysicsMaterialManager();

        /**
         * @brief Destructor
         */
        ~PhysicsMaterialManager();

        /**
         * @brief Deleted copy constructor
         */
        PhysicsMaterialManager(const PhysicsMaterialManager&) = delete;

        /**
         * @brief Deleted copy assignment operator
         */
        PhysicsMaterialManager& operator=(const PhysicsMaterialManager&) = delete;

        // Material registration

        /**
         * @brief Register a physics material
         * @param material Material to register
         * @return true if registered, false otherwise
         */
        bool RegisterMaterial(std::unique_ptr<PhysicsMaterial> material);

        /**
         * @brief Unregister a physics material
         * @param name Material name
         * @return true if unregistered, false otherwise
         */
        bool UnregisterMaterial(const std::string& name);

        /**
         * @brief Get physics material by name
         * @param name Material name
         * @return Material pointer or nullptr if not found
         */
        PhysicsMaterial* GetMaterial(const std::string& name) const;

        /**
         * @brief Check if material exists
         * @param name Material name
         * @return true if exists, false otherwise
         */
        bool HasMaterial(const std::string& name) const;

        /**
         * @brief Get all registered materials
         * @return Vector of material pointers
         */
        std::vector<PhysicsMaterial*> GetAllMaterials() const;

        /**
         * @brief Get material count
         * @return Number of registered materials
         */
        size_t GetMaterialCount() const;

        // Material creation

        /**
         * @brief Create material from preset
         * @param name Material name
         * @param preset Material preset
         * @return Created material
         */
        std::unique_ptr<PhysicsMaterial> CreateMaterial(const std::string& name, MaterialPreset preset);

        /**
         * @brief Create custom material
         * @param name Material name
         * @param properties Material properties
         * @return Created material
         */
        std::unique_ptr<PhysicsMaterial> CreateCustomMaterial(const std::string& name,
                                                             const PhysicsMaterialProperties& properties);

        // Default materials

        /**
         * @brief Get default material
         * @return Default material
         */
        PhysicsMaterial* GetDefaultMaterial() const { return m_defaultMaterial; }

        /**
         * @brief Set default material
         * @param material New default material
         */
        void SetDefaultMaterial(PhysicsMaterial* material) { m_defaultMaterial = material; }

        // Material interaction matrix

        /**
         * @brief Calculate interaction between two materials
         * @param material1 First material
         * @param material2 Second material
         * @param friction Output combined friction
         * @param restitution Output combined restitution
         * @param rollingFriction Output combined rolling friction
         */
        void CalculateMaterialInteraction(const PhysicsMaterial* material1,
                                        const PhysicsMaterial* material2,
                                        float& friction,
                                        float& restitution,
                                        float& rollingFriction) const;

        // Utility functions

        /**
         * @brief Clear all materials
         */
        void ClearAll();

        /**
         * @brief Initialize default materials
         */
        void InitializeDefaults();

        /**
         * @brief Validate all materials
         * @return Vector of validation errors
         */
        std::vector<std::string> ValidateAll() const;

        /**
         * @brief Get manager statistics
         * @return Statistics string
         */
        std::string GetStatistics() const;

    private:
        std::unordered_map<std::string, std::unique_ptr<PhysicsMaterial>> m_materials;
        mutable std::shared_mutex m_materialsMutex;
        PhysicsMaterial* m_defaultMaterial;

        // Default material names
        static constexpr const char* DEFAULT_MATERIAL_NAME = "Default";
        static constexpr const char* METAL_MATERIAL_NAME = "Metal";
        static constexpr const char* WOOD_MATERIAL_NAME = "Wood";
        static constexpr const char* PLASTIC_MATERIAL_NAME = "Plastic";
        static constexpr const char* RUBBER_MATERIAL_NAME = "Rubber";
        static constexpr const char* GLASS_MATERIAL_NAME = "Glass";
        static constexpr const char* STONE_MATERIAL_NAME = "Stone";
        static constexpr const char* DIRT_MATERIAL_NAME = "Dirt";
        static constexpr const char* SAND_MATERIAL_NAME = "Sand";
        static constexpr const char* WATER_MATERIAL_NAME = "Water";
        static constexpr const char* ICE_MATERIAL_NAME = "Ice";
        static constexpr const char* SNOW_MATERIAL_NAME = "Snow";
        static constexpr const char* GRASS_MATERIAL_NAME = "Grass";
        static constexpr const char* FLESH_MATERIAL_NAME = "Flesh";
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_PHYSICS_PHYSICS_MATERIAL_HPP
