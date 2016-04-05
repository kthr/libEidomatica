/*
 * alpha_shapes.cpp
 *
 *  Created on: Apr 5, 2016
 *      Author: kthierbach
 */

#include "alpha_shapes.hpp"

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Weighted_point.h>
#include <CGAL/Weighted_alpha_shape_euclidean_traits_2.h>
#include <CGAL/Regular_triangulation_2.h>
#include <CGAL/Alpha_shape_2.h>

namespace elib
{

AlphaShapes::AlphaShapes(const std::shared_ptr<Tensor<float>> pts, float alpha, bool mode) : tensor(pts), alpha(alpha), mode(mode)
{
}

AlphaShapes::~AlphaShapes()
{
	// TODO Auto-generated destructor stub
}

std::vector<float> AlphaShapes::getSegments()
{
	typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
	typedef K::FT FT;
	typedef K::Point_2 Point_base;
	typedef CGAL::Weighted_point<Point_base,FT>  Point;
	typedef CGAL::Weighted_alpha_shape_euclidean_traits_2<K> Gt;
	typedef CGAL::Regular_triangulation_vertex_base_2<Gt> Rvb;
	typedef CGAL::Alpha_shape_vertex_base_2<Gt,Rvb> Vb;
	typedef CGAL::Regular_triangulation_face_base_2<Gt> Rf;
	typedef CGAL::Alpha_shape_face_base_2<Gt, Rf>  Fb;
	typedef CGAL::Triangulation_data_structure_2<Vb,Fb> Tds;
	typedef CGAL::Regular_triangulation_2<Gt,Tds> Triangulation_2;
	typedef CGAL::Alpha_shape_2<Triangulation_2>  Alpha_shape_2;

	std::list<Point> points;
	float *data = tensor->getData();
	for(int i=0; i<tensor->getFlattenedLength(); i+=2)
	{
		Point_base p(data[i], data[i+1]);
		points.push_back(Point(p, FT(10)));
	}
	Alpha_shape_2 as2(points.begin(), points.end(), FT(alpha));
	if(mode)
		as2.set_mode(Alpha_shape_2::REGULARIZED);
	else
		as2.set_mode(Alpha_shape_2::GENERAL);

	std::vector<Gt::Segment_2> segments;
	auto out=std::back_inserter(segments);
	for(auto it =  as2.alpha_shape_edges_begin(); it != as2.alpha_shape_edges_end(); ++it)
	{
		*out++ = as2.segment(*it);
	}

	std::vector<float> result;
	for(auto segment : segments)
	{
		result.push_back(segment[0].x());
		result.push_back(segment[0].y());
		result.push_back(segment[1].x());
		result.push_back(segment[1].y());
	}
	return result;
}

} /* namespace elib */

