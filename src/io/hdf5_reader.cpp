/*
 * HDF5Reader.cpp
 *
 *  Created on: Jul 30, 2014
 *      Author: kthierbach
 */

#include "hdf5_reader.hpp"

#include "hdf5_wrapper.hpp"

namespace elib
{

HDF5Reader::HDF5Reader(WolframLibraryData libData, std::string file_name)
{
	this->file_name=file_name;
	this->libData=libData;
}

HDF5Reader::~HDF5Reader()
{
}

void HDF5Reader::readDatasetNames(std::vector<std::string> &names, std::string root, int depth)
{
}
void HDF5Reader::readData(std::vector<std::string> &dataset_names)
{

}
void HDF5Reader::readGroupNames(std::vector<std::string> &names, std::string root, int depth)
{
	H5F file = H5F(file_name.c_str());
	if(!H5Oexists_by_name(file.getId(), root.data(), H5P_DEFAULT))
	{
		throw H5Exception("Group " + root + " doesn't exist in file " + file_name + "!");
	}
	H5O object = H5O(file, root);
	if(depth==0)
	{
		if(H5Ovisit(object.getId(), H5_INDEX_NAME, H5_ITER_NATIVE, put_group_name, names.data()) < 0)
		{
			throw H5Exception("Failed to visit ");
		}
		if(H5Lvisit(object.getId(), H5_INDEX_NAME, H5_ITER_NATIVE, put_link_name, names.data()) < 0)
		{
			throw H5Exception("Failed to visit ");
		}
	}
	for(int i=0; i<names.size(); ++i)
	{
		names[i]= root + names[i];
	}
}

herr_t put_group_name(hid_t o_id, const char *name, const H5O_info_t *info, void *op_data)
{
	std::vector <std::string> *groupNames = (std::vector<std::string> *) op_data;
	if (info->type == H5O_TYPE_GROUP)
		groupNames->push_back("/" + std::string(name));
	return 0;
}

herr_t put_link_name( hid_t g_id, const char *name, const H5L_info_t *info, void *op_data)
{
	std::vector <std::string> *linkNames = (std::vector<std::string> *) op_data;
	if (info->type == H5L_TYPE_SOFT || info->type == H5L_TYPE_HARD)
		linkNames->push_back("/" + std::string(name));
	return 0;
}

} /* namespace elib */
