/*
 * GraphCutImage.hpp
 *
 *  Created on: Jan 22, 2013
 *      Author: kthierbach
 */

#ifndef GRAPHCUTIMAGE_HPP_
#define GRAPHCUTIMAGE_HPP_

#include <math.h>

#include "templates/image.hpp"
#include "utilities/parameters.hpp"

namespace elib{

Image<short>* graphcut(Image<int> &input_image, Parameters &params);
void graphcutSphere(int* binary, int nVertices, double *vertices, int *prior, int nNeighbors, int *neighbours, int bitDepth, int *intensities, double c0, double c1, double lambda1, double lambda2);
double calculateEnergy(int *image, int* binary, int width, int height, int bitDepth, double c0, double c1, double lambda1, double lambda2, double beta);
double calculateError(int *binaryLabel, int *groundTruthLabel, int width, int height);

} /* end namespace elib */

#endif /* GRAPHCUTIMAGE_HPP_ */
