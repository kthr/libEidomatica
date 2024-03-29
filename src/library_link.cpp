/*
 * library_link.cpp
 *
 *  Created on: Sep 13, 2013
 *      Author: kthierbach
 */

#include "library_link.hpp"

#include <algorithm>
#include <string>
#include <vector>

#include "alg/alpha_shapes.hpp"
#include "alg/bounding_volumes.hpp"
#include "alg/delaunay_triangulation.hpp"
#include "alg/density.hpp"
#include "alg/graphcut.hpp"
#include "alg/multi_label_graphcut.hpp"
#include "io/hdf5_reader.hpp"
#include "io/hdf5_wrapper.hpp"
#include "library_link_utilities.hpp"
#include "revision.hpp"
#include "templates/image.hpp"
#include "templates/tensor.hpp"

DLLEXPORT int llAlphaShape(WolframLibraryData libData, mint nargs, MArgument* input, MArgument output)
{
	using elib::Tensor;
	using elib::AlphaShapes;
	std::vector<float> vec;
	MTensor segments;

	std::shared_ptr<Tensor<float>> points;
	float alpha;
	bool mode;

//	int debug = 1;
//	while(debug);

	points = elib::LibraryLinkUtilities<float>::llGetRealTensor(libData, MArgument_getMTensor(input[0]));
	alpha = MArgument_getReal(input[1]);
	mode = MArgument_getBoolean(input[2]);

	AlphaShapes as(points, alpha, mode);
	vec = as.getSegments();

	mint dimensions = vec.size();
	libData->MTensor_new(MType_Real, 1, &dimensions, &segments);
	std::copy(vec.begin(), vec.end(), libData->MTensor_getRealData(segments));
	MArgument_setMTensor(output, segments);


	return LIBRARY_NO_ERROR;
}

DLLEXPORT int llBoundingVolumes(WolframLibraryData libData, mint nargs, MArgument* input, MArgument output)
{
	using elib::Tensor;
	using elib::BoundingVolumes;
	std::vector<double> vec;
	MTensor segments;

	std::shared_ptr<Tensor<float>> points;
	int volume_type;
	bool mode;

//	int debug = 1;
//	while(debug);

	points = elib::LibraryLinkUtilities<float>::llGetRealTensor(libData, MArgument_getMTensor(input[0]));
	volume_type = MArgument_getInteger(input[1]);
	mode = MArgument_getBoolean(input[2]);

	BoundingVolumes bv(points, volume_type, mode);
	vec = bv.getBoundary();

	mint dimensions = vec.size();
	libData->MTensor_new(MType_Real, 1, &dimensions, &segments);
	std::copy(vec.begin(), vec.end(), libData->MTensor_getRealData(segments));
	MArgument_setMTensor(output, segments);


	return LIBRARY_NO_ERROR;
}

DLLEXPORT int llDelaunay(WolframLibraryData libData, mint nargs, MArgument* input, MArgument output)
{
	using elib::Tensor;
	using elib::DelaunayTriangulation;
	std::vector<float> vec;
	MTensor segments;
	std::shared_ptr<Tensor<float>> points;

//	int debug = 1;
//	while(debug);

	points = elib::LibraryLinkUtilities<float>::llGetRealTensor(libData, MArgument_getMTensor(input[0]));

	DelaunayTriangulation dt(points);
	vec = dt.getTriangulation();

	mint dimensions = vec.size();
	libData->MTensor_new(MType_Real, 1, &dimensions, &segments);
	std::copy(vec.begin(), vec.end(), libData->MTensor_getRealData(segments));
	MArgument_setMTensor(output, segments);


	return LIBRARY_NO_ERROR;
}

DLLEXPORT int llGraphCut(WolframLibraryData libData, mint nargs, MArgument* input, MArgument output)
{
	elib::Image<int> *input_image;
	elib::Image<short> *binary_image;
	elib::Parameters params;
	MTensor binary_tensor;

//	int debug = 1;
//	while(debug);

	//get input
	input_image = elib::LibraryLinkUtilities<int>::llGetIntegerImage(libData, MArgument_getMTensor(input[0]),
			MArgument_getInteger(input[1]), 1);

	params.addParameter("C0", MArgument_getReal(input[2])); // c0
	params.addParameter("C1", MArgument_getReal(input[3])); // c1
	params.addParameter("Lambda", MArgument_getReal(input[4])); // lambda
	params.addParameter("Sigma", MArgument_getReal(input[5])); // sigma

	//compute cut
	binary_image = graphcut(*input_image, params);
	if (binary_image == nullptr)
	{
		return LIBRARY_FUNCTION_ERROR;
	}

	//transform and write data to output
	mint dimensions[input_image->getRank()];
	std::reverse_copy(input_image->getDimensions()->begin(), input_image->getDimensions()->end(), dimensions);
	libData->MTensor_new(MType_Integer, input_image->getRank(), dimensions, &binary_tensor);
	std::copy(binary_image->getData(), binary_image->getData() + binary_image->getFlattenedLength(),
			libData->MTensor_getIntegerData(binary_tensor));
	MArgument_setMTensor(output, binary_tensor);

	delete input_image;
	delete binary_image;
	return LIBRARY_NO_ERROR;
}

DLLEXPORT int llGraphcutDistribution(WolframLibraryData libData, mint nargs, MArgument* input, MArgument output)
{
	using elib::Image;
	using elib::Tensor;
	using elib::Parameters;

	Image<int> *input_image;
	std::unique_ptr<Image<short>> binary_image;
	Parameters params;
	MTensor binary_tensor;

//	int debug = 1;
//	while(debug);

	//get input
	input_image = elib::LibraryLinkUtilities<int>::llGetIntegerImage(libData, MArgument_getMTensor(input[0]),
			MArgument_getInteger(input[1]), 1);

	std::shared_ptr<Tensor<float>> c0 = elib::LibraryLinkUtilities<float>::llGetRealTensor(libData,
			MArgument_getMTensor(input[2]));
	std::shared_ptr<Tensor<float>> c1 = elib::LibraryLinkUtilities<float>::llGetRealTensor(libData,
			MArgument_getMTensor(input[3]));
	params.addParameter("C0", *c0); // c0
	params.addParameter("C1", *c1); // c1
	params.addParameter("Lambda", MArgument_getReal(input[4])); // lambda
	params.addParameter("Sigma", MArgument_getReal(input[4])); // sigma

	//compute cut
	graphcut(binary_image, *input_image, params);
	if (binary_image == nullptr)
	{
		return LIBRARY_FUNCTION_ERROR;
	}

	//transform and write data to output
	mint dimensions[input_image->getRank()];
	std::reverse_copy(input_image->getDimensions()->begin(), input_image->getDimensions()->end(), dimensions);
	libData->MTensor_new(MType_Integer, input_image->getRank(), dimensions, &binary_tensor);
	std::copy(binary_image->getData(), binary_image->getData() + binary_image->getFlattenedLength(),
			libData->MTensor_getIntegerData(binary_tensor));
	MArgument_setMTensor(output, binary_tensor);

	delete input_image;
	return LIBRARY_NO_ERROR;
}

DLLEXPORT int llMultiLabelGraphcut(WolframLibraryData libData, mint nargs, MArgument* input, MArgument output)
{
	elib::Image<int> *input_image, *input_label_image;
	std::shared_ptr<elib::Image<int>> label_image;
	elib::Parameters params;
	MTensor tensor;

//		int debug = 1;
//		while(debug);

	//get input
	input_image = elib::LibraryLinkUtilities<int>::llGetIntegerImage(libData, MArgument_getMTensor(input[0]),
			MArgument_getInteger(input[1]), 1);
	input_label_image = elib::LibraryLinkUtilities<int>::llGetIntegerImage(libData, MArgument_getMTensor(input[2]),
			MArgument_getInteger(input[3]), 1);

	params.addParameter("NumberLabels", int(MArgument_getInteger(input[4]))); // number of labels
	params.addParameter("C0", MArgument_getReal(input[5])); // c0
	params.addParameter("C1", MArgument_getReal(input[6])); // c1
	params.addParameter("Lambda", MArgument_getReal(input[7])); // lambda
	params.addParameter("Sigma", MArgument_getReal(input[8])); // sigma
	params.addParameter("Mu", MArgument_getReal(input[9])); // mu

	//compute cut
	elib::MultiLabelGraphcut mlgc;
	label_image = mlgc.multilabel_graphcut(*input_label_image, *input_image, params);
	if (label_image == nullptr)
	{
		return LIBRARY_FUNCTION_ERROR;
	}

	//transform and write data to output
	mint dimensions[input_image->getRank()];
	std::reverse_copy(input_image->getDimensions()->begin(), input_image->getDimensions()->end(), dimensions);
	libData->MTensor_new(MType_Integer, input_image->getRank(), dimensions, &tensor);
	std::copy(label_image->getData(), label_image->getData() + label_image->getFlattenedLength(),
			libData->MTensor_getIntegerData(tensor));
	MArgument_setMTensor(output, tensor);

	delete input_image;
	delete input_label_image;
	return LIBRARY_NO_ERROR;
}

DLLEXPORT int llAdaptiveMultiLabelGraphcut(WolframLibraryData libData, mint nargs, MArgument* input, MArgument output)
{
	elib::Image<int> *input_image, *input_label_image;
	std::shared_ptr<elib::Image<int>> label_image;
	elib::Parameters params;
	MTensor tensor;

//		int debug = 1;
//		while(debug);

	//get input
	input_image = elib::LibraryLinkUtilities<int>::llGetIntegerImage(libData, MArgument_getMTensor(input[0]),
			MArgument_getInteger(input[1]), 1);
	input_label_image = elib::LibraryLinkUtilities<int>::llGetIntegerImage(libData, MArgument_getMTensor(input[2]),
			MArgument_getInteger(input[3]), 1);

	params.addParameter("NumberLabels", int(MArgument_getInteger(input[4]))); // number of labels
	params.addParameter("Lambda", MArgument_getReal(input[5])); // lambda
	params.addParameter("Sigma", MArgument_getReal(input[6])); // lambda1
	params.addParameter("Mu", MArgument_getReal(input[7])); // mu

	//compute cut
	elib::MultiLabelGraphcut mlgc;
	label_image = mlgc.adaptive_multilabel_graphcut(*input_label_image, *input_image, params);
	if (label_image == nullptr)
	{
		return LIBRARY_FUNCTION_ERROR;
	}

	//transform and write data to output
	mint dimensions[input_image->getRank()];
	std::reverse_copy(input_image->getDimensions()->begin(), input_image->getDimensions()->end(), dimensions);
	libData->MTensor_new(MType_Integer, input_image->getRank(), dimensions, &tensor);
	std::copy(label_image->getData(), label_image->getData() + label_image->getFlattenedLength(),
			libData->MTensor_getIntegerData(tensor));
	MArgument_setMTensor(output, tensor);

	delete input_image;
	delete input_label_image;
	return LIBRARY_NO_ERROR;
}

DLLEXPORT int llDensity(WolframLibraryData libData, mint nargs, MArgument* input, MArgument output)
{
	elib::Parameters params;
	std::shared_ptr<elib::Tensor<double>> points;
	elib::Tensor<double> *result;
	MTensor density;
	std::shared_ptr<elib::Tensor<int>> dimensions, original_dimensions;

//	int debug = 1;
//	while(debug);

	points = elib::LibraryLinkUtilities<double>::llGetRealTensor(libData, MArgument_getMTensor(input[0]));
	dimensions = elib::LibraryLinkUtilities<int>::llGetIntegerTensor(libData, MArgument_getMTensor(input[1]));
	params.addParameter("Dimensions", *dimensions);
	original_dimensions = elib::LibraryLinkUtilities<int>::llGetIntegerTensor(libData, MArgument_getMTensor(input[2]));
	params.addParameter("OriginalDimensions", *original_dimensions);
	params.addParameter("Rank", 2);
	params.addParameter("Radius", MArgument_getReal(input[3]));
	params.addParameter("LateralProjectionRange", MArgument_getReal(input[4]));
	params.addParameter("BandWidth", MArgument_getReal(input[5]));
	params.addParameter("Type", int(MArgument_getInteger(input[6])));
	params.addParameter("CentralMeridian", MArgument_getReal(input[7]));
	params.addParameter("StandardParallel", MArgument_getReal(input[8]));

	result = elib::Density::calculateDensity(*points, params);
	if (result == nullptr)
	{
		return LIBRARY_FUNCTION_ERROR;
	}

	mint dims[result->getRank()];
	std::copy(result->getDimensions()->begin(), result->getDimensions()->end(), dims);
	libData->MTensor_new(MType_Real, result->getRank(), dims, &density);
	std::copy(result->getData(), result->getData() + result->getFlattenedLength(),
			libData->MTensor_getRealData(density));
	MArgument_setMTensor(output, density);

	delete result;
	return LIBRARY_NO_ERROR;
}

DLLEXPORT int llFeatureMap(WolframLibraryData libData, mint nargs, MArgument* input, MArgument output)
{
	elib::Parameters params;
	std::shared_ptr<elib::Tensor<double>> points, features;
	elib::Tensor<double> *result;
	MTensor density;
	std::shared_ptr<elib::Tensor<int>> dimensions, original_dimensions;

//	int debug = 1;
//	while(debug);

	points = elib::LibraryLinkUtilities<double>::llGetRealTensor(libData, MArgument_getMTensor(input[0]));
	features = elib::LibraryLinkUtilities<double>::llGetRealTensor(libData, MArgument_getMTensor(input[1]));
	dimensions = elib::LibraryLinkUtilities<int>::llGetIntegerTensor(libData, MArgument_getMTensor(input[2]));
	params.addParameter("Dimensions", *dimensions);
	original_dimensions = elib::LibraryLinkUtilities<int>::llGetIntegerTensor(libData, MArgument_getMTensor(input[3]));
	params.addParameter("OriginalDimensions", *original_dimensions);
	params.addParameter("Rank", 2);
	params.addParameter("Radius", MArgument_getReal(input[4]));
	params.addParameter("LateralProjectionRange", MArgument_getReal(input[5]));
	params.addParameter("BandWidth", MArgument_getReal(input[6]));
	params.addParameter("Type", int(MArgument_getInteger(input[7])));
	params.addParameter("CentralMeridian", MArgument_getReal(input[8]));
	params.addParameter("StandardParallel", MArgument_getReal(input[9]));

	result = elib::Density::calculateFeatureMap(*points, *features, params);
	if (result == nullptr)
	{
		return LIBRARY_FUNCTION_ERROR;
	}

	mint dims[result->getRank()];
	std::copy(result->getDimensions()->begin(), result->getDimensions()->end(), dims);
	libData->MTensor_new(MType_Real, result->getRank(), dims, &density);
	std::copy(result->getData(), result->getData() + result->getFlattenedLength(),
			libData->MTensor_getRealData(density));
	MArgument_setMTensor(output, density);

	delete result;
	return LIBRARY_NO_ERROR;
}

DLLEXPORT int llHDF5Import(WolframLibraryData libData, MLINK mlp)
{
	const char *file_name;
	const char *root;
	std::vector<std::string> roots;
	int type, depth;
	long length;

//	    int debug = 1;
//	    while (debug);

	if(!MLCheckFunction(mlp, "List", &length))
	{
		sendMessage(libData, "llHDF5Import", "wrong number of parameters.");
		MLPutSymbol(mlp, "$Failed");
		return LIBRARY_NO_ERROR;
	}
	if(length!=4)
	{
		sendMessage(libData, "llHDF5Import", "function requests exactly four parameters.");
		MLPutSymbol(mlp, "$Failed");
		return LIBRARY_NO_ERROR;
	}
	if(!MLGetString(mlp, &file_name))
	{
		sendMessage(libData, "llHDF5Import", "could not read parameter 'fileName'.");
		MLPutSymbol(mlp, "$Failed");
		return LIBRARY_NO_ERROR;
	}
	if(!MLGetInteger(mlp, &type))
	{
		sendMessage(libData, "llHDF5Import", "could not read parameter 'what'.");
		MLPutSymbol(mlp, "$Failed");
		return LIBRARY_NO_ERROR;
	}
	if(!MLCheckFunction(mlp, "List", &length))
	{
		sendMessage(libData, "llHDF5Import", "could not read number of entries in 'where'.");
		MLPutSymbol(mlp, "$Failed");
		return LIBRARY_NO_ERROR;
	}
	if(length<1)
	{
		sendMessage(libData, "llHDF5Import", "wrong number of parameters for 'where'.");
		MLPutSymbol(mlp, "$Failed");
		return LIBRARY_NO_ERROR;
	}
	for(int i=0; i<length; ++i)
	{
		if(!MLGetString(mlp, &root))
		{
			sendMessage(libData, "llHDF5Import", "failed to read entry from 'where'.");
			MLPutSymbol(mlp, "$Failed");
			return LIBRARY_NO_ERROR;
		}
		if(std::string(root).compare("")==0)
		{
			roots.push_back("/");
		}
		else
		{
			roots.push_back(std::string(root));
		}
	}
	if(!MLGetInteger(mlp, &depth))
	{
		sendMessage(libData, "llHDF5Import", "could not read parameter 'depth'.");
		MLPutSymbol(mlp, "$Failed");
		return LIBRARY_NO_ERROR;
	}

	try
	{
		elib::HDF5Reader reader = elib::HDF5Reader(mlp, std::string(file_name));
		switch (type)
		{
			case 0: /* read annotations */
			{
                reader.readAnnotations(roots);
			}
				break;
			case 1:
			{
				reader.readData(roots);
			}
				break;
			case 2: /* read names */
			{
				std::unique_ptr<std::vector<std::string>> names = std::unique_ptr<std::vector<std::string>>(new std::vector<std::string>);
				reader.readNames(roots, depth, names.get());
				MLPutFunction(mlp, "List", names->size());
				for (std::string i : *names)
				{
					MLPutString(mlp, i.c_str());
				}
			}
				break;
			default:
			{
				throw(elib::H5Exception("Don't know what to read!"));
			}
				break;
		}
	}
	catch (elib::H5Exception &e)
	{
		sendMessage(libData, "llHDF5Import", e.what());
		MLReleaseString(mlp, file_name);
		MLReleaseString(mlp, root);
		MLPutSymbol(mlp, "$Failed");
		return LIBRARY_NO_ERROR;
	}

	MLReleaseString(mlp, file_name);
	MLReleaseString(mlp, root);
	return LIBRARY_NO_ERROR;
}

DLLEXPORT int llVersion(WolframLibraryData libData, mint nargs, MArgument* input, MArgument output)
{
	char *version = new char[1024];
#ifndef ELIB_REVISION
	strncpy(version,"Unknown",1024);
	MArgument_setUTF8String(output, version);
#else
	strncpy(version, ELIB_REVISION, 1024);
	MArgument_setUTF8String(output, version);
#endif
	return LIBRARY_NO_ERROR;
}

void sendMessage(WolframLibraryData libData, const char *function_name, const char *message)
{
	MLINK loopback = libData->getMathLink(libData);
    MLPutFunction(loopback, "EvaluatePacket", 1);
	MLPutFunction(loopback, "Message", 2);
	MLPutFunction(loopback, "MessageName", 2);
    MLPutSymbol(loopback, function_name);
	MLPutString(loopback, "error");
	MLPutString(loopback, message);
	libData->processMathLink(loopback);
	MLNextPacket(loopback);
	MLNewPacket(loopback);
}
