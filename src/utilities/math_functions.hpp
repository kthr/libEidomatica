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

	template <typename T>
	inline T cot(T val)
	{
		return 1/tan(val);
	}

	template <typename T>
	inline int sgn(T val)
	{
	    return (T(0) < val) - (val < T(0));
	}

	template <typename T>
	inline int isnan(T val)
	{
		return val != val;
	}

} /*end namespace elib*/


#endif /* MATH_FUNCTIONS_HPP_ */
