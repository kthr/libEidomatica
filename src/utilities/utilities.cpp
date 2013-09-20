/*
 * utilities.cpp
 *
 *  Created on: Jul 3, 2013
 *      Author: kthierbach
 */

#include "utilities.hpp"

#include <boost/filesystem.hpp>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace elib
{

Utilities::Utilities()
{
	// TODO Auto-generated constructor stub

}

Utilities::~Utilities()
{
	// TODO Auto-generated destructor stub
}

std::string Utilities::createFileName(std::string folder, std::string file_name, std::string extension, int index, int length)
{
	std::stringstream name;
	boost::filesystem::path dir(folder);
	name << file_name << std::setw(length) << std::setfill('0') << index << extension;
	dir /= name.str();
	return dir.string();

}
std::string Utilities::getTime()
{
	std::stringstream tmp;
	time_t rawtime;
	struct tm * ptm;

	time(&rawtime);
	ptm = localtime(&rawtime);
	tmp << ptm->tm_year + 1900 << "-" << ptm->tm_mon +1 << "-" << ptm->tm_mday << "_"
		<< std::setw(2) << std::setfill('0') << ptm->tm_hour << ":" << std::setw(2) << std::setfill('0') << ptm->tm_min << ":" << std::setw(2) << std::setfill('0') << ptm->tm_sec;

	return tmp.str();
}

} /* namespace elib */
