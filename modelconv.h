/**
 * \file modelconv.h
 * \brief Class to handle parsing and converting 3D model data.
 * \author Gregory Gluszek.
 */

#ifndef _MODEL_CONV_
#define _MODEL_CONV_

#include <stdint.h>
#include <string>
#include <vector>
#include <list>
#include <unordered_map>
#include <math.h>

class ModelConv
{
public:
	ModelConv(const char* filename);
	~ModelConv();

	void exportBinStl(const char* filename);

	void exportSvg(const char* filename);

	void debugPrint();

protected:

//TODO: make into class? construction and init being taken care of correctly in code?
	struct Normal
	{
		float i;
		float j;
		float k;

		bool operator==(const Normal& rhs) const
		{
//TODO: Is this the best way to control thresholding the imprecision of float conversions?
			return (fabs(rhs.i-i) < .00001) && 
				(fabs(rhs.j-j) < .00001) && 
				(fabs(rhs.k- k) < .00001);
		}
	};

//TODO: make into class? construction and init being taken care of correctly in code?
	struct Vertex
	{
		float x;
		float y;
		float z;

		bool operator==(const Vertex& rhs) const
		{
			return (rhs.x == x) && (rhs.y == y) && (rhs.z == z);
		}
	};

	// Since this struct is used to read from a packed file, we need to
	//  compress data to match
	#pragma pack(push, 1)
	struct BinStlTriangle
	{
		Normal normal; //!< Normal vector of triangle.
		Vertex vertices[3]; 
		uint16_t attrByteCnt; //!< Attribute byte count. Unused.
	};
	#pragma pack(pop)
	// Back to default packing 

//TODO: make into class? construction and init being taken care of correctly in code?
	struct Triangle
	{
		Normal normal; //!< Normal vector of triangle.
		Vertex* vertices[3]; //!< A triangle is defined by three
			//!< vertices. Each entry points to an object stored in
			//!< vertices.
		Triangle* neighbors[3]; //!< A triangle can have up to
			//!< three adjacent triangles. NULL indicates an
			//!< unconnected or open edge in the object.
			//!< neighbor[0] is on edge made by vertices 0 to 1
			//!< neighbor[1] is on edge made by vertices 1 to 2
			//!< neighbor[2] is on edge made by vertices 2 to 0
	};

//TODO: make into class? construction and init being taken care of correctly in code?
	struct Face
	{
		Normal normal; 
		std::vector<const Triangle*> triangles; //!< All triangles that
			//!< make up a face. Mostly included for debug or
			//!< face to object export.
		std::list<const Vertex*> border; //!< Vertices that define
			//!< border of the face.
	};

	std::string to_string(const Normal& normal);
	std::string to_string(const Vertex& vertex);
	std::string to_string(const Triangle& triangle);

	Vertex& addVertex(const Vertex& vertex);
	void checkAdjacent(Triangle& tri1, Triangle& tri2);
	void addNeighbor(Triangle& tri, Triangle& neighbor, 
		uint8_t sharedVtxs);
	void buildFace(const Triangle& tri, Face& face, 
		std::unordered_map<const Triangle*, int>& faceMap,
		std::unordered_map<const Triangle*, int>& travMap);
	void insertVertex(Face& face, const Triangle& tri, int neighborIndex);

	void exportBinStl(const char* filename, 
		const std::vector<const Triangle*>& triangles);

	uint8_t binStlHeader[80]; //!< Header read from binary STL file.

	std::vector<Vertex*> vertices; //!< Unique entry for each vertex in
		//!< object. 

	std::vector<Triangle*> triangles; //!< Unique entry for each trianlge 
		//!< in the object.

	std::vector<Face*> faces; //!< Unique entry for each face of the object.
};

#endif /* _MODEL_CONV_ */
