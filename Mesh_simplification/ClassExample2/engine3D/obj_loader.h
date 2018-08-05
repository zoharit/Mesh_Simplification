
#ifndef OBJ_LOADER_H_INCLUDED
#define OBJ_LOADER_H_INCLUDED

#include <glm/glm.hpp>
#include <vector>
#include <list>
#include <string>
#include <map>

struct OBJIndex
{
	unsigned int vertexIndex;
	unsigned int uvIndex;
	unsigned int normalIndex;
	unsigned int edgeIndex;
	std::list<OBJIndex>::iterator faceIndex;
	unsigned int tmpIndx;
	bool isEdgeUpdated;
	bool operator<(const OBJIndex& r) const { return vertexIndex < r.vertexIndex; }
	bool operator==(const OBJIndex& r) const { return vertexIndex == r.vertexIndex; }
};

struct IndexedModel
{

	std::vector<glm::vec3> positions;
	std::vector<glm::vec2> texCoords;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec3> colors;
	std::vector<unsigned int> indices;
};

struct Edge
{
	std::pair <OBJIndex, OBJIndex> edge;
	glm::vec3 newpos;
	double error = 0;
	glm::mat4 Q = glm::mat4(0);
	bool operator==(const Edge& r) const { return (edge.first.vertexIndex == r.edge.first.vertexIndex && edge.second.vertexIndex == r.edge.second.vertexIndex) || (edge.second.vertexIndex == r.edge.first.vertexIndex && edge.first.vertexIndex == r.edge.second.vertexIndex); }


	Edge(OBJIndex i, OBJIndex j, double error) {
		if (i.vertexIndex < j.vertexIndex) {
			this->edge = std::make_pair(i, j);
		}
		else {
			this->edge = std::make_pair(j, i);
		}
		this->error = error;
	}

};

class OBJModel
{
public:

	bool globalchange = true;
	std::list<OBJIndex> OBJIndices;
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec3> colors;
	std::vector<glm::mat4> Q_errors;
	std::vector<Edge> edges;											
	std::multimap<int, int> vertex_map;									
	void calcEdge(std::list<OBJIndex> faces);							
	void start(int maxFaces);												
	void OBJModel::calcFaces(std::list<OBJIndex> faces, int firstEdgeInd, int secondEdgeInd);			
	void OBJModel::printFaces(std::list<OBJIndex> faces);
	void setNeighbor();
	void printVector(std::vector<Edge> vec);
	bool hasUVs;

	glm::vec3 OBJModel::Calcpos(Edge e, glm::mat4 m);


	bool hasNormals;
	std::vector<Edge> OBJModel::remove_duplicate(std::vector<Edge> rem);
	OBJModel(const std::string& fileName,int d,int max_faces);
	std::vector<Edge> OBJModel::remove_duplicate3(std::vector<Edge> toRemove, int v1index, int v2index);
	glm::mat4 OBJModel::vertex_error(int vertexIndex);
	void OBJModel::edge_Error(struct Edge& e);
	bool OBJModel::check_triangl(int second, int third);
	IndexedModel ToIndexedModel();


	// comperator for edges


private:
	unsigned int FindLastVertexIndex(const std::vector<OBJIndex*>& indexLookup, const OBJIndex* currentIndex, const IndexedModel& result);
	void CreateOBJFace(const std::string& line);

	glm::vec2 ParseOBJVec2(const std::string& line);
	glm::vec3 ParseOBJVec3(const std::string& line);
	OBJIndex ParseOBJIndex(const std::string& token, bool* hasUVs, bool* hasNormals);
	void CalcNormals();

};


#endif // OBJ_LOADER_H_INCLUDED

