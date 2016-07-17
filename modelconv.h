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

	#pragma pack(1)
	struct tsTriangle
	{
		tsNormal msNormal; //!< Normal vector of triangle.
		tsVertex msVertex1; //!< First vertex of triangle (TODO: any implied position or direction to next vertex (i.e. clockwise)??)
		tsVertex msVertex2;
		tsVertex msVertex3;
		uint16_t mnAttrByteCnt; //!< Attribute byte count. Unused.
	};

	uint8_t maBinStlHeader[80]; //!< Header read from binary STL file.
	uint32_t mnNumTriangles; //!< Number of elements in mpTriangles.
	tsTriangle* mpTriangles; //!< Trianlges that define imported object.
};

#endif /* _MODEL_CONV_ */
