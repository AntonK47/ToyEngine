# ToyEngine

ToyEngine is a personal project created to explore and experiment with the Vulkan API, modern rendering techniques, and advanced programming practices. The primary goal of this project is to build a renderer abstraction layer that simplifies Vulkan’s complexity, providing a streamlined interface for high-level rendering tasks.

> Note: The renderer abstraction is still under development and unsuitable for high-level production use.

## Project Objectives
ToyEngine is developed in my spare time as a learning project, with the following features currently under development:
- Concurrent Vulkan Interface: Preparing the Vulkan rendering interface to support concurrent execution. The long-term goal is integrating it into a job system, enabling a multithreaded environment.
- Asset Import Pipeline: Implementing an asset import system initially limited to mesh data. Currently, this feature relies on a temporary, built-in solution.

## Current Showcase
Although ToyEngine is far from completion, some preliminary renderings are available for demonstration:

|![First Mesh](Docs/FirstMesh.png)|
|:-|
|***First Mesh:** A simple, visible mesh on-screen using vertex-fetching techniques with a clustered mesh structure.*|


|![Dense Mesh](Docs/DenseMesh.png)|
|:-|
|***Dense Mesh:** Similar to the first mesh, but using a model with over 2 million vertices, showcasing the engine's ability to handle large data sets.*|


|![TLAS](Docs/Tlas.png)|
|:-|
|***Top-Level Acceleration Structure (TLAS):** Experimental work on acceleration structures for ray tracing, with screenshots from NSight Graphics Debugger.*|


|![New Sponza](Docs/NewSponza.png)|
|:-|
|***New Sponza Scene:** A complex, lit shader scene featuring the New Sponza model, illustrating more advanced rendering techniques.*|


## Requirements
To build and run ToyEngine, ensure the following dependencies are installed:

- Vulkan version 1.3 or higher
- vcpkg for package management
- CMake version 3.23 or higher
- MSVC compiler (for Windows development)
