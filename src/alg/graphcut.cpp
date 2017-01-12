/*
 * GraphCutImage.cpp
 *
 *  Created on: Jan 21, 2013
 *      Author: kthierbach
 */

#include "graphcut.hpp"

#include <math.h>

#include "maxflow/energy.h"
#include "maxflow/graph.h"
#include "utilities/math_functions.hpp"

namespace elib{

#define GC_INFINITY 300000.

Image<short>* graphcut(Image<int> &input_image, Parameters &parameters)
{
	using graphcut::Energy;

	Image<short> *binary_image = new Image<short>(input_image.getRank(), *input_image.getDimensions(), input_image.getBitDepth(), input_image.getChannels());
	int *nh,
		nh_length,
		nh2d[24] = {-1,0,0,-1,-1,0,0,-1,0,1,-1,0,1,0,0,1,1,0,0,1,0,-1,1,0}, // 8-neighborhood indices
		nh3d[42] = {-1,0,0,0,0,-1,1,0,0,0,0,1,0,1,0,0,-1,0,-1,1,-1,1,1,-1,-1,1,1,1,1,1,-1,-1,-1,1,-1,-1,-1,-1,1,1,-1,1};
	if(input_image.getRank()==2)
	{
		nh=nh2d;
		nh_length=24;
	}
	else
	{
		nh=nh3d;
		nh_length=42;
	}
	int width = input_image.getWidth(),
		height = input_image.getHeight(),
        depth = input_image.getDepth(),
        bitDepth = input_image.getBitDepth();
    
	double c0, c1, lambda, sigma;
	if(
		elib::isnan(c0 = parameters.getDoubleParameter("C0")) ||
		elib::isnan(c1 = parameters.getDoubleParameter("C1")) ||
        elib::isnan(lambda = parameters.getDoubleParameter("Lambda")) ||
        elib::isnan(sigma = parameters.getDoubleParameter("Sigma"))
	)
	{
		return nullptr;
	}

	int *input_image_data = input_image.getData();
	short *binary_image_data = binary_image->getData();

	/****** Create the Energy *************************/
	Energy::Var *varx = new Energy::Var[width*height*depth];
	Energy *energy = new Energy();

    int nodeCount;
    float value;
    float maxIntensity = powf(2.,bitDepth)-1.;
    float bg = c0*maxIntensity,
          fg = c1*maxIntensity;
	/****** Build Unary Term *************************/
	for(int k=0; k<depth; ++k )
	{
		for (int j=0; j < height; ++j)
		{
			for (int i = 0; i < width; ++i)
			{
				nodeCount = i + j*width + k*width*height;

				// add node
				varx[nodeCount] = energy->add_variable();

				// add likelihood
				value = input_image_data[nodeCount];
				energy->add_term1(varx[nodeCount], (1-lambda)*fabsf(value - bg)/maxIntensity, (1-lambda)*fabsf(value - fg)/maxIntensity);
			}
		}
	}

	/******* Build pairwise terms ********************/
	int other;
    int x, y, z;
	for(int k=0; k<depth; ++k)
	{
		for (int j=0; j<height; ++j)
		{
			for (int i=0; i<width; ++i)
			{
				nodeCount = i + j*width + k*width*height;
				for(int l = 0; l < nh_length; l+=3)
				{
					x = i + nh[l];
					y = j + nh[l+1];
					z = k + nh[l+2];
					//if not outside of image
					other = x + y*width + z*width*height;
					if (!(x<0 || x>=width || y<0 || y>=height || z<0 || z>=depth))
					{
						value = lambda*expf(-powf(float(input_image_data[nodeCount] - input_image_data[other])/maxIntensity,2)/sigma);
						energy->add_term2(varx[nodeCount], varx[other], 0., value, value, 0.);
					}
				}
			}
		}
	}

	/******* Minimize energy ********************/
	energy->minimize();

	/******* Show binary image and clean up ********************/
	for(int k=0; k<depth; ++k)
	{
		for (int j = 0; j < height; ++j)
		{
			for (int i = 0; i < width; ++i)
			{
				nodeCount = i + j * width + k*width*height;
				if (energy->get_var(varx[nodeCount]))
				{
					binary_image_data[nodeCount] = 1;
				}
				else
				{
					binary_image_data[nodeCount] = 0;
				}
			}

		}
	}

	delete[] varx;
	delete energy;

	return binary_image;
}

void graphcut(std::unique_ptr<Image<short>> &binary_image, Image<int> &input_image, Parameters &parameters)
{
	using graphcut::Energy;

	const Tensor<float> *background, *foreground;
	float lambda, sigma;
	if(
		(background=parameters.getFloatTensorParameter("C0")) == nullptr ||
		(foreground=parameters.getFloatTensorParameter("C1")) == nullptr ||
		elib::isnan(lambda=parameters.getDoubleParameter("Lambda")) ||
		elib::isnan(sigma=parameters.getDoubleParameter("Sigma"))
	)
	{
		binary_image = nullptr;
		return;
	}
	int width = input_image.getWidth(),
		height = input_image.getHeight(),
		depth = input_image.getDepth(),
		bit_depth = input_image.getBitDepth();

	if(background->getFlattenedLength() != (pow(2,bit_depth)) || foreground->getFlattenedLength() != (pow(2,bit_depth)))
	{
		binary_image = nullptr;
		return;
	}

	int *input_image_data = input_image.getData();
	binary_image = std::unique_ptr<Image<short>>(new Image<short>(input_image.getRank(), *input_image.getDimensions(), 8, 1));
	short *binary_image_data = binary_image->getData();

	/****** Create the Energy *************************/
	Energy::Var *varx = new Energy::Var[width*height*depth];
	Energy *energy = new Energy();

	int nodeCount;
	int value;
	/****** Build Unary Term *************************/
	for(int k=0; k<depth; ++k )
	{
		for (int j=0; j < height; ++j)
		{
			for (int i = 0; i < width; ++i)
			{
				nodeCount = i + j*width + k*width*height;

				// add node
				varx[nodeCount] = energy->add_variable();

				// add likelihood
				value = input_image_data[nodeCount];
				energy->add_term1(varx[nodeCount], (1-lambda)*(1.-background->get(value)), (1-lambda)*(1.-foreground->get(value)));
			}
		}
	}

	int *nh,
		nh_length,
		nh2d[24] = {-1,0,0,-1,-1,0,0,-1,0,1,-1,0,1,0,0,1,1,0,0,1,0,-1,1,0}, // 8-neighborhood indices
		nh3d[42] = {-1,0,0,0,0,-1,1,0,0,0,0,1,0,1,0,0,-1,0,-1,1,-1,1,1,-1,-1,1,1,1,1,1,-1,-1,-1,1,-1,-1,-1,-1,1,1,-1,1};
	if(input_image.getRank()==2)
	{
		nh=nh2d;
		nh_length=24;
	}
	else
	{
		nh=nh3d;
		nh_length=42;
	}
	/******* Build pairwise terms ********************/
	int other;
	int x, y, z;
    float maxIntensity = powf(2.,input_image.getBitDepth())-1.;
	for(int k=0; k<depth; ++k)
	{
		for (int j=0; j<height; ++j)
		{
			for (int i=0; i<width; ++i)
			{
				nodeCount = i + j*width + k*width*height;
				for(int l = 0; l < nh_length; l+=3)
				{
					x = i + nh[l];
					y = j + nh[l+1];
					z = k + nh[l+2];
					//if not outside of image
					other = x + y*width + z*width*height;
					if (!(x<0 || x>=width || y<0 || y>=height || z<0 || z>=depth))
					{
						value = lambda*expf(-powf(float(input_image_data[nodeCount] - input_image_data[other])/maxIntensity,2)/sigma);
						energy->add_term2(varx[nodeCount], varx[other], 0., value, value, 0.);
					}
				}
			}
		}
	}

	/******* Minimize energy ********************/
	energy->minimize();

	/******* Show binary image and clean up ********************/
	for(int k=0; k<depth; ++k)
	{
		for (int j = 0; j < height; ++j)
		{
			for (int i = 0; i < width; ++i)
			{
				nodeCount = i + j * width + k*width*height;
				if (energy->get_var(varx[nodeCount]))
				{
					binary_image_data[nodeCount] = 1;
				}
				else
				{
					binary_image_data[nodeCount] = 0;
				}
			}

		}
	}

	delete[] varx;
	delete energy;
}
void graphcutSphere(int* binary, int nVertices, double *vertices, int *prior, int nNeighbors, int *neighbours, int bitDepth, int *intensities, double c0, double c1, double lambda1, double lambda2)
{
	using graphcut::Energy;

	double maxIntensity, value, fg, bg;
	int nb;

	maxIntensity = pow(2, bitDepth) - 1;
	bg = c0*maxIntensity;
	fg = c1*maxIntensity;

	/****** Create the Energy *************************/
	Energy::Var *varx = new Energy::Var[nVertices];
	Energy *energy = new Energy();

	/****** Build unary terms *************************/
	for (int i = 0; i < nVertices; ++i)
	{
		// add node
		varx[i] = energy->add_variable();

		// add likelihood
		if(prior[i])
		{
			energy->add_term1(varx[i], prior[i]*GC_INFINITY, -prior[i]*GC_INFINITY); // prior[i]==-1 -> background or prior[i]==1 -> foreground
		}
		else
		{
			value = ((double) intensities[i]);
			energy->add_term1(varx[i], abs(value - bg), abs(value - fg));
		}
	}

	/******* Build pairwise terms ********************/
	for (int i=0; i<nVertices; ++i)
	{
		for(int j=0; j<nNeighbors; ++j)
		{
			nb = neighbours[i*nNeighbors+j];
			value = lambda1 + lambda2 *
					exp(-pow(intensities[i] - intensities[nb], 2)) *
					exp(-sqrt(pow(vertices[i*3]-vertices[nb*3],2) + pow(vertices[i*3+1]-vertices[nb*3+1],2) + pow(vertices[i*3+2]-vertices[nb*3+2],2)));
			energy->add_term2(varx[i], varx[nb], 0., value, value, 0.);
		}
	}

	/******* Minimize energy ********************/
	energy->minimize();

	/******* Show binary image and clean up ********************/
	for (int i = 0; i < nVertices; ++i)
	{
		if (energy->get_var(varx[i]))
		{
			binary[i] = 1;
		}
		else
		{
			binary[i] = 0;
		}
	}

	delete[] varx;
	delete energy;
}

double calculateEnergy(int *image, int* binary, int width, int height, int bitDepth, double c0, double c1, double lambda1, double lambda2, double beta)
{
	int nodeCount, x, y;;
	double value, maxIntensity, fg, bg, energy=0;
	int nh[16] = {-1,0,-1,-1,0,-1,1,-1,1,0,1,1,0,1,-1,1};

	maxIntensity = pow(2,bitDepth)-1;
	bg = c0*maxIntensity;
	fg = c1*maxIntensity;

	/****** Build Unary Term *************************/
	for (int j = 0; j < height; ++j)
	{
		for (int i = 0; i < width; ++i)
		{
			nodeCount = i + j * width;
			energy += binary[nodeCount]*abs(image[nodeCount]-fg)+(1-binary[nodeCount])*abs(image[nodeCount]-bg);
		}
	}

	/******* Build pairwise terms ********************/
	for (int j = 0; j < height; ++j)
	{
		for (int i = 0; i < width; ++i)
		{
			nodeCount = i + j * width;
			for (int k = 0; k < 16; k += 2)
			{
				x = i + nh[k];
				y = j + nh[k + 1];
				//if not outside of image
				if (!(x < 0 || x >= width || y < 0 || y >= height))
				{
					if(binary[nodeCount]!=binary[x + y * width])
					{
						value = lambda1 + lambda2 * exp(-beta * pow(image[nodeCount] - image[x + y * width], 2));
						energy += value;
					}
				}
			}
		}
	}
	return energy;
}

double calculateError(int *binaryLabel, int *groundTruthLabel, int width, int height)
{
	int nodeCount=0, nBinaryLabels=0, nGroundTruthLabels=0;
	double intersection, totalUnion, minimum, totalError=0, size, totalSize=0;

	nodeCount=width*height;
	//count labels
	for(int i=0; i<nodeCount; ++i)
	{
		if(nBinaryLabels<binaryLabel[i])
			nBinaryLabels = binaryLabel[i];
		if(nGroundTruthLabels<groundTruthLabel[i])
			nGroundTruthLabels = groundTruthLabel[i];
	}

	//create label vectors
	unsigned char** gtv = new unsigned char*[nGroundTruthLabels];
	for(int i=0; i<nGroundTruthLabels; ++i)
		gtv[i] = new unsigned char[nodeCount];
	unsigned char** bv = new unsigned char*[nBinaryLabels];
	for(int i=0; i<nBinaryLabels; ++i)
		bv[i] = new unsigned char[nodeCount];

	for(int j=0; j<nodeCount; ++j)
	{
		for(int i=1; i<=nGroundTruthLabels; ++i)
		{
			if(groundTruthLabel[j] == i)
				gtv[i-1][j] = 1;
			else
				gtv[i-1][j] = 0;
		}
		for(int i=1; i<=nBinaryLabels; ++i)
		{
			if(binaryLabel[j] == i)
				bv[i-1][j] = 1;
			else
				bv[i-1][j] = 0;
		}
	}

	for(int i=0; i<nGroundTruthLabels; ++i)
	{
		minimum = 0.;
		size = 0;
		for(int j=0; j<nBinaryLabels; ++j)
		{
			intersection=0;
			totalUnion = 0;
			for(int k=0; k<nodeCount; ++k)
			{
				intersection+=gtv[i][k] & bv[j][k];
				totalUnion+=gtv[i][k] | bv[j][k];
				size+=gtv[i][k];
			}
			minimum=fmax(minimum,intersection/totalUnion);
		}
		totalSize+=size;
		totalError+=size*minimum;
	}

	//clean up
	for(int i=0; i<nGroundTruthLabels; ++i)
		delete[] gtv[i];

	for(int i=0; i<nBinaryLabels; ++i)
		delete[] bv[i];
	delete[] gtv;
	delete[] bv;

	return totalError/totalSize;
}

} /* end namespace elib */
