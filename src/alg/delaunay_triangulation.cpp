/*
 * delaunay_triangulation.cpp
 *
 *  Created on: Apr 19, 2016
 *      Author: kthierbach
 */

#include "delaunay_triangulation.hpp"

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Projection_traits_xy_3.h>
#include <CGAL/Delaunay_triangulation_2.h>

namespace elib
{

	DelaunayTriangulation::DelaunayTriangulation(const std::shared_ptr<Tensor<float>> pts) : tensor(pts)
	{
	}

	DelaunayTriangulation::~DelaunayTriangulation()
	{
	}

	std::vector<float> DelaunayTriangulation::getTriangulation()
	{
		typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
		typedef CGAL::Projection_traits_xy_3<K>  Gt;
		typedef CGAL::Delaunay_triangulation_2<Gt> Delaunay;
		typedef K::Point_2 Point;

		std::list<Point> points;
		float *data = tensor->getData();
		for(int i=0; i<tensor->getFlattenedLength(); i+=2)
		{
			Point p(data[i], data[i+1]);
			points.push_back(p);
		}

		Delaunay dt(points.begin(), points.end());
	}

} /* namespace elib */
