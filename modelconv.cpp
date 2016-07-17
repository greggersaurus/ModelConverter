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
: mnNumTriangles(0)
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

//TODO: Add code to check file type, etc. For now only support binary STL

	// Open file for reading
	file = fopen(apFilename, "r");	
	if (!file)
	{
		fprintf(stderr, "Failed to open file \"%s\"\n", apFilename);
		return -1;
	}	

	// Read header
	elem_read = fread(maBinStlHeader, sizeof(maBinStlHeader[0]), 
		ARRAY_SIZE(maBinStlHeader), file);
	if (ARRAY_SIZE(maBinStlHeader) != elem_read)
	{
		fprintf(stderr, "Only able to read %lu of %lu header bytes from"
			" file \"%s\"\n", elem_read, ARRAY_SIZE(maBinStlHeader),
			apFilename);
		return -1;
	}
	
	// Read number of triangles in file
	elem_read = fread(&mnNumTriangles, 1, sizeof(mnNumTriangles), file);
	if (sizeof(mnNumTriangles) != elem_read)
	{
		fprintf(stderr, "Only able to read %lu of %lu num triangle "
			"field bytes from file \"%s\"\n", elem_read, 
			ARRAY_SIZE(maBinStlHeader), apFilename);
		return -1;
	}

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
	
	// Read the triangle data
	elem_read = fread(mpTriangles, sizeof(mpTriangles[0]), mnNumTriangles,
		file);
	if (mnNumTriangles != elem_read)
	{
		fprintf(stderr, "Only read %lu of %u triangles from "
			"\"%s\" file.\n", elem_read, mnNumTriangles, 
			apFilename);
		return -1;
	}


//TODO: debug. Maybe turn this into a function, or break into functions for printing vertices, etc.?
	for (int cnt = 0; cnt < mnNumTriangles; cnt++)		
	{
		printf("Triangle %u:\n", cnt);
		printf("\tNormal Vec: %f %f %f\n", mpTriangles[cnt].msNormal.i, mpTriangles[cnt].msNormal.j, mpTriangles[cnt].msNormal.k);
		printf("\tVertex 1: %f %f %f\n", mpTriangles[cnt].msVertex1.x, mpTriangles[cnt].msVertex1.y, mpTriangles[cnt].msVertex1.z);
		printf("\tVertex 2: %f %f %f\n", mpTriangles[cnt].msVertex2.x, mpTriangles[cnt].msVertex2.y, mpTriangles[cnt].msVertex2.z);
		printf("\tVertex 3: %f %f %f\n", mpTriangles[cnt].msVertex3.x, mpTriangles[cnt].msVertex3.y, mpTriangles[cnt].msVertex3.z);
	}

	return 0;
}

