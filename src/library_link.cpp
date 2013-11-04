/*
 * library_link.cpp
 *
 *  Created on: Sep 13, 2013
 *      Author: kthierbach
 */

#include "library_link.hpp"

#include "alg/graphcut.hpp"
#include "library_link_utilities.hpp"
#include "revision.hpp"
#include "templates/image.hpp"


DLLEXPORT int llGraphCut(WolframLibraryData libData, mint nargs, MArgument* input, MArgument output)
{
	elib::Image<float> *input_image;
	elib::Image<int> *binary_image;
	elib::Parameters params;
	MTensor binary_tensor;

//	int debug = 1;
//	while(debug);

	//get input
	input_image = elib::LibraryLinkUtilities<float>::llGetRealImage(libData, MArgument_getMTensor(input[0]), MArgument_getInteger(input[1]), 1);

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
	libData->MTensor_new(MType_Integer, input_image->getRank(), reinterpret_cast<const mint*>(input_image->getDimensions()), &binary_tensor);
	std::copy(binary_image->getData(), binary_image->getData()+binary_image->getFlattenedLength(), libData->MTensor_getIntegerData(binary_tensor));
	MArgument_setMTensor(output,binary_tensor);

	delete input_image;
	delete binary_image;
	return LIBRARY_NO_ERROR;
}

DLLEXPORT int llVersion(WolframLibraryData libData, mint nargs, MArgument* input, MArgument output)
{
#ifndef ELIB_REVISION
	char version[10] = "Unknown";
	MArgument_setUTF8String(output, version);
#else
	char version[10] = ELIB_REVISION;
	MArgument_setUTF8String(output, version);
#endif
	return LIBRARY_NO_ERROR;
}
