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
	};

	struct tsVertex
	{
		float x;
		float y;
		float z;
	};

	// Since this struct is used to read from a packed file, we need to
	//  compress data to match
	#pragma pack(push, 1)
	struct tsBinStlTriangle
	{
		tsNormal msNormal; //!< Normal vector of triangle.
		tsVertex msVertex1; 
		tsVertex msVertex2;
		tsVertex msVertex3;
		uint16_t mnAttrByteCnt; //!< Attribute byte count. Unused.
	};
	#pragma pack(pop)
	// Back to default packing 

	struct tsTriangle
	{
		tsNormal msNormal; //!< Normal vector of triangle.
		tsVertex* mpVertices[3]; //!< A triangle is defined by three
			//!< vertices. Each entry points to object stored in
			//!< mcVertices.
		tsTriangle* mpNeighbors[3]; //!< A triangle can have up to
			//!< three adjacent triangles. NULL indicates an
			//!< unconnected or open edge in the obkect.
	};

	struct tsFace
	{
		std::vector<tsVertex*> mcBorder; //!< Vertices that define
			//!< border of the face.
	};

	std::string to_string(const tsNormal& arNormal);
	std::string to_string(const tsVertex& arVertex);
	std::string to_string(const tsTriangle& arTriangle);

	tsVertex& addVertex(const tsVertex& arVertex);
	void checkAdjacent(tsTriangle& arTriangle1, tsTriangle& arTriangle2);

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
