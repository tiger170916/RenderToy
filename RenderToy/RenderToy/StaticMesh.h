#pragma once

#include "Includes.h"
#include "Vectors.h"

class StaticMesh
{
private:
	std::vector<FVector3> m_points;

	std::vector<int> m_triangles;

public:
	StaticMesh();

	~StaticMesh();

	void AddPoint(float& x, float& y, float& z);

	void AddTriangle(int& v1, int& v2, int& v3);
};