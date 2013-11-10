/*
 * parameters.cpp
 *
 *  Created on: Jun 10, 2013
 *      Author: kthierbach
 */

#include "parameters.hpp"

namespace elib{

Parameters::Parameters()
{
}
Parameters::~Parameters()
{
}

bool Parameters::addParameter(std::string identifier, int value)
{
	auto res = integer_params.insert(std::pair<std::string,int>(identifier,value));
	return res.second;
}

bool Parameters::addParameter(std::string identifier, double value)
{
	auto res = double_params.insert(std::pair<std::string,double>(identifier,value));
	return res.second;
}

bool Parameters::addParameter(std::string identifier, elib::Tensor<int> &value)
{
	auto res = integer_tensor_params.insert(std::pair<std::string, elib::Tensor<int> >(identifier, value));
	return res.second;
}

const int* Parameters::getIntegerParameter(std::string identifier) const
{
	auto res = integer_params.find(identifier);
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
	auto res = double_params.find(identifier);
	if(res == double_params.end())
	{
		return nullptr;
	}
	else
	{
		return &(res->second);
	}
}

const elib::Tensor<int>* Parameters::getIntegerTensorParameter(std::string identifier) const
{
	auto res = integer_tensor_params.find(identifier);
	if(res == integer_tensor_params.end())
	{
		return nullptr;
	}
	else
	{
		return &(res->second);
	}
}

} /* end namespace elib */

