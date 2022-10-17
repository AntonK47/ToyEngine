#pragma once
#include <Scene.h>

class MeshletBuilder
{
public:
	void process(const toy::core::scene::Mesh& mesh, toy::core::scene::RuntimeMesh& processedMesh);
};

