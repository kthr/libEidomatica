/*
 * hdf5_read.hpp
 *
 *  Created on: Nov 21, 2014
 *      Author: kthierbach
 */

#ifndef HDF5_READ_HPP_
#define HDF5_READ_HPP_

#include <vector>
#include <H5Cpp.h>

#include "mathlink.h"

namespace elib
{

extern "C"
{
	herr_t put_link_name(hid_t g_id, const char *name, const H5L_info_t *info, void *op_data);
}

class HDF5Read
{
	public:
		HDF5Read(MLINK ws_link, std::string file_name);
		virtual ~HDF5Read();

		void readAnnotations(std::vector<std::string> &object_names);
		void readData(std::vector<std::string> &dataset_names);
		void readNames(const std::vector<std::string> &roots, int depth, std::vector<std::string> *names);

	private:
		void readIntegerData(MLINK loopback, std::string dataset_name, const H5::DataSet &dataset);
		void readFloatData(MLINK loopback, std::string dataset_name, const H5::DataSet &dataset);
		void readStringData(MLINK loopback, std::string dataset_name, const H5::DataSet &dataset);

		H5::H5File file;
		MLINK ws_link = nullptr;
};

} /* namespace elib */

#endif /* HDF5_READ_HPP_ */
