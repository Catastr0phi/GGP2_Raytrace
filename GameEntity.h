#pragma once

#include "Transform.h"
#include "Mesh.h"
#include "Material.h"
#include <memory>

class GameEntity
{
private:
	std::shared_ptr<Transform> transform;
	std::shared_ptr<Mesh> mesh;
	std::shared_ptr<Material> material;

public:
	GameEntity(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material);
	std::shared_ptr<Mesh> GetMesh();
	std::shared_ptr<Material> GetMaterial();
	void SetMesh(std::shared_ptr<Mesh> newMesh);
	void setMaterial(std::shared_ptr<Material> newMaterial);
	std::shared_ptr<Transform> GetTransform();
};