/*
 * library_link.cpp
 *
 *  Created on: Sep 13, 2013
 *      Author: kthierbach
 */

#include "library_link.hpp"

#include <string>

#include "alg/density.hpp"
#include "alg/graphcut.hpp"
#include "library_link_utilities.hpp"
#include "revision.hpp"
#include "templates/image.hpp"


DLLEXPORT int llGraphCut(WolframLibraryData libData, mint nargs, MArgument* input, MArgument output)
{
	elib::Image<int> *input_image;
	elib::Image<int> *binary_image;
	elib::Parameters params;
	MTensor binary_tensor;

//	int debug = 1;
//	while(debug);

	//get input
	input_image = elib::LibraryLinkUtilities<int>::llGetIntegerImage(libData, MArgument_getMTensor(input[0]), MArgument_getInteger(input[1]), 1);

	params.addParameter("C0", MArgument_getReal(input[2])); // c0
	params.addParameter("C1", MArgument_getReal(input[3])); // c1
	params.addParameter("Lambda1", MArgument_getReal(input[4])); // lambda1
	params.addParameter("Lambda2", MArgument_getReal(input[5])); // lambda2
	params.addParameter("Beta", MArgument_getReal(input[6])); // beta

	//compute cut
	binary_image = graphcut(*input_image, params);
	if(binary_image == NULL)
	{
		return LIBRARY_FUNCTION_ERROR;
	}

	//transform and write data to output
	mint dimensions[input_image->getRank()];
	std::copy(input_image->getDimensions(), input_image->getDimensions()+input_image->getRank(), dimensions);
	libData->MTensor_new(MType_Integer, input_image->getRank(), dimensions, &binary_tensor);
	std::copy(binary_image->getData(), binary_image->getData()+binary_image->getFlattenedLength(), libData->MTensor_getIntegerData(binary_tensor));
	MArgument_setMTensor(output,binary_tensor);

	delete input_image;
	delete binary_image;
	return LIBRARY_NO_ERROR;
}

DLLEXPORT int llDensity(WolframLibraryData libData, mint nargs, MArgument* input, MArgument output)
{
	elib::Parameters params;
	elib::Tensor<double> *points, *result;
	MTensor density;
	elib::Tensor<int> *dimensions, *original_dimensions;

//	int debug = 1;
//	while(debug);

	points=elib::LibraryLinkUtilities<double>::llGetRealTensor(libData,  MArgument_getMTensor(input[0]));
	dimensions=elib::LibraryLinkUtilities<int>::llGetIntegerTensor(libData,  MArgument_getMTensor(input[1]));
	params.addParameter("Dimensions", *dimensions);
	original_dimensions=elib::LibraryLinkUtilities<int>::llGetIntegerTensor(libData,  MArgument_getMTensor(input[2]));
	params.addParameter("OriginalDimensions", *original_dimensions);
	params.addParameter("Rank", 2);
	params.addParameter("Radius", MArgument_getReal(input[3]));
	params.addParameter("LateralProjectionRange", MArgument_getReal(input[4]));
	params.addParameter("BandWidth", MArgument_getReal(input[5]));
	params.addParameter("Type",int(MArgument_getInteger(input[6])));
	params.addParameter("CentralMeridian",MArgument_getReal(input[7]));
	params.addParameter("StandardParallel",MArgument_getReal(input[8]));

	result = elib::Density::calculateDensity(*points, params);
	if(result == nullptr)
	{
		return LIBRARY_FUNCTION_ERROR;
	}

	mint dims[result->getRank()];
	std::copy(result->getDimensions(), result->getDimensions()+result->getRank(), dims);
	libData->MTensor_new(MType_Real, result->getRank(), dims, &density);
	std::copy(result->getData(), result->getData()+result->getFlattenedLength(), libData->MTensor_getRealData(density));
	MArgument_setMTensor(output,density);

	delete points;
	delete dimensions;
	delete original_dimensions;
	delete result;
	return LIBRARY_NO_ERROR;
}

DLLEXPORT int llVersion(WolframLibraryData libData, mint nargs, MArgument* input, MArgument output)
{
	char *version = new char[1024];
#ifndef ELIB_REVISION
	strncpy(version,"Unknown",1024);
	MArgument_setUTF8String(output, version);
#else
	strncpy(version,ELIB_REVISION, 1024);
	MArgument_setUTF8String(output, version);
#endif
	return LIBRARY_NO_ERROR;
}
