/*
 * alpha_shapes.h
 *
 *  Created on: Apr 5, 2016
 *      Author: kthierbach
 */

#ifndef ALPHA_SHAPES_H_
#define ALPHA_SHAPES_H_

#include <algorithm>

#include "templates/tensor.hpp"

namespace elib
{

class AlphaShapes
{
	public:
		AlphaShapes(const std::shared_ptr<Tensor<float>> pts, float alpha, bool mode);
		virtual ~AlphaShapes();

		std::vector<float> getSegments();

	private:
		std::shared_ptr<Tensor<float>> tensor;
		float alpha;
		bool mode;
};

} /* namespace elib */

#endif /* ALPHA_SHAPES_H_ */
