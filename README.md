3D Model Converter
==================

Introduction
------------

The idea of this project is to create a command line application that takes
 common 3D object file types (i.e. stl and obj), isolates faces (i.e.
 connected triangles on the same plane) and then produces a file with the
 outlines of these faces so that they can be cut of material using a laser
 wood cutter and then assembled in the real world. 

Inputs
------

### Object Files

Below are outlined the supported file formats for importing 3D model data.

#### STL (STereoLithography) Files

##### ASCII STL

Planning support for parsing ASCII STL files. TODO.

##### Binary STL

Initially this application will only be able to handle STL files saved in
 binary format.

#### OBJ Files

Planning support for parsing OBJ files. TODO.

### Processing Parameters

TODO

Outputs
-------

### SVG (Scalable Vector Graphics)

The idea is to output an SVG file with the outlines of each face of the object
 so that they can be cut out and assembled from different materials (i.e. paper,
 cardboard). 

Edges will be labeled to aid in assembly. 

Planning option to automatically modify edges to form interlocking patterns
 to aid in assembly of model once faces have been cut. 

### Object Files

Output of object file is mostly for testing verification and debug. 

Output of entire object verifies object was read in correctly. 

Output of faces verifies that faces were isolated correctly.

