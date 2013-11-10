/*
 * math_functions.hpp
 *
 *  Created on: Nov 10, 2013
 *      Author: kthierbach
 */

#ifndef MATH_FUNCTIONS_HPP_
#define MATH_FUNCTIONS_HPP_

#include <cmath>

namespace elib
{
	template<typename T>
	inline T haversine(T value)
	{
		return pow(sin(value/2),2);
	}

} /*end namespace elib*/


#endif /* MATH_FUNCTIONS_HPP_ */
