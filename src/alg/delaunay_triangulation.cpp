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
		typedef CGAL::Delaunay_triangulation_2<K> Delaunay;
		typedef K::Point_2   Point;

		std::list<Point> points;
		float *data = tensor->getData();
		for(int i=0; i<tensor->getFlattenedLength(); i+=2)
		{
			Point p(data[i], data[i+1]);
			points.push_back(p);
		}

		Delaunay dt(points.begin(), points.end());

		std::vector<float> face_vertices;
		for(auto fi = dt.finite_faces_begin(); fi != dt.finite_faces_end(); fi++)
		{
			face_vertices.push_back(fi->vertex(0)->point().hx());
			face_vertices.push_back(fi->vertex(0)->point().hy());
			face_vertices.push_back(fi->vertex(1)->point().hx());
			face_vertices.push_back(fi->vertex(1)->point().hy());
			face_vertices.push_back(fi->vertex(2)->point().hx());
			face_vertices.push_back(fi->vertex(2)->point().hy());
		}
		return face_vertices;
	}

} /* namespace elib */
