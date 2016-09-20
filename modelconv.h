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

class tcModelConv
{
public:
	tcModelConv(const char* apFilename);
	~tcModelConv();

	int exportBinStl(const char* apFilename);

	int exportSvg(const char* apFilename);

	void debugPrint();

protected:

	struct tsNormal
	{
		float i;
		float j;
		float k;

		bool operator==(const tsNormal& rhs) const
		{
			return (rhs.i == i) && (rhs.j == j) && (rhs.k == k);
		}
	};

	struct tsVertex
	{
		float x;
		float y;
		float z;

		bool operator==(const tsVertex& rhs) const
		{
			return (rhs.x == x) && (rhs.y == y) && (rhs.z == z);
		}
	};

	// Since this struct is used to read from a packed file, we need to
	//  compress data to match
	#pragma pack(push, 1)
	struct tsBinStlTriangle
	{
		tsNormal msNormal; //!< Normal vector of triangle.
		tsVertex maVertices[3]; 
		uint16_t mnAttrByteCnt; //!< Attribute byte count. Unused.
	};
	#pragma pack(pop)
	// Back to default packing 

	struct tsTriangle
	{
		tsNormal msNormal; //!< Normal vector of triangle.
		tsVertex* mpVertices[3]; //!< A triangle is defined by three
			//!< vertices. Each entry points to an object stored in
			//!< mcVertices.
		tsTriangle* mpNeighbors[3]; //!< A triangle can have up to
			//!< three adjacent triangles. NULL indicates an
			//!< unconnected or open edge in the object.
			//!< neighbor[0] is on edge made by vertices 0 to 1
			//!< neighbor[1] is on edge made by vertices 1 to 2
			//!< neighbor[2] is on edge made by vertices 3 to 0
	};

	struct tsFace
	{
		tsNormal msNormal; 
		std::list<tsTriangle*> mcTriangles; //!< All triangles that
			//!< make up a face. Mostly included for debug or
			//!< face to object export.
		std::list<tsVertex*> mcBorder; //!< Vertices that define
			//!< border of the face.
	};

	std::string to_string(const tsNormal& arNormal);
	std::string to_string(const tsVertex& arVertex);
	std::string to_string(const tsTriangle& arTriangle);

	tsVertex& addVertex(const tsVertex& arVertex);
	void checkAdjacent(tsTriangle& arTri1, tsTriangle& arTri2);
	void addNeighbor(tsTriangle& arTri1, tsTriangle& arTri2, 
		uint8_t anShared);
	void insertVertex(tsFace& arFace, const tsTriangle& arTri1, 
		const tsTriangle& arTri2);
	void buildFace(tsFace& arFace, 
		std::unordered_map<void*, int>& arTravMap, 
		const tsNormal& arNorm, const tsTriangle& arTri);
//TODO
//	void exportStl(std::list<tsTriangle*>& arTris);

	uint8_t maBinStlHeader[80]; //!< Header read from binary STL file.

	std::vector<tsVertex> mcVertices; //!< Array of all vertex points in 
		//!< object. Each vertex is guaranteed unique in terms of 
		//!< location this vector.

	std::vector<tsTriangle> mcTriangles; //!< All trianlges that define 
		//!< the object.

	std::vector<tsFace> mcFaces; //!< Defines connected triangles that are 
		//!< all on the same plane.
};

#endif /* _MODEL_CONV_ */
