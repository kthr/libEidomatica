/*
 * hdf5_read.cpp
 *
 *  Created on: Nov 21, 2014
 *      Author: kthierbach
 */

#include "hdf5_read.hpp"

#include <boost/filesystem.hpp>

namespace elib
{

HDF5Read::HDF5Read(MLINK ws_link, std::string file_name) : ws_link(ws_link)
{
	try
	{
		file = H5::H5File(file_name.c_str(), H5F_ACC_RDONLY);
	}
	catch(H5::FileIException &e)
	{
		throw e;
	}
}

HDF5Read::~HDF5Read()
{
	this->file.close();
}

void HDF5Read::readAnnotations(std::vector<std::string> &object_names)
{
	/* Get the list of datasets to be read from Mathematica*/
	long n = object_names.size();
	if (n < 0)
	{
		MLPutFunction(ws_link, "List", 0);
		return;
	}

	/* Create a loopback link to store the list until we are sure it can be fully
	 filled. This way if something fails we can abort and send $Failed back. */

	try
	{
		MLINK loopback = nullptr;
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

		if(n>1)
		{
			MLPutFunction(loopback, "List", n);
		}

		/* Loop over all requested datasets */
		for (std::string dataset_name : object_names)
		{
			H5G_stat_t type;
			file.getObjinfo(dataset_name, type);
			switch(type.type)
			{
				case H5G_GROUP:
				{
					H5::DataSet dataset = file.openDataSet(dataset_name);
					int num_attributes = dataset.getNumAttrs();
					MLPutFunction(loopback, "List", num_attributes);
					for(int i=0; i<num_attributes; ++i)
					{
					}
				}
				break;
				case H5G_DATASET:
				{

				}
				break;
				default:
				{
					throw H5::Exception("", "Reading attributes only supported for datasets and groups!");
				}
				break;
			}
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
		throw &e;
	}

	/* Transfer data from loopback to actual Mathematica link */
	MLTransferToEndOfLoopbackLink(mlp, loopback);
	MLClose(loopback);
	MLDeinitialize(env);
}

void HDF5Read::readData(std::vector<std::string> &dataset_names)
{
	try
	{
		MLINK loopback = nullptr;
		MLENV env;
		int error;

		env = MLInitialize((char *)0);
		if(env == (MLENV)0)
		{
			throw H5::Exception("", "Unable to create ws environment!");
		}

		loopback = MLLoopbackOpen(env, &error);
		if(loopback == (MLINK)0 || error != MLEOK)
		{
			throw H5::Exception("", "Unable to open loopback link!");
		}

		if(dataset_names.size() > 1)
		{
			MLPutFunction(loopback, "List", dataset_names.size());
		}
		for(std::string name : dataset_names)
		{
//			if(MLAbort)
//			{
//				MLClose(loopback);
//				MLDeinitialize(env);
//				MLPutSymbol(ws_link, "$Aborted");
//			}
			H5::DataSet dataset = file.openDataSet(name.c_str());
			H5T_class_t type_class = dataset.getTypeClass();
			switch (type_class)
			{
				case H5T_INTEGER:
				{
					readIntegerData(loopback, name, dataset);
				}
					break;
				case H5T_FLOAT:
				{
					readFloatData(loopback, name, dataset);
				}
					break;
				case H5T_STRING:
				{
					readStringData(loopback, name, dataset);
				}
					break;
				default:
				{
					throw H5::Exception("", "Dataset type not supported for dataset '" + name + "'!");
				}
					break;
			}
		}
		MLTransferExpression(ws_link, loopback);
		MLClose(loopback);
		MLDeinitialize(env);
	}
	catch(H5::Exception &e)
	{
		throw H5::Exception("HDF5Read::readData()", e.getDetailMsg());
	}

}

void HDF5Read::readNames(const std::vector<std::string> &roots, int depth, std::vector<std::string> *names)
{
	try
	{
		if(depth == 0)
		{
			for (std::string root : roots)
			{
//				if(MLAbort)
//				{
//					names->clear();
//					return;
//				}
				if (H5Oexists_by_name(file.getId(), root.c_str(), H5P_DEFAULT) != true)
				{
					throw H5::Exception("", "Object '" + root + "' doesn't exist in file '" + file.getFileName() + "'!");
				}
				hid_t object_id;
				if((object_id = H5Oopen(file.getId(), root.c_str(), H5P_DEFAULT))==0)
				{
					throw H5::Exception("", "Could not open '" + root + "' in file '" + file.getFileName() + "'!");
				}
				std::vector<std::string> tmp;
				if (depth == 0)
				{
					if (H5Lvisit(object_id, H5_INDEX_NAME, H5_ITER_NATIVE, put_link_name, &tmp) < 0)
					{
						throw H5::Exception("", "Failed to visit object '" + root + "'!");
					}
				}
				boost::filesystem::path root_path(root);
				for (int i = 0; i < tmp.size(); ++i)
				{
					tmp[i] = (root_path / boost::filesystem::path(tmp[i])).string();
				}
				names->insert(names->end(), tmp.begin(), tmp.end());
			}
		}
		else if(depth >= 1)
		{
			for (std::string root : roots)
			{
//				if(MLAbort)
//				{
//					names->clear();
//					return;
//				}
				if (H5Oexists_by_name(file.getId(), root.c_str(), H5P_DEFAULT) != true)
				{
					throw H5::Exception("", "Object '" + root + "' doesn't exist in file '" + file.getFileName() + "'!");
				}
				hid_t object_id;
				if((object_id = H5Oopen(file.getId(), root.c_str(), H5P_DEFAULT))==0)
				{
					throw H5::Exception("", "Could not open '" + root + "' in file '" + file.getFileName() + "'!");
				}
				std::vector<std::string> tmp;
				H5Literate(object_id,  H5_INDEX_NAME , H5_ITER_NATIVE, NULL, put_link_name, &tmp);
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
			throw H5::Exception("", "Depth has to be greater or equal than 0!");
		}
	} catch (H5::Exception &e)
	{
		throw H5::Exception("HDF5Read::readNames()", e.getDetailMsg());
	}
}

void HDF5Read::readIntegerData(MLINK loopback, std::string dataset_name, const H5::DataSet &dataset)
{
	try
	{
		H5::DataSpace dataspace(dataset.getId());
		/* Get the number of dimensions in this dataset */
		int rank = dataspace.getSimpleExtentNdims();
		/*  Get the size of each dimension */
		std::vector<hsize_t> dimensions(rank);
		dataspace.getSimpleExtentDims(dimensions.data());

		std::vector<long int> long_dimensions(dimensions.begin(), dimensions.end());
		std::vector<int> int_dimensions(dimensions.begin(), dimensions.end());

		hsize_t number_elements = 1;
		for(auto i : dimensions)
		{
			number_elements *= i;
		}

		H5::DataType datatype(dataset.getId());
		/* get size of integer in bytes */
		size_t size = datatype.getSize();

		if(size == 1 || size == 2)
		{
			std::vector<short> data;
			data.reserve(number_elements);
			dataset.read(data.data(), datatype);
			if (rank == 0)
				MLPutInteger16(loopback, data[0]);
			else
				MLPutInteger16Array(loopback, data.data(), int_dimensions.data(), 0, rank);
		}
		else if(size == 4)
		{
			std::vector<int> data(number_elements);
			dataset.read(data.data(), datatype);
			if (rank == 0)
				MLPutInteger(loopback, data[0]);
			else
				MLPutIntegerArray(loopback, data.data(), long_dimensions.data(), 0, rank);

		}
		else if(size == 8)
		{
			std::vector<mlint64> data(number_elements);
			dataset.read(data.data(), datatype);
			if (rank == 0)
				MLPutInteger64(loopback, data[0]);
			else
				MLPutInteger64Array(loopback, data.data(), int_dimensions.data(), 0, rank);
		}
		else
		{
			throw H5::Exception("", "Bitdepth not supported for " + dataset_name + "!");
		}
	}
	catch(H5::Exception &e)
	{
		throw H5::Exception("HDF5Read::readIntegerData", e.getDetailMsg());
	}
}
void HDF5Read::readFloatData(MLINK loopback, std::string dataset_name, const H5::DataSet &dataset)
{
	try
	{
		H5::DataSpace dataspace(dataset.getId());
		/* Get the number of dimensions in this dataset */
		int rank = dataspace.getSimpleExtentNdims();
		/*  Get the size of each dimension */
		std::vector<hsize_t> dimensions(rank);
		dataspace.getSimpleExtentDims(dimensions.data());

		std::vector<int> int_dimensions(dimensions.begin(), dimensions.end());

		hsize_t number_elements = 1;
		for(auto i : dimensions)
		{
			number_elements *= i;
		}

		H5::DataType datatype(dataset.getId());
		/* get size of integer in bytes */
		size_t size = datatype.getSize();

		if(size == 4)
		{
			std::vector<float> data(number_elements);
			dataset.read(data.data(), datatype);
			if (rank == 0)
				MLPutReal32(loopback, data[0]);
			else
				MLPutReal32Array(loopback, data.data(), int_dimensions.data(), 0, rank);
		}
		else if(size == 8)
		{
			std::vector<double> data(number_elements);
			dataset.read(data.data(), datatype);
			if (rank == 0)
				MLPutReal64(loopback, data[0]);
			else
				MLPutReal64Array(loopback, data.data(), int_dimensions.data(), 0, rank);
		}
		else
		{
			throw H5::Exception("", "Bitdepth not supported for '" + dataset_name + "'!");
		}
	}
	catch(H5::Exception &e)
	{
		throw H5::Exception("HDF5Read::readFloatData", e.getDetailMsg());
	}
}
void HDF5Read::readStringData(MLINK loopback, std::string dataset_name, const H5::DataSet &dataset)
{
	try
	{
		H5::DataType datatype(dataset.getId());
		H5std_string string;
		dataset.read(string, datatype);
		MLPutString(loopback, string.data());
	}
	catch(H5::Exception &e)
	{
		throw H5::Exception("HDF5Read::readStringData", e.getDetailMsg());
	}
}

herr_t put_link_name(hid_t g_id, const char *name, const H5L_info_t *info, void *op_data)
{
	std::vector<std::string> *linkNames = (std::vector<std::string> *) op_data;
	if (info->type == H5L_TYPE_SOFT || info->type == H5L_TYPE_HARD)
		linkNames->push_back(std::string(name));
	return 0;
}

} /* namespace elib */
