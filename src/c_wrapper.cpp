/*
 * c_wrapper.c
 *
 *  Created on: Jan 22, 2013
 *      Author: kthierbach
 */
#include "c_wrapper.h"

#include "alg/components_measurements.hpp"
#include "alg/graphcut.hpp"
#include "glm/glm.hpp"
#include "templates/image.hpp"
#include "utilities/parameters.hpp"

#define CHAR_BUFFER_SIZE 1024

cimage* graphcut_c(cimage *input_image, parameters *params)
{
	elib::Image<int> input(input_image);
	elib::Parameters parameters(params);
	elib::Image<int> *binary = graphcut(input, parameters);
	if(binary != nullptr)
	{
		cimage *ret = binary->to_cimage();
		delete binary;
		return ret;
	}
	else
	{
		return NULL;
	}
}

cmeasurements* component_measurements_c(cimage *input_image, parameters *params)
{
	elib::Image<int> input(input_image);
	elib::ComponentsMeasurements<glm::ivec3> cm(input);
	cmeasurements *res = new cmeasurements;

	res->number_of_objects = mint(cm.getNumberOfObjects());
	res->labels=new int[res->number_of_objects];
	std::copy(cm.getLabels()->begin(), cm.getLabels()->end(), res->labels);

	res->masks = new int*[res->number_of_objects];
	res->sizes = new int[res->number_of_objects];
	elib::MaskList<glm::ivec3> mask_list = cm.getMasks();
	std::vector<glm::ivec3> *points;
	glm::ivec3 point;
	for(int i=0; i<res->number_of_objects; ++i)
	{
		res->sizes[i] = mask_list[res->labels[i]]->getSize();
		res->masks[i] = new int[res->sizes[i]*input.getRank()];
		points = mask_list[res->labels[i]]->getPoints();
		for(int j=0; j<mask_list[res->labels[i]]->getSize(); ++j)
		{
			point = (*points)[j];
			if(input.getRank() == 2)
			{
				res->masks[i][2*j] = point.x+1;
				res->masks[i][2*j+1] = point.y+1;
			}
			else
			{
				res->masks[i][3*j] = point.x+1;
				res->masks[i][3*j+1] = point.y+1;
				res->masks[i][3*j+2] = point.z+1;
			}
		}
	}
	return res;
}

cimage* cloneImage(cimage* image)
{
	cimage *new_image;

	new_image = (cimage*)malloc(sizeof(cimage));
	new_image->bit_depth = image->bit_depth;
	new_image->channels = image->channels;
	new_image->rank = image->rank;
	new_image->dimensions = (mint*)malloc(sizeof(mint)*image->rank);
	std::copy(image->dimensions, image->dimensions+image->rank, new_image->dimensions);
	new_image->flattened_length = image->flattened_length;
	new_image->type = image->type;
	if(new_image->type == INTEGER_TYPE)
	{
		new_image->integer_data = new mint[new_image->flattened_length];
		std::copy(image->integer_data, image->integer_data+image->flattened_length, new_image->integer_data);
	}
	else
	{
		new_image->double_data = new double[new_image->flattened_length];
		std::copy(image->double_data, image->double_data+image->flattened_length, new_image->double_data);
	}
	new_image->shared = 0;

	return new_image;
}

cimage* createImage(cimage* image)
{
	cimage *new_image;

	new_image = new cimage;
	new_image->bit_depth = image->bit_depth;
	new_image->channels = image->channels;
	new_image->rank = image->rank;
	new_image->dimensions = new mint[image->rank];
	std::copy(image->dimensions, image->dimensions+image->rank, new_image->dimensions);
	new_image->flattened_length = image->flattened_length;
	new_image->type = image->type;
	if(new_image->type == INTEGER_TYPE)
	{
		new_image->integer_data = new mint[new_image->flattened_length];
	}
	else
	{
		new_image->double_data = new double[new_image->flattened_length];
	}
	new_image->shared = 0;

	return new_image;
}

cimage* createImage2(mint rank, mint *dimensions, mint bit_depth, mint channels)
{
	mint i;
	cimage *new_image;

	new_image = new cimage;
	new_image->bit_depth = bit_depth;
	new_image->channels = channels;
	new_image->rank = rank;
	new_image->dimensions = new mint[rank];
	std::copy(dimensions, dimensions+rank, new_image->dimensions);
	new_image->flattened_length = channels;
	for(i=0; i<new_image->rank; ++i)
	{
		new_image->flattened_length*=dimensions[i];
	}
	new_image->integer_data = new mint[new_image->flattened_length];

	return new_image;
}

parameters* createParameters(mint int_params_size, mint double_params_size)
{
	parameters *params;

	params = new parameters;
	params->int_params_size = int_params_size;
	params->double_params_size = double_params_size;
	params->int_params = new mint[int_params_size];
	params->double_params = new double[double_params_size];
	params->int_names = new char*[int_params_size];
	for(int i=0; i<int_params_size; ++i)
		params->int_names[i] = new char[CHAR_BUFFER_SIZE];
	params->double_names = new char*[double_params_size];
	for(int i=0; i<double_params_size; ++i)
		params->double_names[i] = new char[CHAR_BUFFER_SIZE];

	return params;
}

void freeTensor(cimage* image)
{
	if(image->shared)
	{
		delete image;
	}
	else
	{
		if(image->type == INTEGER_TYPE)
			delete[] image->integer_data;
		else
			delete[] image->double_data;
		delete[] image->dimensions;
		delete image;
	}
}

void freeParameters(parameters* params)
{
	delete[] params->int_params;
	delete[] params->double_params;
	for(int i=0; i<params->int_params_size; ++i)
		delete[] (params->int_names)[i];
	for(int i=0; i<params->double_params_size; ++i)
		delete[] (params->double_names)[i];
	delete[] params->int_names;
	delete[] params->double_names;
	delete params;
}

void freeMeasurements(cmeasurements *cm)
{
	delete[] cm->sizes;
	delete[] cm->labels;
	for(int i=0; i<cm->number_of_objects; ++i)
		delete[] cm->masks[i];
	delete[] cm->masks;
}

