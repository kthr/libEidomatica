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

HDF5Reader::HDF5Reader(MLINK mlp, std::string file_name) : file_name(file_name), file(H5F(file_name)), mlp(mlp)
{
}

HDF5Reader::~HDF5Reader()
{
}

void HDF5Reader::readAnnotations(std::vector<std::string> &object_names)
{
	/* Get the list of datasets to be read from Mathematica*/
	long n = object_names.size();
	if (n < 0)
	{
		MLPutFunction(mlp, "List", 0);
		return;
	}

	/* Create a loopback link to store the list until we are sure it can be fully
	 filled. This way if something fails we can abort and send $Failed back. */
	MLINK loopback = NULL;
	MLENV env;
	int error;

	try
	{
		env = MLInitialize((char *)0);
		if(env == (MLENV)0)
		{
			throw H5Exception("Unable to create ml environment!");
		}

		loopback = MLLoopbackOpen(env, &error);
		if(loopback == (MLINK)0 || error != MLEOK)
		{
			throw H5Exception("Unable to open loopback link!");
		}

		if(n>1)
		{
			MLPutFunction(loopback, "List", n);
		}

		/* Loop over all requested datasets */
		for (std::string dataset_name : object_names)
		{
			try{
				hid_t object;
				if (!((object = H5Oopen(file.getId(), dataset_name.c_str(), H5P_DEFAULT)) > 0))
					throw(H5Exception("Could not open object in " + dataset_name));
				int nAttrs;
				switch (H5Iget_type(object))
				{
					case H5I_GROUP:
					{
						H5G group(file, dataset_name);
						nAttrs = group.getNumAttrs();
						MLPutFunction(loopback, "List", nAttrs);
						H5Aiterate(group.getId(), H5_INDEX_NAME, H5_ITER_NATIVE, 0, put_dataset_attribute, &loopback);
					}
						break;
					case H5I_DATASET:
					{
						H5D dataset(file, dataset_name);
						nAttrs = dataset.getNumAttrs();
						MLPutFunction(loopback, "List", nAttrs);
						H5Aiterate(dataset.getId(), H5_INDEX_NAME, H5_ITER_NATIVE, 0, put_dataset_attribute, &loopback);
					}
						break;
					default:
						throw(H5Exception("Reading annotations only supported for Datasets and Groups!"));
						break;
				}
				H5Oclose(object);
			}
			catch(H5Exception &e)
			{
				throw H5Exception("Failed to read annotation in dataset " + dataset_name + ":" + e.what());
			}
		}
	}
	catch (H5Exception &e)
	{
		MLClose(loopback);
		MLDeinitialize(env);
		throw e;
	}

	/* Transfer data from loopback to actual Mathematica link */
	MLTransferToEndOfLoopbackLink(mlp, loopback);
	MLClose(loopback);
	MLDeinitialize(env);
}

void HDF5Reader::readData(std::vector<std::string> &dataset_names)
{
	MLINK loopback = NULL;
	MLENV env;
	int error;

	env = MLInitialize((char *)0);
	if(env == (MLENV)0)
	{
		throw H5Exception("Unable to create ml environment!");
	}

	loopback = MLLoopbackOpen(env, &error);
	if(loopback == (MLINK)0 || error != MLEOK)
	{
		throw H5Exception("Unable to open loopback link!");
	}
	try
	{
		if(dataset_names.size() > 1)
		{
			MLPutFunction(loopback, "List", dataset_names.size());
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
					readIntegerData(loopback, dataset_name, dataset);
				}
					break;
				case H5T_FLOAT:
				{
					readFloatData(loopback, dataset_name, dataset);
				}
					break;
				case H5T_STRING:
				{
					readStringData(loopback, dataset_name, dataset);
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
		MLClose(loopback);
		MLDeinitialize(env);
		throw e;
	}
	MLTransferExpression(mlp, loopback);
	MLClose(loopback);
	MLDeinitialize(env);
}
void HDF5Reader::readNames(std::vector<std::string> &roots, int depth, std::vector<std::string> *names)
{
	try
	{
		if(depth == 0)
		{
			for (std::string root : roots)
			{
				if (H5Oexists_by_name(file.getId(), root.c_str(), H5P_DEFAULT) != true)
				{
					throw H5Exception("Object '" + root + "' doesn't exist in file '" + file_name + "'!");
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
				names->insert(names->end(), tmp.begin(), tmp.end());
			}
		}
		else if(depth >= 1)
		{
			for (std::string root : roots)
			{
				if (H5Oexists_by_name(file.getId(), root.c_str(), H5P_DEFAULT) != true)
				{
					throw H5Exception("Object '" + root + "' doesn't exist in file '" + file_name + "'!");
				}
				H5O object = H5O(file, root);
				std::vector<std::string> tmp;
				H5Literate(object.getId(),  H5_INDEX_NAME , H5_ITER_NATIVE, NULL, put_link_name, &tmp);
				boost::filesystem::path dir(root);
				for (int i = 0; i < tmp.size(); ++i)
				{
					tmp[i] = (dir / boost::filesystem::path(tmp[i])).string();
				}
				if(depth>1)
				{
					readNames(tmp, depth-1, names);
				}
				names->insert(names->end(), tmp.begin(), tmp.end());
			}
		}
		else
		{
			throw H5Exception("Depth has to be greater or equal than 0!");
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

herr_t put_dataset_attribute(hid_t location_id, const char *attr_name, const H5A_info_t *ainfo, void *op_data)
{
	MLINK loopback = *((MLINK*) op_data);

	H5A attr(location_id, attr_name);

	H5T datatype(attr);
	H5T_class_t typeclass = H5Tget_class(datatype.getId());
	size_t size = datatype.getSize();

	MLPutFunction(loopback, "Rule", 2);
	MLPutString(loopback, attr_name);

	try
	{
		if ((typeclass == H5T_INTEGER) || (typeclass == H5T_FLOAT))
		{
			H5S dataspace(attr);
			const int rank = dataspace.getSimpleExtentNDims();
			std::vector<hsize_t> dims(rank);
			dataspace.getSimpleExtentDims(dims.data());
			int nElems = 1;
			for (int k = 0; k < rank; k++)
			{
				nElems *= dims[k];
			}

			std::vector<long int> idims(rank);
			for (int k = 0; k < rank; k++)
			{
				idims[k] = dims[k];
			}
			std::vector<long int> long_dim(rank);
			std::vector<int> int_dim(rank);
			for (int j = 0; j < rank; j++)
			{
				long_dim[j] = dims[j];
				int_dim[j] = dims[j];
			}

			if (typeclass == H5T_INTEGER)
			{
				if(size == 1 || size == 2)
				{
					std::vector<short> data(nElems);
					if (H5Aread(attr.getId(), datatype.getNativeId(), (void *) data.data()) < 0)
					{
						throw H5Exception("Failed to read data for dataset " + std::string(attr_name));
					}
					if (rank == 0)
						MLPutInteger16(loopback, data[0]);
					else
						MLPutInteger16Array(loopback, data.data(), int_dim.data(), 0, rank);
				}
				else if(size == 4)
				{
					std::vector<int> data(nElems);
					if (H5Aread(attr.getId(), datatype.getNativeId(), (void *) data.data()) < 0)
					{
						throw H5Exception("Failed to read data for dataset " + std::string(attr_name));
					}
					if (rank == 0)
						MLPutInteger(loopback, data[0]);
					else
						MLPutIntegerArray(loopback, data.data(), long_dim.data(), 0, rank);

				}
				else if(size == 8)
				{
					std::vector<mlint64> data(nElems);
					if (H5Aread(attr.getId(), datatype.getNativeId(), (void *) data.data()) < 0)
					{
						throw H5Exception("Failed to read data for dataset " + std::string(attr_name));
					}
					if (rank == 0)
						MLPutInteger64(loopback, data[0]);
					else
						MLPutInteger64Array(loopback, data.data(), int_dim.data(), 0, rank);
				}
				else
				{
					throw H5Exception("Bitdepth not supported for " + std::string(attr_name) + "!");
				}

			}
			else if (typeclass == H5T_FLOAT)
			{
				if(size == 4)
				{
					std::vector<float> data(nElems);
					if (H5Aread(attr.getId(), datatype.getNativeId(), (void *) data.data()) < 0)
					{
						throw H5Exception("Failed to read data for dataset " + std::string(attr_name));
					}
					if (rank == 0)
						MLPutReal32(loopback, data[0]);
					else
						MLPutReal32Array(loopback, data.data(), int_dim.data(), 0, rank);
				}
				else if(size == 8)
				{
					std::vector<double> data(nElems);
					if (H5Aread(attr.getId(), datatype.getNativeId(), (void *) data.data()) < 0)
					{
						throw H5Exception("Failed to read data for dataset " + std::string(attr_name));
					}
					if (rank == 0)
						MLPutReal64(loopback, data[0]);
					else
						MLPutReal64Array(loopback, data.data(), int_dim.data(), 0, rank);
				}
				else
				{
					throw H5Exception("Bitdepth not supported for " + std::string(attr_name) + "!");
				}
			}
		}
		else if (typeclass == H5T_STRING)
		{
			/* Include space for a null terminator in case it isn't in the attribute */
			std::vector<char> str(size + 1);
			str[size] = '\0';
			if (H5Aread(attr.getId(), datatype.getNativeId(), (void *) str.data()) < 0)
				throw(H5Exception("Failed to read data for attribute"));
			MLPutString(loopback, str.data());
		}
		else
		{
			throw H5Exception("Datatype not supported for " + std::string(attr_name) + "!");
		}
    }
    catch(H5Exception &e)
    {
        throw e;
    }

	return 0;
}

} /* namespace elib */
