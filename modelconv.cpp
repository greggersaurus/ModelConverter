/**
 * \file modelconv.cpp
 * \brief Class to handle parsing and converting 3D model data.
 * \author Gregory Gluszek.
 */

#include "modelconv.h"

#include <stdio.h>
#include <string.h>
//TODO: Added for use of exit() which is cheap way around not using exceptions for initial work on this class. FIXME
#include <stdlib.h>

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(*(x))) //!< Used for calculating      
	//!< static array sizes

/**
 * Constructor.
 *
 * \param[in] apFilename File containing 3D model data.
 */
tcModelConv::tcModelConv(const char* apFilename)
: mcVertices({})
, mcTriangles({})
, mcFaces({})
{
	FILE* file = NULL;
	// Number of elements read by fread
	size_t elem_read = 0;
	uint32_t num_triangles = 0;
	// Used when building faces to know if we're already included a triangle
	std::unordered_map<void*, int> tri_face_map = {};

//TODO: Add code to check file type, etc. For now only support binary STL. Will add functions for parsing different file types later?

	tsBinStlTriangle bin_stl_triangle;

	memset(maBinStlHeader, 0, ARRAY_SIZE(maBinStlHeader));

	// Open file for reading
	file = fopen(apFilename, "r");	
	if (!file)
	{
		fprintf(stderr, "Failed to open file \"%s\"\n", apFilename);
		//TODO: add proper exception throwing
		exit(EXIT_FAILURE);
	}	

	// Read header data from STL file
	elem_read = fread(maBinStlHeader, sizeof(maBinStlHeader[0]), 
		ARRAY_SIZE(maBinStlHeader), file);
	if (ARRAY_SIZE(maBinStlHeader) != elem_read)
	{
		fprintf(stderr, "Only able to read %lu of %lu header bytes from"
			" file \"%s\"\n", elem_read, ARRAY_SIZE(maBinStlHeader),
			apFilename);
		fclose(file);
		//TODO: add proper exception throwing
		exit(EXIT_FAILURE);
	}

	// Read number of triangles in file from the STL file
	elem_read = fread(&num_triangles, 1, sizeof(num_triangles), file);
	if (sizeof(num_triangles) != elem_read)
	{
		fprintf(stderr, "Only able to read %lu of %lu num triangle "
			"field bytes from file \"%s\"\n", elem_read, 
			ARRAY_SIZE(maBinStlHeader), apFilename);
		fclose(file);
		//TODO: add proper exception throwing
		exit(EXIT_FAILURE);
	}
	
	// Clean up vertex storage memory
	mcVertices.clear();
	// If object is closed, there will be at one vertex per triangle. 
	//  Start off with vector of this size to minimize dynamic resizing.
	mcVertices.reserve(num_triangles);

	// Clean up triangle storage memory
	mcTriangles.clear();
	// We know exactly how many triangle there are and this should make sure
	//  we allocate entries for all of them now
	mcTriangles.resize(num_triangles);
	
	// Read the triangle data from the STL file
	for (uint32_t newest_idx = 0; newest_idx < num_triangles; newest_idx++)
	{
		elem_read = fread(&bin_stl_triangle, 1, 
			sizeof(bin_stl_triangle), file);
		if (sizeof(bin_stl_triangle) != elem_read)
		{
			fprintf(stderr, "Only read %lu of %lu bytes for "
				"triangle %u of %u from \"%s\" file.\n", 
				elem_read, sizeof(bin_stl_triangle), newest_idx, 
				num_triangles, apFilename);
			fclose(file);
			//TODO: add proper exception throw
			exit(EXIT_FAILURE);
		}

		// Copy normal vector data
		mcTriangles[newest_idx].msNormal = bin_stl_triangle.msNormal;

		// Potentially add vertex data to array and get pointer
		//  to vertx data in mpVertices
		mcTriangles[newest_idx].mpVertices[0] = 
			&addVertex(bin_stl_triangle.maVertices[0]);
		mcTriangles[newest_idx].mpVertices[1] = 
			&addVertex(bin_stl_triangle.maVertices[1]);
		mcTriangles[newest_idx].mpVertices[2] = 
			&addVertex(bin_stl_triangle.maVertices[2]);

		// Start off assuming new triangle has no neighbors
		mcTriangles[newest_idx].mpNeighbors[0] = NULL;
		mcTriangles[newest_idx].mpNeighbors[1] = NULL;
		mcTriangles[newest_idx].mpNeighbors[2] = NULL;

		// Search for neighbors for newest triangle
		for (uint32_t older_idx = 0; older_idx < newest_idx; 
			older_idx++)
		{
			checkAdjacent(mcTriangles[newest_idx], 
				mcTriangles[older_idx]);
		}

		// Create zeroed entry to face building later
		tri_face_map[&mcTriangles[newest_idx]] = 0;
	}

	if (fclose(file))
	{
		fprintf(stderr, "Failed to close file \"%s\" after reading "
			"data.\n", apFilename);
		//TODO: add proper exception throwing
		exit(EXIT_FAILURE);
	}

	// Create faces now that we have graph representing all triangles
	for (std::vector<tsTriangle>::iterator it = mcTriangles.begin();
		it != mcTriangles.end(); it++) 
	{
		// When building a face this keeps track of whether we have
		//  visited the triangle already or not
		// TODO: zero out map for all possible entries?
		std::unordered_map<void*, int> tri_trav_map = {};
		tsTriangle* triangle = &(*it);
		//TODO: make pointer
		tsFace face;

		// Skip this triangle if it is already included in a face
		if (tri_face_map[triangle])
			continue;

		// Start a new face
		face.mcBorder = {};

		// Mark that we have added this triangle to a face
		tri_face_map[triangle]++;	
		// Mark that we have touched this triangle as part of building 
		//  the face
		tri_trav_map[triangle]++;

		// BFS finding edges where triangles are not on same plane

//TODO: traverse all neighbors (and neighbors of neighbors) that are on the same plane. Save vertices that are shared with triangles not on same plane as they form border.

	
	}
}

/**
 * Destructor.
 */
tcModelConv::~tcModelConv()
{
}

/**
 * TODO: want to print useful info. but shoudl this be to_string?
 */
void tcModelConv::debugPrint()
{
	printf("%lu unique vertices found amongst %lu triangles.\n\n", 
		mcVertices.size(), mcTriangles.size());

	uint32_t cnt = 0;
	for (std::vector<tsTriangle>::iterator it = mcTriangles.begin();
		it != mcTriangles.end(); it++) 
	{
		printf("Triangle %u (%p): %s\n", cnt++, (void*)&(*it), to_string(*it).c_str());
		for (int n_cnt = 0; n_cnt < 3; n_cnt++)
		{
			printf("\tNeighbor %d = (%p)\n", n_cnt, (void*)it->mpNeighbors[n_cnt]);
		}
	}
	printf("\n");
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
	return "Normal: (" + to_string(arTriangle.msNormal) + ") " + 
		"Vertex 1: (" + to_string(*arTriangle.mpVertices[0]) + ") " + 
		"Vertex 2: (" + to_string(*arTriangle.mpVertices[1]) + ") " + 
		"Vertex 3: (" + to_string(*arTriangle.mpVertices[2]) + ") ";
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
		// Compare vertex 
		if (arVertex == *it)
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
 * Check if the two triangle are adjacent (i.e. shares two vertices). If they 
 *  are, add them to each others neighbors list.
 *
 * \param arTri1 
 * \param arTri2
 *
 * \return None.
 */
void tcModelConv::checkAdjacent(tsTriangle& arTri1, tsTriangle& arTri2)
{
	// Bits 0-2 indicates which verticies are shared for each triangle.
	//  Used for knowing where to position triangle as neighbor.
	uint8_t tri1_shared = 0;
	uint8_t tri2_shared = 0;

	for (int t1_cnt = 0; t1_cnt < 3; t1_cnt++)
	{
		for (int t2_cnt = 0; t2_cnt < 3; t2_cnt++)
		{
			// We can compare pointers here because mpVertices 
			//  point to mcVertices which only has unique entries
			//  for each point in space.
			if (arTri1.mpVertices[t1_cnt] == 
				arTri2.mpVertices[t2_cnt])
			{
				tri1_shared |= (uint8_t)(1 << t1_cnt);
				tri2_shared |= (uint8_t)(1 << t2_cnt);
				break;
			}
		}
	}

	if (tri1_shared >= 0x7 || tri2_shared >= 0x7)
	{
		fprintf(stderr, "%s: Triangles have all the same vertices.\n", 
			__func__);
		//TODO: add exception throw
		exit(EXIT_FAILURE);
	}

	addNeighbor(arTri1, arTri2, tri1_shared);
	addNeighbor(arTri2, arTri1, tri2_shared);
}

/**
 * Add arTri2 as a neighbor to arTri1. Neighbor location is based on anShared
 *  bit field.
 *
 * \param arTri1
 * \param arTri2 
 * \param anShared 
 * 
 * \return None.
 */
void tcModelConv::addNeighbor(tsTriangle& arTri1, tsTriangle& arTri2, uint8_t anShared)
{
	switch (anShared)
	{
		case 0x3:
			if (arTri1.mpNeighbors[0])
			{
				fprintf(stderr, "Neighbor not NULL\n");
				//TODO: add proper exception throwing
				exit(EXIT_FAILURE);
			}
			arTri1.mpNeighbors[0] = &arTri2;
			break;
		case 0x5:
			if (arTri1.mpNeighbors[1])
			{
				fprintf(stderr, "Neighbor not NULL\n");
				//TODO: add proper exception throwing
				exit(EXIT_FAILURE);
			}
			arTri1.mpNeighbors[1] = &arTri2;
			break;
		case 0x6:
			if (arTri1.mpNeighbors[2])
			{
				fprintf(stderr, "Neighbor not NULL\n");
				//TODO: add proper exception throwing
				exit(EXIT_FAILURE);
			}
			arTri1.mpNeighbors[2] = &arTri2;
			break;
	}
}

/**
 * Insert vertices common to triangles into border of face.
 *
 * \param[out] arFace Vertices will be added to this face.
 * \param[in] arTri1
 * \param[in] arTri2
 * 
 * \return None.
 */
void tcModelConv::insertVertex(tsFace& arFace, const tsTriangle& arTri1, 
	const tsTriangle& arTri2)
{
	// Find two common verticies

	// Insert into border in appropriate location
}

/**
 * Search through neighbors of triangle
 */
void tcModelConv::buildFace(tsFace& arFace, 
	std::unordered_map<void*, int>& arTravMap, const tsNormal& arNorm, 
	const tsTriangle& arTri)
{
	tsTriangle* neighbor = NULL;

	for (int cnt = 0; cnt < 3; cnt++)
	{
		if (!neighbor)
			//TODO: Need to add vertices on open edge
			continue;

		if (arNorm == neighbor->msNormal)
		{
			// Keep searching for edge of face
			buildFace(arFace, arTravMap, arNorm, *neighbor);
		}
		else
		{
			// Found an edge of the face
			insertVertex(arFace, arTri, *neighbor);
		}
	}			
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
		bin_stl_triangle.maVertices[0] = *(mcTriangles[cnt].mpVertices[0]);
		bin_stl_triangle.maVertices[1] = *(mcTriangles[cnt].mpVertices[1]);
		bin_stl_triangle.maVertices[2] = *(mcTriangles[cnt].mpVertices[2]);
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

