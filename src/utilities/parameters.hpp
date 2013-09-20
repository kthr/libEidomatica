/*
 * parameters.hpp
 *
 *  Created on: Jun 10, 2013
 *      Author: kthierbach
 */

#ifndef PARAMETERS_HPP_
#define PARAMETERS_HPP_

#include <stdint.h>
#include <string>
#include <set>
#include <unordered_map>
#include <vector>

#include "c_wrapper.h"

namespace elib
{

class Parameters
{
	public:
		Parameters();
		Parameters(parameters *param);
		virtual ~Parameters();
		bool addParameter(std::string identifier, int value);
		bool addParameter(std::string identifier, double value);
		const int* getIntegerParameter(std::string identifier) const;
		const double* getDoubleParameter(std::string identifier) const;


	private:
		std::set<std::string> identifiers;
		std::unordered_map<std::string, int> integer_params;
		std::unordered_map<std::string, double> double_params;
};

} /* namespace elib */

#endif /* PARAMETERS_HPP_ */
