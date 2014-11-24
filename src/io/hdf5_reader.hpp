/*
 * hdf5reader.hpp
 *
 *  Created on: Jul 30, 2014
 *      Author: kthierbach
 */

#ifndef HDF5READER_HPP_
#define HDF5READER_HPP_

#include <hdf5.h>
#include <string>
#include <vector>

#include "hdf5_wrapper.hpp"
#include "mathlink.h"
#include "WolframLibrary.h"

namespace elib
{

extern "C"
{
	herr_t put_group_name(hid_t o_id, const char *name, const H5O_info_t *object_info, void *op_data);
	herr_t put_link_name(hid_t g_id, const char *name, const H5L_info_t *info, void *op_data);
	herr_t put_dataset_attribute(hid_t location_id, const char *attr_name, const H5A_info_t *ainfo, void *op_data);
}

class HDF5Reader
{
	public:
		HDF5Reader(MLINK mlp, std::string file_name);
		virtual ~HDF5Reader();

		void readAnnotations(std::vector<std::string> &object_names);
		void readData(std::vector<std::string> &dataset_names);
		void readNames(std::vector<std::string> &roots, int depth, std::vector<std::string> *names);

	private:
		void readIntegerData(MLINK loop, std::string dataset_name, const H5D &dataset);
		void readFloatData(MLINK loop, std::string dataset_name, const H5D &dataset);
		void readStringData(MLINK loop, std::string dataset_name, const H5D &dataset);

		std::string file_name = "";
		H5F file;
		MLINK mlp = nullptr;
};

} /* namespace elib */

#endif /* HDF5READER_HPP_ */
