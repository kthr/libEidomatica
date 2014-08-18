/*
 * HDF5Reader.cpp
 *
 *  Created on: Jul 30, 2014
 *      Author: kthierbach
 */

#include "hdf5_reader.hpp"

#include <boost/filesystem.hpp>
#include <math.h>
#include <sstream>

#include "io/hdf5_wrapper.hpp"

namespace elib
{

HDF5Reader::HDF5Reader()
{

}

HDF5Reader::HDF5Reader(MLINK mlp, std::string file_name) : file_name(file_name), mlp(mlp)
{
}

HDF5Reader::~HDF5Reader()
{
}

void readAnnotations(std::vector<std::string> &object_names)
{

}

void HDF5Reader::readData(std::vector<std::string> &dataset_names)
{
	MLINK loop = NULL;
	MLENV env = NULL;
	int error;

	env = MLInitialize((char *)0);
	if(env == (MLENV)0)
	{
		throw H5Exception("Unable to create loopback environment!");
	}
	loop = MLLoopbackOpen(env, &error);
	if(loop == (MLINK)0 || error != MLEOK)
	{
		throw H5Exception("Unable to open loopback link!");
	}
	try
	{
		H5F file = H5F(file_name.c_str());
		if(dataset_names.size() > 1)
		{
			MLPutFunction(loop, "List", dataset_names.size());
		}
		for (std::string dataset_name : dataset_names)
		{
			H5O object(file, dataset_name);
			H5O_info_t object_info;
			if (H5Oget_info(object.getId(), &object_info) < 0)
			{
				throw H5Exception("Could not read object info for '" + dataset_name + "'!");
			}
			if (object_info.type != H5O_TYPE_DATASET)
			{
				throw H5Exception("'" + dataset_name + "' is not a dataset!");
			}
			H5D dataset(file, dataset_name);
			H5T datatype(dataset);
			H5T_class_t typeclass = H5Tget_class(datatype.getId());
			switch (typeclass)
			{
				case H5T_INTEGER:
				{
					readIntegerData(loop, dataset_name, dataset);
				}
					break;
				case H5T_FLOAT:
				{
					readFloatData(loop, dataset_name, dataset);
				}
					break;
				case H5T_STRING:
				{
					readStringData(loop, dataset_name, dataset);
				}
					break;
				default:
				{
					throw H5Exception("Dataset type not supported for dataset '" + dataset_name + "'!");
				}
					break;
			}
		}
	}
	catch (H5Exception &e)
	{
		MLDeinitialize(env);
		MLClose(loop);
		MLPutSymbol(mlp, "$Failed");
		throw e;
	}
	MLTransferExpression(mlp, loop);
	MLClose(loop);
	MLDeinitialize(env);
}
void HDF5Reader::readNames(std::vector<std::string> &names, std::vector<std::string> &roots, int depth)
{
	try
	{
		H5F file = H5F(file_name.c_str());
		for (std::string root : roots)
		{
			if (H5Oexists_by_name(file.getId(), root.c_str(), H5P_DEFAULT) != true)
			{
				throw H5Exception("Group '" + root + "' doesn't exist in file '" + file_name + "'!");
			}
			H5O object = H5O(file, root);
			std::vector<std::string> tmp;
			if (depth == 0)
			{
				if (H5Lvisit(object.getId(), H5_INDEX_NAME, H5_ITER_NATIVE, put_link_name, &tmp) < 0)
				{
					throw H5Exception("Failed to visit object '" + root + "'");
				}
			}
			boost::filesystem::path dir(root);
			for (int i = 0; i < tmp.size(); ++i)
			{
				tmp[i] = (dir / boost::filesystem::path(tmp[i])).string();
			}
			names.insert(names.end(), tmp.begin(), tmp.end());
		}
	} catch (H5Exception &e)
	{
		throw e;
	}
}

void HDF5Reader::readIntegerData(MLINK loop, std::string dataset_name, const H5D &dataset)
{
	try
	{
		H5S dataspace(dataset);
		/* Get the number of dimensions in this dataset */
		int rank = dataspace.getSimpleExtentNDims();
		/*  Get the size of each dimension */
		std::vector<hsize_t> dimensions(rank);
		dataspace.getSimpleExtentDims(dimensions.data());

		std::vector<long int> long_dim(rank);
		std::vector<int> int_dim(rank);
		for (int j = 0; j < rank; j++)
		{
			long_dim[j] = dimensions[j];
			int_dim[j] = dimensions[j];
		}

		hsize_t number_elements = 1;
		for(auto i : dimensions)
		{
			number_elements *= i;
		}

		H5T datatype(dataset);
		/* get size of integer in bytes */
		size_t size = datatype.getSize();

		if(size == 1 || size == 2)
		{
			std::vector<short> data(number_elements);
			if (H5Dread(dataset.getId(), H5T_NATIVE_SHORT, H5S_ALL, dataspace.getId(), H5P_DEFAULT, data.data()) < 0)
			{
				throw H5Exception("Failed to read data for dataset " + dataset_name);
			}
			if (rank == 0)
				MLPutInteger16(loop, data[0]);
			else
				MLPutInteger16Array(loop, data.data(), int_dim.data(), 0, rank);
		}
		else if(size == 4)
		{
			std::vector<int> data(number_elements);
			if (H5Dread(dataset.getId(), datatype.getNativeId(), H5S_ALL, dataspace.getId(), H5P_DEFAULT, data.data()) < 0)
			{
				throw H5Exception("Failed to read data for dataset " + dataset_name);
			}
			if (rank == 0)
				MLPutInteger(loop, data[0]);
			else
				MLPutIntegerArray(loop, data.data(), long_dim.data(), 0, rank);

		}
		else if(size == 8)
		{
			std::vector<mlint64> data(number_elements);
			if (H5Dread(dataset.getId(), datatype.getNativeId(), H5S_ALL, dataspace.getId(), H5P_DEFAULT, data.data()) < 0)
			{
				throw H5Exception("Failed to read data for dataset " + dataset_name);
			}
			if (rank == 0)
				MLPutInteger64(loop, data[0]);
			else
				MLPutInteger64Array(loop, data.data(), int_dim.data(), 0, rank);
		}
		else
		{
			throw H5Exception("Bitdepth not supported for " + dataset_name + "!");
		}
	}
	catch(H5Exception &e)
	{
		throw e;
	}
}

void HDF5Reader::readFloatData(MLINK loop, std::string dataset_name, const H5D &dataset)
{
	try
	{
		H5S dataspace(dataset);
		/* Get the number of dimensions in this dataset */
		int rank = dataspace.getSimpleExtentNDims();
		/*  Get the size of each dimension */
		std::vector<hsize_t> dimensions(rank);
		dataspace.getSimpleExtentDims(dimensions.data());

		std::vector<long int> long_dim(rank);
		std::vector<int> int_dim(rank);
		for (int j = 0; j < rank; j++)
		{
			long_dim[j] = dimensions[j];
			int_dim[j] = dimensions[j];
		}

		hsize_t number_elements = 1;
		for(auto i : dimensions)
		{
			number_elements *= i;
		}

		H5T datatype(dataset);
		/* get size of integer in bytes */
		size_t size = datatype.getSize();

		if(size == 4)
		{
			std::vector<float> data(number_elements);
			if (H5Dread(dataset.getId(), H5T_NATIVE_SHORT, H5S_ALL, dataspace.getId(), H5P_DEFAULT, data.data()) < 0)
			{
				throw H5Exception("Failed to read data for dataset " + dataset_name);
			}
			if (rank == 0)
				MLPutReal32(loop, data[0]);
			else
				MLPutReal32Array(loop, data.data(), int_dim.data(), 0, rank);
		}
		else if(size == 8)
		{
			std::vector<double> data(number_elements);
			if (H5Dread(dataset.getId(), datatype.getNativeId(), H5S_ALL, dataspace.getId(), H5P_DEFAULT, data.data()) < 0)
			{
				throw H5Exception("Failed to read data for dataset " + dataset_name);
			}
			if (rank == 0)
				MLPutReal64(loop, data[0]);
			else
				MLPutReal64Array(loop, data.data(), int_dim.data(), 0, rank);
		}
		else
		{
			throw H5Exception("Bitdepth not supported for " + dataset_name + "!");
		}
	}
	catch(H5Exception &e)
	{
		throw e;
	}

}

void HDF5Reader::readStringData(MLINK loop, std::string dataset_name, const H5D &dataset)
{
	try
	{
		H5T datatype(dataset);
		size_t size = datatype.getSize();
		/* Include space for a null terminator in case it isn't in the data */
		std::vector<char> str(size + 1);
		str[size] = '\0';
		if (H5Dread(dataset.getId(), datatype.getNativeId(), H5S_ALL, H5S_ALL, H5P_DEFAULT, (void *) str.data()) < 0)
		{
			throw(H5Exception("Failed to read data for dataset " + dataset_name));
		}
		MLPutString(loop, str.data());
	}
	catch(H5Exception &e)
	{
		throw e;
	}
}

herr_t put_group_name(hid_t o_id, const char *name, const H5O_info_t *info, void *op_data)
{
	std::vector<std::string> *groupNames = (std::vector<std::string> *) op_data;
	if (info->type == H5O_TYPE_DATASET)
		groupNames->push_back(std::string(name));
	return 0;
}

herr_t put_link_name(hid_t g_id, const char *name, const H5L_info_t *info, void *op_data)
{
	std::vector<std::string> *linkNames = (std::vector<std::string> *) op_data;
	if (info->type == H5L_TYPE_SOFT || info->type == H5L_TYPE_HARD)
		linkNames->push_back(std::string(name));
	return 0;
}

} /* namespace elib */
