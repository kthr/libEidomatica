/*
 * MathematicaLibaryLink.c
 *
 *  Created on: Jan 22, 2013
 *      Author: kthierbach
 */

#include "mathlink.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "c_wrapper.h"
#include "WolframLibrary.h"

#include "revision.hpp"

#define FALSE 0
#define TRUE 1

#define RANGE_MINUS_PI_TO_PI 0
#define RANGE_MINUS_ONE_TO_ONE 1
#define RANGE_ZERO_TO_ONE 2

mreal max(mreal* data, mint length);
void polar(mreal *x, mreal *y, unsigned int range); // returns {length,angle} where angle ranges from -Pi..Pi
mreal* toPolarCoordinates(mreal *data, mint length, unsigned int rescale, unsigned int range);

cimage* llGetTensor(WolframLibraryData libData, MTensor* tensor, mint bit_depth, mint channels);
cimage* mlGetIntegerTensor(MLINK mlp);

DLLEXPORT int llVersion(WolframLibraryData libData, mint nargs, MArgument* input, MArgument output)
{
#ifndef REVISION
	MArgument_setUTF8String(output, "Unknown");
#else
	MArgument_setUTF8String(output, REVISION);
#endif
	return LIBRARY_NO_ERROR;
}

DLLEXPORT int llVectorFieldPlot(WolframLibraryData libData, mint nargs, MArgument* input, MArgument output)
{
	cimage *vector_field;
	MTensor polar_field;
	mreal  *polarData;
	mint *dimensions, new_dimensions[3];

	vector_field = llGetTensor(libData, &MArgument_getMTensor(input[0]), 8, 1);
	dimensions = vector_field->dimensions;
	if(vector_field->type == DOUBLE_TYPE)
		polarData = toPolarCoordinates(vector_field->double_data, vector_field->flattened_length, TRUE, RANGE_MINUS_PI_TO_PI);
	else
		return LIBRARY_FUNCTION_ERROR;

	new_dimensions[0] = dimensions[0];
	new_dimensions[1] = dimensions[1];
	new_dimensions[2] = dimensions[2]+1;
	libData->MTensor_new(MType_Real, vector_field->rank, new_dimensions, &polar_field);

	mint coord[3], tmp;
	mreal tmp2;
	mint i,j;
	for(i=1; i<=dimensions[0]; ++i)
	{

		for(j=1;j<=dimensions[1]; ++j)
		{
			coord[0] = i;
			coord[1] = j;

			tmp = (i-1)*dimensions[1]*dimensions[2]+(j-1)*dimensions[2];
			tmp2 = -.5*polarData[tmp+1]/M_PI;
			coord[2] = 1;
			libData->MTensor_setReal(polar_field,coord,tmp2-floor(tmp2));
			coord[2] = 2;
			libData->MTensor_setReal(polar_field,coord,polarData[tmp]);
			coord[2] = 3;
			libData->MTensor_setReal(polar_field,coord,.9);
		}
	}

	MArgument_setMTensor(output,polar_field);
	freeTensor(vector_field);
	return LIBRARY_NO_ERROR;
}

DLLEXPORT int llGraphCut(WolframLibraryData libData, mint nargs, MArgument* input, MArgument output)
{
	cimage *input_image, *binary_image;
	parameters *params;
	MTensor binary_tensor;

//	int debug = 1;
//	while(debug);

	//get input
	input_image = llGetTensor(libData, &MArgument_getMTensor(input[0]), MArgument_getInteger(input[1]), 1);

	params = createParameters(0, 5);
	params->double_params[0] = MArgument_getReal(input[2]); // c0
	strcpy(params->double_names[0],"C0");
	params->double_params[1] = MArgument_getReal(input[3]); // c1
	strcpy(params->double_names[1], "C1");
	params->double_params[2] = MArgument_getReal(input[4]); // lambda1
	strcpy(params->double_names[2], "Lambda1");
	params->double_params[3] = MArgument_getReal(input[5]); // lambda2
	strcpy(params->double_names[3], "Lambda2");
	params->double_params[4] = MArgument_getReal(input[6]); // beta
	strcpy(params->double_names[4], "Beta");

	//compute cut
	binary_image = graphcut_c(input_image, params);

	if(binary_image == NULL)
	{
		return LIBRARY_FUNCTION_ERROR;
	}

	//transform and write data to output
	libData->MTensor_new(MType_Integer, input_image->rank, input_image->dimensions, &binary_tensor);
	memcpy(libData->MTensor_getIntegerData(binary_tensor), binary_image->integer_data, sizeof(mint)*binary_image->flattened_length);
	MArgument_setMTensor(output,binary_tensor);

	freeTensor(input_image);
	freeTensor(binary_image);
	freeParameters(params);
	return LIBRARY_NO_ERROR;
}

DLLEXPORT int llComponentMeasurements(WolframLibraryData libData, MLINK mlp)
{
	cimage *input_image;
	parameters *params = NULL;
	cmeasurements *measures;

//	int debug=1;
//	while(debug);

	if((input_image = mlGetIntegerTensor(mlp)) == NULL)
		return LIBRARY_FUNCTION_ERROR;
	measures = component_measurements_c(input_image, params);

	if(!MLPutFunction(mlp, "List", measures->number_of_objects))
		return LIBRARY_FUNCTION_ERROR;
	for(int i=0; i<measures->number_of_objects; ++i)
	{
		if(!MLPutFunction(mlp, "Rule", 2))
			return LIBRARY_FUNCTION_ERROR;
		if(!MLPutInteger32(mlp, (int)measures->labels[i]))
			return LIBRARY_FUNCTION_ERROR;
		if(!MLPutFunction(mlp, "List", measures->sizes[i]))
			return LIBRARY_FUNCTION_ERROR;
		for(int j=0; j<measures->sizes[i]; ++j)
		{
			if(input_image->rank == 2)
			{
				if(!MLPutFunction(mlp, "List", 2))
					return LIBRARY_FUNCTION_ERROR;
				if(!MLPutInteger32(mlp,measures->masks[i][2*j]))
					return LIBRARY_FUNCTION_ERROR;
				if(!MLPutInteger32(mlp,measures->masks[i][2*j+1]))
					return LIBRARY_FUNCTION_ERROR;
			}
			else
			{
				if(!MLPutFunction(mlp, "List", 3))
					return LIBRARY_FUNCTION_ERROR;
				if(!MLPutInteger32(mlp,measures->masks[i][3*j]))
					return LIBRARY_FUNCTION_ERROR;
				if(!MLPutInteger32(mlp,measures->masks[i][3*j+1]))
					return LIBRARY_FUNCTION_ERROR;
				if(!MLPutInteger32(mlp,measures->masks[i][3*j+2]))
					return LIBRARY_FUNCTION_ERROR;
			}
		}
	}

	freeMeasurements(measures);
	freeTensor(input_image);
	return LIBRARY_NO_ERROR;
}

cimage* llGetTensor(WolframLibraryData libData, MTensor* tensor, mint bit_depth, mint channels)
{
	cimage *image;

	image = (cimage*)malloc(sizeof(cimage));
	image->bit_depth = bit_depth;
	image->channels = channels;
	if(channels==1)
	{
		image->rank = libData->MTensor_getRank(*tensor);
	}
	else
	{
		image->rank = libData->MTensor_getRank(*tensor)-1;
	}
	image->dimensions = libData->MTensor_getDimensions(*tensor);
	image->flattened_length = libData->MTensor_getFlattenedLength(*tensor);
	switch(libData->MTensor_getType(*tensor))
	{
		case MType_Integer:
			image->type = INTEGER_TYPE;
			image->integer_data = libData->MTensor_getIntegerData(*tensor);
			break;
		case MType_Real:
			image->type = DOUBLE_TYPE;
			image->double_data = libData->MTensor_getRealData(*tensor);
			break;
	}
	image->shared = 1;

	return image;
}

cimage* mlGetIntegerTensor(MLINK mlp)
{
	cimage *image;
	int tmp;
	long len;
	int *data;

	image = (cimage*)malloc(sizeof(cimage));
	image->type = INTEGER_TYPE;
	if(!MLCheckFunction(mlp,"List",&len))
		return NULL;
	if(!MLGetInteger32(mlp, &tmp))
		return NULL;
	image->bit_depth = tmp;
	if(!MLGetInteger32(mlp, &tmp))
		return NULL;
	image->channels = tmp;
	if(!MLGetInteger32List(mlp, &data, &tmp))
		return NULL;
	image->dimensions = (mint*)malloc(sizeof(mint)*tmp);
	image->rank = (mint)tmp;
	for(int i=0; i<tmp; ++i)
	{
		image->dimensions[i] = (mint)data[i];
	}
	MLReleaseInteger32List(mlp, data, tmp);
	if(!MLGetInteger32List(mlp, &data, &tmp))
		return NULL;
	image->flattened_length = (mint)tmp;
	image->integer_data = (mint*)malloc(sizeof(mint)*tmp);
	for(int i=0; i<tmp; ++i)
	{
		image->integer_data[i] = (mint)data[i];
	}
	MLReleaseInteger32List(mlp, data, tmp);
	image->shared = 0;

	return image;
}

mreal* toPolarCoordinates(mreal *data, mint length, unsigned int rescale, unsigned int range)
{
	mint i;
	mreal max = 1.;
	mreal *polarData;

	polarData = (mreal*) malloc(sizeof(mreal)*length);

	#pragma omp parallel for shared(polarData)
	for(i=0; i<length; ++i)
		polarData[i] = data[i];

	#pragma omp parallel
	{
		mreal priv_max = 1.; //only values greater than 1. are concerned

		#pragma omp for
		for (i = 0; i < length; i += 2)
		{
			polar(&polarData[i], &polarData[i + 1], range);
			if (polarData[i] > priv_max)
				priv_max = polarData[i];
		}
		#pragma omp flush(max)
		{
			if (priv_max > max)
			{
				#pragma omp critical
				{
					if (priv_max > max)
						max = priv_max;
				}
			}
		}
	}
	if (max != 1. && rescale)
	{
		#pragma omp parallel
		{
			#pragma omp for
			for (i = 0; i < length; i += 2)
			{
				polarData[i] = polarData[i] / max;
			}
		}
	}
	return polarData;
}

mreal max(mreal* data, mint length)
{
	mreal max;
	mint i;

	if (length == 0)
		return 0;
	max = data[0];

#pragma omp parallel
	{
		mreal priv_max = data[0];

#pragma omp for
		for (i = 0; i < length; ++i)
		{
			if (data[i] > priv_max)
				priv_max = data[i];
		}
#pragma omp flush (max)
		if (priv_max > max)
		{
#pragma omp critical
			{
				if (priv_max > max)
					max = priv_max;
			}
		}
	}
	return max;
}

inline void polar(mreal *x, mreal *y, unsigned int range) // returns {length,angle} where angle ranges from -Pi..Pi
{
	mreal u = *x, v = *y;

	*x = sqrt(pow(u, 2) + pow(v, 2));//compute length
	//compute angle
	if (u > 0.)
	{
		*y = atan(v / u);
		return;
	}
	if (u < 0.)
	{
		if (v >= 0.)
		{
			*y = atan(v / u) + M_PI;
			return;
		}
		else
		{
			*y = atan(v / u) - M_PI;
			return;
		}
	}
	if (v > 0.)
	{
		*y = M_PI / 2;
		return;
	}
	if (v < 0.)
	{
		*y = -M_PI / 2;
		return;
	}
	//if v==0.
	*y = 0.;

	switch (range)
	{
		case RANGE_MINUS_ONE_TO_ONE:
			*y=*y/M_PI;
			break;
		case RANGE_ZERO_TO_ONE:
			*y=(*y/M_PI+1)/2;
			break;
		default: //RANGE_MINUS_PI_TO_PI
			break;
	}
}

DLLEXPORT int WolframLibrary_getVersion(){return WolframLibraryVersion;}
DLLEXPORT int WolframLibrary_initialize(WolframLibraryData libData) {return 0;}
DLLEXPORT void WolframLibrary_uninitialize( WolframLibraryData libData) {}
