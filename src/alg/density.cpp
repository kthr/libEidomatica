/*
 * density.cpp
 *
 *  Created on: Nov 9, 2013
 *      Author: kthierbach
 */

#include "density.hpp"

#include <cmath>
#include <iostream>

#include "glm/gtx/norm.hpp"
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
		elib::isnan(rank = params.getIntegerParameter("Rank")) ||
		(dimensions = params.getIntegerTensorParameter("Dimensions")) == nullptr ||
		(original_dimensions = params.getIntegerTensorParameter("OriginalDimensions")) == nullptr ||
		elib::isnan(radius = params.getDoubleParameter("Radius")) ||
		elib::isnan(lateral_projection_range = params.getDoubleParameter("LateralProjectionRange")) ||
		elib::isnan(band_width = params.getDoubleParameter("BandWidth")) ||
		elib::isnan(type = params.getIntegerParameter("Type")) ||
		elib::isnan(central_meridian = params.getDoubleParameter("CentralMeridian")) ||
		elib::isnan(standard_parallel = params.getDoubleParameter("StandardParallel"))
	)
	{
		return nullptr;
	}
	Tensor<double> *density = new Tensor<double>(rank, dimensions->getData());
	double *tensor_data = density->getData();
	int *dims = const_cast<Tensor<int>* >(dimensions)->getData();
	int *original_dims = const_cast<Tensor<int>* >(original_dimensions)->getData();


	if(rank == 2)
	{
		double* point_data = points.getData();
		switch(type)
		{
			case static_cast<int>(density_type::BONNE):
				for(int k=0; k<points.getFlattenedLength(); k+=2)
				{
					polar_points.push_back(toPolar(glm::vec2(point_data[k],point_data[k+1]), radius, lateral_projection_range, original_dimensions->getData()));
				}
				for(int j = 0; j < dims[1]; ++j)
				{
					for (int i = 0; i < dims[0]; ++i)
					{
						coordinates.push_back(glm::ivec2(i, j));
						p = glm::vec3((double(i)/double(dims[0]))*M_PI*2-M_PI, (double(j)/double(dims[1]))*M_PI*2-M_PI,radius);
						polar_coordinates.push_back(inverseBonne(p, standard_parallel, central_meridian));
					}
				}
				for(unsigned int i=0; i<polar_coordinates.size(); ++i)
				{
					p = polar_coordinates[i];
					c = coordinates[i];
					if(fabs(p.x) <= M_PI && fabs(p.y) <= M_PI_2)
					{
						for(unsigned int j=0; j<polar_points.size(); ++j)
						{
							if(greatCircleDistance(p+glm::vec3(M_PI, M_PI_2, 0), polar_points[j]) <= band_width)
								tensor_data[c.x + c.y*dims[0]] += 1;
						}
					}
					else
					{
						tensor_data[c.x + c.y*dims[0]] = 0;
					}
				}
				break;
			case static_cast<int>(density_type::CARTESIAN):
				for(int k=0; k<points.getFlattenedLength(); k+=2)
				{
					polar_points.push_back(glm::vec3(point_data[k]/original_dims[0],point_data[k+1]/original_dims[1],0));
				}
				for(int j=0; j<dims[1]; ++j)
				{
					for(int i=0; i<dims[0]; ++i)
					{
						p = glm::vec3(i/dims[0],j/dims[1],0);
						for(auto k=polar_points.begin(); k!=polar_points.end(); ++k)
							if(std::sqrt(glm::l2Norm(p-*k)) <= band_width)
								tensor_data[i + j*dims[0]] += 1;
					}
				}
				break;
			case static_cast<int>(density_type::MERCATOR):
				for(int k=0; k<points.getFlattenedLength(); k+=2)
				{
					polar_points.push_back(toPolar(glm::vec2(point_data[k],point_data[k+1]), radius, lateral_projection_range, original_dimensions->getData()));
				}
				for(int j=0; j<dims[1]; ++j)
				{
					for(int i=0; i<dims[0]; ++i)
					{
						coordinates.push_back(glm::ivec2(i,j));
						polar_coordinates.push_back(toPolar(glm::vec2(i,j), radius, lateral_projection_range, dims));
					}
				}
				for(unsigned int i=0; i<polar_coordinates.size(); ++i)
				{
					for(unsigned int j=0; j<polar_points.size(); ++j)
					{
						c = coordinates[i];
						if(greatCircleDistance(polar_coordinates[i],polar_points[j]) <= band_width)
							tensor_data[c.x + c.y*dims[0]] += 1;
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

inline glm::vec3 Density::toPolar(glm::vec2 point, float radius, float lateral_projection_range, const int* dimension)
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
inline bool Density::bonneRegionFunction(glm::vec3 p, double standard_parallel, double central_meridian)
{
	glm::vec3 tmp=inverseBonne(p, standard_parallel, central_meridian);
	return 	fabs(tmp.y)<=M_PI_2 && fabs(tmp.x)<=M_PI;
}
glm::vec3 Density::inverseBonne(glm::vec3 p, double standard_parallel, double central_meridian)
{
	double 	tmp = cot(standard_parallel),
			rho = sgn(standard_parallel)*sqrt(pow(p.x,2)+pow(tmp-p.y,2)),
			latitude = tmp+standard_parallel-rho;
	std::cout << "inverse bonne: " << p.x << "," << p.y << "," << p.z << " " << central_meridian+rho*atan2(p.x,(tmp-p.y))/cos(latitude) << "," << latitude << "," << p.z << std::endl;
	return glm::vec3(central_meridian+rho*atan2(p.x,(tmp-p.y))/cos(latitude), latitude, p.z);
}

} /* namespace elib */
