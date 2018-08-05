#pragma once
#include "MovableGLM.h"
#include "mesh.h"
#include "lineMesh.h"
#include "texture.h"
#include <list>

class Shape : public MovableGLM
{
	Mesh *mesh;
	LineMesh *lineMesh;
	Texture *tex;
	bool isCopy;
	int maxfaces;
public:
	enum{triangles,lines};
	Shape(const Shape& shape);
	Shape(const std::string& fileName,bool d,int maxfaces);
	Shape(const std::string& fileName,const std::string& textureFileName,bool d,int max_faces);
	Shape(Vertex* vertices, unsigned int numVertices, unsigned int* indices, unsigned int numIndices);
	Shape(LineVertex* vertices, unsigned int numVertices, unsigned int* indices, unsigned int numIndices);
	Shape(Vertex* vertices, unsigned int numVertices, unsigned int* indices, unsigned int numIndices, const std::string& textureFileName);
	void addMesh(Vertex* vertices, unsigned int numVertices,unsigned int* indices, unsigned int numIndices, const std::string& textureFileName);
	void addMesh(Vertex* vertices, unsigned int numVertices,unsigned int* indices, unsigned int numIndices);
	void addLineMesh(LineVertex* vertices, unsigned int numVertices,unsigned int* indices, unsigned int numIndices);
	void draw(int mode);
	void getPoints(std::list<glm::vec3> &pList);
	virtual ~Shape(void);
};

