/**
 * \file modelconv.h
 * \brief Class to handle parsing and converting 3D model data.
 * \author Gregory Gluszek.
 */

#ifndef _MODEL_CONV_
#define _MODEL_CONV_

#include <stdint.h>

class tcModelConv
{
public:
	tcModelConv();
	~tcModelConv();

	int importModel(const char* apFilename);

	int createFaces(float anThreshold);

	int exportBinStl(const char* apFilename);

	int exportSvg(const char* apFilename);

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
		tsVertex msVertex1; //!< First vertex of triangle (TODO: any implied position or direction to next vertex (i.e. clockwise)??)
		tsVertex msVertex2;
		tsVertex msVertex3;
		uint16_t mnAttrByteCnt; //!< Attribute byte count. Unused.
	};
	#pragma pack(pop)
	// Back to default packing for everything else

	struct tsTriangle
	{
		tsNormal msNormal; //!< Normal vector of triangle.
		tsVertex* mpVertex1; //!< First vertex of triangle (TODO: any implied position or direction to next vertex (i.e. clockwise)??)
		tsVertex* mpVertex2;
		tsVertex* mpVertex3;
	};

	tsVertex* addVertex(const tsVertex* apVertex);

	uint32_t mnNumVerticies; //!< Number of valid vertices in mpVertices.
	uint32_t mnVerticiesArraySize; //!< Size of mpVertices.
	tsVertex* mpVertices; //!< Array of all vertex point in object, so that
		//!< if modifications to object are made, we do not risk 
		//!< separating connected triangles.

	uint8_t maBinStlHeader[80]; //!< Header read from binary STL file.

	uint32_t mnNumTriangles; //!< Number of elements in mpTriangles.
	tsTriangle* mpTriangles; //!< Trianlges that define imported object.
};

#endif /* _MODEL_CONV_ */
