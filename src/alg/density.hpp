/*
 * density.hpp
 *
 *  Created on: Nov 9, 2013
 *      Author: kthierbach
 */

#ifndef DENSITY_HPP_
#define DENSITY_HPP_

#include <vector>

#include "glm/glm.hpp"
#include "utilities/parameters.hpp"
#include "templates/tensor.hpp"

namespace elib
{

class Density
{
	public:
		static elib::Tensor<double>* calculateDensity(elib::Tensor<double> &points, elib::Parameters &params);
		static elib::Tensor<double>* calculateFeatureMap(elib::Tensor<double> &points, elib::Tensor<double> &features, elib::Parameters &params);

		enum class density_type {BONNE, CARTESIAN, MERCATOR};
	private:
		static inline glm::vec3 toPolar(glm::vec2 point, float radius, float lateral_projection_range, const int* dimension);
		static inline double greatCircleDistance(glm::vec3 p1, glm::vec3 p2);
		static inline bool bonneRegionFunction(glm::vec3 p, double standard_parallel, double central_meridian);
		static glm::vec3 inverseBonne(glm::vec3 p, double standard_parallel, double central_meridian);
};

} /* namespace elib */
#endif /* DENSITY_HPP_ */
