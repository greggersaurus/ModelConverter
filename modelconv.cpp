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
: mnNumVerticies(0)
, mnVerticiesArraySize(0)
, mpVertices(NULL)
, mnNumTriangles(0)
, mpTriangles(NULL)
{
	memset(maBinStlHeader, 0, ARRAY_SIZE(maBinStlHeader));
}

/**
 * Destructor.
 */
tcModelConv::~tcModelConv()
{
	// Clean up triangle storage memory, if necessary
	if (mpVertices)
		delete [] mpVertices;
	if (mpTriangles)
		delete [] mpTriangles;
}

/**
 * Import model data from the given file.
 *
 * \param[in] apFilename File containing 3D model data.
 *
 * \return 0 on success.
 */
int tcModelConv::importModel(const char* apFilename)
{
	FILE* file = NULL;
	// Number of elements read by fread
	size_t elem_read = 0;

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
	elem_read = fread(&mnNumTriangles, 1, sizeof(mnNumTriangles), file);
	if (sizeof(mnNumTriangles) != elem_read)
	{
		fprintf(stderr, "Only able to read %lu of %lu num triangle "
			"field bytes from file \"%s\"\n", elem_read, 
			ARRAY_SIZE(maBinStlHeader), apFilename);
		return -1;
	}
	
	// Clean up vertex storage memory, if necessary
	mnNumVerticies = 0;
	if (mpVertices)
		delete [] mpVertices;
	// Size array for worst case (i.e. no shared vertices bewteen all
	//  triangles). We could make this more memory efficient at the cost
	//  of being more time efficient. For now we choose time over memory.
	mnVerticiesArraySize = mnNumTriangles * 3;
	mpVertices = new tsVertex[mnVerticiesArraySize];

	// Clean up triangle storage memory, if necessary
	if (mpTriangles)
		delete [] mpTriangles;
	// Allocate space for storing triangle data
	mpTriangles = new tsTriangle[mnNumTriangles];
	if (!mpTriangles)
	{
		fprintf(stderr, "Unable to allocate memory for %u triangles "
			"required to store model data from file \"%s\"\n", 
			mnNumTriangles, apFilename);
		return -1;
	}
	
	// Read the triangle data from the STL file
	for (uint32_t cnt = 0; cnt < mnNumTriangles; cnt++)
	{
		elem_read = fread(&bin_stl_triangle, 1, 
			sizeof(bin_stl_triangle), file);
		if (sizeof(bin_stl_triangle) != elem_read)
		{
			fprintf(stderr, "Only read %lu of %lu bytes for "
				"triangle %u of %u from \"%s\" file.\n", 
				elem_read, sizeof(bin_stl_triangle), cnt, 
				mnNumTriangles, apFilename);
			return -1;
		}

		// Copy normal vector data
		mpTriangles[cnt].msNormal = bin_stl_triangle.msNormal;

		// (Potentially) add vertex data to array and get pointer
		//  to vertx data in mpVertices
		mpTriangles[cnt].mpVertex1 = 
			addVertex(&bin_stl_triangle.msVertex1);
		if (!mpTriangles[cnt].mpVertex1)
		{
			fprintf(stderr, "Could not add Vertex 1 of "
				"triangle %u of %u from \"%s\" file.\n", 
				cnt, mnNumTriangles, apFilename);
			return -1;
		}

		mpTriangles[cnt].mpVertex2 = 
			addVertex(&bin_stl_triangle.msVertex2);
		if (!mpTriangles[cnt].mpVertex2)
		{
			fprintf(stderr, "Could not add Vertex 2 of "
				"triangle %u of %u from \"%s\" file.\n", 
				cnt, mnNumTriangles, apFilename);
			return -1;
		}

		mpTriangles[cnt].mpVertex3 = 
			addVertex(&bin_stl_triangle.msVertex3);
		if (!mpTriangles[cnt].mpVertex3)
		{
			fprintf(stderr, "Could not add Vertex 3 of "
				"triangle %u of %u from \"%s\" file.\n", 
				cnt, mnNumTriangles, apFilename);
			return -1;
		}
	}

	if (fclose(file))
	{
		fprintf(stderr, "Failed to close file \"%s\" after reading "
			"data.\n", apFilename);
		return -1;
	}

//TODO: debug. Maybe turn this into a function, or break into functions for printing vertices, etc.?
	printf("%u unique vertices found amongst %u triangles.\n", 
		mnNumVerticies, mnNumTriangles);

	for (int cnt = 0; cnt < mnNumTriangles; cnt++)		
	{
		printf("Triangle %u:\n", cnt);
		printf("\tNormal Vec: %f %f %f\n", mpTriangles[cnt].msNormal.i, mpTriangles[cnt].msNormal.j, mpTriangles[cnt].msNormal.k);
		printf("\tVertex 1: %f %f %f\n", mpTriangles[cnt].mpVertex1->x, mpTriangles[cnt].mpVertex1->y, mpTriangles[cnt].mpVertex1->z);
		printf("\tVertex 2: %f %f %f\n", mpTriangles[cnt].mpVertex2->x, mpTriangles[cnt].mpVertex2->y, mpTriangles[cnt].mpVertex2->z);
		printf("\tVertex 3: %f %f %f\n", mpTriangles[cnt].mpVertex3->x, mpTriangles[cnt].mpVertex3->y, mpTriangles[cnt].mpVertex3->z);
	}

//TODO: Creat faces, where each trianlge is a face at this point?

	return 0;
}

/**
 * Add the given vertex data to mpVertices and return a pointer to that vertex.
 *  If the vertex data already exists in mpVertices, just return a pointer.
 *
 * \param[in] apVertex Vertex data to be copied into element in 
 *
 * \return Pointer to vertex data in mpVertices, or NULL on error.
 */
tcModelConv::tsVertex* tcModelConv::addVertex(const tsVertex* apVertex)
{
	tsVertex* vertex = NULL;
	int offset = 0;

	for (offset = 0; offset < mnNumVerticies; offset++)
	{
		// Compare all data in vertex struct
		if (apVertex->x == mpVertices[offset].x && 
			apVertex->y == mpVertices[offset].y && 
			apVertex->z == mpVertices[offset].z)
		{
			// Save pointer to vertex and break out of loop on match
			vertex = &mpVertices[offset];
			break;
		}
	}

	// If we made it to the end of the valid data in the array, the
	//  new vertex data and return pointer to that
	if (offset == mnNumVerticies)
	{
		// Check that there is room left in the array
		if (mnNumVerticies == mnVerticiesArraySize)
		{
			fprintf(stderr, "mpVertices (%u elements) is full, but "
				"we need to add another vertex to the array.\n",
				mnVerticiesArraySize);
			return NULL;
		}

		// Add the vertex to the array
		mpVertices[offset] = *apVertex;
		// Mark that there is one more valid vertex in the array
		mnNumVerticies++;
		// Return pointer to newly added vertex
		vertex = &mpVertices[offset];
	}

	return vertex;
}

/**
 * Combine triangles on the same plane (or triangles almost on the same plane based on threshold).
 *
 * \param anThreshold TODO
 * 
 * \return 0 on success.
 */
int tcModelConv::createFaces(float anThreshold)
{
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
	elem_wr = fwrite(&mnNumTriangles, 1, sizeof(mnNumTriangles), file);
	if (sizeof(mnNumTriangles) != elem_wr)
	{
		fprintf(stderr, "Only wrote %lu of %lu bytes from "
			"mnNumTriangles to file \"%s\"\n", elem_wr, 
			sizeof(mnNumTriangles), apFilename);
		return -1;
	}

	// Write triangle data
	for (uint32_t cnt = 0; cnt < mnNumTriangles; cnt++)
	{
		// Copy trianlge data to struct for writing
		bin_stl_triangle.msNormal = mpTriangles[cnt].msNormal;
		bin_stl_triangle.msVertex1 = *(mpTriangles[cnt].mpVertex1);
		bin_stl_triangle.msVertex2 = *(mpTriangles[cnt].mpVertex2);
		bin_stl_triangle.msVertex3 = *(mpTriangles[cnt].mpVertex3);
		bin_stl_triangle.mnAttrByteCnt = 0;

		// Write triangle data to file
		elem_wr = fwrite(&bin_stl_triangle, 1, sizeof(bin_stl_triangle),
			file);
		if (sizeof(bin_stl_triangle) != elem_wr)
		{
			fprintf(stderr, "Only wrote %lu of %lu bytes from "
				"%u of %u triangle to file \"%s\"\n", elem_wr, 
				sizeof(bin_stl_triangle), cnt, mnNumTriangles,
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

