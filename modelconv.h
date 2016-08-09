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
	tcModelConv();
	~tcModelConv();

	int importModel(const char* apFilename);

	int combineFaces(float anThreshold);

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
		tsVertex* mpVertex1; 
		tsVertex* mpVertex2;
		tsVertex* mpVertex3;
	};

	struct tsFace
	{
		tsNormal msNormal; //!< Normal vector of the face.
		std::vector<tsTriangle*> mcTriangles; //!< Object triangles
			//!< that can be joined to form a face with a single
			//!< normal vector (i.e. all faces are on one plane).
		std::list<tsVertex*> mcVertices; //!< Vertices that define
			//!< the border of the face if traversed from front to 
			//!< end.
	};

	std::string to_string(const tsNormal& arNormal);
	std::string to_string(const tsVertex& arVertex);
	std::string to_string(const tsTriangle& arTriangle);
	std::string to_string(const tsFace& arFace);

	tsVertex& addVertex(const tsVertex& arVertex);
	int createFaces();

	uint8_t maBinStlHeader[80]; //!< Header read from binary STL file.

	std::vector<tsVertex> mcVertices; //!< Array of all vertex points in 
		//!< object, so that if modifications to object are made, we do 
		//!< not risk separating connected triangles.

	std::vector<tsTriangle> mcTriangles; //!< Trianlges that define object.

	std::list<tsFace> mcFaces; //!< Faces (i.e. series of triangles on
		//!< the same plane) that define the object.
};

#endif /* _MODEL_CONV_ */
