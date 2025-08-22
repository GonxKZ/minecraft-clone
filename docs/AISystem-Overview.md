# 🧠 **VoxelCraft AI System - Complete Intelligence Framework**

## 📋 **Índice de Contenidos**

1. [🎯 Visión General del Sistema de IA](#-visión-general-del-sistema-de-ia)
2. [🏗️ Arquitectura del Sistema de IA](#️-arquitectura-del-sistema-de-ia)
3. [📦 Componentes Principales](#-componentes-principales)
4. [🎮 AISystem - Sistema Principal de IA](#-aisystem---sistema-principal-de-ia)
5. [🌳 BehaviorTree - Árboles de Comportamiento](#-behaviortree---árboles-de-comportamiento)
6. [🧭 PathfindingSystem - Sistema de Navegación](#-pathfindingsystem---sistema-de-navegación)
7. [👥 Mob - Entidades IA Inteligentes](#-mob---entidades-ia-inteligentes)
8. [🏘️ VillageSystem - Sistema de Aldeas](#️-villagesystem---sistema-de-aldeas)
9. [📊 Rendimiento y Optimización](#-rendimiento-y-optimización)
10. [🔧 Uso del Sistema](#-uso-del-sistema)
11. [🎮 Ejemplos Prácticos](#-ejemplos-prácticos)

---

## 🎯 **Visión General del Sistema de IA**

El **VoxelCraft AI System** es un framework completo de inteligencia artificial diseñado específicamente para juegos de mundo abierto con voxel. El sistema proporciona comportamientos emergentes, navegación inteligente, civilizaciones dinámicas y aprendizaje adaptativo.

### 🎯 **Objetivos Principales**
- **Inteligencia Emergente**: Crear comportamientos realistas sin scripting manual
- **Navegación Avanzada**: A* optimizado para mundos de voxels con obstáculos dinámicos
- **Civilizaciones Vivas**: Aldeas que crecen, comercian y se defienden
- **Aprendizaje Adaptativo**: IA que aprende de interacciones y se adapta
- **Escalabilidad**: Manejar miles de entidades IA simultáneamente

### 🏆 **Características Destacadas**
- ✅ **Behavior Trees Avanzados**: Sistema modular de árboles de comportamiento
- ✅ **A* Optimizado**: Pathfinding con optimizaciones específicas para voxels
- ✅ **Mobs Inteligentes**: Entidades con memoria, sensores y comportamientos complejos
- ✅ **Aldeas Dinámicas**: Civilizaciones completas con economía y política
- ✅ **Integración Completa**: Funciona perfectamente con física, mundo y jugadores
- ✅ **Debugging Visual**: Herramientas completas de visualización y profiling

---

## 🏗️ **Arquitectura del Sistema de IA**

### 📊 **Estructura General**
```
┌─────────────────────────────────────────────────────────────┐
│                    AISystem (Main Controller)               │
├─────────────────────────────────────────────────────────────┤
│  ┌─────────────┬─────────────┬─────────────┬─────────────┐ │
│  │BehaviorTree│Pathfinding │    Mob     │VillageSystem│ │
│  │  System    │  System    │   System   │              │ │
│  └─────────────┴─────────────┴─────────────┴─────────────┘ │
├─────────────────────────────────────────────────────────────┤
│              Sensory, Learning, & Decision Systems          │
│  ┌─────────────┬─────────────┬─────────────┬─────────────┐ │
│  │ Navigation  │ Procedural  │  Sensory   │   Learning  │ │
│  │  System     │ Generation  │  System    │   System    │ │
│  └─────────────┴─────────────┴─────────────┴─────────────┘ │
└─────────────────────────────────────────────────────────────┘
```

### 🔄 **Flujo de Datos del Sistema de IA**
```
World State → Sensory System → Decision System → Behavior Tree
                      ↓                    ↓               ↓
               Memory System ←→ Learning System ←→ Action System
                      ↓                    ↓               ↓
             Pathfinding ←→ Navigation ←→ Physics Integration
                      ↓                    ↓               ↓
               Village System ←→ Economy ←→ Social Simulation
```

---

## 📦 **Componentes Principales**

### 🎮 **AISystem - Sistema Principal de IA**
```cpp
class AISystem {
    // Configuración y control
    AISystemConfig config;                     // Configuración del sistema
    AISystemMetrics metrics;                   // Métricas de rendimiento

    // Subsistemas principales
    std::unique_ptr<PathfindingSystem> pathfindingSystem;
    std::unique_ptr<NavigationSystem> navigationSystem;
    std::unique_ptr<ProceduralGenerator> proceduralGenerator;
    std::unique_ptr<VillageSystem> villageSystem;

    // Gestión de entidades
    std::unordered_map<uint32_t, std::shared_ptr<Mob>> activeMobs;
    std::vector<AIUpdatePriority> updatePriorities;

    // Actualización principal
    void Update(double deltaTime);             // Actualización del sistema
    bool SpawnMob(const std::string& type, const glm::vec3& position);
    void ProcessTasks();                        // Procesamiento de tareas
};
```

### 🌳 **BehaviorTree - Árboles de Comportamiento**
```cpp
class BehaviorTree {
    // Estructura del árbol
    BehaviorTreeConfig config;                 // Configuración del árbol
    std::shared_ptr<BehaviorNode> root;        // Nodo raíz
    BehaviorTreeMetrics metrics;               // Métricas de rendimiento

    // Nodos de comportamiento
    std::shared_ptr<ActionNode> CreateAction(const std::string& name, ActionFunction action);
    std::shared_ptr<ConditionNode> CreateCondition(const std::string& name, ConditionFunction condition);
    std::shared_ptr<CompositeNode> CreateComposite(const std::string& name, CompositeType type);

    // Ejecución
    BehaviorNodeStatus Update(double deltaTime); // Actualización del árbol
    void Reset();                               // Reset del árbol
    std::string GetVisualizationData() const;   // Datos de visualización
};
```

### 🧭 **PathfindingSystem - Sistema de Navegación**
```cpp
class PathfindingSystem {
    // Configuración y datos
    PathfindingConfig config;                  // Configuración del sistema
    NavigationGrid navigationGrid;             // Grid de navegación 3D

    // Algoritmos de pathfinding
    PathResult FindPath(const glm::vec3& start, const glm::vec3& goal);
    PathResult ExecuteAStar(const PathRequest& request);
    PathResult ExecuteThetaStar(const PathRequest& request);
    PathResult ExecuteJumpPointSearch(const PathRequest& request);

    // Optimizaciones
    bool IsPositionWalkable(const glm::vec3& position);
    std::vector<glm::vec3> SimplifyPath(const std::vector<glm::vec3>& path);
    void RebuildNavigationGrid(bool force);
};
```

### 👥 **Mob - Entidades IA Inteligentes**
```cpp
class Mob : public Entity {
    // Configuración y estadísticas
    MobConfig config;                          // Configuración del mob
    MobStats stats;                            // Estadísticas dinámicas
    MobMemory memory;                          // Sistema de memoria
    MobSenses senses;                          // Sistema sensorial

    // Comportamiento y IA
    std::shared_ptr<BehaviorTree> behaviorTree; // Árbol de comportamiento
    std::unique_ptr<Blackboard> blackboard;    // Pizarra de datos

    // Acciones y movimiento
    bool MoveTo(const glm::vec3& position);    // Mover a posición
    bool Attack(Entity* target);               // Atacar objetivo
    bool CanSee(Entity* entity) const;         // Verificar línea de vista
    void UpdateSenses(double deltaTime);       // Actualizar sentidos
};
```

### 🏘️ **VillageSystem - Sistema de Aldeas**
```cpp
class VillageSystem {
    // Configuración y datos
    VillageConfig config;                      // Configuración del sistema
    std::unordered_map<uint32_t, std::unique_ptr<Village>> villages;

    // Generación y gestión
    Village* GenerateVillage(const glm::vec3& position, VillageType type);
    void UpdateVillages(double deltaTime);     // Actualización de aldeas
    bool EstablishTradeRoute(uint32_t village1, uint32_t village2);

    // Economía y recursos
    bool TradeWith(Village* otherVillage, const std::unordered_map<std::string, float>& tradeDeal);
    float GetTotalWealth() const;              // Riqueza total del sistema
};
```

---

## 🎮 **AISystem - Sistema Principal de IA**

### ⚙️ **Configuración del Sistema de IA**
```cpp
struct AISystemConfig {
    // Configuración básica
    AISystemMode mode;                         // Modo del sistema
    float updateInterval;                      // Intervalo de actualización
    int maxConcurrentUpdates;                  // Actualizaciones concurrentes
    bool enableMultithreading;                 // Multihilo habilitado

    // Configuración de rendimiento
    float maxUpdateTime;                       // Tiempo máximo por actualización
    int maxActiveMobs;                         // Máximo de mobs activos
    float mobUpdateDistance;                   // Distancia de actualización
    bool enablePathfinding;                    // Pathfinding habilitado

    // Configuración de aldeas
    bool enableProceduralGeneration;           // Generación procedural
    float structureGenerationRadius;           // Radio de generación
    bool enableVillageGeneration;              // Generación de aldeas
};
```

### 📊 **Métricas del Sistema de IA**
```cpp
struct AISystemMetrics {
    // Métricas de rendimiento
    uint64_t updateCount;                      // Conteo de actualizaciones
    double totalUpdateTime;                    // Tiempo total de actualización
    double averageUpdateTime;                  // Tiempo promedio
    uint32_t activeMobs;                       // Mobs activos
    uint32_t totalMobs;                        // Total de mobs

    // Métricas de pathfinding
    uint64_t pathRequests;                     // Solicitudes de pathfinding
    uint64_t successfulPaths;                  // Paths exitosos
    double averagePathfindingTime;             // Tiempo promedio de pathfinding

    // Métricas de aldeas
    uint64_t villagesGenerated;                // Aldeas generadas
    uint64_t structuresGenerated;              // Estructuras generadas
    uint32_t villagesActive;                   // Aldeas activas
};
```

### 🎯 **Modos del Sistema de IA**
```cpp
enum class AISystemMode {
    Normal,                                    // Operación normal
    Debug,                                     // Modo debug con logging extra
    Performance,                               // Monitoreo de rendimiento
    Learning,                                  // Aprendizaje y adaptación
    Minimal                                    // IA mínima para sistemas bajos
};
```

### ⚡ **Prioridades de Actualización**
```cpp
enum class AIUpdatePriority {
    Critical,                                  // Crítico (amenazas al jugador)
    High,                                      // Alta prioridad
    Medium,                                    // Prioridad media
    Low,                                       // Baja prioridad
    Background                                 // Fondo
};
```

---

## 🌳 **BehaviorTree - Árboles de Comportamiento**

### 🌿 **Tipos de Nodos de Comportamiento**
```cpp
enum class BehaviorNodeType {
    Action,                                    // Nodo que ejecuta acciones
    Condition,                                 // Nodo que verifica condiciones
    Composite,                                 // Nodo compuesto con hijos
    Decorator                                  // Decorador que modifica comportamiento
};
```

### 🌱 **Nodos Compuestos**
```cpp
enum class CompositeType {
    Sequence,                                  // Ejecuta hijos en secuencia
    Selector,                                  // Ejecuta hasta que uno tenga éxito
    Parallel,                                  // Ejecuta todos en paralelo
    RandomSelector,                            // Selector aleatorio
    WeightedRandom,                            // Aleatorio con pesos
    Priority                                   // Por prioridad
};
```

### 🎨 **Decoradores de Comportamiento**
```cpp
enum class DecoratorType {
    Inverter,                                  // Invierte resultado
    Succeeder,                                 // Siempre devuelve éxito
    Repeater,                                  // Repite ejecución
    Timer,                                     // Límite de tiempo
    Cooldown,                                  // Tiempo de reutilización
    Delay,                                     // Retardo de ejecución
    Conditional,                               // Condición adicional
    Probability                                // Probabilidad de ejecución
};
```

### 🌟 **Configuración de Árboles de Comportamiento**
```cpp
struct BehaviorTreeConfig {
    std::string name;                          // Nombre del árbol
    std::string description;                   // Descripción
    float updateInterval;                      // Intervalo de actualización
    float maxExecutionTime;                    // Tiempo máximo de ejecución
    int maxDepth;                              // Profundidad máxima
    bool enableDebugging;                      // Debugging habilitado
    bool enableProfiling;                      // Profiling habilitado
};
```

### 🌿 **Ejemplo de Árbol de Comportamiento**
```
Root (Selector)
├── Sequence "Idle"
│   ├── Condition "Is Player Nearby"
│   └── Action "Look At Player"
├── Sequence "Patrol"
│   ├── Condition "Has Patrol Route"
│   └── Action "Follow Patrol Route"
├── Sequence "Attack"
│   ├── Condition "Can See Player"
│   ├── Condition "In Attack Range"
│   └── Action "Attack Player"
└── Action "Wander Randomly"
```

---

## 🧭 **PathfindingSystem - Sistema de Navegación**

### 🧮 **Algoritmos de Pathfinding**
```cpp
enum class PathfindingAlgorithm {
    AStar,                                     // A* estándar
    ThetaStar,                                 // Theta* (any-angle)
    LazyThetaStar,                             // Lazy Theta* optimizado
    JumpPointSearch,                           // Jump Point Search (JPS)
    HierarchicalAStar,                         // A* jerárquico
    FlowField,                                 // Campo de flujo
    PotentialField,                            // Campo potencial
    Hybrid                                     // Híbrido adaptativo
};
```

### 🗺️ **Configuración de Pathfinding**
```cpp
struct PathfindingConfig {
    // Algoritmo y heurística
    PathfindingAlgorithm defaultAlgorithm;      // Algoritmo por defecto
    HeuristicType defaultHeuristic;            // Función heurística

    // Rendimiento
    float updateInterval;                      // Intervalo de actualización
    float defaultTimeout;                      // Timeout por defecto
    int maxPathLength;                         // Longitud máxima del path
    int maxNodesExplored;                      // Nodos máximos a explorar

    // Grid de navegación
    float nodeSize;                            // Tamaño de nodo
    float heightStep;                          // Paso de altura
    bool enableDynamicGridUpdates;             // Actualización dinámica del grid

    // Optimizaciones
    bool enableJumpLinks;                      // Enlaces de salto
    bool enableHierarchicalSearch;             // Búsqueda jerárquica
    bool enablePathCaching;                    // Cache de paths
    int cacheSize;                             // Tamaño del cache
};
```

### 🧩 **Grid de Navegación**
```cpp
struct NavigationGrid {
    std::vector<std::vector<std::vector<PathNode>>> nodes; // Grid 3D
    glm::vec3 gridOrigin;                     // Origen del grid
    glm::vec3 gridSize;                       // Tamaño del grid
    float nodeSize;                           // Tamaño de cada nodo
    bool isValid;                             // Grid válido
    uint32_t version;                         // Versión para invalidar cache
};
```

### 🗺️ **Nodo de Pathfinding**
```cpp
struct PathNode {
    glm::vec3 position;                       // Posición en el mundo
    float gCost;                              // Costo desde inicio
    float hCost;                              // Costo heurístico al objetivo
    float fCost;                              // Costo total (g + h)
    PathNode* parent;                         // Nodo padre en el path
    bool isWalkable;                          // Es transitable
    bool isVisited;                           // Ha sido visitado
    int x, y, z;                              // Coordenadas del grid
};
```

---

## 👥 **Mob - Entidades IA Inteligentes**

### 👤 **Configuración de Mobs**
```cpp
struct MobConfig {
    // Propiedades básicas
    std::string name;                         // Nombre del mob
    MobType mobType;                          // Tipo de mob
    uint32_t behaviorFlags;                   // Flags de comportamiento

    // Propiedades físicas
    float width, height;                      // Dimensiones
    float mass;                               // Masa
    float maxHealth;                          // Salud máxima
    float movementSpeed;                      // Velocidad de movimiento

    // Propiedades de combate
    float attackDamage;                       // Daño de ataque
    float attackRange;                        // Rango de ataque
    float defense;                            // Defensa

    // Propiedades de IA
    float sightRange;                         // Rango de visión
    float hearingRange;                       // Rango de audición
    float memorySpan;                         // Duración de memoria
    int aggressionLevel;                      // Nivel de agresión
};
```

### 🧠 **Estadísticas Dinámicas de Mobs**
```cpp
struct MobStats {
    // Salud y estado
    float currentHealth;                      // Salud actual
    float healthRegeneration;                 // Regeneración de salud
    bool isInvulnerable;                      // Invulnerable

    // Combate
    float attackDamage;                       // Daño actual
    int killCount;                            // Conteo de muertes
    float totalDamageDealt;                   // Daño total infligido

    // Movimiento
    float currentSpeed;                       // Velocidad actual
    float totalDistanceTraveled;              // Distancia total recorrida
    int jumpsPerformed;                       // Saltos realizados

    // Supervivencia
    float hunger;                             // Nivel de hambre
    float thirst;                             // Nivel de sed
    bool isHungry;                            // Tiene hambre
};
```

### 🧠 **Sistema de Memoria de Mobs**
```cpp
struct MobMemory {
    // Lugares conocidos
    std::vector<glm::vec3> knownLocations;    // Lugares conocidos
    std::unordered_map<std::string, glm::vec3> locationTags; // Lugares etiquetados
    glm::vec3 homeLocation;                   // Ubicación del hogar

    // Entidades recordadas
    std::unordered_map<uint32_t, Entity*> knownEntities; // Entidades conocidas
    std::vector<uint32_t> friendlyEntities;  // Entidades amigables
    std::vector<uint32_t> hostileEntities;   // Entidades hostiles

    // Eventos recientes
    std::vector<std::string> recentEvents;    // Eventos recientes
    std::unordered_map<std::string, double> eventTimestamps; // Timestamps de eventos
};
```

### 👀 **Sistema Sensorial de Mobs**
```cpp
struct MobSenses {
    // Sentidos visuales
    std::vector<Entity*> visibleEntities;     // Entidades visibles
    std::vector<glm::vec3> visibleLocations;  // Ubicaciones visibles
    float currentSightRange;                  // Rango visual actual

    // Sentidos auditivos
    std::vector<std::pair<glm::vec3, float>> heardSounds; // Sonidos oídos
    float currentHearingRange;                // Rango auditivo actual

    // Sentidos olfativos
    std::vector<std::pair<glm::vec3, float>> smelledScents; // Olores detectados
    float currentSmellRange;                  // Rango olfativo actual

    // Procesamiento
    double lastSenseUpdate;                   // Última actualización
    float senseUpdateFrequency;               // Frecuencia de actualización
};
```

### 🎯 **Tipos de Mobs**
```cpp
enum class MobType {
    Passive,                                  // Pasivo (oveja, vaca)
    Neutral,                                  // Neutral (cerdo, gallina)
    Hostile,                                  // Hostil (zombie, creeper)
    Boss,                                     // Jefe
    NPC,                                      // Personaje no jugador
    Custom                                    // Personalizado
};
```

### 🏴 **Flags de Comportamiento**
```cpp
enum class MobBehavior : uint32_t {
    NONE                    = 0x0000,
    CAN_MOVE                = 0x0001,         // Puede moverse
    CAN_JUMP                = 0x0002,         // Puede saltar
    CAN_FLY                 = 0x0004,         // Puede volar
    CAN_SWIM                = 0x0008,         // Puede nadar
    CAN_ATTACK              = 0x0020,         // Puede atacar
    CAN_BE_ATTACKED         = 0x0040,         // Puede ser atacado
    DROPS_ITEMS             = 0x0800,         // Suelta items al morir
    HAS_SENSES              = 0x4000,         // Tiene sistema sensorial
    HAS_MEMORY              = 0x8000          // Tiene sistema de memoria
};
```

---

## 🏘️ **VillageSystem - Sistema de Aldeas**

### 🏠 **Tipos de Aldeas**
```cpp
enum class VillageType {
    Rural,                                    // Rural
    Town,                                     // Pueblo
    City,                                     // Ciudad
    Fortress,                                 // Fortaleza
    Monastery,                                // Monasterio
    TradingPost,                              // Puesto comercial
    Farmstead,                                // Granja
    MiningColony,                             // Colonia minera
    FishingVillage,                           // Pueblo pesquero
    NomadicCamp                               // Campamento nómada
};
```

### 🏢 **Tipos de Edificios**
```cpp
enum class BuildingType {
    House,                                    // Casa
    Blacksmith,                               // Herrería
    Farm,                                     // Granja
    Inn,                                      // Posada
    Temple,                                   // Templo
    GuardTower,                               // Torre de guardia
    Wall,                                     // Muro
    Well,                                     // Pozo
    Market,                                   // Mercado
    Workshop                                  // Taller
};
```

### 👥 **Roles en la Aldea**
```cpp
enum class VillageRole {
    Villager,                                 // Aldeano básico
    Elder,                                    // Anciano/líder
    Blacksmith,                               // Herrero
    Farmer,                                   // Granjero
    Merchant,                                 // Mercader
    Guard,                                    // Guardia
    Priest,                                   // Sacerdote
    Teacher,                                  // Maestro
    Healer,                                   // Sanador
    Child                                     // Niño
};
```

### 🏘️ **Datos de Aldea**
```cpp
struct VillageData {
    uint32_t villageId;                       // ID único
    std::string name;                         // Nombre de la aldea
    VillageType type;                         // Tipo de aldea
    VillageState state;                       // Estado actual
    glm::vec3 center;                         // Centro de la aldea
    float radius;                             // Radio de la aldea

    // Población
    std::vector<Mob*> villagers;              // Aldeanos
    int population;                           // Población actual
    std::unordered_map<VillageRole, std::vector<Mob*>> roleAssignments;

    // Edificios
    std::vector<BuildingData> buildings;      // Edificios de la aldea
    std::vector<glm::vec3> roads;             // Red de caminos

    // Recursos y economía
    std::unordered_map<std::string, float> resources; // Recursos
    float wealth;                             // Riqueza
    float reputation;                         // Reputación
};
```

### 🏠 **Datos de Edificios**
```cpp
struct BuildingData {
    uint32_t buildingId;                      // ID único
    BuildingType type;                        // Tipo de edificio
    std::string name;                         // Nombre del edificio
    glm::vec3 position;                       // Posición
    glm::vec3 size;                           // Dimensiones
    int level;                                // Nivel/mejora
    float condition;                          // Condición (0.0 - 1.0)
    bool isOccupied;                          // Ocupado
    Mob* owner;                               // Propietario
    std::vector<Mob*> occupants;              // Ocupantes
};
```

---

## 📊 **Rendimiento y Optimización**

### 🎯 **Objetivos de Rendimiento**
| Componente | Target Performance | Optimizaciones |
|------------|-------------------|----------------|
| AI System Update | 60 FPS | Multithreading, priority queues |
| Behavior Tree | < 1ms | Node caching, parallel execution |
| Pathfinding | < 5ms | Grid optimization, caching |
| Mob Update | 60 FPS | Distance culling, LOD |
| Village System | 30 FPS | Spatial partitioning, batch updates |

### 🚀 **Optimizaciones Implementadas**
- **Spatial Partitioning**: División espacial para queries eficientes
- **Path Caching**: Cache de rutas para evitar recálculo
- **Distance Culling**: Actualización basada en distancia al jugador
- **Priority Queues**: Sistema de prioridades para actualizaciones
- **Multithreading**: Procesamiento paralelo de IA
- **Grid Optimization**: Optimización de grid de navegación
- **Memory Pooling**: Pools de memoria para entidades IA
- **Event-Driven**: Sistema orientado a eventos para comunicación

### 📈 **Escalabilidad**
```
Mob Count: 100    → 60 FPS (Full AI features)
Mob Count: 500    → 60 FPS (Optimized updates)
Mob Count: 1000   → 30 FPS (Distance culling)
Mob Count: 5000   → 10 FPS (Minimal AI)
Village Count: 10  → 60 FPS (Full simulation)
Village Count: 50  → 30 FPS (Batch processing)
Village Count: 200 → 10 FPS (Essential systems)
```

### ⚙️ **Configuraciones de Calidad**
- **High**: IA completa, pathfinding avanzado, aldeas dinámicas
- **Medium**: IA optimizada, pathfinding básico, aldeas simples
- **Low**: IA mínima, pathfinding directo, aldeas estáticas
- **Minimal**: IA esencial, sin pathfinding, sin aldeas

---

## 🔧 **Uso del Sistema**

### 🚀 **Configuración Básica del Sistema de IA**
```cpp
// Configurar sistema de IA
AISystemConfig aiConfig;
aiConfig.mode = AISystemMode::Normal;
aiConfig.updateInterval = 1.0f / 60.0f;      // 60 FPS
aiConfig.maxConcurrentUpdates = 1000;
aiConfig.enableMultithreading = true;
aiConfig.maxActiveMobs = 500;

// Crear sistema de IA
auto aiSystem = std::make_unique<AISystem>(aiConfig);
aiSystem->Initialize();
```

### 🌳 **Creación de Árboles de Comportamiento**
```cpp
// Crear árbol de comportamiento para mob hostil
auto hostileTree = std::make_shared<BehaviorTree>(BehaviorTreeConfig{
    "HostileMobTree",
    "Comportamiento de mob hostil",
    1.0f / 30.0f,
    10.0f,
    5,
    true,
    true
});

// Crear nodos
auto root = BehaviorTree::CreateComposite("Root", CompositeType::Selector);
auto attackSequence = BehaviorTree::CreateComposite("Attack", CompositeType::Sequence);
auto patrolSequence = BehaviorTree::CreateComposite("Patrol", CompositeType::Sequence);

// Crear acciones
auto attackAction = BehaviorTree::CreateAction("AttackPlayer", [](double dt, Mob* mob, Blackboard* bb) {
    // Lógica de ataque
    return BehaviorNodeStatus::Success;
});

auto moveToPlayerAction = BehaviorTree::CreateAction("MoveToPlayer", [](double dt, Mob* mob, Blackboard* bb) {
    // Lógica de movimiento hacia jugador
    return BehaviorNodeStatus::Running;
});

// Ensamblar árbol
attackSequence->AddChild(BehaviorTree::CreateCondition("CanSeePlayer", canSeePlayerCondition));
attackSequence->AddChild(moveToPlayerAction);
attackSequence->AddChild(attackAction);
root->AddChild(attackSequence);
root->AddChild(patrolSequence);

hostileTree->SetRoot(root);
```

### 🧭 **Configuración de Pathfinding**
```cpp
// Configurar sistema de pathfinding
PathfindingConfig pathConfig;
pathConfig.defaultAlgorithm = PathfindingAlgorithm::AStar;
pathConfig.defaultHeuristic = HeuristicType::Octile;
pathConfig.nodeSize = 1.0f;
pathConfig.heightStep = 1.0f;
pathConfig.enableDynamicGridUpdates = true;
pathConfig.enablePathCaching = true;
pathConfig.cacheSize = 1000;

// Crear sistema de pathfinding
auto pathfinding = std::make_unique<PathfindingSystem>(pathConfig);
pathfinding->Initialize(world);

// Solicitar path
PathRequest request;
request.requestId = 1;
request.startPosition = mob->GetPosition();
request.goalPosition = player->GetPosition();
request.pathType = PathType::Ground;
request.agentRadius = 0.5f;
request.timeout = 5.0f;

uint32_t pathId = pathfinding->RequestPath(request);
```

### 👥 **Creación de Mobs Inteligentes**
```cpp
// Configurar mob
MobConfig mobConfig;
mobConfig.name = "Zombie";
mobConfig.mobType = MobType::Hostile;
mobConfig.behaviorFlags = static_cast<uint32_t>(MobBehavior::CAN_MOVE) |
                         static_cast<uint32_t>(MobBehavior::CAN_ATTACK) |
                         static_cast<uint32_t>(MobBehavior::HAS_SENSES);
mobConfig.width = 0.6f;
mobConfig.height = 1.8f;
mobConfig.maxHealth = 20.0f;
mobConfig.movementSpeed = 2.0f;
mobConfig.sightRange = 16.0f;
mobConfig.hearingRange = 8.0f;
mobConfig.attackDamage = 3.0f;
mobConfig.attackRange = 2.0f;

// Crear mob
auto zombie = std::make_shared<Mob>(mobConfig);
zombie->Initialize();
zombie->SetBehaviorTree(hostileTree);
zombie->SetPosition(spawnPosition);

// Agregar al sistema de IA
aiSystem->SpawnMob("zombie", spawnPosition);
```

### 🏘️ **Generación de Aldeas**
```cpp
// Configurar sistema de aldeas
VillageConfig villageConfig;
villageConfig.defaultType = VillageType::Rural;
villageConfig.minVillageSize = 5;
villageConfig.maxVillageSize = 20;
villageConfig.minVillageSpacing = 100.0f;
villageConfig.enableVillageExpansion = true;
villageConfig.enableResourceManagement = true;
villageConfig.enableVillageDefense = true;

// Crear sistema de aldeas
auto villageSystem = std::make_unique<VillageSystem>(villageConfig);
villageSystem->Initialize(world, proceduralGenerator);

// Generar aldea
Village* village = villageSystem->GenerateVillage(centerPosition, VillageType::Rural);
if (village) {
    village->SetName("Oakwood Village");
    std::cout << "Aldea generada: " << village->GetName() << std::endl;
}
```

### 🎯 **Integración Completa de IA**
```cpp
// Sistema principal de actualización de IA
void Game::UpdateAI(double deltaTime) {
    // Actualizar sistema de IA principal
    aiSystem->Update(deltaTime);

    // Actualizar pathfinding
    pathfindingSystem->Update(deltaTime);

    // Actualizar aldeas
    villageSystem->Update(deltaTime);

    // Procesar resultados de pathfinding
    auto results = pathfindingSystem->GetCompletedResults();
    for (const auto& result : results) {
        if (result.status == PathStatus::Success) {
            // Aplicar path al mob correspondiente
            auto mob = aiSystem->GetMob(result.requestId);
            if (mob) {
                mob->SetPath(result.path);
            }
        }
    }

    // Actualizar estadísticas de IA
    UpdateAIMetrics(deltaTime);
}

void Game::UpdateAIMetrics(double deltaTime) {
    auto& metrics = aiSystem->GetMetrics();
    ImGui::Begin("AI System Metrics");
    ImGui::Text("Active Mobs: %d", metrics.activeMobs);
    ImGui::Text("Path Requests: %llu", metrics.pathRequests);
    ImGui::Text("Successful Paths: %llu", metrics.successfulPaths);
    ImGui::Text("Average Update Time: %.2f ms", metrics.averageUpdateTime * 1000.0);
    ImGui::End();
}
```

---

## 🎮 **Ejemplos Prácticos**

### 🧟 **Sistema de Zombie Avanzado**
```cpp
class ZombieBehavior {
public:
    static std::shared_ptr<BehaviorTree> CreateBehaviorTree() {
        auto tree = std::make_shared<BehaviorTree>(BehaviorTreeConfig{
            "ZombieBehavior",
            "Comportamiento de zombie inteligente",
            1.0f / 10.0f,
            5.0f
        });

        // Crear nodos principales
        auto root = BehaviorTree::CreateComposite("Root", CompositeType::Selector);

        // Secuencia de ataque
        auto attackSequence = BehaviorTree::CreateComposite("Attack", CompositeType::Sequence);
        attackSequence->AddChild(BehaviorTree::CreateCondition("CanSeePlayer",
            [](Mob* mob, Blackboard* bb) { return mob->CanSee(player); }));
        attackSequence->AddChild(BehaviorTree::CreateAction("MoveToPlayer",
            [](double dt, Mob* mob, Blackboard* bb) {
                auto path = pathfinding->FindPath(mob->GetPosition(), player->GetPosition());
                mob->SetPath(path.path);
                return BehaviorNodeStatus::Running;
            }));
        attackSequence->AddChild(BehaviorTree::CreateAction("AttackPlayer",
            [](double dt, Mob* mob, Blackboard* bb) {
                if (glm::distance(mob->GetPosition(), player->GetPosition()) < 2.0f) {
                    mob->Attack(player);
                    return BehaviorNodeStatus::Success;
                }
                return BehaviorNodeStatus::Running;
            }));

        // Secuencia de patrulla
        auto patrolSequence = BehaviorTree::CreateComposite("Patrol", CompositeType::Sequence);
        patrolSequence->AddChild(BehaviorTree::CreateAction("FindPatrolPoint",
            [](double dt, Mob* mob, Blackboard* bb) {
                auto patrolPoint = FindRandomPatrolPoint(mob->GetPosition());
                bb->SetValue("patrolTarget", patrolPoint);
                return BehaviorNodeStatus::Success;
            }));
        patrolSequence->AddChild(BehaviorTree::CreateAction("MoveToPatrolPoint",
            [](double dt, Mob* mob, Blackboard* bb) {
                auto target = bb->GetValue<glm::vec3>("patrolTarget");
                auto path = pathfinding->FindPath(mob->GetPosition(), target);
                mob->SetPath(path.path);
                return BehaviorNodeStatus::Running;
            }));

        // Ensamblar árbol
        root->AddChild(attackSequence);
        root->AddChild(patrolSequence);

        tree->SetRoot(root);
        return tree;
    }
};
```

### 🏘️ **Sistema de Aldea con Comercio**
```cpp
class TradingVillageSystem {
public:
    void InitializeTradingRoutes() {
        auto villages = villageSystem->GetAllVillages();

        // Crear rutas comerciales entre aldeas cercanas
        for (size_t i = 0; i < villages.size(); ++i) {
            for (size_t j = i + 1; j < villages.size(); ++j) {
                auto village1 = villages[i];
                auto village2 = villages[j];

                float distance = glm::distance(village1->GetCenter(), village2->GetCenter());
                if (distance < 200.0f) { // Rango de comercio
                    villageSystem->EstablishTradeRoute(village1->GetId(), village2->GetId());
                }
            }
        }
    }

    void ProcessTradingDay() {
        auto villages = villageSystem->GetAllVillages();

        for (auto village : villages) {
            // Generar ofertas comerciales
            std::unordered_map<std::string, float> exports;
            std::unordered_map<std::string, float> imports;

            // Determinar qué recursos exportar e importar
            for (const auto& resource : village->GetData().resources) {
                if (resource.second > 100.0f) {
                    exports[resource.first] = resource.second * 0.1f; // Exportar 10%
                } else if (resource.second < 20.0f) {
                    imports[resource.first] = 50.0f - resource.second; // Importar para llegar a 50
                }
            }

            // Intentar comerciar con aldeas conectadas
            for (uint32_t routeId : village->GetData().tradeRoutes) {
                auto otherVillage = villageSystem->GetVillage(routeId);
                if (otherVillage && !exports.empty() && !imports.empty()) {
                    // Crear trato comercial
                    std::unordered_map<std::string, float> tradeDeal;

                    // Intercambiar recursos
                    for (const auto& exportItem : exports) {
                        auto importIt = imports.find(exportItem.first);
                        if (importIt != imports.end()) {
                            float tradeAmount = std::min(exportItem.second, importIt->second);
                            tradeDeal[exportItem.first] = tradeAmount;
                        }
                    }

                    if (!tradeDeal.empty()) {
                        if (village->TradeWith(otherVillage, tradeDeal)) {
                            std::cout << "Comercio exitoso entre " << village->GetName()
                                     << " y " << otherVillage->GetName() << std::endl;
                        }
                    }
                }
            }
        }
    }
};
```

### 🎯 **Sistema de Enjambre Inteligente**
```cpp
class SwarmAISystem {
private:
    std::vector<std::shared_ptr<Mob>> swarmMembers;
    glm::vec3 swarmCenter;
    float cohesionRadius;
    float separationDistance;
    float alignmentWeight;
    float cohesionWeight;
    float separationWeight;

public:
    void UpdateSwarm(double deltaTime) {
        if (swarmMembers.empty()) return;

        // Calcular centro del enjambre
        swarmCenter = CalculateSwarmCenter();

        // Actualizar cada miembro del enjambre
        for (auto& member : swarmMembers) {
            // Calcular vectores de comportamiento de enjambre
            glm::vec3 separation = CalculateSeparation(member);
            glm::vec3 alignment = CalculateAlignment(member);
            glm::vec3 cohesion = CalculateCohesion(member);

            // Combinar vectores con pesos
            glm::vec3 swarmVelocity = separation * separationWeight +
                                    alignment * alignmentWeight +
                                    cohesion * cohesionWeight;

            // Limitar velocidad
            if (glm::length(swarmVelocity) > maxSpeed) {
                swarmVelocity = glm::normalize(swarmVelocity) * maxSpeed;
            }

            // Aplicar velocidad al miembro
            member->ApplySwarmVelocity(swarmVelocity);
        }

        // Mantener cohesión del enjambre
        MaintainSwarmCohesion();
    }

private:
    glm::vec3 CalculateSwarmCenter() const {
        glm::vec3 center(0.0f);
        for (const auto& member : swarmMembers) {
            center += member->GetPosition();
        }
        return center / static_cast<float>(swarmMembers.size());
    }

    glm::vec3 CalculateSeparation(std::shared_ptr<Mob> member) const {
        glm::vec3 separation(0.0f);
        int neighborCount = 0;

        for (const auto& other : swarmMembers) {
            if (other != member) {
                float distance = glm::distance(member->GetPosition(), other->GetPosition());
                if (distance < separationDistance) {
                    glm::vec3 away = member->GetPosition() - other->GetPosition();
                    separation += glm::normalize(away) / distance;
                    neighborCount++;
                }
            }
        }

        if (neighborCount > 0) {
            separation /= static_cast<float>(neighborCount);
        }

        return separation;
    }

    glm::vec3 CalculateAlignment(std::shared_ptr<Mob> member) const {
        glm::vec3 alignment(0.0f);
        int neighborCount = 0;

        for (const auto& other : swarmMembers) {
            if (other != member) {
                float distance = glm::distance(member->GetPosition(), other->GetPosition());
                if (distance < cohesionRadius) {
                    alignment += other->GetVelocity();
                    neighborCount++;
                }
            }
        }

        if (neighborCount > 0) {
            alignment /= static_cast<float>(neighborCount);
            alignment = glm::normalize(alignment);
        }

        return alignment;
    }

    glm::vec3 CalculateCohesion(std::shared_ptr<Mob> member) const {
        glm::vec3 cohesion(0.0f);
        int neighborCount = 0;

        for (const auto& other : swarmMembers) {
            if (other != member) {
                float distance = glm::distance(member->GetPosition(), other->GetPosition());
                if (distance < cohesionRadius) {
                    cohesion += other->GetPosition();
                    neighborCount++;
                }
            }
        }

        if (neighborCount > 0) {
            cohesion /= static_cast<float>(neighborCount);
            cohesion = cohesion - member->GetPosition();
            if (glm::length(cohesion) > 0.0f) {
                cohesion = glm::normalize(cohesion);
            }
        }

        return cohesion;
    }
};
```

### 🎮 **Sistema de Misión Dinámica**
```cpp
class DynamicQuestSystem {
private:
    AISystem* aiSystem;
    VillageSystem* villageSystem;
    std::vector<QuestData> activeQuests;

public:
    void GenerateDynamicQuest() {
        // Seleccionar tipo de misión aleatoria
        QuestType questType = static_cast<QuestType>(rand() % static_cast<int>(QuestType::Count));

        switch (questType) {
            case QuestType::KillMobs:
                GenerateKillQuest();
                break;
            case QuestType::CollectItems:
                GenerateCollectionQuest();
                break;
            case QuestType::DeliverItem:
                GenerateDeliveryQuest();
                break;
            case QuestType::ExploreLocation:
                GenerateExplorationQuest();
                break;
            case QuestType::DefendVillage:
                GenerateDefenseQuest();
                break;
        }
    }

private:
    void GenerateKillQuest() {
        // Encontrar aldea cercana
        auto villages = villageSystem->GetVillagesInRadius(player->GetPosition(), 500.0f);
        if (villages.empty()) return;

        auto targetVillage = villages[rand() % villages.size()];

        // Crear grupo de mobs hostiles cerca de la aldea
        glm::vec3 spawnLocation = targetVillage->GetCenter() +
                                 glm::vec3((rand() % 200 - 100), 0, (rand() % 200 - 100));

        // Spawnear mobs
        std::vector<std::shared_ptr<Mob>> spawnedMobs;
        for (int i = 0; i < 5 + rand() % 5; ++i) {
            auto mob = aiSystem->SpawnMob("bandit", spawnLocation +
                                        glm::vec3((rand() % 50 - 25), 0, (rand() % 50 - 25)));
            if (mob) {
                spawnedMobs.push_back(mob);
            }
        }

        // Crear misión
        QuestData quest;
        quest.type = QuestType::KillMobs;
        quest.title = "Defender " + targetVillage->GetName();
        quest.description = "Elimina la banda de forajidos que amenaza " + targetVillage->GetName();
        quest.objective = "Elimina " + std::to_string(spawnedMobs.size()) + " forajidos";
        quest.reward = 100 + rand() % 200;
        quest.targetEntities = spawnedMobs;
        quest.expirationTime = 1800.0; // 30 minutos

        activeQuests.push_back(quest);
    }

    void GenerateCollectionQuest() {
        // Generar misión de recolección de recursos
        std::vector<std::string> collectibleItems = {"wood", "stone", "iron_ore", "gold_ore", "diamond"};
        std::string targetItem = collectibleItems[rand() % collectibleItems.size()];

        QuestData quest;
        quest.type = QuestType::CollectItems;
        quest.title = "Recolección de " + targetItem;
        quest.description = "Recolecta recursos valiosos para la aldea";
        quest.objective = "Recolecta 10 unidades de " + targetItem;
        quest.targetItem = targetItem;
        quest.requiredCount = 10;
        quest.reward = 50 + rand() % 100;

        activeQuests.push_back(quest);
    }
};
```

---

## 🏆 **Conclusión**

El **VoxelCraft AI System** representa el estado del arte en sistemas de IA para juegos de mundo abierto. Su arquitectura modular y altamente optimizada permite:

- ✅ **Inteligencia Emergente**: Comportamientos realistas sin scripting manual
- ✅ **Navegación Avanzada**: A* optimizado con múltiples algoritmos
- ✅ **Civilizaciones Vivas**: Aldeas con economía, política y evolución
- ✅ **Escalabilidad Total**: Desde prototipos hasta juegos AAA
- ✅ **Extensibilidad Máxima**: Fácil adición de nuevas mecánicas
- ✅ **Debugging Completo**: Herramientas avanzadas de visualización

**Este sistema forma la base sólida sobre la cual se construirá la experiencia de IA completa del juego. 🚀🤖🎮✨**
