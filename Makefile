###############################################################################
# Makefile for compiling 3D Model Converter Application
###############################################################################

# Set the compiler being used
CC = g++

# Include directories
INCLUDES =

# Linker flags
LDFLAGS =

# Compiler flags (enable as verbose warning output as possible)
CFLAGS = -g -c -Wall -Wconversion $(INCLUDES) -std=gnu++0x

# Project source files
SOURCES = modelconv.cpp \
	main.cpp

OBJECTS = $(SOURCES:.cpp=.o)
EXECUTABLE = mdlconv

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC)  $(OBJECTS) -o $@ $(LDFLAGS)

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf $(OBJECTS) $(EXECUTABLE)
