/*
 * density.cpp
 *
 *  Created on: Nov 9, 2013
 *      Author: kthierbach
 */

#include "density.hpp"

#include <cmath>

#include "templates/tensor.hpp"
#include "utilities/math_functions.hpp"

namespace elib
{

Tensor<double>* Density::calculateDensity(elib::Tensor<double> &points, elib::Parameters &params)
{
	std::vector<glm::vec2> coordinates;
	glm::vec2 p;
	std::vector<glm::vec3> polar_coordinates, polar_points;

	const int *rank, *type;
	const Tensor<int> *dimensions, *original_dimensions;
	const double *radius, *lateral_projection_range, *band_width, *standard_parallel, *central_meridian;
	if(
		(rank = params.getIntegerParameter("Rank")) == nullptr ||
		(dimensions = params.getIntegerTensorParameter("Dimensions")) == nullptr ||
		(original_dimensions = params.getIntegerTensorParameter("OriginalDimensions")) == nullptr ||
		(radius = params.getDoubleParameter("Radius")) == nullptr ||
		(lateral_projection_range = params.getDoubleParameter("LateralProjectionRange")) == nullptr ||
		(band_width = params.getDoubleParameter("BandWidth")) == nullptr ||
		(standard_parallel = params.getDoubleParameter("StandardParallel")) == nullptr ||
		(central_meridian = params.getDoubleParameter("CentralMeridian")) == nullptr ||
		(type = params.getIntegerParameter("Type")) == nullptr
	)
	{
		return nullptr;
	}
	Tensor<double> *density = new Tensor<double>(*rank, const_cast<Tensor<int>* >(dimensions)->getData());
	double *tensor_data = density->getData();
	int *dims = const_cast<Tensor<int>* >(dimensions)->getData();

	if(*rank == 2)
	{
		for(int j=0; j<dims[1]; ++j)
		{
			for(int i=0; i<dims[0]; ++i)
			{
				coordinates.push_back(glm::vec2(i,j));
				polar_coordinates.push_back(toPolar(glm::vec2(i,j), *radius, *lateral_projection_range, const_cast<Tensor<int>* >(dimensions)->getData()));
			}
		}
		double* point_data = points.getData();
		for(int k=0; k<points.getFlattenedLength(); k+=2)
		{
			polar_points.push_back(toPolar(glm::vec2(point_data[k],point_data[k+1]), *radius, *lateral_projection_range, const_cast<Tensor<int>* >(original_dimensions)->getData()));
		}
		for(int i=0; i<polar_coordinates.size(); ++i)
		{
			for(int j=0; j<polar_points.size(); ++j)
			{
				p = coordinates[i];
				tensor_data[int(p.x+ p.y*dims[1])] += exp(-greatCircleDistance(polar_coordinates[i],polar_points[j])/(*band_width));
			}
		}
	}
	return density;
}

inline glm::vec3 Density::toPolar(glm::vec2 point, float radius, float lateral_projection_range, int* dimension)
{
	return glm::vec3((point.x*2*M_PI)/dimension[0],asin(tanh(lateral_projection_range*(-0.5+point.y/dimension[1])))+M_PI_2,radius);
}

inline double Density::greatCircleDistance(glm::vec3 p1, glm::vec3 p2)
{
	return p1.z*2*asin(
				sqrt(
					haversine((p2.y-M_PI_2)-(p1.y-M_PI_2)) + cos(p1.y-M_PI_2)*cos(p2.y-M_PI_2)*haversine(p2.x-p1.x)
				)
			);
}


} /* namespace elib */
