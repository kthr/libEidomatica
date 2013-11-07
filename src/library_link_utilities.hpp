/*
 * library_link_utilities.hpp
 *
 *  Created on: Sep 13, 2013
 *      Author: kthierbach
 */

#ifndef LIBRARY_LINK_UTILITIES_HPP_
#define LIBRARY_LINK_UTILITIES_HPP_

#include <string>

#include "WolframLibrary.h"

#include "templates/image.hpp"
#include "templates/tensor.hpp"

namespace elib
{

template <typename T>
class LibraryLinkUtilities
{
	public:
		static Tensor<T>* llGetIntegerTensor(WolframLibraryData libData, MTensor& tensor)
		{
			elib::Tensor<T> new_tensor(libData->MTensor_getRank(tensor), libData->MTensor_getDimensions);
			std::copy(libData->MTensor_getIntegerData(tensor), libData->MTensor_getIntegerData(tensor)+libData->MTensor_getFlattenedLength(tensor), new_tensor.getData());
			return new_tensor;
		}

		static Tensor<T>* llGetRealTensor(WolframLibraryData libData, MTensor& tensor)
		{
			elib::Tensor<T> new_tensor(libData->MTensor_getRank(tensor), libData->MTensor_getDimensions);
			std::copy(libData->MTensor_getRealData(tensor), libData->MTensor_getRealData(tensor)+libData->MTensor_getFlattenedLength(tensor), new_tensor.getData());
			return new_tensor;
		}

		static Image<T>* llGetIntegerImage(WolframLibraryData libData, MTensor& tensor, mint bit_depth, mint channels)
		{
			int rank;
			if(channels > 1)
			{
				rank = int(libData->MTensor_getRank(tensor)-1);
			}
			else
			{
				rank = int(libData->MTensor_getRank(tensor));
			}
			int dimensions[rank];
			std::copy(libData->MTensor_getDimensions(tensor),libData->MTensor_getDimensions(tensor)+rank, dimensions);
			elib::Image<T> *image =  new Image<T>(rank, dimensions, int(bit_depth), int(channels));
			std::copy(libData->MTensor_getIntegerData(tensor), libData->MTensor_getIntegerData(tensor)+libData->MTensor_getFlattenedLength(tensor), image->getData());
			return image;
		}

		static Image<T>* llGetRealImage(WolframLibraryData libData, MTensor& tensor, mint bit_depth, mint channels)
		{
			int rank;
			if(channels > 1)
			{
				rank = int(libData->MTensor_getRank(tensor)-1);
			}
			else
			{
				rank = int(libData->MTensor_getRank(tensor));
			}
			int dimensions[rank];
			std::copy(libData->MTensor_getDimensions(tensor),libData->MTensor_getDimensions(tensor)+rank, dimensions);
			elib::Image<T> *image = new Image<T>(rank, dimensions, int(bit_depth), int(channels));
			std::copy(libData->MTensor_getRealData(tensor), libData->MTensor_getRealData(tensor)+libData->MTensor_getFlattenedLength(tensor), image->getData());
			return image;
		}
};

} /* namespace elib */
#endif /* LIBRARY_LINK_UTILITIES_HPP_ */
