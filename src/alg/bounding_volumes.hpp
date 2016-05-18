/*
 * bounding_volumes.hpp
 *
 *  Created on: May 18, 2016
 *      Author: kthierbach
 */

#ifndef ALG_BOUNDING_VOLUMES_HPP_
#define ALG_BOUNDING_VOLUMES_HPP_

#include <algorithm>
#include <vector>

#include "templates/tensor.hpp"

namespace elib
{

class BoundingVolumes
{
	public:
		BoundingVolumes(const std::shared_ptr<Tensor<float>> pts, int volume_type, bool mode);
		virtual ~BoundingVolumes();

		std::vector<double> getBoundary();

	private:
		std::shared_ptr<Tensor<float>> tensor;
		int volume_type;
		int mode;
};

} /* namespace elib */

#endif /* ALG_BOUNDING_VOLUMES_HPP_ */
