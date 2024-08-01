# ToyEngine

The ToyEngine is a personal project for educational purposes. Here I learn and experiment with Vulkan API, modern rendering techniques, and general programming techniques.
The first milestone is to develop an abstract renderer layer and hide Vulkan complexity.

Currently, the renderer abstraction is far from ready for use in some high-level contexts.

The project is developed in my spare time, and currently, the following features are being in development:
 - preparing the Vulkan rendering interface to be capable of executing in a concurrent environment. Later on, it will be run in a job system like a multithreaded environment.
 - appropriate asset import pipeline (I will restrict it for mesh-only data at first), currently it uses some build-in dirty solution.

## Showcase
Even though it is far from being finished, I have some renderings to share:

|![First Mesh](Docs/FirstMesh.png)|
|:-|
|*The first visible mesh on a screen uses vertex fetching techniques and a mesh has a clustered structure.*|


|![Dense Mesh](Docs/DenseMesh.png)|
|:-|
|*Same as before but with a model with over 2 million vertices.*|


|![TLAS](Docs/Tlas.png)|
|:-|
|*Here I played a little bit with an acceleration structure building for Raytracing. This screenshot is taken from the NSight graphics debugging tool.*|


|![New Sponza](Docs/NewSponza.png)|
|:-|
|*New Sponza Scene. Same technics as before with a different lit shader and a more complex scene.*|


## Requisitions
- Vulkan 1.3
- vcpkg
- cmake version > 3.23
- msvc compiler
