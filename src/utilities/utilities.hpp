/*
 * utilities.hpp
 *
 *  Created on: Jul 3, 2013
 *      Author: kthierbach
 */

#ifndef UTILITIES_HPP_
#define UTILITIES_HPP_

#include <boost/regex.hpp>
#include <string>
#include <vector>

namespace elib
{

class Utilities
{
	public:
		Utilities();
		virtual ~Utilities();

		static std::string createFileName(std::string folder, std::string file_name, std::string extension, int index, int length=4);
		static std::string getTime();
};

} /* namespace elib */
#endif /* UTILITIES_HPP_ */
