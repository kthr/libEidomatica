/*
 * Image.hpp
 *
 *  Created on: May 27, 2013
 *      Author: kthierbach
 */

#ifndef IMAGE_HPP_
#define IMAGE_HPP_

#include <algorithm>
#include <string>

#include "c_wrapper.h"
//#include "CImg.h"

namespace elib{

template <typename type>
class Image
{
	public:
		Image()
		{
			dimensions = NULL;
			data = NULL;
		}
		Image(const Image &other)
		: bit_depth(other.bit_depth), channels(other.channels), flattened_length(other.flattened_length), rank(other.rank)
		{
			dimensions = new int[rank];
			data = new type[flattened_length];

			std::copy(other.dimensions, other.dimensions+rank, dimensions);
			std::copy(other.data, other.data+flattened_length, data);
		}
		Image(const Image &&other)
		: Image()
		{
			this->operator=(other);
		}
		Image(int rank, const int *dimensions, int bit_depth, int channels)
		: bit_depth(bit_depth), channels(channels), rank(rank)
		{
			this->dimensions  = new int[rank];
			std::copy(dimensions, dimensions+rank, this->dimensions);
			this->flattened_length = channels;
			for(int i=0; i<rank; ++i)
			{
				this->flattened_length*=dimensions[i];
			}
			this->data = new type[flattened_length];
			std::fill_n(this->data, flattened_length, 0);
		}

		explicit Image(cimage *image)
		: bit_depth(image->bit_depth), channels(image->channels), flattened_length(image->flattened_length), rank(image->rank)
		{
			dimensions = new int[rank];
			std::copy(image->dimensions, image->dimensions+rank, dimensions);
			data = new type[flattened_length];
			if(image->type == INTEGER_TYPE)
				std::copy(image->integer_data, image->integer_data+flattened_length, data);
			else
				std::copy(image->double_data, image->double_data+flattened_length, data);
		}
//		explicit Image(cimg_library::CImg<type> *image)
//		{
//			if(image->depth() == 1)
//			{
//				rank = 2;
//				dimensions = new int[rank];
//				dimensions[0] = image->width();
//				dimensions[1] = image->height();
//			}
//			else
//			{
//				rank = 3;
//				dimensions = new int[rank];
//				dimensions[0] = image->width();
//				dimensions[1] = image->height();
//				dimensions[2] = image->depth();
//			}
//			channels = 1;
//			if(image->spectrum()>1)
//			{
//				cimg_library::CImg<type> tmp = image->get_channel(0);
//				flattened_length = tmp.size();
//				data = new type[flattened_length];
//				std::copy(tmp.data(), tmp.data() + tmp.size(), data);
//			}
//			else
//			{
//				flattened_length = image->size();
//				data = new type[flattened_length];
//				std::copy(image->data(), image->data() + image->size(), data);
//			}
//			if(image->max() > pow(2,16)-1)//32bit
//				bit_depth = 32;
//			if(image->max() > pow(2,8)-1)//16bit
//				bit_depth = 16;
//			else//8bit
//				bit_depth = 8;
//		}
		virtual ~Image()
		{
			delete[] dimensions;
			delete[] data;
		}

		/*******************************************************************************************************
		* operators
		*******************************************************************************************************/
		Image& operator=(Image other)
		{
			swap(*this,other);
			return *this;
		}

		/*******************************************************************************************************
		* compute functions
		*******************************************************************************************************/
		type min()
		{
			type minimum;

			minimum = data[0];
			for(int i=1; i<flattened_length; ++i)
				minimum = std::min(minimum, data[i]);
			return minimum;
		}
		type max()
		{
			type maximum;

			maximum = data[0];
			for(int i=1; i<flattened_length; ++i)
				maximum = std::max(maximum, data[i]);
			return maximum;
		}

		/*******************************************************************************************************
		* conversion and io functions
		*******************************************************************************************************/
		cimage* to_cimage(int data_type = INTEGER_TYPE)
		{
			cimage *new_image;

			new_image = new cimage;
			new_image->bit_depth = this->getBitDepth();
			new_image->channels = this->getChannels();
			new_image->rank = this->getRank();
			new_image->dimensions = new mint[this->getRank()];
			std::copy(this->getDimensions(), this->getDimensions()+this->getRank(), new_image->dimensions);
			new_image->flattened_length = this->getFlattenedLength();
			if(data_type == INTEGER_TYPE)
			{
				new_image->type = INTEGER_TYPE;
				new_image->integer_data = new mint[new_image->flattened_length];
				std::copy(this->getData(), this->getData()+new_image->flattened_length, new_image->integer_data);
			}
			else
			{
				new_image->type = DOUBLE_TYPE;
				new_image->double_data = new double[new_image->flattened_length];
				std::copy(this->getData(), this->getData()+new_image->flattened_length, new_image->double_data);
			}
			new_image->shared = 0;

			return new_image;
		}
//		static Image<type>* open(std::string file_name)
//		{
//			try
//			{
//				cimg_library::CImg<type> img(file_name.c_str());
//				return new Image<type>(&img);
//			}
//			catch(cimg_library::CImgException &e)
//			{
//				return nullptr;
//			}
//		}
//		void save(std::string file_name)
//		{
//			try
//			{
//				if(this->getRank() == 2)
//				{
//					int width, height, spectrum;
//
//					width = this->getDimensions()[0];
//					height = this->getDimensions()[1];
//					spectrum = this->getChannels();
//					cimg_library::CImg<type> img(width, height, 1, spectrum);
//					std::copy(this->getData(), this->getData()+this->getFlattenedLength(), img.data());
//					img.save(file_name.c_str());
//				}
//				else
//				{
//				}
//			}
//			catch(cimg_library::CImgException &e)
//			{
//			}
//		}

		/*******************************************************************************************************
		 * getter and setter
		 *******************************************************************************************************/
		int getBitDepth() const
		{
			return bit_depth;
		}
		int getChannels() const
		{
			return channels;
		}
		type* getData() const
		{
			return data;
		}
		int* getDimensions() const
		{
			return dimensions;
		}
		int getFlattenedLength() const
		{
			return flattened_length;
		}
		int getRank() const
		{
			return rank;
		}
		void setData(type* data)
		{
			std::copy(data, data+flattened_length, this->data);
		}
		int getWidth()
		{
			if(rank>0)
				return dimensions[0];
			else
				return 0;
		}
		int getHeight()
		{
			if(rank>1)
				return dimensions[1];
			else
				return 0;
		}
		int getDepth()
		{
			if(rank>2)
				return dimensions[2];
			else
				return 1;
		}

	private:
		int 	*dimensions,
				bit_depth,
				channels,
				flattened_length,
				rank;
		type 	*data;

		friend void swap(Image& first,Image& second)
		{
			using std::swap;

			swap(first.dimensions, second.dimensions);
			swap(first.bit_depth, second.bit_depth);
			swap(first.channels, second.channels);
			swap(first.flattened_length, second.flattened_length);
			swap(first.rank, second.rank);
			swap(first.data, second.data);
		}
};

} /* namespace elib */

#endif /* IMAGE_HPP_ */
