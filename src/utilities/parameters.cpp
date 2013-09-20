/*
 * parameters.cpp
 *
 *  Created on: Jun 10, 2013
 *      Author: kthierbach
 */

#include "parameters.hpp"

#include <algorithm>
#include <iostream>
#include <sstream>

namespace elib{

Parameters::Parameters()
{
}
Parameters::Parameters(parameters *params)
{
	for(int i=0; i<params->int_params_size; ++i)
	{
		this->addParameter(std::string(params->int_names[i]), int(params->int_params[i]));
	}
	for(int i=0; i<params->double_params_size; ++i)
	{
		this->addParameter(std::string(params->double_names[i]), params->double_params[i]);
	}
}

Parameters::~Parameters()
{
}

bool Parameters::addParameter(std::string identifier, int value)
{
	std::pair<std::unordered_map<std::string,int>::iterator,bool> res = integer_params.insert(std::pair<std::string,int>(identifier,value));
	return res.second;
}

bool Parameters::addParameter(std::string identifier, double value)
{
	std::pair<std::unordered_map<std::string,double>::iterator,bool> res = double_params.insert(std::pair<std::string,double>(identifier,value));
	return res.second;
}

const int* Parameters::getIntegerParameter(std::string identifier) const
{
	std::unordered_map<std::string,int>::const_iterator res = integer_params.find(identifier);
	if(res == integer_params.end())
	{
		return nullptr;
	}
	else
	{
		return &(res->second);
	}
}

const double* Parameters::getDoubleParameter(std::string identifier) const
{
	std::unordered_map<std::string,double>::const_iterator res = double_params.find(identifier);
	if(res == double_params.end())
	{
		return nullptr;
	}
	else
	{
		return &(res->second);
	}
}

} /* end namespace elib */
