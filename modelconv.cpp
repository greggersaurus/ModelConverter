/**
 * \file modelconv.cpp
 * \brief Class to handle parsing and converting 3D model data.
 * \author Gregory Gluszek.
 */

#include "modelconv.h"

#include <stdio.h>
#include <string.h>

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(*(x))) //!< Used for calculating      
	//!< static array sizes

/**
 * Constructor.
 */
tcModelConv::tcModelConv()
{
	memset(maBinStlHeader, 0, ARRAY_SIZE(maBinStlHeader));
}

/**
 * Destructor.
 */
tcModelConv::~tcModelConv()
{
	//TODO: need to manually clear lists and vectors?
}

/**
 * Import model data from the given file.
 *
 * \param[in] apFilename File containing 3D model data.
 *
 * \return 0 on success.
 */
//TODO: move this into constructor instead of separate function?
int tcModelConv::importModel(const char* apFilename)
{
	FILE* file = NULL;
	// Number of elements read by fread
	size_t elem_read = 0;
	uint32_t num_triangles = 0;

//TODO: Add code to check file type, etc. For now only support binary STL. Will add functions for parsing different file types later?

	tsBinStlTriangle bin_stl_triangle;

	// Open file for reading
	file = fopen(apFilename, "r");	
	if (!file)
	{
		fprintf(stderr, "Failed to open file \"%s\"\n", apFilename);
		return -1;
	}	

	// Read header data from STL file
	elem_read = fread(maBinStlHeader, sizeof(maBinStlHeader[0]), 
		ARRAY_SIZE(maBinStlHeader), file);
	if (ARRAY_SIZE(maBinStlHeader) != elem_read)
	{
		fprintf(stderr, "Only able to read %lu of %lu header bytes from"
			" file \"%s\"\n", elem_read, ARRAY_SIZE(maBinStlHeader),
			apFilename);
		return -1;
	}

	// Read number of triangles in file from the STL file
	elem_read = fread(&num_triangles, 1, sizeof(num_triangles), file);
	if (sizeof(num_triangles) != elem_read)
	{
		fprintf(stderr, "Only able to read %lu of %lu num triangle "
			"field bytes from file \"%s\"\n", elem_read, 
			ARRAY_SIZE(maBinStlHeader), apFilename);
		return -1;
	}
	
	// Clean up vertex storage memory
	mcVertices.clear();
	// If object is closed, there will be at one vertex per triangle. 
	//  Start off with vector of this size to minimize dynamic resizing.
//TODO: or should we assume largest possible and spend more memory in threat of wasting time reallocating? Or do we want this optimized for closed objects?
	mcVertices.reserve(num_triangles);

	// Clean up triangle storage memory
	mcTriangles.clear();
	// We know exactly how many triangle there are and this should make sure
	//  we allocate entries for all of them now
	mcTriangles.resize(num_triangles);

	// Initialize each face to be a triangle for starters
	mcFaces.clear();
	mcFaces.resize(num_triangles);
	
	// Read the triangle data from the STL file
	for (uint32_t cnt = 0; cnt < num_triangles; cnt++)
	{
		elem_read = fread(&bin_stl_triangle, 1, 
			sizeof(bin_stl_triangle), file);
		if (sizeof(bin_stl_triangle) != elem_read)
		{
			fprintf(stderr, "Only read %lu of %lu bytes for "
				"triangle %u of %u from \"%s\" file.\n", 
				elem_read, sizeof(bin_stl_triangle), cnt, 
				num_triangles, apFilename);
			return -1;
		}

		// Copy normal vector data
		mcTriangles[cnt].msNormal = bin_stl_triangle.msNormal;

		// (Potentially) add vertex data to array and get pointer
		//  to vertx data in mpVertices
		mcTriangles[cnt].mpVertex1 = &addVertex(bin_stl_triangle.msVertex1);
		mcTriangles[cnt].mpVertex2 = &addVertex(bin_stl_triangle.msVertex2);
		mcTriangles[cnt].mpVertex3 = &addVertex(bin_stl_triangle.msVertex3);

		// Populate faces, with each triangle as a face for starters
//TODO: this will not work since mcFaces is list now, since faces may be removed at random later
		mcFaces[cnt].msNormal = bin_stl_triangle.msNormal;
		mcFaces[cnt].mcTriangles.push_back(&mcTriangles[cnt]);
		mcFaces[cnt].mcVertices.push_back(mcTriangles[cnt].mpVertex1);
		mcFaces[cnt].mcVertices.push_back(mcTriangles[cnt].mpVertex2);
		mcFaces[cnt].mcVertices.push_back(mcTriangles[cnt].mpVertex3);
	}

	if (fclose(file))
	{
		fprintf(stderr, "Failed to close file \"%s\" after reading "
			"data.\n", apFilename);
		return -1;
	}

		

	return 0;
}

/**
 * TODO: want to print useful info. but shoudl this be to_string?
 */
void tcModelConv::debugPrint()
{

//TODO: debug print.
	printf("%lu unique vertices found amongst %lu triangles.\n\n", 
		mcVertices.size(), mcTriangles.size());

	uint32_t cnt = 0;
	for (std::vector<tsTriangle>::iterator it = mcTriangles.begin();
		it != mcTriangles.end(); it++) 
	{
		printf("Triangle %u:\n", cnt++);
		printf("%s\n", to_string(*it).c_str());
	}
	printf("\n");

	cnt = 0;
	for (std::vector<tsFace>::iterator it = mcFaces.begin();
		it != mcFaces.end(); it++) 
	{
		printf("Face %u:\n", cnt++);
		printf("%s\n", to_string(*it).c_str());
	}
}

/**
 * \return The string representation of a tsNormal.
 */
std::string tcModelConv::to_string(const tsNormal& arNormal)
{
	return "i = " + std::to_string(arNormal.i) + 
		" j = " + std::to_string(arNormal.j) + 
		" k = " + std::to_string(arNormal.k);	
}

/**
 * \return The string representation of a tsVertex.
 */
std::string tcModelConv::to_string(const tsVertex& arVertex)
{
	return "x = " + std::to_string(arVertex.x) + 
		" y = " + std::to_string(arVertex.y) + 
		" z = " + std::to_string(arVertex.z);	
}

/**
 * \return The string representation of a tsTriangle.
 */
std::string tcModelConv::to_string(const tsTriangle& arTriangle)
{
	return "Normal: " + to_string(arTriangle.msNormal) + "\n" + 
		"Vertex 1: " + to_string(*arTriangle.mpVertex1) + "\n" + 
		"Vertex 2: " + to_string(*arTriangle.mpVertex2) + "\n" + 
		"Vertex 3: " + to_string(*arTriangle.mpVertex3);
}

/**
 * \return The string representation of a tsFace.
 */
std::string tcModelConv::to_string(const tsFace& arFace)
{
	std::string retval = "";

	retval += "Normal Vector:\n";
	retval += to_string(arFace.msNormal);
	retval += "\n";

	retval += "Triangles:\n";
	for (std::vector<tsTriangle*>::const_iterator itr = 
		arFace.mcTriangles.begin(); itr != arFace.mcTriangles.end(); 
		itr++)
	{
		retval += to_string(**itr);
		retval += "\n";
	}

	retval += "Vertices:\n";
	for (std::list<tsVertex*>::const_iterator itr = 
		arFace.mcVertices.begin(); itr != arFace.mcVertices.end(); 
		itr++)
	{
		retval += to_string(**itr);
		retval += "\n";
	}

	return retval;
}

/**
 * Add the given vertex data to mpVertices and return a pointer to that vertex.
 *  If the vertex data already exists in mpVertices, just return a pointer.
 *
 * \param[in] arVertex Vertex data to be copied into element in 
 *
 * \return Reference to vertex data in mcVertices.
 */
tcModelConv::tsVertex& tcModelConv::addVertex(const tsVertex& arVertex)
{
	// Search to see if vertex already exists in vector
	for (std::vector<tsVertex>::iterator it = mcVertices.begin();
		it != mcVertices.end(); it++)
	{
		// Compare all data in vertex struct
		if (arVertex.x == it->x && arVertex.y == it->y && 
			arVertex.z == it->z)
		{
			return *it;
		}
	}

	// If we made it out of the loop without exiting the function we did
	//  did not find the vertex in the vector already and need to add it
	mcVertices.push_back(arVertex);

	// Return pointer to newly added element
	return mcVertices.back();
}

/**
 * Combine faces on the same plane (or triangles almost on the same plane based on threshold).
 *
 * \param anThreshold TODO
 * 
 * \return 0 on success.
 */
int tcModelConv::combineFaces(float anThreshold)
{

//TODO:
//  Search through mcFaces finding connecting edges and matching normal vectors
//  First check if normals match (i.e. same degree)
//  If they do, then search for common vertices...
//   can just insert one last into other?

	return -1;
}

/**
 * Export model data to STL file format with binary data.
 *
 * \param[in] apFilename Filename to write STL data to.
 *
 * \return 0 on success.
 */
int tcModelConv::exportBinStl(const char* apFilename)
{
	FILE* file = NULL;
	// Number of elements written by fwrite
	size_t elem_wr = 0;
	tsBinStlTriangle bin_stl_triangle;
	uint32_t num_triangles = (uint32_t)mcTriangles.size();

	file = fopen(apFilename, "w");
	if (!file)
	{
		fprintf(stderr, "Failed to open file \"%s\" for writing.\n",
			apFilename);
		return -1;
	}

	// Write header data
	elem_wr = fwrite(maBinStlHeader, sizeof(maBinStlHeader[0]),
		ARRAY_SIZE(maBinStlHeader), file);
	if (ARRAY_SIZE(maBinStlHeader) != elem_wr)
	{
		fprintf(stderr, "Only wrote %lu of %lu bytes from "
			"maBinStlHeader to file \"%s\"\n", elem_wr, 
			ARRAY_SIZE(maBinStlHeader), apFilename);
		return -1;
	}

	// Write number of triangle
	elem_wr = fwrite(&num_triangles, 1, sizeof(num_triangles), file);
	if (sizeof(num_triangles) != elem_wr)
	{
		fprintf(stderr, "Only wrote %lu of %lu bytes from "
			"num_triangles to file \"%s\"\n", elem_wr, 
			sizeof(num_triangles), apFilename);
		return -1;
	}

	// Write triangle data
	for (uint32_t cnt = 0; cnt < num_triangles; cnt++)
	{
		// Copy trianlge data to struct for writing
		bin_stl_triangle.msNormal = mcTriangles[cnt].msNormal;
		bin_stl_triangle.msVertex1 = *(mcTriangles[cnt].mpVertex1);
		bin_stl_triangle.msVertex2 = *(mcTriangles[cnt].mpVertex2);
		bin_stl_triangle.msVertex3 = *(mcTriangles[cnt].mpVertex3);
		bin_stl_triangle.mnAttrByteCnt = 0;

		// Write triangle data to file
		elem_wr = fwrite(&bin_stl_triangle, 1, sizeof(bin_stl_triangle),
			file);
		if (sizeof(bin_stl_triangle) != elem_wr)
		{
			fprintf(stderr, "Only wrote %lu of %lu bytes from "
				"%u of %u triangle to file \"%s\"\n", elem_wr, 
				sizeof(bin_stl_triangle), cnt, num_triangles,
				apFilename);
			return -1;
		}
		
	}

	if (fclose(file))
	{
		fprintf(stderr, "Failed to close file \"%s\" after writing "
			"data.\n", apFilename);
		return -1;
	}

	return 0;
}

/**
 * Output Scalable Vector Graphs with each face as an outlined object.
 *
 * \param[in] apFilename Filename to write SVG data to.
 *
 * \return 0 on success.
 */
int tcModelConv::exportSvg(const char* apFilename)
{
//TODO: what if there are no faces created, or will they be created as soon as data is imported??

	return -1;
}

