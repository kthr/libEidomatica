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
	std::vector<glm::ivec2> coordinates;
	std::vector<glm::vec3> polar_coordinates, polar_points;
	glm::vec3 p;
	glm::ivec2 c;

	int rank, type;
	const Tensor<int> *dimensions, *original_dimensions;
	double radius, lateral_projection_range, band_width, central_meridian, standard_parallel;
	if(
		isnan(rank = params.getIntegerParameter("Rank")) ||
		(dimensions = params.getIntegerTensorParameter("Dimensions")) == nullptr ||
		(original_dimensions = params.getIntegerTensorParameter("OriginalDimensions")) == nullptr ||
		isnan(radius = params.getDoubleParameter("Radius")) ||
		isnan(lateral_projection_range = params.getDoubleParameter("LateralProjectionRange")) ||
		isnan(band_width = params.getDoubleParameter("BandWidth")) ||
		isnan(type = params.getIntegerParameter("Type")) ||
		isnan(central_meridian = params.getDoubleParameter("CentralMeridian")) ||
		isnan(standard_parallel = params.getDoubleParameter("StandardParallel"))
	)
	{
		return nullptr;
	}
	Tensor<double> *density = new Tensor<double>(rank, const_cast<Tensor<int>* >(dimensions)->getData());
	double *tensor_data = density->getData();
	int *dims = const_cast<Tensor<int>* >(dimensions)->getData();

	if(rank == 2)
	{
		double* point_data = points.getData();
		for(int k=0; k<points.getFlattenedLength(); k+=2)
		{
			polar_points.push_back(toPolar(glm::vec2(point_data[k],point_data[k+1]), radius, lateral_projection_range, const_cast<Tensor<int>* >(original_dimensions)->getData()));
		}
		switch(type)
		{
			case static_cast<int>(density_type::BONNE):
				for(int j = 0; j < dims[1]; ++j)
				{
					for (int i = 0; i < dims[0]; ++i)
					{
						coordinates.push_back(glm::ivec2(i, j));
						polar_coordinates.push_back(glm::vec3((double(i)/double(dims[0]-1))*M_PI*2-M_PI, (double(j)/double(dims[1]-1))*M_PI*2-M_PI,radius));
					}
				}
				for(int i=0; i<polar_coordinates.size(); ++i)
				{
					p = polar_coordinates[i];
					c = coordinates[i];
					if(bonneRegionFunction(p, standard_parallel, central_meridian))
					{
						for(int j=0; j<polar_points.size(); ++j)
						{
							tensor_data[c.x + c.y*dims[0]] += exp(-greatCircleDistance(inverseBonne(polar_coordinates[i], standard_parallel, central_meridian),polar_points[j])/band_width);
						}
					}
					else
					{
						tensor_data[c.x + c.y*dims[0]] = 0;
					}
				}
				break;
			case static_cast<int>(density_type::MERCATOR):
				for(int j=0; j<dims[1]; ++j)
				{
					for(int i=0; i<dims[0]; ++i)
					{
						coordinates.push_back(glm::ivec2(i,j));
						polar_coordinates.push_back(toPolar(glm::vec2(i,j), radius, lateral_projection_range, dims));
					}
				}
				for(int i=0; i<polar_coordinates.size(); ++i)
				{
					for(int j=0; j<polar_points.size(); ++j)
					{
						c = coordinates[i];
						tensor_data[c.x + c.y*dims[0]] += exp(-greatCircleDistance(polar_coordinates[i],polar_points[j])/band_width);
					}
				}
				break;
			default:
				density=nullptr;
				break;
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
inline bool Density::bonneRegionFunction(glm::vec3 p, const double standard_parallel, const double central_meridian)
{
	glm::vec3 tmp=inverseBonne(p, standard_parallel, central_meridian);
	return 	abs(tmp.y)<=M_PI_2 && abs(tmp.x)<=M_PI;
}
glm::vec3 Density::inverseBonne(glm::vec3 p, double standard_parallel, double central_meridian)
{
	double 	tmp = cot(standard_parallel),
			rho = sgn(standard_parallel)*sqrt(pow(p.x,2)+pow(tmp-p.y,2)),
			latitude = tmp+standard_parallel-rho;
	return glm::vec3(central_meridian+rho*atan2(p.x,(tmp-p.y))/cos(latitude), latitude, p.z);
}

} /* namespace elib */
