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

#include "gco/GCoptimization.h"
#include "glm/glm.hpp"
#include "utilities/math_functions.hpp"


#define GC_INFINITY 300000.

using elib::MultiLabelGraphcut;
using elib::Image;

std::shared_ptr<Image<int>> MultiLabelGraphcut::multilabel_graphcut(Image<int> &label_image, Image<int> &input_image, Parameters &input_params)
{
	int width, height, bit_depth, num_labels;
	float 	c0 = .1,
			c1 = .9,
			lambda = 1.,
			mu = 1.;
	int c;
	std::set<int> labels;
	std::set<int>::iterator it;
	std::unordered_map<int,int> label_map;
	int label, num_pixels, *label_data, *image_data;
	std::shared_ptr<Image<int>> new_label_image = std::shared_ptr<Image<int>>(new Image<int>(label_image.getRank(), label_image.getDimensions(), label_image.getBitDepth(), label_image.getChannels()));

	if(
		isnan(num_labels = input_params.getIntegerParameter("NumberLabels")) ||
		isnan(c0 = input_params.getDoubleParameter("C0")) ||
		isnan(c1 = input_params.getDoubleParameter("C1")) ||
		isnan(lambda = input_params.getDoubleParameter("Lambda")) ||
		isnan(mu = input_params.getDoubleParameter("Mu"))
	)
	{
		return nullptr;
	}
	width = input_image.getWidth();
	height = input_image.getHeight();
	bit_depth = input_image.getBitDepth();
	label_data = label_image.getData();
	image_data = input_image.getData();

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
		c=c0*(pow(2,bit_depth)-1);
		for (int i = 0; i < num_pixels; i++ )
		{
			label = label_map.find(label_data[i])->second;
			gc->setDataCost(i,0, (label_dist(0-label))+fabs(float(image_data[i])-c));
		}
		c=c1*(pow(2,bit_depth)-1);
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
				gc->setDataCost(i,1, fabs(image_data[i]-c));
			}
		}
		//other labels
		for (int l = 2; l < num_labels; l++ )
		{
			for (int i = 0; i < num_pixels; i++ )
			{
				label = label_map.find(label_data[i])->second;
				gc->setDataCost(i,l, (label_dist(l-label)+fabs(image_data[i]-c)));
			}
		}

//		 //next set up smoothness costs individually
//				for ( int l1 = 0; l1 < numLabels; l1++ )
//					for (int l2 = 0; l2 < numLabels; l2++ )
//					{
//						gc->setSmoothCost(l1,l2,l1==l2 ? 0 : lambda);
//					}
		ForSmoothFn data;
		data.image = input_image.getData();
		data.lambda = lambda;
		data.mu = mu;
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

float elib::smoothFn(int p1, int p2, int l1, int l2, void *data)
{
	ForSmoothFn fsf = *((ForSmoothFn*) data);
	if(l1==l2)
		return fsf.lambda*exp(-pow(fsf.image[p1]-fsf.image[p2],2));
	else
	{
		if((l1==1 && l2>1) || (l1>1 && l2==1))
		{
			return GC_INFINITY;
		}
		if((l1==0 && l2>1) || (l1>1 && l2==0))
		{
			return fsf.lambda*float(exp(-pow(fsf.image[p1]-fsf.image[p2],2)));
		}
		else
		{
			return fsf.lambda*float(fsf.mu*label_dist(l1-l2) + exp(-pow(fsf.image[p1]-fsf.image[p2],2)));
		}
	}
}

inline int elib::label_dist(int value)
{
	if(value==0)
		return 0;
	else
		return 1;
}
