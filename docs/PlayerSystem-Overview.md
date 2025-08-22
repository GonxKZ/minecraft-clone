# 🎮 **VoxelCraft Player & Camera System - Complete Overview**

## 📋 **Índice de Contenidos**

1. [🎯 Visión General](#-visión-general)
2. [🏗️ Arquitectura del Sistema de Jugador](#️-arquitectura-del-sistema-de-jugador)
3. [📦 Componentes Principales](#-componentes-principales)
4. [🧑‍🎮 Player - Sistema Principal del Jugador](#-player---sistema-principal-del-jugador)
5. [📹 Camera - Sistema de Cámara Avanzado](#-camera---sistema-de-cámara-avanzado)
6. [🎮 PlayerInput - Sistema de Entrada](#-playerinput---sistema-de-entrada)
7. [🏃‍♂️ CharacterController - Controlador de Personaje](#️-charactercontroller---controlador-de-personaje)
8. [🎒 Inventory - Sistema de Inventario](#-inventory---sistema-de-inventario)
9. [📊 Rendimiento y Optimización](#-rendimiento-y-optimización)
10. [🔧 Uso del Sistema](#-uso-del-sistema)
11. [🎮 Ejemplos Prácticos](#-ejemplos-prácticos)

---

## 🎯 **Visión General**

El **Player & Camera System** de VoxelCraft es un sistema integral de jugador que incluye gestión de personaje, cámara avanzada, controles de entrada, controlador de personaje y sistema de inventario. El sistema está diseñado para proporcionar una experiencia de jugador inmersiva y fluida.

### 🎯 **Objetivos Principales**
- **Control Total**: Sistema de controles preciso y responsive
- **Cámara Dinámica**: Cámara que se adapta al entorno y acciones del jugador
- **Gestión de Inventario**: Sistema completo de items, crafting y equipamiento
- **Física de Personaje**: Movimiento realista con física integrada
- **Experiencia Inmersiva**: Animaciones, efectos y feedback visual

### 🏆 **Características Destacadas**
- ✅ **Sistema de Cámara Avanzado**: Tercera persona con colisión, zoom, efectos
- ✅ **Controles Completos**: Teclado, mouse, gamepad con gestos y accesibilidad
- ✅ **Controlador de Personaje**: Movimiento 3D con física, saltos, escalada
- ✅ **Inventario Extenso**: Items, crafting, equipamiento, gestión de peso
- ✅ **Integración Completa**: Con física, mundo, entidades y UI
- ✅ **Debugging Visual**: Visualización completa de sistemas del jugador

---

## 🏗️ **Arquitectura del Sistema de Jugador**

### 📊 **Estructura General**
```
┌─────────────────────────────────────────────────────────────┐
│                    Player (Main System)                     │
├─────────────────────────────────────────────────────────────┤
│  ┌─────────────┬─────────────┬─────────────┬─────────────┐ │
│  │   Camera    │PlayerInput │CharacterCtrl│  Inventory   │ │
│  │             │            │             │              │ │
│  └─────────────┴─────────────┴─────────────┴─────────────┘ │
├─────────────────────────────────────────────────────────────┤
│              Entity & Physics Integration                   │
│  ┌─────────────┬─────────────┬─────────────┬─────────────┐ │
│  │  Animation  │   Audio     │   Visual    │   Effects   │ │
│  │ Controller  │ Controller  │ Controller  │ Controller  │ │
│  └─────────────┴─────────────┴─────────────┴─────────────┘ │
└─────────────────────────────────────────────────────────────┘
```

### 🔄 **Flujo de Datos del Sistema de Jugador**
```
Input Events → PlayerInput → CharacterController → Player
                      ↓                    ↓               ↓
              Camera Update ←→ Physics Update ←→ Animation Update
                      ↓                    ↓               ↓
               World Interaction ←→ Inventory Update ←→ UI Update
```

---

## 📦 **Componentes Principales**

### 🧑‍🎮 **Player - Sistema Principal del Jugador**
```cpp
class Player : public Entity {
    // Identidad y configuración
    PlayerConfig config;                      // Configuración del jugador
    PlayerStats stats;                        // Estadísticas del jugador
    PlayerState state;                        // Estado actual del jugador

    // Subsistemas principales
    std::unique_ptr<Camera> camera;           // Cámara del jugador
    std::unique_ptr<PlayerInput> input;       // Sistema de entrada
    std::unique_ptr<CharacterController> controller; // Controlador de personaje
    std::unique_ptr<Inventory> inventory;     // Inventario del jugador

    // Actualización principal
    void Update(double deltaTime);            // Actualización del jugador
    bool TakeDamage(float damage, const std::string& type, Entity* attacker);
    bool Heal(float amount);                  // Curar al jugador
};
```

### 📹 **Camera - Sistema de Cámara Avanzado**
```cpp
class Camera {
    // Configuración y estado
    CameraConfig config;                      // Configuración de cámara
    CameraState state;                        // Estado actual de cámara
    CameraMode mode;                          // Modo de cámara actual

    // Funcionalidades principales
    void Update(double deltaTime);            // Actualización de cámara
    bool DetectCollisions();                  // Detección de colisiones
    bool ResolveCollision();                  // Resolución de colisiones
    void ApplyShake(float intensity, float frequency, float duration);
};
```

### 🎮 **PlayerInput - Sistema de Entrada**
```cpp
class PlayerInput {
    // Configuración y estado
    InputConfig config;                       // Configuración de entrada
    InputStateData state;                     // Estado de entrada actual

    // Procesamiento de entrada
    void Update(double deltaTime);            // Actualización de entrada
    void ProcessEvents();                     // Procesar eventos de entrada
    bool IsPressed(InputAction action) const; // Verificar acción presionada
    float GetAxisValue(InputAxis axis) const; // Obtener valor de eje
};
```

### 🏃‍♂️ **CharacterController - Controlador de Personaje**
```cpp
class CharacterController {
    // Estado y configuración
    CharacterConfig config;                   // Configuración del controlador
    CharacterStateData state;                 // Estado actual del personaje

    // Movimiento y control
    void Update(double deltaTime);            // Actualización del controlador
    void Move(const glm::vec3& direction, float speed); // Mover personaje
    bool Jump(float force);                   // Saltar
    bool IsOnGround() const;                  // Verificar si está en el suelo
};
```

### 🎒 **Inventory - Sistema de Inventario**
```cpp
class Inventory {
    // Configuración y estado
    InventoryConfig config;                   // Configuración del inventario
    InventoryMetrics metrics;                 // Métricas de rendimiento

    // Operaciones de inventario
    int AddItem(std::shared_ptr<Item> item, int count); // Agregar item
    int RemoveItem(const std::string& itemId, int count); // Remover item
    bool CraftItem();                         // Fabricar item
    bool EquipItem(std::shared_ptr<Item> item, EquipmentSlot slot); // Equipar item
};
```

---

## 🧑‍🎮 **Player - Sistema Principal del Jugador**

### 👤 **Configuración del Jugador**
```cpp
struct PlayerConfig {
    std::string playerName;                   // Nombre del jugador
    std::string skinPath;                     // Ruta de la skin
    std::string modelPath;                    // Ruta del modelo 3D
    float mouseSensitivity;                   // Sensibilidad del mouse
    float fov;                               // Campo de visión
    bool enableAutoJump;                     // Salto automático
    bool enableFirstPersonMode;              // Modo primera persona
};
```

### 📊 **Estadísticas del Jugador**
```cpp
struct PlayerStats {
    // Salud y daño
    float health;                            // Salud actual
    float maxHealth;                         // Salud máxima
    float armor;                             // Armadura

    // Movimiento
    float walkSpeed;                         // Velocidad de caminar
    float runSpeed;                          // Velocidad de correr
    float jumpHeight;                        // Altura de salto

    // Supervivencia
    float hunger;                            // Nivel de hambre
    float thirst;                            // Nivel de sed
    float stamina;                           // Nivel de estamina

    // Progreso
    uint32_t level;                          // Nivel del jugador
    uint64_t experience;                     // Experiencia actual
    uint64_t playTime;                       // Tiempo de juego
};
```

### 🎯 **Estados del Jugador**
```cpp
enum class PlayerState {
    Idle,                                    // Inactivo
    Walking,                                 // Caminando
    Running,                                 // Corriendo
    Jumping,                                 // Saltando
    Falling,                                 // Cayendo
    Sneaking,                                // Agachado
    Swimming,                                // Nadando
    Flying,                                  // Volando
    Climbing,                                // Escalando
    Dead,                                    // Muerto
    Spectating                               // Espectador
};
```

### ⚡ **Acciones del Jugador**
```cpp
enum class PlayerAction {
    None,                                    // Sin acción
    Jump,                                    // Saltar
    Attack,                                  // Atacar
    Use,                                     // Usar item
    Sneak,                                   // Agacharse
    Sprint,                                  // Correr
    Fly,                                     // Volarear
    DropItem,                                // Soltar item
    SwitchItem,                              // Cambiar item
    OpenInventory,                           // Abrir inventario
    Pause                                    // Pausar juego
};
```

---

## 📹 **Camera - Sistema de Cámara Avanzado**

### 📷 **Modos de Cámara**
```cpp
enum class CameraMode {
    FirstPerson,                             // Primera persona
    ThirdPerson,                             // Tercera persona
    Free,                                    // Cámara libre
    Orbital,                                 // Cámara orbital
    Cinematic,                               // Cámara cinematográfica
    Spectator                                // Cámara de espectador
};
```

### ⚙️ **Configuración de Cámara**
```cpp
struct CameraConfig {
    CameraMode mode;                         // Modo de cámara
    uint32_t behaviorFlags;                  // Flags de comportamiento
    glm::vec3 position;                      // Posición de cámara
    glm::quat orientation;                   // Orientación de cámara
    float distance;                          // Distancia del objetivo
    float fieldOfView;                       // Campo de visión
    float movementSpeed;                     // Velocidad de movimiento
    float rotationSpeed;                     // Velocidad de rotación
    float collisionRadius;                   // Radio de colisión
};
```

### 🎯 **Estado de Cámara**
```cpp
struct CameraState {
    glm::vec3 position;                      // Posición actual
    glm::quat orientation;                   // Orientación actual
    glm::vec3 velocity;                      // Velocidad de cámara
    float currentDistance;                   // Distancia actual
    bool isColliding;                        // Está colisionando
    bool targetVisible;                      // Objetivo visible
    float shakeTimeRemaining;                // Tiempo restante de sacudida
    glm::vec3 shakeOffset;                   // Offset de sacudida
};
```

### 🎛️ **Flags de Comportamiento**
```cpp
enum class CameraBehavior : uint32_t {
    NONE                    = 0x0000,
    COLLISION_DETECTION     = 0x0001,         // Detección de colisiones
    SMOOTH_FOLLOW           = 0x0002,         // Seguimiento suave
    ZOOM_ON_COLLISION       = 0x0004,         // Zoom al colisionar
    AUTO_ADJUST_DISTANCE    = 0x0008,         // Ajuste automático de distancia
    LOOK_AT_TARGET          = 0x0010,         // Mirar al objetivo
    ORBIT_AROUND_TARGET     = 0x0020,         // Orbitar alrededor del objetivo
    ENABLE_SHAKE            = 0x0040,         // Habilitar sacudida
    ENABLE_DOF              = 0x0080,         // Habilitar depth of field
    ENABLE_MOTION_BLUR      = 0x0100,         // Habilitar motion blur
    DEBUG_DRAW              = 0x1000          // Dibujo de debug
};
```

---

## 🎮 **PlayerInput - Sistema de Entrada**

### 🎮 **Dispositivos de Entrada**
```cpp
enum class InputDevice {
    Keyboard,                                // Teclado
    Mouse,                                   // Mouse
    Gamepad,                                 // Gamepad
    Touch                                    // Pantalla táctil
};
```

### 🎯 **Acciones de Entrada**
```cpp
enum class InputAction {
    MoveForward,                             // Mover adelante
    MoveBackward,                            // Mover atrás
    Jump,                                    // Saltar
    LookUp,                                  // Mirar arriba
    LookDown,                                // Mirar abajo
    Attack,                                  // Atacar
    UseItem,                                 // Usar item
    OpenInventory,                           // Abrir inventario
    Sprint,                                  // Correr
    Sneak                                    // Agacharse
};
```

### 📊 **Ejes de Entrada**
```cpp
enum class InputAxis {
    MoveX,                                   // Movimiento X
    MoveY,                                   // Movimiento Y
    LookX,                                   // Mirada X
    LookY,                                   // Mirada Y
    TriggerLeft,                             // Gatillo izquierdo
    TriggerRight                             // Gatillo derecho
};
```

### 🎛️ **Configuración de Entrada**
```cpp
struct InputConfig {
    float mouseSensitivity;                   // Sensibilidad del mouse
    float gamepadSensitivity;                 // Sensibilidad del gamepad
    bool invertMouseY;                        // Invertir mouse Y
    bool enableVibration;                     // Habilitar vibración
    float deadzone;                           // Zona muerta
    bool enableKeyRepeat;                     // Repetición de tecla
};
```

---

## 🏃‍♂️ **CharacterController - Controlador de Personaje**

### 🚶‍♂️ **Estados del Personaje**
```cpp
enum class CharacterState {
    Idle,                                    // Inactivo
    Walking,                                 // Caminando
    Running,                                 // Corriendo
    Jumping,                                 // Saltando
    Falling,                                 // Cayendo
    Landing,                                 // Aterrizando
    Sneaking,                                // Agachado
    Swimming                                 // Nadando
};
```

### 🏔️ **Estado del Suelo**
```cpp
enum class GroundState {
    InAir,                                   // En el aire
    OnGround,                                // En el suelo
    OnSlope,                                 // En una pendiente
    Sliding,                                 // Deslizándose
    OnIce,                                   // En hielo
    OnWater                                  // En agua
};
```

### ⚙️ **Configuración del Controlador**
```cpp
struct CharacterConfig {
    float height;                            // Altura del personaje
    float radius;                            // Radio del personaje
    float mass;                              // Masa del personaje
    float stepHeight;                        // Altura máxima de escalón
    float walkSpeed;                         // Velocidad de caminar
    float runSpeed;                          // Velocidad de correr
    float jumpHeight;                        // Altura de salto
    int maxJumpCount;                        // Máximo número de saltos
    float gravityMultiplier;                 // Multiplicador de gravedad
};
```

### 🎯 **Datos de Estado del Personaje**
```cpp
struct CharacterStateData {
    glm::vec3 position;                      // Posición actual
    glm::vec3 velocity;                      // Velocidad actual
    glm::vec3 inputDirection;                // Dirección de entrada
    GroundState groundState;                 // Estado del suelo
    CharacterState characterState;           // Estado del personaje
    bool isMoving;                           // Está moviéndose
    bool isJumping;                          // Está saltando
    bool isOnGround;                         // Está en el suelo
    bool isInWater;                          // Está en agua
    int jumpCount;                           // Conteo de saltos
};
```

### 🏔️ **Flags de Movimiento**
```cpp
enum class MovementFlags : uint32_t {
    NONE                    = 0x0000,
    CAN_WALK                = 0x0001,         // Puede caminar
    CAN_RUN                 = 0x0002,         // Puede correr
    CAN_JUMP                = 0x0004,         // Puede saltar
    CAN_SNEAK               = 0x0008,         // Puede agacharse
    CAN_FLY                 = 0x0010,         // Puede volar
    CAN_SWIM                = 0x0020,         // Puede nadar
    CAN_CLIMB               = 0x0040,         // Puede escalar
    CAN_SLIDE               = 0x0080,         // Puede deslizarse
    ENABLE_GRAVITY          = 0x4000,         // Afectado por gravedad
    ENABLE_FRICTION         = 0x8000          // Afectado por fricción
};
```

---

## 🎒 **Inventory - Sistema de Inventario**

### 📦 **Tipos de Slots de Inventario**
```cpp
enum class InventorySlotType {
    Regular,                                 // Slot regular
    Hotbar,                                  // Barra rápida
    Equipment,                               // Equipamiento
    Crafting,                                // Fabricación
    Result                                   // Resultado de fabricación
};
```

### 👕 **Slots de Equipamiento**
```cpp
enum class EquipmentSlot {
    Head,                                    // Cabeza (casco)
    Chest,                                   // Pecho (pechera)
    Legs,                                    // Piernas (pantalones)
    Feet,                                    // Pies (botas)
    MainHand,                                // Mano principal
    OffHand                                  // Mano secundaria
};
```

### 📊 **Categorías de Items**
```cpp
enum class ItemCategory {
    Tool,                                    // Herramienta
    Weapon,                                  // Arma
    Armor,                                   // Armadura
    Block,                                   // Bloque
    Material,                                // Material
    Food,                                    // Comida
    Potion,                                  // Poción
    Miscellaneous                            // Misceláneo
};
```

### ⭐ **Rareza de Items**
```cpp
enum class ItemRarity {
    Common,                                  // Común
    Uncommon,                                // Poco común
    Rare,                                    // Raro
    Epic,                                    // Épico
    Legendary,                               // Legendario
    Mythic                                   // Mítico
};
```

### 🎒 **Configuración de Inventario**
```cpp
struct InventoryConfig {
    int mainInventorySize;                   // Tamaño del inventario principal
    int hotbarSize;                          // Tamaño de la barra rápida
    int equipmentSlotsCount;                 // Número de slots de equipamiento
    int craftingGridSize;                    // Tamaño de la rejilla de fabricación
    bool enableAutoStack;                    // Apilado automático
    bool enableWeightSystem;                 // Sistema de peso
    float maxCarryWeight;                    // Peso máximo a llevar
};
```

---

## 📊 **Rendimiento y Optimización**

### 🎯 **Objetivos de Rendimiento**
| Componente | Target Performance | Optimizaciones |
|------------|-------------------|----------------|
| Player Update | 60 FPS | Cache coherency, SIMD |
| Camera Update | 60 FPS | Frustum culling, LOD |
| Input Processing | < 1ms | Event batching, polling |
| Character Controller | 60 FPS | Physics optimization |
| Inventory Operations | < 5ms | Hash maps, memory pools |

### 🚀 **Optimizaciones Implementadas**
- **Input Batching**: Procesamiento de entrada en lotes para eficiencia
- **Camera Culling**: Frustum culling y occlusion culling para rendimiento
- **Physics Integration**: Integración optimizada con el motor de física
- **Memory Pooling**: Pools de memoria para operaciones frecuentes de inventario
- **Event System**: Sistema de eventos eficiente para comunicación entre componentes
- **Thread Safety**: Sincronización segura para operaciones multi-hilo
- **Cache Coherency**: Optimización de acceso a caché en estructuras de datos
- **Lazy Evaluation**: Evaluación perezosa de estados y cálculos

### 📈 **Escalabilidad**
```
Player Count: 1    → 60 FPS (Full features)
Player Count: 10   → 60 FPS (Multiplayer optimization)
Player Count: 100  → 30 FPS (Advanced optimization)
Player Count: 1000 → 10 FPS (Extreme optimization)
```

### 🎛️ **Configuraciones de Calidad**
- **High**: Cámara completa, colisiones, efectos, animaciones completas
- **Medium**: Cámara optimizada, colisiones básicas, efectos reducidos
- **Low**: Cámara simple, sin colisiones, efectos mínimos
- **Minimal**: Cámara básica, sin efectos, funcionalidad esencial

---

## 🔧 **Uso del Sistema**

### 🚀 **Configuración Básica del Jugador**
```cpp
// Crear configuración del jugador
PlayerConfig config;
config.playerName = "Steve";
config.mouseSensitivity = 0.5f;
config.fov = 90.0f;
config.enableAutoJump = true;

// Crear jugador
auto player = std::make_unique<Player>(config);
player->Initialize(world, rigidBody);
```

### 📹 **Configuración de Cámara**
```cpp
// Configurar cámara en tercera persona
CameraConfig cameraConfig;
cameraConfig.mode = CameraMode::ThirdPerson;
cameraConfig.distance = 5.0f;
cameraConfig.collisionRadius = 0.5f;
cameraConfig.enableCollisionDetection = true;
cameraConfig.smoothFollow = true;

// Crear cámara
auto camera = std::make_unique<Camera>(cameraConfig);
camera->SetTarget(player.get());
player->SetCamera(std::move(camera));
```

### 🎮 **Configuración de Controles**
```cpp
// Configurar entrada del jugador
InputConfig inputConfig;
inputConfig.mouseSensitivity = 0.5f;
inputConfig.invertMouseY = false;
inputConfig.enableVibration = true;

// Crear sistema de entrada
auto input = std::make_unique<PlayerInput>(inputConfig);
player->SetInput(std::move(input));
```

### 🏃‍♂️ **Configuración del Controlador de Personaje**
```cpp
// Configurar controlador de personaje
CharacterConfig charConfig;
charConfig.height = 1.8f;
charConfig.walkSpeed = 4.0f;
charConfig.runSpeed = 6.0f;
charConfig.jumpHeight = 1.2f;
charConfig.maxJumpCount = 2; // Doble salto

// Crear controlador
auto controller = std::make_unique<CharacterController>(charConfig);
player->SetCharacterController(std::move(controller));
```

### 🎒 **Configuración de Inventario**
```cpp
// Configurar inventario
InventoryConfig invConfig;
invConfig.mainInventorySize = 36; // 4 filas x 9 columnas
invConfig.hotbarSize = 9;
invConfig.enableWeightSystem = true;
invConfig.maxCarryWeight = 50.0f;

// Crear inventario
auto inventory = std::make_unique<Inventory>(invConfig);
player->SetInventory(std::move(inventory));
```

### 🎯 **Uso Avanzado - Creación de Items**
```cpp
// Crear item personalizado
ItemProperties swordProps;
swordProps.id = "diamond_sword";
swordProps.name = "Espada de Diamante";
swordProps.category = ItemCategory::Weapon;
swordProps.rarity = ItemRarity::Epic;
swordProps.attackDamage = 7.0f;
swordProps.attackSpeed = 1.6f;
swordProps.durability = 1561;
swordProps.weight = 1.5f;

auto diamondSword = std::make_shared<Item>(swordProps);

// Agregar al inventario
player->GetInventory()->AddItem(diamondSword);

// Equipar en mano principal
player->GetInventory()->EquipItem(diamondSword, EquipmentSlot::MainHand);
```

### 🎮 **Uso Avanzado - Controles Personalizados**
```cpp
// Crear binding personalizado
InputBinding dashBinding;
dashBinding.action = InputAction::Custom1;
dashBinding.device = InputDevice::Keyboard;
dashBinding.keyCode = GLFW_KEY_LEFT_SHIFT;
dashBinding.description = "Dash";

// Agregar binding
player->GetInput()->AddBinding(dashBinding);

// Verificar acción personalizada
if (player->GetInput()->IsPressed(InputAction::Custom1)) {
    player->GetCharacterController()->Dash(direction, 5.0f);
}
```

### 📹 **Uso Avanzado - Cámara Cinematográfica**
```cpp
// Configurar cámara cinematográfica
CameraConfig cinematicConfig;
cinematicConfig.mode = CameraMode::Cinematic;
cinematicConfig.fieldOfView = 45.0f;
cinematicConfig.movementSpeed = 2.0f;
cinematicConfig.enableMotionBlur = true;
cinematicConfig.vignetteStrength = 0.3f;

player->GetCamera()->SetConfig(cinematicConfig);

// Aplicar sacudida de cámara para efectos
player->GetCamera()->ApplyShake(0.5f, 10.0f, 2.0f);
```

### 🏃‍♂️ **Uso Avanzado - Movimiento Personalizado**
```cpp
// Implementar movimiento de parkour
auto controller = player->GetCharacterController();

// Wall jump
if (controller->CanWallJump()) {
    controller->WallJump();
}

// Glide
if (controller->GetVelocity().y < 0.0f) {
    controller->SetGliding(true);
}

// Dash con cooldown
static double lastDashTime = 0.0;
double currentTime = GetCurrentTime();
if (currentTime - lastDashTime > 3.0) { // 3 segundos de cooldown
    controller->Dash(direction, 5.0f);
    lastDashTime = currentTime;
}
```

### 🎒 **Uso Avanzado - Sistema de Crafting**
```cpp
// Crear receta personalizada
CraftingRecipe swordRecipe;
swordRecipe.id = "diamond_sword_recipe";
swordRecipe.name = "Espada de Diamante";
swordRecipe.ingredients = {
    {"diamond", 2},
    {"stick", 1}
};
swordRecipe.result = {"diamond_sword", 1};
swordRecipe.craftingTime = 5;

// Agregar receta al inventario
player->GetInventory()->AddRecipe(swordRecipe);

// Intentar fabricar
if (player->GetInventory()->CraftItem()) {
    std::cout << "Espada de diamante fabricada!" << std::endl;
}
```

---

## 🎮 **Ejemplos Prácticos**

### 🚗 **Sistema de Vehículo con Player**
```cpp
class VehicleController {
    Player* driver;
    RigidBody* vehicleBody;
    Camera* vehicleCamera;

    void EnterVehicle(Player* player) {
        driver = player;

        // Configurar cámara de vehículo
        CameraConfig vehicleCam;
        vehicleCam.mode = CameraMode::ThirdPerson;
        vehicleCam.distance = 8.0f;
        vehicleCam.heightOffset = 2.0f;

        vehicleCamera = new Camera(vehicleCam);
        vehicleCamera->SetTarget(vehicleBody);

        // Ocultar al jugador
        driver->SetVisible(false);

        // Conectar controles
        ConnectVehicleControls();
    }

    void Update(double deltaTime) {
        if (driver && vehicleBody) {
            // Procesar entrada del conductor
            ProcessDriverInput(deltaTime);

            // Actualizar cámara del vehículo
            vehicleCamera->Update(deltaTime);

            // Aplicar físicas del vehículo
            ApplyVehiclePhysics(deltaTime);
        }
    }
};
```

### 🎯 **Sistema de Combate Avanzado**
```cpp
class CombatSystem {
    Player* player;
    std::vector<AttackPattern> attackPatterns;

    void PerformAttack(const std::string& attackName) {
        auto pattern = GetAttackPattern(attackName);
        if (!pattern) return;

        // Verificar cooldown
        if (IsOnCooldown(attackName)) return;

        // Verificar estamina
        if (player->GetStats().stamina < pattern->staminaCost) return;

        // Ejecutar ataque
        ExecuteAttack(pattern);

        // Aplicar efectos visuales
        ApplyAttackEffects(pattern);

        // Aplicar cámara shake
        player->GetCamera()->ApplyShake(pattern->cameraShake, 15.0f, 0.3f);

        // Consumir estamina
        player->GetStats().stamina -= pattern->staminaCost;
    }

    void ExecuteAttack(AttackPattern* pattern) {
        // Crear hitbox
        AABB attackHitbox = CalculateAttackHitbox(pattern);

        // Buscar enemigos en rango
        auto enemies = FindEnemiesInRange(attackHitbox);

        // Aplicar daño
        for (auto enemy : enemies) {
            float damage = CalculateDamage(pattern, enemy);
            enemy->TakeDamage(damage, "melee", player);

            // Aplicar knockback
            ApplyKnockback(enemy, pattern->knockbackForce);
        }

        // Reproducir efectos de sonido
        PlayAttackSound(pattern);
    }
};
```

### 🌊 **Sistema de Natación Avanzado**
```cpp
class SwimmingSystem {
    Player* player;
    CharacterController* controller;

    void UpdateSwimming(double deltaTime) {
        if (!controller->IsInWater()) return;

        auto velocity = controller->GetVelocity();
        auto input = player->GetInput()->GetMovementInput();

        // Movimiento bajo el agua
        if (input.y > 0.0f) { // Nadar hacia arriba
            velocity.y = glm::mix(velocity.y, swimUpSpeed, deltaTime * 5.0f);
        } else if (input.y < 0.0f) { // Nadar hacia abajo
            velocity.y = glm::mix(velocity.y, -swimDownSpeed, deltaTime * 5.0f);
        }

        // Movimiento horizontal bajo el agua
        glm::vec3 horizontalInput(input.x, 0.0f, input.z);
        if (glm::length(horizontalInput) > 0.0f) {
            horizontalInput = glm::normalize(horizontalInput) * swimSpeed;
            velocity.x = glm::mix(velocity.x, horizontalInput.x, deltaTime * 3.0f);
            velocity.z = glm::mix(velocity.z, horizontalInput.z, deltaTime * 3.0f);
        }

        // Aplicar resistencia del agua
        velocity *= (1.0f - waterDrag * deltaTime);

        controller->SetVelocity(velocity);

        // Actualizar cámara bajo el agua
        UpdateUnderwaterCamera();
    }

    void UpdateUnderwaterCamera() {
        auto camera = player->GetCamera();

        // Efecto de niebla bajo el agua
        camera->SetFogEnabled(true);
        camera->SetFogColor(underwaterFogColor);
        camera->SetFogDensity(underwaterFogDensity);

        // Efecto de distorsión
        camera->SetDistortionEnabled(true);
        camera->SetDistortionStrength(0.3f);
    }
};
```

### 🎒 **Sistema de Inventario con UI**
```cpp
class InventoryUI {
    Inventory* inventory;
    Player* player;

    void RenderInventory() {
        // Renderizar fondo del inventario
        RenderInventoryBackground();

        // Renderizar slots del inventario
        for (int i = 0; i < inventory->GetTotalSize(); i++) {
            RenderInventorySlot(i);
        }

        // Renderizar barra rápida
        RenderHotbar();

        // Renderizar equipamiento
        RenderEquipmentSlots();

        // Renderizar rejilla de fabricación
        RenderCraftingGrid();

        // Renderizar información del item
        if (hoveredSlot != -1) {
            RenderItemTooltip(hoveredSlot);
        }
    }

    void HandleInventoryInput() {
        auto input = player->GetInput();

        // Drag and drop
        if (input->IsPressed(InputAction::Interact)) {
            StartDragOperation();
        }

        // Quick move (shift + click)
        if (input->IsHeld(InputAction::Sneak) &&
            input->WasJustPressed(InputAction::Interact)) {
            PerformQuickMove();
        }

        // Stack items (control + click)
        if (input->IsHeld(InputAction::Sprint) &&
            input->WasJustPressed(InputAction::Interact)) {
            PerformItemStacking();
        }
    }

    void PerformQuickMove() {
        if (selectedItem) {
            // Encontrar slot apropiado para el item
            int targetSlot = FindQuickMoveTarget(selectedItem);

            if (targetSlot != -1) {
                // Mover item al slot destino
                inventory->SwapItems(selectedSlot, targetSlot);
            }
        }
    }
};
```

### 🎥 **Sistema de Cámara Cinematográfica**
```cpp
class CinematicCameraSystem {
    Player* player;
    Camera* cinematicCamera;
    std::vector<CinematicPoint> cinematicPath;

    struct CinematicPoint {
        glm::vec3 position;
        glm::quat rotation;
        float duration;
        float fov;
        bool enableShake;
        float shakeIntensity;
    };

    void StartCinematic(const std::string& cinematicName) {
        // Cargar puntos de la cinemática
        LoadCinematicPath(cinematicName);

        // Configurar cámara cinematográfica
        CameraConfig cinematicConfig;
        cinematicConfig.mode = CameraMode::Cinematic;
        cinematicConfig.enableMotionBlur = true;
        cinematicConfig.enableVignette = true;
        cinematicConfig.enableDepthOfField = true;

        cinematicCamera = new Camera(cinematicConfig);

        // Ocultar cámara del jugador
        player->GetCamera()->SetActive(false);

        // Iniciar cinemática
        currentPointIndex = 0;
        cinematicActive = true;
    }

    void UpdateCinematic(double deltaTime) {
        if (!cinematicActive) return;

        auto currentPoint = cinematicPath[currentPointIndex];

        // Interpolar posición y rotación
        float t = currentTime / currentPoint.duration;
        if (t >= 1.0f) {
            // Avanzar al siguiente punto
            currentPointIndex++;
            if (currentPointIndex >= cinematicPath.size()) {
                EndCinematic();
                return;
            }
            currentTime = 0.0f;
            return;
        }

        auto nextPoint = cinematicPath[currentPointIndex + 1];
        glm::vec3 position = glm::mix(currentPoint.position, nextPoint.position, t);
        glm::quat rotation = glm::slerp(currentPoint.rotation, nextPoint.rotation, t);
        float fov = glm::mix(currentPoint.fov, nextPoint.fov, t);

        cinematicCamera->SetPosition(position);
        cinematicCamera->SetOrientation(rotation);
        cinematicCamera->SetFieldOfView(fov);

        // Aplicar efectos
        if (currentPoint.enableShake) {
            cinematicCamera->ApplyShake(currentPoint.shakeIntensity, 5.0f, deltaTime);
        }

        currentTime += deltaTime;
    }

    void EndCinematic() {
        cinematicActive = false;
        player->GetCamera()->SetActive(true);
        delete cinematicCamera;
        cinematicCamera = nullptr;
    }
};
```

---

## 🏆 **Conclusión**

El **Player & Camera System** de VoxelCraft representa el estado del arte en sistemas de jugador para juegos modernos. Su arquitectura modular y altamente optimizada permite:

- ✅ **Experiencia de Jugador Inmersiva**: Controles precisos, cámara dinámica, inventario completo
- ✅ **Rendimiento Superior**: Optimización para 60+ FPS con múltiples sistemas activos
- ✅ **Escalabilidad Total**: Desde prototipos hasta juegos AAA con configuraciones adaptables
- ✅ **Extensibilidad Máxima**: Fácil adición de nuevas mecánicas y sistemas
- ✅ **Integración Completa**: Perfecta integración con física, mundo, entidades y UI

**Este sistema forma la base sólida sobre la cual se construirá la experiencia de jugador completa del juego. 🚀🎮✨**
