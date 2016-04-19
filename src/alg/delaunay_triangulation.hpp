/*
 * delaunay_triangulation.hpp
 *
 *  Created on: Apr 19, 2016
 *      Author: kthierbach
 */

#ifndef ALG_DELAUNAY_TRIANGULATION_HPP_
#define ALG_DELAUNAY_TRIANGULATION_HPP_

#include <algorithm>
#include <vector>

#include "templates/tensor.hpp"

namespace elib
{

	class DelaunayTriangulation
	{
		public:
			DelaunayTriangulation(const std::shared_ptr<Tensor<float>> pts);
			virtual ~DelaunayTriangulation();

			std::vector<float> getTriangulation();
		private:
			std::shared_ptr<Tensor<float>> tensor;
	};

} /* namespace elib */

#endif /* ALG_DELAUNAY_TRIANGULATION_HPP_ */
