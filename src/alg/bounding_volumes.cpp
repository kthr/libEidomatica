/*
 * bounding_volumes.cpp
 *
 *  Created on: May 18, 2016
 *      Author: kthierbach
 */

#include "bounding_volumes.hpp"

#include <math.h>

#include <CGAL/Cartesian.h>
#include <CGAL/Min_ellipse_2.h>
#include <CGAL/Min_ellipse_2_traits_2.h>
#include <CGAL/Exact_rational.h>
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Min_circle_2.h>
#include <CGAL/Min_circle_2_traits_2.h>

namespace elib
{

	BoundingVolumes::BoundingVolumes(const std::shared_ptr<Tensor<float>> pts, int volume_type, bool mode) : tensor(pts), volume_type(volume_type), mode(mode)
	{
	}

	BoundingVolumes::~BoundingVolumes()
	{
		// TODO Auto-generated destructor stub
	}



	std::vector<double> BoundingVolumes::getBoundary()
	{
		using namespace CGAL;
		typedef  Exact_rational             NT;
		typedef  Cartesian<NT>              K;
		typedef  Point_2<K>                 Point;
		typedef  Min_ellipse_2_traits_2<K>  Traits;
		typedef  Min_ellipse_2<Traits>      Min_ellipse;

		typedef  Exact_predicates_exact_constructions_kernel KC;
		typedef  Min_circle_2_traits_2<KC>  TraitsC;
		typedef  Min_circle_2<TraitsC>      Min_circle;

		std::list<Point> points;
		float *data = tensor->getData();
		for(int i=0; i<tensor->getFlattenedLength(); i+=2)
		{
			points.push_back(Point(data[i], data[i+1]));
		}

		std::vector<double> parameters;
		switch(volume_type)
		{
			case 0: // ellipse fit
			{
				parameters.resize(6);
				Min_ellipse me2(points.begin(), points.end(), mode);
				me2.ellipse().double_coefficients(parameters[0], parameters[2], parameters[1], parameters[3] ,parameters[4], parameters[5]);
			}
			break;
			case 1: // sphere fit
			{
//				Min_circle mc2(points.begin(), points.end(), mode);
//				double radius = sqrt(mc2.circle().squared_radius().get_relative_precision_of_to_double());
//				parameters.push_back(mc2.circle().center().x().get_relative_precision_of_to_double());
//				parameters.push_back(mc2.circle().center().y().get_relative_precision_of_to_double());
//				parameters.push_back(radius);
			}
			break;
		}
		return parameters;
	}

} /* namespace elib */
