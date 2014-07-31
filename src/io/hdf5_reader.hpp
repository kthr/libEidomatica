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
#include <WolframLibrary.h>

namespace elib
{

extern "C"
{
  herr_t put_group_name(hid_t o_id, const char *name, const H5O_info_t *object_info, void *op_data);
  herr_t put_link_name( hid_t g_id, const char *name, const H5L_info_t *info, void *op_data);
}

class HDF5Reader
{
	public:
		HDF5Reader(WolframLibraryData libData, std::string file_name);
		virtual ~HDF5Reader();

		void readDatasetNames(std::vector<std::string> &names, std::string root, int depth) throw();
		void readData(std::vector<std::string> &dataset_names) throw();
		void readGroupNames(std::vector<std::string> &names, std::string root, int depth) throw();

	private:
		std::string file_name;
		WolframLibraryData libData;
};

} /* namespace elib */

#endif /* HDF5READER_HPP_ */
