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
 * \param[in] filename File containing 3D model data.
 */
ModelConv::ModelConv(const char* filename)
: vertices({})
, triangles({})
, faces({})
{
	FILE* file = NULL;
	// Number of elements read by fread
	size_t elem_read = 0;
	uint32_t num_triangles = 0;
	// Used when building faces to know if we've already included a triangle in a face
	std::unordered_map<const Triangle*, int> tri_face_map = {};
	// Used when building a face to not repeat visits for that particular face.
	std::unordered_map<const Triangle*, int> tri_trav_map = {};
	Face face;

//TODO: Add code to check file type, etc. For now only support binary STL. Will add functions for parsing different file types later?

	BinStlTriangle bin_stl_triangle;

	memset(binStlHeader, 0, ARRAY_SIZE(binStlHeader));

	// Open file for reading
	file = fopen(filename, "r");	
	if (!file)
	{
		fprintf(stderr, "Failed to open file \"%s\"\n", filename);
		//TODO: add proper exception throwing
		exit(EXIT_FAILURE);
	}	

	// Read header data from STL file
	elem_read = fread(binStlHeader, sizeof(binStlHeader[0]), 
		ARRAY_SIZE(binStlHeader), file);
	if (ARRAY_SIZE(binStlHeader) != elem_read)
	{
		fprintf(stderr, "Only able to read %lu of %lu header bytes from"
			" file \"%s\"\n", elem_read, ARRAY_SIZE(binStlHeader),
			filename);
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
			ARRAY_SIZE(binStlHeader), filename);
		fclose(file);
		//TODO: add proper exception throwing
		exit(EXIT_FAILURE);
	}
	
	// If object is closed, there will be at one vertex per triangle. 
	//  Start off with vector of this size to minimize dynamic resizing.
	vertices.reserve(num_triangles);

	// We know exactly how many triangle there are and this should make sure
	//  we allocate entries for all of them now
	triangles.resize(num_triangles);
	
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
				num_triangles, filename);
			fclose(file);
			//TODO: add proper exception throw
			exit(EXIT_FAILURE);
		}

		// Copy normal vector data
//TODO: is this safe? Does resize call default constructors and allow us reference "new" objects this way?
		triangles[newest_idx].normal = bin_stl_triangle.normal;

		// Potentially add vertex data to array and get pointer
		//  to vertx data in vertices
		triangles[newest_idx].vertices[0] = 
			&addVertex(bin_stl_triangle.vertices[0]);
		triangles[newest_idx].vertices[1] = 
			&addVertex(bin_stl_triangle.vertices[1]);
		triangles[newest_idx].vertices[2] = 
			&addVertex(bin_stl_triangle.vertices[2]);

		// Start off assuming new triangle has no neighbors
		triangles[newest_idx].neighbors[0] = NULL;
		triangles[newest_idx].neighbors[1] = NULL;
		triangles[newest_idx].neighbors[2] = NULL;

		// Search for neighbors for newest triangle
		for (uint32_t older_idx = 0; older_idx < newest_idx; 
			older_idx++)
		{
			checkAdjacent(triangles[newest_idx], 
				triangles[older_idx]);
		}
	}

	if (fclose(file))
	{
		fprintf(stderr, "Failed to close file \"%s\" after reading "
			"data.\n", filename);
		//TODO: add proper exception throwing
		exit(EXIT_FAILURE);
	}

	// Create faces now that we have graph representing all triangles
	for (std::vector<Triangle>::iterator itr = triangles.begin();
		itr != triangles.end(); itr++) 
	{
		Triangle* triangle = &(*itr);

		// Skip constructing a face starting at this triangle, if it is
		//  already in a face
		if (tri_face_map[triangle])
			continue;

		// Clear our traversal map for this face
		tri_trav_map.clear();

		face.normal = triangle->normal;
		face.triangles.clear();
		face.border.clear();

		// BFS finding edges where triangles are not on same plane
		buildFace(*triangle, face, tri_face_map, tri_trav_map);

		faces.push_back(face);
	}
}

/**
 * Destructor.
 */
ModelConv::~ModelConv()
{
}

/**
 * TODO: want to print useful info. but shoudl this be to_string?
 */
void ModelConv::debugPrint()
{
	printf("%lu unique vertices found amongst %lu triangles.\n\n", 
		vertices.size(), triangles.size());

	uint32_t cnt = 0;
	for (std::vector<Triangle>::iterator it = triangles.begin();
		it != triangles.end(); it++) 
	{
		printf("Triangle %u (%p): %s\n", cnt++, (void*)&(*it), to_string(*it).c_str());
		for (int n_cnt = 0; n_cnt < 3; n_cnt++)
		{
			printf("\tNeighbor %d = (%p)\n", n_cnt, (void*)it->neighbors[n_cnt]);
		}
	}
	printf("\n");
}

/**
 * \return The string representation of a Normal.
 */
std::string ModelConv::to_string(const Normal& normal)
{
	return "i = " + std::to_string(normal.i) + 
		" j = " + std::to_string(normal.j) + 
		" k = " + std::to_string(normal.k);	
}

/**
 * \return The string representation of a Vertex.
 */
std::string ModelConv::to_string(const Vertex& vertex)
{
	return "x = " + std::to_string(vertex.x) + 
		" y = " + std::to_string(vertex.y) + 
		" z = " + std::to_string(vertex.z);	
}

/**
 * \return The string representation of a Triangle.
 */
std::string ModelConv::to_string(const Triangle& triangle)
{
	return "Normal: (" + to_string(triangle.normal) + ") " + 
		"Vertex 1: (" + to_string(*triangle.vertices[0]) + ") " + 
		"Vertex 2: (" + to_string(*triangle.vertices[1]) + ") " + 
		"Vertex 3: (" + to_string(*triangle.vertices[2]) + ") ";
}


/**
 * Add the given vertex data to vertices and return a pointer to that vertex.
 *  If the vertex data already exists in vertices, just return a pointer.
 *
 * \param[in] vertex Vertex data to be copied into element in 
 *
 * \return Reference to vertex data in vertices.
 */
ModelConv::Vertex& ModelConv::addVertex(const Vertex& vertex)
{
//TODO: it would be more efficient to use a map here, so we don't iterate over everything we have each time we add
//	need to define hash function for vertex in order to use map

	// Search to see if vertex already exists in vector
	for (std::vector<Vertex>::iterator itr = vertices.begin();
		itr != vertices.end(); itr++)
	{
		// Compare vertex 
		if (vertex == *itr)
		{
			return *itr;
		}
	}

	// If we made it out of the loop without exiting the function we did
	//  did not find the vertex in the vector already and need to add it
	vertices.push_back(vertex);

	// Return pointer to newly added element
	return vertices.back();
}

/**
 * Check if the two triangle are adjacent (i.e. shares two vertices, also known
 *  as an edge). If they are, add them to each others neighbors list.
 *
 * \param[inout] tri1 First triangle given for check.
 * \param[inout] tri2 Second triangle given for check.
 *
 * \return None.
 */
void ModelConv::checkAdjacent(Triangle& tri1, Triangle& tri2)
{
	// Bits 0-2 indicates which verticies are shared for each triangle.
	//  Used for knowing where to position triangle as neighbor.
	uint8_t tri1_shared = 0;
	uint8_t tri2_shared = 0;

	for (int t1_cnt = 0; t1_cnt < 3; t1_cnt++)
	{
		for (int t2_cnt = 0; t2_cnt < 3; t2_cnt++)
		{
			// We can compare pointers here because vertices 
			//  point to vertices which only has unique entries
			//  for each point in space.
			if (tri1.vertices[t1_cnt] == tri2.vertices[t2_cnt])
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

	addNeighbor(tri1, tri2, tri1_shared);
	addNeighbor(tri2, tri1, tri2_shared);
}

/**
 * Add in neighboring triangle. Triangles are added to into array based on
 *  which edge/vertices are shared.
 *
 * \param[inout] tri The triangle who will have a neighbor slot filled in.
 * \param[in] neighbor The triangle who is a neighbor to tri. 
 * \param sharedVtxs Bits 0-2 designate which vertices are shared, and hence in
 *	which neighbor slot neighbor should be placed in tri.
 * 
 * \return None.
 */
void ModelConv::addNeighbor(Triangle& tri, Triangle& neighbor, 
	uint8_t sharedVtxs)
{
	switch (sharedVtxs)
	{
		case 0x3:
			if (tri.neighbors[0])
			{
				fprintf(stderr, "%s: Neighbor 0 not NULL\n", 
					__func__);
				//TODO: add proper exception throwing
				exit(EXIT_FAILURE);
			}
			tri.neighbors[0] = &neighbor;
			break;
		case 0x5:
			if (tri.neighbors[1])
			{
				fprintf(stderr, "%s: Neighbor 1 not NULL\n", 
					__func__);
				//TODO: add proper exception throwing
				exit(EXIT_FAILURE);
			}
			tri.neighbors[1] = &neighbor;
			break;
		case 0x6:
			if (tri.neighbors[2])
			{
				fprintf(stderr, "%s: Neighbor 2 not NULL\n", 
					__func__);
				fprintf(stderr, "Neighbor not NULL\n");
				//TODO: add proper exception throwing
				exit(EXIT_FAILURE);
			}
			tri.neighbors[2] = &neighbor;
			break;
		default:
			fprintf(stderr, "%s: Invalid bit combination 0x%x\n", 
				__func__, sharedVtxs);
			//TODO: add proper exception throwing
			exit(EXIT_FAILURE);
			break;
	}
}

/**
 * Find all connected triangle on the same plane. 
 *
 * \param[in] tri Triangle to add to face.
 * \param[inout] face Face that is in process of being built up.
 * \param[inout] faceMap Keeps track of triangles already part of a face.
 * \param[inout] travMap Keeps track of triangles already checked for building
 * 	this particular face.
 *
 * \return None.
 */
void ModelConv::buildFace(const Triangle& tri, Face& face, 
	std::unordered_map<const Triangle*, int>& faceMap,
	std::unordered_map<const Triangle*, int>& travMap)
{
	// Make sure we have no already added this face
	if (faceMap[&tri])
		return;;

	faceMap[&tri] = 1;
	travMap[&tri] = 1;	
	face.triangles.push_back(&tri);

	for (int cnt = 0; cnt < 3; cnt++)
	{
		Triangle* neighbor = tri.neighbors[cnt];

		if (!neighbor)
		{
			insertVertex(face, tri, cnt);
			continue;
		}

		// Make sure we have not already visited this triangle for this
		//  face construction
		if (travMap[neighbor])
			continue;
	
		if (tri.normal == neighbor->normal)
		{
			// Neighbor is face of this face
			buildFace(*neighbor, face, faceMap, travMap);
		}
		else
		{
			// neighbor is not on this face
			travMap[neighbor] = 1;
			insertVertex(face, tri, cnt);
		}
	}			
}

/**
 * Insert vertices common to triangles into border of face.
 *
 * \param[inout] face Face for which vertices are being inserted into border.
 * \param[in] tri Triangle in face that has a border edge.
 * \param[in] neighborIndex Index of neighbor in tri that is edge of face, 
 *	which indicates which vertices to add to face border.
 * 
 * \return None.
 */
void ModelConv::insertVertex(Face& face, const Triangle& tri, int neighborIndex)
{
	Vertex* vtx1 = NULL;
	Vertex* vtx2 = NULL;
	
	switch(neighborIndex)
	{
//TODO: could calc index with math instead of switch statement
		case 0:
			vtx1 = tri.vertices[0];
			vtx2 = tri.vertices[1];
			break;
		case 1:
			vtx1 = tri.vertices[1];
			vtx2 = tri.vertices[2];
			break;
		case 2:
			vtx1 = tri.vertices[2];
			vtx2 = tri.vertices[0];
			break;
		default:
			fprintf(stderr, "%s: Invalid bit neighbor index %d\n", 
				__func__, neighborIndex);
			//TODO: add proper exception throwing
			exit(EXIT_FAILURE);
			break;
	}

	// Insert into border in appropriate location
//TODO
}

/**
 * Export model data to STL file format with binary data.
 *
 * \param[in] filename Filename to write STL data to.
 *
 * \return 0 on success.
 */
int ModelConv::exportBinStl(const char* filename)
{
	FILE* file = NULL;
	// Number of elements written by fwrite
	size_t elem_wr = 0;
	BinStlTriangle bin_stl_triangle;
	uint32_t num_triangles = (uint32_t)triangles.size();

	file = fopen(filename, "w");
	if (!file)
	{
		fprintf(stderr, "Failed to open file \"%s\" for writing.\n",
			filename);
		return -1;
	}

	// Write header data
	elem_wr = fwrite(binStlHeader, sizeof(binStlHeader[0]),
		ARRAY_SIZE(binStlHeader), file);
	if (ARRAY_SIZE(binStlHeader) != elem_wr)
	{
		fprintf(stderr, "Only wrote %lu of %lu bytes from "
			"binStlHeader to file \"%s\"\n", elem_wr, 
			ARRAY_SIZE(binStlHeader), filename);
		return -1;
	}

	// Write number of triangle
	elem_wr = fwrite(&num_triangles, 1, sizeof(num_triangles), file);
	if (sizeof(num_triangles) != elem_wr)
	{
		fprintf(stderr, "Only wrote %lu of %lu bytes from "
			"num_triangles to file \"%s\"\n", elem_wr, 
			sizeof(num_triangles), filename);
		return -1;
	}

	// Write triangle data
	for (uint32_t cnt = 0; cnt < num_triangles; cnt++)
	{
		// Copy trianlge data to struct for writing
		bin_stl_triangle.normal = triangles[cnt].normal;
		bin_stl_triangle.vertices[0] = *(triangles[cnt].vertices[0]);
		bin_stl_triangle.vertices[1] = *(triangles[cnt].vertices[1]);
		bin_stl_triangle.vertices[2] = *(triangles[cnt].vertices[2]);
		bin_stl_triangle.attrByteCnt = 0;

		// Write triangle data to file
		elem_wr = fwrite(&bin_stl_triangle, 1, sizeof(bin_stl_triangle),
			file);
		if (sizeof(bin_stl_triangle) != elem_wr)
		{
			fprintf(stderr, "Only wrote %lu of %lu bytes from "
				"%u of %u triangle to file \"%s\"\n", elem_wr, 
				sizeof(bin_stl_triangle), cnt, num_triangles,
				filename);
			return -1;
		}
		
	}

	if (fclose(file))
	{
		fprintf(stderr, "Failed to close file \"%s\" after writing "
			"data.\n", filename);
		return -1;
	}

	return 0;
}

/**
 * Output Scalable Vector Graphs with each face as an outlined object.
 *
 * \param[in] filename Filename to write SVG data to.
 *
 * \return 0 on success.
 */
int ModelConv::exportSvg(const char* filename)
{
//TODO: what if there are no faces created, or will they be created as soon as data is imported??
//TODO: don't forget we need to map to from X,Y,Z to X,Y plane

	return -1;
}

