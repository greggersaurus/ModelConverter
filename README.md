3D Model Converter
==================

Introduction
------------

The idea of this project is to create a command line application that takes
 common 3D object file types (i.e. stl and obj files) and converts them into
 different formats and/or modifies the object data for future processing or
 conversion.

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
 cardboard). Various processing options are planned to automatically reduce
 the number of faces of an object to make assembly more reasonable. Edges will
 be labeled to aid in assembly. 

### Object Files

The application will able to output an object file in the case that 
 modifications are being made to the model (i.e. polygon reduction or selective
 scaling). TODO

