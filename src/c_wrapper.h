/*
 * c_wrapper.h
 *
 *  Created on: Jan 22, 2013
 *      Author: kthierbach
 */

#ifndef C_WRAPPER_H_
#define C_WRAPPER_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "WolframLibrary.h"

#define INTEGER_TYPE 0
#define DOUBLE_TYPE 1

typedef struct {
	mint *integer_data;
	mreal *double_data;
	mint type;
	mint *dimensions;
	mint bit_depth;
	mint channels;
	mint flattened_length;
	mint rank;
	short shared;
} cimage;

typedef struct {
	double *double_params;
	mint *int_params;
	mint double_params_size;
	mint int_params_size;
	char **int_names;
	char **double_names;
} parameters;

typedef struct {
	int number_of_objects;
	int *labels;
	int *sizes;
	int **masks;
} cmeasurements;

	/* Write C function declarations here */
	cimage* graphcut_c(cimage *input_image, parameters *params);
	cmeasurements* component_measurements_c(cimage *input_image, parameters *params);

	cimage* cloneImage(cimage *image);
	cimage* createImage(cimage *image);
	cimage* createImage2(mint rank, mint *dimensions, mint bit_depth, mint channels);
	parameters* createParameters(mint int_params_size, mint double_params_size);
	void freeTensor(cimage *image);
	void freeParameters(parameters *param);
	void freeMeasurements(cmeasurements *measures);
#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* C_WRAPPER_H_ */
