
#include "obj_loader.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <set>







static bool CompareOBJIndexPtr(const OBJIndex* a, const OBJIndex* b);
static inline unsigned int FindNextChar(unsigned int start, const char* str, unsigned int length, char token);
static inline unsigned int ParseOBJIndexValue(const std::string& token, unsigned int start, unsigned int end);
static inline float ParseOBJFloatValue(const std::string& token, unsigned int start, unsigned int end);
static inline std::vector<std::string> SplitString(const std::string &s, char delim);




bool comp1(const struct Edge& e1, const struct Edge& e2) {
	int firstx = e1.edge.first.vertexIndex;
	int firsty = e1.edge.second.vertexIndex;
	int secondx = e2.edge.first.vertexIndex;
	int secondy = e2.edge.second.vertexIndex;
	int tmp = 0;

	if (secondx < firstx)
		return true;
	if (secondx == firstx)
	{
		return (secondy <firsty);
	}

	return false;
}


bool  comp(const struct Edge& e1, const struct Edge& e2)  {
	if (e2.error < e1.error)
		return true;
	return false;
}



OBJModel::OBJModel(const std::string& fileName,int d,int max_faces)
{
	hasUVs = false;
	hasNormals = false;
	std::ifstream file;
	file.open(fileName.c_str());

	int bla1;
	std::string line;
	if (file.is_open())
	{
		while (file.good())
		{
			getline(file, line);

			unsigned int lineLength = line.length();

			if (lineLength < 2)
				continue;

			const char* lineCStr = line.c_str();

			switch (lineCStr[0])
			{
			case 'v':
				if (lineCStr[1] == 't')
					this->uvs.push_back(ParseOBJVec2(line));
				else if (lineCStr[1] == 'n')
				{
					this->normals.push_back(ParseOBJVec3(line));
					this->colors.push_back((normals.back() + glm::vec3(1, 1, 1))*0.5f);
				}
				else if (lineCStr[1] == ' ' || lineCStr[1] == '\t')
					this->vertices.push_back(ParseOBJVec3(line));
				break;
			case 'f':
				CreateOBJFace(line);
				break;
			default: break;
			};
		}
		if (d) {
			calcEdge(OBJIndices);
			setNeighbor();
			for (int i = 0; i < vertices.size(); i++)
			{
				this->Q_errors.push_back(vertex_error(i));
			}

			for (int i = 0; i < edges.size(); i++) {
				edge_Error(edges[i]);
			}

			std::make_heap(edges.begin(), edges.end(), comp);
			start(max_faces);
			bla1 = OBJIndices.size();

			std::cout << "size2 " << bla1 << std::endl;
		}
		// calc normals
		//calcNormals();
	}
	else
	{
		std::cerr << "Unable to load mesh: " << fileName << std::endl;
	}

	
}
void OBJModel::printFaces(std::list<OBJIndex> faces) {
	int counter = 0;
	OBJIndex first;
	OBJIndex second;
	OBJIndex third;
	for (std::list<OBJIndex>::iterator it = faces.begin(); it != faces.end(); it++) {

		if (counter == 0) {
			first = *it;
		}
		if (counter == 1) {
			second = *it;
		}
		if (counter == 2) {
			third = *it;
			counter = 0;
			std::cout << "first: " << first.vertexIndex << " second: " << second.vertexIndex << " third: " << third.vertexIndex << std::endl;

			continue;

		}
		counter++;
	}

}

void OBJModel::calcFaces(std::list<OBJIndex> faces, int v1index, int v2index) {
	int counter = 0;
	OBJIndex first;
	OBJIndex second;
	OBJIndex third;
	bool goback = false;
	std::list<OBJIndex>::iterator it2 = OBJIndices.begin();
	std::list<OBJIndex>::iterator it3 = OBJIndices.begin();
	std::list<OBJIndex>::iterator it4 = OBJIndices.begin();

	for (std::list<OBJIndex>::iterator it = OBJIndices.begin(); it != OBJIndices.end(); ) {
		first = *it;
		it2 = it;
		it++;
		it3 = it;
		second = *it;
		it++;
		it4 = it;
		third = *it;
		it++;
		if ((v1index == first.vertexIndex || v1index == second.vertexIndex || v1index == third.vertexIndex) && (v2index == first.vertexIndex || v2index == second.vertexIndex || v2index == third.vertexIndex)) {
			OBJIndices.erase(it2);
			OBJIndices.erase(it3);
			OBJIndices.erase(it4);
		}
	}
	for (std::list<OBJIndex>::iterator it5 = OBJIndices.begin(); it5 != OBJIndices.end(); it5++) {
		if ((*it5).vertexIndex == v2index)
			(*it5).vertexIndex = v1index;
	}


}




std::vector<Edge> OBJModel::remove_duplicate(std::vector<Edge> rem) {
	std::cout << "removeDups start.." << std::endl;
	std::vector<Edge>::iterator it;
	std::sort(rem.begin(), rem.end(), comp1);
	it = std::unique(rem.begin(), rem.end());
	rem.erase(it, rem.end());
	std::cout << "removeDups end" << std::endl;
	return rem;

}




void OBJModel::calcEdge(std::list<OBJIndex> faces) {
	int counter = 0;
	OBJIndex first;
	OBJIndex second;
	OBJIndex third;
	for (std::list<OBJIndex>::iterator it = faces.begin(); it !=
		faces.end(); it++) {

		if (counter == 0) {
			first = *it;
		}
		if (counter == 1) {
			second = *it;
		}
		if (counter == 2) {
			third = *it;
			counter = 0;
			struct Edge edge1(first, second, 0);
			struct Edge edge2(first, third, 0);
			struct Edge edge3(second, third, 0);


			edges.push_back(edge1);
			edges.push_back(edge2);
			edges.push_back(edge3);
			continue;
		}
		counter++;
	}
	edges = remove_duplicate(edges);

}


std::vector<Edge> OBJModel::remove_duplicate3(std::vector<Edge> toRemove, int v1index, int v2index) {
	//2 to1
	int* arr = new int[this->vertices.size()];
	
	for (int i = 0; i < this->vertices.size(); i++)
	{
		arr[i] = 0;
	}
	for (std::vector<Edge>::iterator it = toRemove.begin(); it != toRemove.end();)
	{
		if (it->edge.first.vertexIndex == v1index)
		{
			if (it->edge.second.vertexIndex == v1index)
			{
				arr[v1index]++;
				it = toRemove.erase(it);
				continue;
			}
			else//not v1,v1
			{
				int x = it->edge.second.vertexIndex;
				if (arr[x] == 0)
				{
					arr[x]++;
					it++;
					continue;
				}
				else {

					arr[x]++;
					it = toRemove.erase(it);
				}
			}
		}
		else if (it->edge.second.vertexIndex == v1index)
		{
			if (it->edge.first.vertexIndex == v1index)
			{
				arr[v1index]++;

				it = toRemove.erase(it);
				continue;
			}

			else//not v1,v1
			{
				int x = it->edge.first.vertexIndex;
				if (arr[x] == 0)
				{
					arr[x]++;
					it++;
					continue;
				}
				else {
					arr[x]++;
					it = toRemove.erase(it);
				}
			}
		}
		else
		{
			it++;
			continue;
		}
	}



	return toRemove;

}



void  OBJModel::setNeighbor() {
	std::cout << "setNeighbor start.." << std::endl;

	for (size_t i = 0; i < vertices.size(); i++) {
		for (size_t j = 0; j < edges.size(); j++) {
			if (i == edges[j].edge.first.vertexIndex)
				vertex_map.insert(std::pair<int, int>(i, edges[j].edge.second.vertexIndex));
			if (i == edges[j].edge.second.vertexIndex)
				vertex_map.insert(std::pair<int, int>(i, edges[j].edge.first.vertexIndex));
		}
	}
		std::cout << "setNeighbor end" << std::endl;
}

glm::mat4 OBJModel::vertex_error(int vertexIndex) {
	glm::mat4 qMat;
	std::pair <std::multimap<int, int>::iterator, std::multimap<int, int>::iterator> result = vertex_map.equal_range(vertexIndex);
	for (std::multimap<int, int>::iterator it = result.first; it != result.second; it++)
	{

		for (std::multimap<int, int>::iterator it2 = it; it2 != result.second; it2++)
		{
			if (it2->second != it->second) {
				if (check_triangl(it->second, it2->second)) {
					glm::vec3 n = glm::cross(vertices[it2->second] - vertices[vertexIndex], vertices[it->second] - vertices[vertexIndex]);
					if (glm::length(n) < 0.00005)
					{
					n = glm::vec3(0.0f, 0.0f, 0.0f);
					std::cout<<"zero normal\n\n\nzero normalzero normalzero normalzero normalzero normalzero normalzero normalzero normal\n\nzero normalzero normal" << std::endl;
					}
					else
					{
						n = glm::normalize(n);
					}
					glm::vec4 v_tag = glm::vec4(n, -(glm::dot(vertices[vertexIndex], n)));
					for (int i = 0; i < 4; i++) {
						for (int j = 0; j < 4; j++) {
							qMat[i][j] += v_tag[i] * v_tag[j];
						}
					}
				}
			}
		}
	}
	return qMat;
}


glm::vec4 getans(glm::mat4 m,glm::vec4 v) {
	for (int i = 0; i < 4; i++) {
		int j = 0;
		while (j < 4 && fabs(m[i][j]) <0.0005)
			j++;
		if (j == 4)
			continue;
		for (int k = 0; k < 4; k++) {
			if (k != i) {
				double rate = m[k][j] / m[i][j];
				for (int l = 0; l < 4; l++)
					m[k][l] -= m[i][l] * rate;
				v[k] -= v[i] * rate;
			}
		}
	}
	glm::vec4 X;
	for (int i = 0; i < 4; i++) {
		int j = 0;
		while (j < 4 && fabs(m[i][j]) < 0.0005)
			j++;
		if (j == 4)
			return glm::vec4(0, 0, 0, -1);
		X[i] = v[i] / m[i][j];
	}
//	X[3] = 1;
	return X;
}

glm::vec3 OBJModel::Calcpos(Edge e, glm::mat4 m)
{
	glm::vec3 mid = glm::vec3((vertices[e.edge.first.vertexIndex] + vertices[e.edge.second.vertexIndex]) / 2.0f);
	return mid;
	//if (fabs(glm::determinant(m) > 0.0001))
	//{
		m[3][0] = 0;
		m[3][1] = 0;
		m[3][2] = 0;
		m[3][3] = 1;
		glm::vec4 Y(0, 0, 0, 1);
		glm::vec4 ans = getans(m, Y);
		if (ans[3] > 0.0005)
		{
			return glm::vec3(ans[0], ans[1], ans[2]);
		}
		else
		{
			return mid;
		}

	//}

}

void OBJModel::edge_Error(struct Edge& e) {
	glm::mat4 Q;
		//Q = (vertex_error(e.edge.first.vertexIndex)+ vertex_error(e.edge.second.vertexIndex));
	 Q = (this->Q_errors.at(e.edge.first.vertexIndex) + this->Q_errors.at(e.edge.second.vertexIndex));
	glm::mat4 tmp1=glm::mat4(Q);
	glm::vec3 vec_div = Calcpos(e,Q);
	glm::vec4 X(vec_div.x, vec_div.y, vec_div.z, 1.0);

	glm::vec4 mid = glm::vec4((vertices[e.edge.first.vertexIndex] + vertices[e.edge.second.vertexIndex]) / 2.0f,1.0f);


		// glm::vec4((vertices[e.edge.first.vertexIndex] + vertices[e.edge.second.vertexIndex]) / 2.0f, 1.0);
	glm::vec4 temp2 = Q * mid;
	double err1 = glm::dot(mid, temp2);
	
	//	vec_div = glm::inverse(tmp1)*glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	e.newpos = vec_div;
	glm::vec4 temp = Q * X;
	e.error = glm::dot(X, temp);
	//std::cout << "errormid = " << err1 << "error bonus=   " << e.error << std::endl;
	e.Q = Q;
}


bool OBJModel::check_triangl(int second, int third) {
	std::pair <std::multimap<int, int>::iterator, std::multimap<int, int>::iterator> result = vertex_map.equal_range(second);
	for (std::multimap<int, int>::iterator it = result.first; it != result.second; it++) {
		if (third == it->second) {
			return true;
		}
	}
	return false;
}


void OBJModel::printVector(std::vector<Edge> vec)
{
	std::cout << "Start Print and edge size: " << edges.size() << std::endl;
	for (unsigned int i = 0; i < vec.size(); i++)
	{
		std::cout << "edge[" << i << "] = " << vec[i].edge.first.vertexIndex << "  " << vec[i].edge.second.vertexIndex << " Error: " << vec[i].error << std::endl;
	}
}


void OBJModel::CalcNormals()
{
	float *count = new float[normals.size()];
	for (int i = 0; i < normals.size(); i++)
	{
		count[i] = 0;
	}
	for (std::list<OBJIndex>::iterator it = OBJIndices.begin(); it !=
		OBJIndices.end(); it++)
	{
		int i0 = (*it).vertexIndex;
		(*it).normalIndex = i0;
		it++;
		int i1 = (*it).vertexIndex;
		(*it).normalIndex = i1;
		it++;
		int i2 = (*it).vertexIndex;
		(*it).normalIndex = i2;
		glm::vec3 v1 = vertices[i1] - vertices[i0];
		glm::vec3 v2 = vertices[i2] - vertices[i0];

		glm::vec3 normal = glm::normalize(glm::cross(v2, v1));

		if (count[i0] == 0)
			count[i0] = 1.0f;
		else
			count[i0] = count[i0] / (count[i0] + 1);

		if (count[i1] == 0)
			count[i1] = 1.0f;
		else
			count[i1] = count[i1] / (count[i1] + 1);

		if (count[i2] == 0)
			count[i2] = 1.0f;
		else
			count[i2] = count[i2] / (count[i2] + 1);
		normals[i0] += normal;
		normals[i1] += normal;
		normals[i2] += normal;
	}
	for (int i = 0; i < normals.size(); i++)
	{
		normals[i] = normals[i] * count[i];
	}
	delete count;

}
IndexedModel OBJModel::ToIndexedModel()
{
	IndexedModel result;
	IndexedModel normalModel;
	IndexedModel simpleResult;

	unsigned int numIndices = OBJIndices.size();
	std::vector<OBJIndex*> indexLookup;


	if (!hasNormals)
	{
		for (int i = 0; i < vertices.size(); i++)
		{
			normals.push_back(glm::vec3(0, 0, 0));

		}
		hasNormals = true;
	}
	CalcNormals();

	for (OBJIndex &it1 : OBJIndices)
	{
		indexLookup.push_back(&it1);
	}
	std::sort(indexLookup.begin(), indexLookup.end(), CompareOBJIndexPtr);

	std::map<OBJIndex, unsigned int> normalModelIndexMap;
	std::map<unsigned int, unsigned int> indexMap;

	for (OBJIndex &it1 : OBJIndices)
	{
		OBJIndex* currentIndex = &it1;

		glm::vec3 currentPosition = vertices[currentIndex->vertexIndex];
		glm::vec2 currentTexCoord;
		glm::vec3 currentNormal;
		glm::vec3 currentColor;

		if (hasUVs)
			currentTexCoord = uvs[currentIndex->uvIndex];
		else
			currentTexCoord = glm::vec2(0, 0);

		currentNormal = normals[currentIndex->normalIndex];
		currentColor = normals[currentIndex->normalIndex];
			unsigned int normalModelIndex;
		unsigned int resultModelIndex;

		//Create model to properly generate normals on
		std::map<OBJIndex, unsigned int>::iterator it = normalModelIndexMap.find(*currentIndex);
		if (it == normalModelIndexMap.end())
		{
			normalModelIndex = normalModel.positions.size();

			normalModelIndexMap.insert(std::pair<OBJIndex, unsigned int>(*currentIndex, normalModelIndex));
			normalModel.positions.push_back(currentPosition);
			normalModel.texCoords.push_back(currentTexCoord);
			normalModel.normals.push_back(currentNormal);
			normalModel.colors.push_back(currentColor);
		}
		else
			normalModelIndex = it->second;

		//Create model which properly separates texture coordinates
		unsigned int previousVertexLocation = FindLastVertexIndex(indexLookup, currentIndex, result);

		if (previousVertexLocation == (unsigned int)-1)
		{
			resultModelIndex = result.positions.size();

			result.positions.push_back(currentPosition);
			result.texCoords.push_back(currentTexCoord);
			result.normals.push_back(currentNormal);
			result.colors.push_back(currentColor);
		}
		else
			resultModelIndex = previousVertexLocation;

		normalModel.indices.push_back(normalModelIndex);
		result.indices.push_back(resultModelIndex);
		indexMap.insert(std::pair<unsigned int, unsigned int>(resultModelIndex, normalModelIndex));
	}

	return result;
};
void OBJModel::start(int maxFaces) {
	std::cout << "start function begin.." << std::endl;
	int currFaces = OBJIndices.size() / 3;
	while (currFaces > maxFaces) {
		std::cout << "currFaces is: " << currFaces << std::endl;

		std::pop_heap(edges.begin(), edges.end(), comp);

		Edge removedEdge = edges.back();

		int v1index = removedEdge.edge.first.vertexIndex;
		int v2index = removedEdge.edge.second.vertexIndex;
		glm::vec3 newVertex = removedEdge.newpos;
		vertices[v1index] = newVertex;
		this->Q_errors[v1index] = removedEdge.Q;
		for (int ch = 0; ch < vertices.size(); ch++)
		{
			int count = 0;

			std::pair <std::multimap<int, int>::iterator, std::multimap<int, int>::iterator> ret;
			ret = vertex_map.equal_range(ch);
			for (std::multimap<int, int>::iterator it = ret.first; it != ret.second;) {
				if (ch != v1index) {
					if (it->second == v2index && count == 0) {
						it->second = v1index;
						count++;
						it++;

					}
					else if (it->second == v2index) {
						vertex_map.erase(it++);
					}
					else
						it++;
				}
				else {
					if (it->second == v2index)
						vertex_map.erase(it++);
					else
						it++;
				}
			}
		}

		for (int ch = 0; ch < vertices.size(); ch++)
		{
			int count = 0;

			std::pair <std::multimap<int, int>::iterator, std::multimap<int, int>::iterator> ret;
			ret = vertex_map.equal_range(ch);
			for (std::multimap<int, int>::iterator it = ret.first; it != ret.second;) {
				if (it->second == v1index && count == 0) {
					count++;
					it++;
				}
				else if (it->second == v1index) {
					vertex_map.erase(it++);
				}
				else
					it++;
			}
		}

		edges.pop_back();

		currFaces = currFaces - 2;
		vertex_map.erase(v2index);
		calcFaces(OBJIndices, v1index, v2index);

		for (int i = 0; i < edges.size(); i++) {

			if (edges[i].edge.second.vertexIndex == v2index) {
				edges[i].edge.second.vertexIndex = v1index;
			}
			else if (edges[i].edge.first.vertexIndex == v2index) {
				edges[i].edge.first.vertexIndex = v1index;
			}
		}

		for (int i = 0; i < edges.size(); i++) {

			if (edges[i].edge.second.vertexIndex < edges[i].edge.first.vertexIndex) {
				int tmp = edges[i].edge.second.vertexIndex;
				edges[i].edge.second.vertexIndex = edges[i].edge.first.vertexIndex;
				edges[i].edge.first.vertexIndex = tmp;
			}
		}


		edges = remove_duplicate3(edges, v1index, v2index);

		for (int i = 0; i < edges.size(); i++) {
			if (edges[i].edge.first.vertexIndex == v1index || edges[i].edge.second.vertexIndex == v1index) {
				edge_Error(edges[i]);
			}
		}
		std::make_heap(edges.begin(), edges.end(), comp);
		std::cout << std::endl;
	}

	std::cout << "start function end" << std::endl;
}


unsigned int OBJModel::FindLastVertexIndex(const std::vector<OBJIndex*>& indexLookup, const OBJIndex* currentIndex, const IndexedModel& result)
{
	unsigned int start = 0;
	unsigned int end = indexLookup.size();
	unsigned int current = (end - start) / 2 + start;
	unsigned int previous = start;

	while (current != previous)
	{
		OBJIndex* testIndex = indexLookup[current];

		if (testIndex->vertexIndex == currentIndex->vertexIndex)
		{
			unsigned int countStart = current;

			for (unsigned int i = 0; i < current; i++)
			{
				OBJIndex* possibleIndex = indexLookup[current - i];

				if (possibleIndex == currentIndex)
					continue;

				if (possibleIndex->vertexIndex != currentIndex->vertexIndex)
					break;

				countStart--;
			}

			for (unsigned int i = countStart; i < indexLookup.size() - countStart; i++)
			{
				OBJIndex* possibleIndex = indexLookup[current + i];

				if (possibleIndex == currentIndex)
					continue;

				if (possibleIndex->vertexIndex != currentIndex->vertexIndex)
					break;
				else if ((!hasUVs || possibleIndex->uvIndex == currentIndex->uvIndex)
					&& (!hasNormals || possibleIndex->normalIndex == currentIndex->normalIndex))
				{
					glm::vec3 currentPosition = vertices[currentIndex->vertexIndex];
					glm::vec2 currentTexCoord;
					glm::vec3 currentNormal;
					glm::vec3 currentColor;

					if (hasUVs)
						currentTexCoord = uvs[currentIndex->uvIndex];
					else
						currentTexCoord = glm::vec2(0, 0);

					if (hasNormals)
					{
						currentNormal = normals[currentIndex->normalIndex];
						currentColor = normals[currentIndex->normalIndex];
					}
					else
					{
						currentNormal = glm::vec3(0, 0, 0);
						currentColor = glm::normalize(glm::vec3(1, 1, 1));
					}
					for (unsigned int j = 0; j < result.positions.size(); j++)
					{
						if (currentPosition == result.positions[j]
							&& ((!hasUVs || currentTexCoord == result.texCoords[j])
								&& (!hasNormals || currentNormal == result.normals[j])))
						{
							return j;
						}
					}
				}
			}

			return -1;
		}
		else
		{
			if (testIndex->vertexIndex < currentIndex->vertexIndex)
				start = current;
			else
				end = current;
		}

		previous = current;
		current = (end - start) / 2 + start;
	}

	return -1;
}

void OBJModel::CreateOBJFace(const std::string& line)
{
	std::vector<std::string> tokens = SplitString(line, ' ');
	unsigned int tmpIndex = OBJIndices.size();
	//	std::list<OBJIndex>::iterator faceIt = OBJIndices.end();
	OBJIndices.push_back(ParseOBJIndex(tokens[1], &this->hasUVs, &this->hasNormals));

	OBJIndices.push_back(ParseOBJIndex(tokens[2], &this->hasUVs, &this->hasNormals));

	OBJIndices.push_back(ParseOBJIndex(tokens[3], &this->hasUVs, &this->hasNormals));

	if ((int)tokens.size() > 4)//triangulation
	{
		OBJIndices.push_back(ParseOBJIndex(tokens[1], &this->hasUVs, &this->hasNormals));

		OBJIndices.push_back(ParseOBJIndex(tokens[3], &this->hasUVs, &this->hasNormals));

		OBJIndices.push_back(ParseOBJIndex(tokens[4], &this->hasUVs, &this->hasNormals));

	}
}

OBJIndex OBJModel::ParseOBJIndex(const std::string& token, bool* hasUVs, bool* hasNormals)
{
	unsigned int tokenLength = token.length();
	const char* tokenString = token.c_str();

	unsigned int vertIndexStart = 0;
	unsigned int vertIndexEnd = FindNextChar(vertIndexStart, tokenString, tokenLength, '/');

	OBJIndex result;
	result.vertexIndex = ParseOBJIndexValue(token, vertIndexStart, vertIndexEnd);
	result.uvIndex = 0;
	result.normalIndex = 0;
	//result.faceIndex =-1;
	result.edgeIndex = -1;
	result.isEdgeUpdated = true;

	if (vertIndexEnd >= tokenLength)
		return result;

	vertIndexStart = vertIndexEnd + 1;
	vertIndexEnd = FindNextChar(vertIndexStart, tokenString, tokenLength, '/');

	result.uvIndex = ParseOBJIndexValue(token, vertIndexStart, vertIndexEnd);
	*hasUVs = true;

	if (vertIndexEnd >= tokenLength)
		return result;

	vertIndexStart = vertIndexEnd + 1;
	vertIndexEnd = FindNextChar(vertIndexStart, tokenString, tokenLength, '/');

	result.normalIndex = ParseOBJIndexValue(token, vertIndexStart, vertIndexEnd);
	*hasNormals = true;

	return result;
}

glm::vec3 OBJModel::ParseOBJVec3(const std::string& line)
{
	unsigned int tokenLength = line.length();
	const char* tokenString = line.c_str();

	unsigned int vertIndexStart = 2;

	while (vertIndexStart < tokenLength)
	{
		if (tokenString[vertIndexStart] != ' ')
			break;
		vertIndexStart++;
	}

	unsigned int vertIndexEnd = FindNextChar(vertIndexStart, tokenString, tokenLength, ' ');

	float x = ParseOBJFloatValue(line, vertIndexStart, vertIndexEnd);

	vertIndexStart = vertIndexEnd + 1;
	vertIndexEnd = FindNextChar(vertIndexStart, tokenString, tokenLength, ' ');

	float y = ParseOBJFloatValue(line, vertIndexStart, vertIndexEnd);

	vertIndexStart = vertIndexEnd + 1;
	vertIndexEnd = FindNextChar(vertIndexStart, tokenString, tokenLength, ' ');

	float z = ParseOBJFloatValue(line, vertIndexStart, vertIndexEnd);

	return glm::vec3(x, y, z);

	//glm::vec3(atof(tokens[1].c_str()), atof(tokens[2].c_str()), atof(tokens[3].c_str()))
}

glm::vec2 OBJModel::ParseOBJVec2(const std::string& line)
{
	unsigned int tokenLength = line.length();
	const char* tokenString = line.c_str();

	unsigned int vertIndexStart = 3;

	while (vertIndexStart < tokenLength)
	{
		if (tokenString[vertIndexStart] != ' ')
			break;
		vertIndexStart++;
	}

	unsigned int vertIndexEnd = FindNextChar(vertIndexStart, tokenString, tokenLength, ' ');

	float x = ParseOBJFloatValue(line, vertIndexStart, vertIndexEnd);

	vertIndexStart = vertIndexEnd + 1;
	vertIndexEnd = FindNextChar(vertIndexStart, tokenString, tokenLength, ' ');

	float y = ParseOBJFloatValue(line, vertIndexStart, vertIndexEnd);

	return glm::vec2(x, y);
}

static bool CompareOBJIndexPtr(const OBJIndex* a, const OBJIndex* b)
{
	return a->vertexIndex < b->vertexIndex;
}

static inline unsigned int FindNextChar(unsigned int start, const char* str, unsigned int length, char token)
{
	unsigned int result = start;
	while (result < length)
	{
		result++;
		if (str[result] == token)
			break;
	}

	return result;
}

static inline unsigned int ParseOBJIndexValue(const std::string& token, unsigned int start, unsigned int end)
{
	return atoi(token.substr(start, end - start).c_str()) - 1;
}

static inline float ParseOBJFloatValue(const std::string& token, unsigned int start, unsigned int end)
{
	return atof(token.substr(start, end - start).c_str());
}

static inline std::vector<std::string> SplitString(const std::string &s, char delim)
{
	std::vector<std::string> elems;

	const char* cstr = s.c_str();
	unsigned int strLength = s.length();
	unsigned int start = 0;
	unsigned int end = 0;

	while (end <= strLength)
	{
		while (end <= strLength)
		{
			if (cstr[end] == delim)
				break;
			end++;
		}

		elems.push_back(s.substr(start, end - start));
		start = end + 1;
		end = start;
	}

	return elems;
}