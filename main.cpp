/**
 * \file main.cpp
 * \brief Entry point for 3D Model Converter command line application.
 * \author Gregory Gluszek.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string>
#include <getopt.h>

#include "modelconv.h"

/**
 * Print application usage to stderr.
 *
 * \param[in] apExeName Executable name.
 *
 * \return None.
 */
void print_usage(const char* apExeName)
{
	//TODO
}

/**
 * Command line application entry point.
 *
 * \param argc Number of command line arguments.
 * \param argv Array of command line argument strings.
 *
 * \return EXIT_SUCCESS or EXIT_FAILURE.
 */
int main(int argc, char* argv[])
{
	std::string input_file = "";
	ModelConv* model_conv = NULL; 

	// For command line arg parsing
	int opt;
	int option_index = 0;
	static struct option long_options[] =
	{
		{"input-file", required_argument, 0, 'i'},
		{"help", no_argument, 0, 'h'}
	};

	// Parse command line arguments
	while ((opt = getopt_long(argc, argv, "i:h", long_options,
		&option_index)) != -1)
	{
		switch (opt) {
			case 'i':
				input_file = optarg;
				printf("Input File = %s\n", input_file.c_str());
				break;

			case 'h':
				print_usage(argv[0]);
				exit(EXIT_SUCCESS);
				break;

			default: /* '?' */
				fprintf(stderr, "Unknown input argument.\n");
				print_usage(argv[0]);
				exit(EXIT_FAILURE);
		}
	}

	printf("Hello Wurld\n");

	// Create class to process 3D model data
	model_conv = new ModelConv(input_file.c_str());
	if (!model_conv)
	{
		fprintf(stderr, "Failed to allocate memory for model_conv. "
			"Exiting.\n");
		exit(EXIT_FAILURE);
	}

//	model_conv->debugPrint();

	exit(EXIT_SUCCESS);
}
