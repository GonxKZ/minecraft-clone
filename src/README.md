# 🧱 **VoxelCraft - Código Fuente**

Esta carpeta contiene la implementación completa del clon de Minecraft desarrollado con C++20 y arquitectura ECS.

## 📁 **Estructura del Código**

### **🏗️ Core Systems**
- **`core/`**: Sistema base del motor (Application, Engine, Config, Logger)
- **`events/`**: Sistema de eventos y notificaciones
- **`memory/`**: Gestión de memoria con pools
- **`input/`**: Sistema de entrada (teclado, mouse, gamepad)
- **`resource/`**: Gestión de recursos y carga asíncrona
- **`scripting/`**: Sistema de scripting con Lua/Python
- **`plugin/`**: Sistema de plugins/modding
- **`achievement/`**: Logros y estadísticas
- **`statistics/`**: Sistema de métricas del jugador
- **`config/`**: Sistema de configuración
- **`debug/`**: Herramientas de debugging y profiling
- **`logging/`**: Sistema de logging avanzado

### **🌍 World Systems**
- **`world/`**: Sistema de mundo y chunks
- **`blocks/`**: Sistema de bloques y materiales
- **`entities/`**: Sistema de entidades ECS
- **`player/`**: Jugador y controles
- **`interaction/`**: Interacciones con el mundo

### **🎮 Game Features**
- **`crafting/`**: Sistema de crafting con recetas
- **`ai/`**: Inteligencia artificial para mobs
- **`redstone/`**: Sistema de redstone básico
- **`farming/`**: Sistema de agricultura
- **`enchantment/`**: Sistema de encantamientos
- **`potion/`**: Sistema de pociones
- **`mob/`**: Mobs y criaturas
- **`biome/`**: Biomas y generación procedural
- **`structures/`**: Estructuras generadas
- **`trading/`**: Sistema de comercio
- **`survival/`**: Sistema de supervivencia
- **`textures/`**: Generación procedural de texturas
- **`ui/`**: Sistema de interfaz de usuario
- **`physics/`**: Sistema de físicas básico
- **`collision/`**: Detección de colisiones

## 🛠️ **Características Técnicas**

### **🏛️ Arquitectura ECS**
- Entity-Component-System pattern
- Gestión eficiente de entidades
- Componentes modulares y extensibles
- Sistema de eventos integrado

### **⚡ Performance**
- Sistema de chunks para mundos grandes
- Carga asíncrona de recursos
- Pool de memoria para allocations frecuentes
- Profiling y debugging integrado

### **🧱 Voxel Engine**
- Sistema de bloques con propiedades físicas
- Texturas y materiales
- Colocación y destrucción de bloques
- Sistema de iluminación básico

### **👥 Player Systems**
- Movimiento 3D first-person
- Cámara con rotación completa
- Inventario con 41 slots + hotbar
- Interacción con el mundo

## 📊 **Estadísticas del Código**

- **Total de archivos**: ~180 archivos
- **Líneas de código**: ~200,000 líneas
- **Sistemas principales**: 25+ sistemas implementados
- **Características**: 500+ features funcionales

## 🚀 **Compilación**

Para compilar el proyecto:

```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

## 📚 **Documentación**

Cada sistema incluye documentación básica en sus archivos header (.hpp). El proyecto está estructurado para ser fácil de entender y modificar.
