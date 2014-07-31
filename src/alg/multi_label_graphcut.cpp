/*
 * GraphCutImage.cpp
 *
 *  Created on: Jan 21, 2013
 *      Author: kthierbach
 */

#include "multi_label_graphcut.hpp"

#include <math.h>
#include <unordered_map>
#include <set>

#include <iostream>

#include "alg/components_measurements.hpp"
#include "gco/GCoptimization.h"
#include "glm/glm.hpp"
#include "utilities/math_functions.hpp"
#include "templates/mask.hpp"
#include "templates/mask_list.hpp"


#define GC_INFINITY 300000.

using elib::MultiLabelGraphcut;
using elib::Image;

std::shared_ptr<Image<int>> MultiLabelGraphcut::multilabel_graphcut(Image<int> &label_image, Image<int> &input_image, Parameters &input_params)
{
	int width, height, bit_depth, num_labels;
    std::set<int> labels;
	std::set<int>::iterator it;
	std::unordered_map<int,int> label_map;
	int label, num_pixels;
	std::shared_ptr<Image<int>> new_label_image = std::shared_ptr<Image<int>>(new Image<int>(label_image.getRank(), *label_image.getDimensions(), label_image.getBitDepth(), label_image.getChannels()));

	float 	c0, c1, lambda, sigma, mu;
	if(
		isnan(num_labels = input_params.getIntegerParameter("NumberLabels")) ||
		isnan(c0 = input_params.getDoubleParameter("C0")) ||
		isnan(c1 = input_params.getDoubleParameter("C1")) ||
		isnan(lambda = input_params.getDoubleParameter("Lambda")) ||
		isnan(sigma = input_params.getDoubleParameter("Sigma")) ||
		isnan(mu = input_params.getDoubleParameter("Mu"))
	)
	{
		return nullptr;
	}
	width = input_image.getWidth();
	height = input_image.getHeight();
	bit_depth = input_image.getBitDepth();
	int *label_data = label_image.getData(),
        *image_data = input_image.getData();

	labels.insert(label_image.getData(), label_image.getData()+label_image.getFlattenedLength());
	labels.insert(1); //inserts label 1
	int label_array[labels.size()];
	std::copy(labels.begin(), labels.end(), label_array);

	for(unsigned int i=0; i<labels.size(); ++i)
	{
		label_map.insert(std::pair<int,int>(label_array[i],i));
	}

	try{
		GCoptimizationGridGraph *gc = new GCoptimizationGridGraph(width,height, num_labels);
		num_pixels = width*height;
		// first set up data costs individually

		//background labeling == 0
		float c=c0*(pow(2,bit_depth)-1);
		float max_intensity = pow(2,bit_depth)-1;
		for (int i = 0; i < num_pixels; i++ )
		{
			label = label_map.find(label_data[i])->second;
			gc->setDataCost(i,0, mu*label_dist(label)+fabsf(float(image_data[i])-c)/max_intensity);
		}
		//label for appearing objects == 1
		c=c1*(pow(2,bit_depth)-1);
		for (int i = 0; i < num_pixels; i++ )
		{
			label = label_map.find(label_data[i])->second;
			if(label_dist(label))
			{
				gc->setDataCost(i,1, GC_INFINITY);
			}
			else
			{
				gc->setDataCost(i,1, fabsf(float(image_data[i]-c)/max_intensity));
			}
		}
		//other labels
		for (int l = 2; l < num_labels; l++ )
		{
			for (int i = 0; i < num_pixels; i++ )
			{
				label = label_map.find(label_data[i])->second;
				gc->setDataCost(i,l, mu*label_dist(l-label)+fabsf(float(image_data[i]-c)/max_intensity));
			}
		}

//		 //next set up smoothness costs individually
//				for ( int l1 = 0; l1 < numLabels; l1++ )
//					for (int l2 = 0; l2 < numLabels; l2++ )
//					{
//						gc->setSmoothCost(l1,l2,l1==l2 ? 0 : lambda);
//					}
		ForSmoothFn data;
		data.image = image_data;
		data.lambda = lambda;
		data.sigma = sigma;
		data.mu = mu;
		data.max_intensity = max_intensity;
		gc->setSmoothCost(&smoothFn, &data);
		gc->swap(cycles);
		for (int i = 0; i < num_pixels; i++)
		{
			new_label_image->getData()[i] = label_array[gc->whatLabel(i)];
		}

		delete gc;
	}
	catch (GCException &e){
		e.Report();
	}
	return new_label_image;
}

std::shared_ptr<Image<int>> MultiLabelGraphcut::adaptive_multilabel_graphcut(Image<int> &label_image, Image<int> &input_image, Parameters &input_params)
{
	std::shared_ptr<Image<int>> new_label_image;
	int num_labels;
	double lambda, mu;
	if(
		isnan(num_labels = input_params.getIntegerParameter("NumberLabels")) ||
		isnan(lambda = input_params.getDoubleParameter("Lambda")) ||
		isnan(mu = input_params.getDoubleParameter("Mu"))
	)
	{
		return nullptr;
	}

	std::set<int> labels;
	labels.insert(label_image.getData(), label_image.getData()+label_image.getFlattenedLength());
	labels.insert(1); //inserts label 1
	int label_array[labels.size()];
	std::copy(labels.begin(), labels.end(), label_array);
	std::unordered_map<int,int> label_map;
	for(unsigned int i=0; i<labels.size(); ++i)
	{
		label_map.insert(std::pair<int,int>(label_array[i],i));
	}

	std::vector<float> c0, c1;
	calculateIntensityDistributions(c0, c1, label_image, input_image);

	int width = input_image.getWidth(),
		height = input_image.getHeight();
	try{
		GCoptimizationGridGraph *gc = new GCoptimizationGridGraph(width,height, num_labels);
		int label,
			*label_data = label_image.getData(),
			*image_data = input_image.getData(),
			num_pixels = width*height;
		// first set up data costs individually

		//background labeling == 0
		for (int i = 0; i < num_pixels; i++ )
		{
			label = label_map.find(label_data[i])->second;
			gc->setDataCost(i,0, mu*label_dist(label) + (1.- c0[image_data[i]]));
		}
		//label for appearing objects == 1
		for (int i = 0; i < num_pixels; i++ )
		{
			label = label_map.find(label_data[i])->second;
			if(label_dist(label))
			{
				gc->setDataCost(i,1, GC_INFINITY);
			}
			else
			{
				gc->setDataCost(i,1,  1.- c1[image_data[i]]);
			}
		}
		//other labels
		for (int l = 2; l < num_labels; l++ )
		{
			for (int i = 0; i < num_pixels; i++ )
			{
				label = label_map.find(label_data[i])->second;
				gc->setDataCost(i,l, mu*label_dist(l-label) + (1.- c1[image_data[i]]));
			}
		}

//		 //next set up smoothness costs individually
//				for ( int l1 = 0; l1 < numLabels; l1++ )
//					for (int l2 = 0; l2 < numLabels; l2++ )
//					{
//						gc->setSmoothCost(l1,l2,l1==l2 ? 0 : lambda);
//					}
		ForSmoothFn data;
		data.image = image_data;
		data.lambda = lambda;
		data.mu = mu;
		gc->setSmoothCost(&smoothFn, &data);
		gc->swap(cycles);

		new_label_image = std::shared_ptr<Image<int>>(new Image<int>(label_image.getRank(), *label_image.getDimensions(), label_image.getBitDepth(), 1));
		for (int i = 0; i < num_pixels; i++)
		{
			new_label_image->getData()[i] = label_array[gc->whatLabel(i)];
		}

		delete gc;
	}
	catch (GCException &e){
		e.Report();
	}
	return new_label_image;
}


float elib::smoothFn(int p1, int p2, int l1, int l2, void *data)
{
	ForSmoothFn fsf = *((ForSmoothFn*) data);
	if(l1==l2)
		return fsf.lambda*exp(-powf(float(fsf.image[p1]-fsf.image[p2])/fsf.max_intensity,2)/fsf.sigma);
	else
	{
		if((l1==1 && l2>1) || (l1>1 && l2==1))
		{
			return GC_INFINITY;
		}
		if((l1==0 && l2>1) || (l1>1 && l2==0))
		{
			return fsf.lambda*(fsf.mu*label_dist(l1-l2) + exp(-powf(float(fsf.image[p1]-fsf.image[p2])/fsf.max_intensity,2)/fsf.sigma));
		}
		else
		{
			return fsf.lambda*(fsf.mu*label_dist(l1-l2) + exp(-powf(float(fsf.image[p1]-fsf.image[p2])/fsf.max_intensity,2)/fsf.sigma));
//			return GC_INFINITY;
		}
	}
}

inline float elib::label_dist(int value)
{
	if(value==0)
		return 0.;
	else
		return 1.;
}

void MultiLabelGraphcut::calculateIntensityDistributions(std::vector<float> &c0, std::vector<float> &c1, Image<int> &label_image, Image<int> &input_image)
{
	ComponentsMeasurements<glm::ivec3> cm(label_image);
	MaskList<glm::ivec3> masks = cm.getMasks();
	Mask<glm::ivec3> foreground_mask = masks.fuse(),
					 background_mask = Mask<glm::ivec3>::negate(foreground_mask);

	c0 = std::vector<float>(pow(2, input_image.getBitDepth()));
	c1 = std::vector<float>(pow(2, input_image.getBitDepth()));

	std::vector<int> tmp = input_image.pick(*background_mask.getPoints());
	for(auto i : tmp)
	{
		c0[i] += 1.;
	}
	for(int i=0; i<c0.size(); ++i)
	{
		c0[i] /= tmp.size();
	}
	tmp = input_image.pick(*foreground_mask.getPoints());
	for(auto i : tmp)
	{
		c1[i] += 1.;
	}
	for(int i=0; i<c1.size(); ++i)
	{
		c1[i] /= tmp.size();
	}
}
