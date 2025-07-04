#include "StaticMesh.h"

StaticMesh::StaticMesh()
{

}

StaticMesh::~StaticMesh()
{

}

void StaticMesh::AddPoint(float& x, float& y, float& z)
{
	m_points.push_back(FVector3(x, y, z));
}

void StaticMesh::AddTriangle(int& v1, int& v2, int& v3)
{
	m_triangles.push_back(v1);
	m_triangles.push_back(v2);
	m_triangles.push_back(v3);
}