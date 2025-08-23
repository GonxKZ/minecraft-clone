#pragma once

#include <memory>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace VoxelCraft {

    class PlayerEntity;
    class Camera;
    class Inventory;
    class World;

    /**
     * @enum PlayerState
     * @brief Estados del jugador
     */
    enum class PlayerState {
        IDLE = 0,           ///< En reposo
        WALKING,            ///< Caminando
        RUNNING,            ///< Corriendo
        JUMPING,            ///< Saltando
        FALLING,            ///< Cayendo
        SWIMMING,           ///< Nadando
        FLYING,             ///< Volando (modo creativo)
        SLEEPING,           ///< Durmiendo
        MOUNTED,            ///< Montado
        USING_ITEM,         ///< Usando item
        ATTACKING,          ///< Atacando
        HURT,               ///< Herido
        DEAD                ///< Muerto
    };

    /**
     * @enum PlayerMode
     * @brief Modos de juego del jugador
     */
    enum class PlayerMode {
        SURVIVAL = 0,       ///< Modo supervivencia
        CREATIVE,           ///< Modo creativo
        ADVENTURE,          ///< Modo aventura
        SPECTATOR           ///< Modo espectador
    };

    /**
     * @struct PlayerStats
     * @brief Estadísticas del jugador
     */
    struct PlayerStats {
        float health = 20.0f;           ///< Salud (0-20)
        float maxHealth = 20.0f;        ///< Salud máxima
        float hunger = 20.0f;           ///< Hambre (0-20)
        float saturation = 5.0f;        ///< Saciedad
        float experience = 0.0f;        ///< Experiencia
        int level = 0;                  ///< Nivel
        float armor = 0.0f;             ///< Valor de armadura
        float absorption = 0.0f;        ///< Absorción de daño
        int air = 300;                  ///< Aire bajo agua (0-300)
        float movementSpeed = 0.1f;     ///< Velocidad de movimiento
        float jumpStrength = 0.42f;     ///< Fuerza de salto
        bool isOnFire = false;          ///< En llamas
        bool isSneaking = false;        ///< Agachado
        bool isSprinting = false;       ///< Corriendo
        bool isFlying = false;          ///< Volando
        bool isSwimming = false;        ///< Nadando
        bool isSleeping = false;        ///< Durmiendo
        bool isInvulnerable = false;    ///< Invulnerable
    };

    /**
     * @struct PlayerInput
     * @brief Estado de entrada del jugador
     */
    struct PlayerInput {
        bool forward = false;           ///< Mover hacia adelante
        bool backward = false;          ///< Mover hacia atrás
        bool left = false;              ///< Mover hacia la izquierda
        bool right = false;             ///< Mover hacia la derecha
        bool jump = false;              ///< Saltar
        bool sneak = false;             ///< Agacharse
        bool sprint = false;            ///< Correr
        bool attack = false;            ///< Atacar
        bool use = false;               ///< Usar item
        bool drop = false;              ///< Soltar item
        bool inventory = false;         ///< Abrir inventario
        float lookX = 0.0f;             ///< Rotación horizontal
        float lookY = 0.0f;             ///< Rotación vertical
        bool flyUp = false;             ///< Volar hacia arriba
        bool flyDown = false;           ///< Volar hacia abajo
        int hotbarSlot = 0;             ///< Slot seleccionado (0-8)
    };

    /**
     * @class Player
     * @brief Controlador principal del jugador
     */
    class Player {
    public:
        /**
         * @brief Constructor
         */
        Player();

        /**
         * @brief Destructor
         */
        ~Player();

        /**
         * @brief Inicializar jugador
         * @param playerName Nombre del jugador
         * @return true si la inicialización fue exitosa
         */
        bool Initialize(const std::string& playerName);

        /**
         * @brief Actualizar estado del jugador
         * @param deltaTime Tiempo transcurrido desde el último frame
         */
        void Update(float deltaTime);

        /**
         * @brief Actualizar entrada del jugador
         * @param input Estado de entrada
         */
        void UpdateInput(const PlayerInput& input);

        /**
         * @brief Actualizar física del jugador
         * @param deltaTime Tiempo transcurrido
         */
        void UpdatePhysics(float deltaTime);

        /**
         * @brief Actualizar animaciones del jugador
         * @param deltaTime Tiempo transcurrido
         */
        void UpdateAnimations(float deltaTime);

        // Getters
        const std::string& GetName() const { return m_playerName; }
        PlayerState GetState() const { return m_currentState; }
        PlayerMode GetMode() const { return m_currentMode; }
        const PlayerStats& GetStats() const { return m_stats; }
        const glm::vec3& GetPosition() const { return m_position; }
        const glm::vec3& GetVelocity() const { return m_velocity; }
        const glm::vec3& GetRotation() const { return m_rotation; }
        std::shared_ptr<PlayerEntity> GetEntity() const { return m_entity; }
        std::shared_ptr<Camera> GetCamera() const { return m_camera; }
        std::shared_ptr<Inventory> GetInventory() const { return m_inventory; }
        std::shared_ptr<World> GetWorld() const { return m_world; }

        // Setters
        void SetPosition(const glm::vec3& position) { m_position = position; }
        void SetRotation(const glm::vec3& rotation) { m_rotation = rotation; }
        void SetMode(PlayerMode mode) { m_currentMode = mode; }
        void SetWorld(std::shared_ptr<World> world) { m_world = world; }

        // Acciones del jugador
        void Jump();
        void Attack();
        void UseItem();
        void DropItem();
        void TakeDamage(float damage);
        void Heal(float amount);
        void Respawn();
        void Die();

        // Interacciones
        void InteractWithBlock(int x, int y, int z);
        void BreakBlock(int x, int y, int z);
        void PlaceBlock(int x, int y, int z, int blockId);
        void SelectHotbarSlot(int slot);

        // Movimiento
        void Move(const glm::vec3& direction);
        void Look(float deltaX, float deltaY);
        void StartSprinting();
        void StopSprinting();
        void StartSneaking();
        void StopSneaking();
        void ToggleFlying();
        void StartSwimming();
        void StopSwimming();

        // Estadísticas
        void UpdateHunger(float deltaTime);
        void UpdateHealth(float deltaTime);
        void UpdateExperience(float amount);
        void LevelUp();

        // Serialización
        std::string Serialize() const;
        bool Deserialize(const std::string& data);

        // Debug
        std::string GetDebugInfo() const;
        void EnableDebugMode(bool enable) { m_debugMode = enable; }
        bool IsDebugModeEnabled() const { return m_debugMode; }

    private:
        std::string m_playerName;                    ///< Nombre del jugador
        PlayerState m_currentState;                  ///< Estado actual
        PlayerMode m_currentMode;                    ///< Modo de juego
        PlayerStats m_stats;                         ///< Estadísticas
        PlayerInput m_input;                         ///< Estado de entrada

        glm::vec3 m_position;                        ///< Posición
        glm::vec3 m_velocity;                        ///< Velocidad
        glm::vec3 m_rotation;                        ///< Rotación (yaw, pitch, roll)

        std::shared_ptr<PlayerEntity> m_entity;      ///< Entidad del jugador
        std::shared_ptr<Camera> m_camera;            ///< Cámara del jugador
        std::shared_ptr<Inventory> m_inventory;      ///< Inventario
        std::shared_ptr<World> m_world;              ///< Mundo actual

        // Configuración de movimiento
        float m_walkSpeed = 4.317f;                  ///< Velocidad caminando
        float m_runSpeed = 5.612f;                   ///< Velocidad corriendo
        float m_flySpeed = 10.89f;                   ///< Velocidad volando
        float m_sneakSpeed = 1.31f;                  ///< Velocidad agachado
        float m_swimSpeed = 2.2f;                    ///< Velocidad nadando
        float m_gravity = 32.0f;                     ///< Gravedad
        float m_jumpHeight = 1.25f;                  ///< Altura de salto
        float m_mouseSensitivity = 0.002f;           ///< Sensibilidad del mouse
        float m_friction = 0.91f;                    ///< Fricción

        // Estados internos
        bool m_onGround = false;                     ///< En el suelo
        bool m_inWater = false;                      ///< En agua
        bool m_inLava = false;                       ///< En lava
        float m_fallDistance = 0.0f;                 ///< Distancia de caída
        float m_timeSinceLastJump = 0.0f;            ///< Tiempo desde último salto

        // Debug
        bool m_debugMode = false;

        // Métodos privados
        void UpdateState();
        void ApplyMovement(float deltaTime);
        void ApplyGravity(float deltaTime);
        void ApplyFriction(float deltaTime);
        void CheckCollisions();
        void HandleFallDamage();
        void HandleWaterPhysics();
        void HandleLavaDamage();
        void UpdateCamera();
        void UpdateEntity();
        bool CanMoveTo(const glm::vec3& position) const;
        void ClampRotation();
        void UpdateStats();
    };

} // namespace VoxelCraft