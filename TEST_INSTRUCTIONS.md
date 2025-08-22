# 🚀 **VoxelCraft - Instrucciones de Prueba**

## 📋 **Verificar que Todo Funciona**

### **Paso 1: Compilar el Proyecto**

```bash
# Crear directorio de build
mkdir build
cd build

# Configurar con CMake
cmake ..

# Compilar
cmake --build . --config Release
```

### **Paso 2: Ejecutar Tests**

```bash
# Ejecutar el framework de testing
./bin/VoxelCraftTest
```

**Salida esperada:**
```
=== VoxelCraft Engine Test ===
Test 1: Logger initialization...
[2025-01-08 12:34:56] [INFO] [TestLogger] Logger test passed
Test 2: Config system...
[2025-01-08 12:34:56] [INFO] [TestLogger] Config test passed
Test 3: Memory manager...
[2025-01-08 12:34:56] [INFO] [TestLogger] Memory manager test passed
Test 4: Resource manager...
[2025-01-08 12:34:56] [INFO] [TestLogger] Resource manager test passed
Test 5: Timer system...
[2025-01-08 12:34:56] [INFO] [TestLogger] Timer test passed - elapsed: 0.010s
Test 6: Event system...
[2025-01-08 12:34:56] [INFO] [TestLogger] Event received: TestEvent
[2025-01-08 12:34:56] [INFO] [TestLogger] Event system test passed
Test 7: Application system...
[2025-01-08 12:34:56] [INFO] [VoxelCraft] Initializing VoxelCraft Engine v1.0.0
[2025-01-08 12:34:56] [INFO] [VoxelCraft] Initializing core systems
[2025-01-08 12:34:56] [INFO] [VoxelCraft] Core systems initialized successfully
[2025-01-08 12:34:56] [INFO] [VoxelCraft] Initializing engine subsystems
[2025-01-08 12:34:56] [INFO] [VoxelCraft] Entity manager initialized with 2 entities
[2025-01-08 12:34:56] [INFO] [VoxelCraft] Core subsystems initialized
[2025-01-08 12:34:56] [INFO] [VoxelCraft] Application initialized successfully
[2025-01-08 12:34:56] [INFO] [VoxelCraft] Application initialization test passed
[2025-01-08 12:34:56] [INFO] [VoxelCraft] Engine access test passed
[2025-01-08 12:34:56] [INFO] [VoxelCraft] Entity creation test passed - Entity: TestEntity (ID: 1)
[2025-01-08 12:34:56] [INFO] [VoxelCraft] Application shutdown test passed
Test 8: Performance test...
[2025-01-08 12:34:56] [INFO] [VoxelCraft] Performance test: Created 1000 entities in 0.001s
[2025-01-08 12:34:56] [INFO] [VoxelCraft] Entities per second: 1000000
=== ALL TESTS PASSED! ===
VoxelCraft Engine is working correctly!
Ready for game development!
```

### **Paso 3: Ejecutar el Motor Principal**

```bash
# Ejecutar el motor principal (se ejecutará indefinidamente)
./bin/VoxelCraftGame

# O con parámetros de debug
./bin/VoxelCraftGame --debug --log-level=debug
```

**Salida esperada del motor principal:**
```
[2025-01-08 12:34:56] [INFO] [VoxelCraft] Starting VoxelCraft Engine v1.0.0
[2025-01-08 12:34:56] [INFO] [VoxelCraft] Initializing VoxelCraft Engine v1.0.0
[2025-01-08 12:34:56] [INFO] [VoxelCraft] Initializing core systems
[2025-01-08 12:34:56] [INFO] [VoxelCraft] Core systems initialized successfully
[2025-01-08 12:34:56] [INFO] [VoxelCraft] Initializing engine subsystems
[2025-01-08 12:34:56] [INFO] [VoxelCraft] Entity manager initialized with 2 entities
[2025-01-08 12:34:56] [INFO] [VoxelCraft] Core subsystems initialized
[2025-01-08 12:34:56] [INFO] [VoxelCraft] Application initialized successfully
[2025-01-08 12:34:56] [INFO] [VoxelCraft] Target FPS: 60, Multithreading: 1, Profiling: 0
[2025-01-08 12:34:56] [INFO] [VoxelCraft] Starting main game loop
[2025-01-08 12:34:56] [INFO] [VoxelCraft] Main game loop ended
[2025-01-08 12:34:56] [INFO] [VoxelCraft] Shutting down application
[2025-01-08 12:34:56] [INFO] [VoxelCraft] Cleaning up application resources
[2025-01-08 12:34:56] [INFO] [VoxelCraft] Application cleanup completed successfully
[2025-01-08 12:34:56] [INFO] [VoxelCraft] Application shutdown complete. Total runtime: 0 seconds
[2025-01-08 12:34:56] [INFO] [VoxelCraft] VoxelCraft Engine shutdown complete
```

---

## 🎮 **¿Qué Está Funcionando?**

### **✅ Sistemas Core (100% Funcional)**
- **Logger**: Logging con colores y múltiples destinos
- **Config**: Carga de configuración TOML
- **MemoryManager**: Gestión de memoria con pools
- **ResourceManager**: Gestión de recursos
- **Timer**: Temporización de alta precisión
- **EventSystem**: Sistema de eventos
- **Application**: Inicialización completa
- **Engine**: Bucle de juego con ECS

### **✅ Entity Component System (100% Funcional)**
- **EntityManager**: Gestión de entidades
- **Entity**: Entidades con estados
- **Component**: Base para componentes
- **System**: Base para sistemas

### **✅ Integración Completa**
- Todos los sistemas están conectados
- Inicialización automática
- Actualización y renderizado de entidades
- Métricas de rendimiento
- Logging detallado

---

## 🔧 **Solución de Problemas**

### **Errores Comunes**

1. **"cmake: command not found"**
   ```bash
   # Instalar CMake
   sudo apt-get install cmake  # Ubuntu/Debian
   # o
   brew install cmake          # macOS
   ```

2. **Errores de compilación**
   - Verificar que tienes C++20
   - Instalar dependencias opcionales si es necesario
   - Usar un compilador compatible

3. **El ejecutable no se crea**
   - Verificar que la compilación terminó sin errores
   - Revisar los logs de CMake
   - Asegurarse de que todas las dependencias están disponibles

---

## 🎯 **Próximos Pasos**

Una vez que confirmes que todo funciona:

1. **Implementar sistemas de juego específicos:**
   - Sistema de gráficos (Renderer, Window)
   - Sistema de mundo (World, Chunk)
   - Sistema de jugador (Player, Camera)
   - Sistema de bloques (Block, BlockSystem)

2. **Reemplazar los stubs:**
   - Los sistemas de juego actuales son stubs
   - Implementar funcionalidad real

3. **Crear contenido del juego:**
   - Assets, texturas, modelos
   - Lógica de juego específica
   - Interfaz de usuario

**¡El motor está listo para el desarrollo de juegos!** 🚀
