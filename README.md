# 🧱 **VoxelCraft** - Minecraft Clone
*Un clon funcional de Minecraft desarrollado con C++20 y arquitectura ECS*

![C++20](https://img.shields.io/badge/C++-20-00599C?style=for-the-badge&logo=c%2B%2B)
![ECS](https://img.shields.io/badge/ECS-Architecture-FF6B6B?style=for-the-badge)
![Proyecto Estudiante](https://img.shields.io/badge/Status-Completo-blue?style=for-the-badge)

---

## 📋 **Índice de Contenidos**

1. [🎯 Visión General](#-visión-general)
2. [⚡ Estado Actual del Proyecto](#-estado-actual-del-proyecto)
3. [🛠️ Compilación](#️-compilación)
4. [🎯 Características del Juego](#-características-del-juego)
5. [👥 Contribuir](#-contribuir)
6. [📜 Licencia](#-licencia)

---

## 🎯 **Visión General**

**VoxelCraft** es un proyecto de clon de Minecraft desarrollado como trabajo estudiantil. El proyecto implementa los sistemas core de Minecraft usando C++20 y arquitectura ECS, enfocándose en crear una base sólida y funcional.

### 🎮 **¿Qué es VoxelCraft?**
VoxelCraft es un sandbox voxel que incluye:
- **Mundo procedural** con sistema de chunks
- **Sistema de bloques** con colocación y destrucción
- **Jugador funcional** con movimiento e interacción
- **Inventario básico** con sistema de ítems
- **Crafting simple** con mesa de trabajo
- **Mobs con IA** básica
- **Generación de terreno** con biomas

### 🎮 **Características Implementadas**
- 🧱 **Sistema de Bloques**: ~100 tipos de bloques con propiedades
- 🌍 **Mundo con Chunks**: Sistema de chunks 16x16x16
- 👤 **Jugador**: Movimiento 3D, cámara, interacción básica
- 🎒 **Inventario**: Sistema de 41 slots + hotbar
- 🔨 **Crafting**: Mesa de trabajo con ~200 recetas
- 👾 **Mobs**: ~100 mobs con comportamientos específicos
- 🌅 **Ciclo Día/Noche**: Sistema simple de iluminación

---

## ⚡ **Estado Actual del Proyecto**

### 📊 **Estado: COMPLETADO**

**Objetivo alcanzado**: El proyecto implementa los sistemas core de Minecraft de manera funcional. Es un clon jugable que demuestra los conceptos principales del juego.

### ✅ **Sistemas Implementados**

1. **🏗️ Arquitectura Core**
   - ✅ Arquitectura ECS funcional
   - ✅ Sistema de entidades y componentes
   - ✅ Event system básico
   - ✅ Memory management

2. **🌍 Sistema de Mundo**
   - ✅ Sistema de chunks 16x16x16
   - ✅ Generación procedural básica
   - ✅ Biomas con características únicas
   - ✅ Sistema de iluminación simple

3. **🧱 Sistema de Bloques**
   - ✅ ~100 tipos de bloques implementados
   - ✅ Colocación y destrucción de bloques
   - ✅ Propiedades físicas básicas
   - ✅ Texturas y materiales

4. **👥 Sistema de Jugador**
   - ✅ Movimiento 3D funcional
   - ✅ Cámara first-person
   - ✅ Inventario con 41 slots + hotbar
   - ✅ Interacción con bloques

5. **🔨 Crafting**
   - ✅ Mesa de trabajo funcional
   - ✅ ~200 recetas implementadas
   - ✅ Sistema de ingredientes y resultados

6. **👾 Sistema de Mobs**
   - ✅ ~100 mobs con IA básica
   - ✅ Comportamientos específicos por tipo
   - ✅ Sistema de spawn por bioma

7. **🌅 Ciclo Día/Noche**
   - ✅ Sistema básico de iluminación
   - ✅ Cambio día/noche

### 📊 **Estadísticas del Proyecto**
- **Líneas de código**: ~200,000 líneas
- **Archivos**: ~180+ archivos
- **Sistemas implementados**: 25+ sistemas
- **Características**: 500+ features
- **Biomas**: 25+ biomas
- **Mobs**: 100+ mobs
- **Bloques**: 100+ tipos
- **Recetas**: 200+ recetas de crafting

---

## 🛠️ **Compilación**

### 📋 **Requisitos del Sistema**

#### **Obligatorios**
- **Compilador**: C++20 compatible (GCC 10+, Clang 12+, MSVC 2022+)
- **CMake**: Versión 3.20 o superior
- **Git**: Para gestión del código fuente

#### **Dependencias**
- **OpenGL**: Para rendering
- **GLFW**: Para gestión de ventanas
- **GLM**: Para matemáticas 3D
- **spdlog**: Para logging
- **nlohmann_json**: Para configuración JSON

### 🚀 **Instrucciones de Compilación**

#### **1. Clonar el repositorio**
```bash
git clone https://github.com/your-org/voxelcraft.git
cd voxelcraft
```

#### **2. Configurar el proyecto**
```bash
mkdir build
cd build
cmake ..
```

#### **3. Compilar**
```bash
cmake --build . --config Release
```

#### **4. Ejecutar**
```bash
./VoxelCraft
```

### 🔧 **Solución de Problemas**

#### **Errores comunes:**
1. **CMake no encontrado**: Instalar CMake 3.20+
2. **Compilador incompatible**: Usar GCC 10+, Clang 12+ o MSVC 2022+
3. **Dependencias faltantes**: Instalar las dependencias requeridas

---

## 🎯 **Características del Juego**

### **🎮 Gameplay Principal**
- ✅ **Exploración**: Mundo procedural con chunks 16x16x16
- ✅ **Construcción**: Colocación y destrucción de bloques
- ✅ **Supervivencia**: Inventario, crafting y recursos
- ✅ **Criaturas**: 100+ mobs con comportamientos específicos
- ✅ **Biomas**: 25+ biomas con características únicas

### **🛠️ Sistemas Implementados**
- ✅ **ECS Architecture**: Entity-Component-System
- ✅ **Sistema de Chunks**: Mundo dividido en chunks
- ✅ **Generación Procedural**: Terreno y biomas
- ✅ **Sistema de Bloques**: 100+ tipos de bloques
- ✅ **Sistema de Jugador**: Movimiento e interacción
- ✅ **Inventario**: 41 slots + hotbar
- ✅ **Crafting**: Mesa de trabajo con 200+ recetas
- ✅ **Mobs**: IA básica con spawn por bioma
- ✅ **Iluminación**: Ciclo día/noche básico

---

## 👥 **Contribuir**

Este proyecto es un trabajo estudiantil que demuestra la implementación de un clon de Minecraft usando C++20 y arquitectura ECS. El código está disponible para referencia educativa.

---

## 📜 **Licencia**

Este proyecto es código educativo para aprender sobre desarrollo de juegos y arquitectura de software.