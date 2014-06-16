/*
 * Image.hpp
 *
 *  Created on: May 27, 2013
 *      Author: kthierbach
 */

#ifndef IMAGE_HPP_
#define IMAGE_HPP_

#include <math.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "boundingBox.hpp"
#include "CImg.h"
#include "glm/glm.hpp"
#include "utilities/vector_2D.hpp"
#include "utilities/vector_array_2D.hpp"

namespace elib{

template <typename type>
class Image
{
	public:
		Image()
		{
		}
		Image(const Image &other)
		: bit_depth(other.bit_depth), channels(other.channels), flattened_length(other.flattened_length), rank(other.rank)
		{
			data = std::unique_ptr<type[]>(new type[flattened_length]);
			dimensions=std::vector<int>(other.dimensions.begin(), other.dimensions.end());
			std::copy(other.data.get(), other.data.get()+flattened_length, data.get());
		}
		Image(Image &&other) : dimensions(std::move(other.dimensions)), bit_depth(other.bit_depth), channels(other.channels),
				flattened_length(other.flattened_length), rank(other.rank), data(std::move(other.data))
		{
		}
		Image(int rank, const std::vector<int> &dimensions, int bit_depth, int channels)
		: bit_depth(bit_depth), channels(channels), rank(rank)
		{
			this->dimensions=std::vector<int>(dimensions.begin(), dimensions.end());
			this->flattened_length = channels;
			for(auto i=dimensions.begin(); i!=dimensions.end(); ++i)
			{
				this->flattened_length*=(*i);
			}
			this->data = std::unique_ptr<type[]>(new type[flattened_length]);
			std::fill_n(this->data.get(), flattened_length, 0);
		}
		explicit Image(cimg_library::CImg<type> *image)
		{
			if(image->depth() == 1)
			{
				rank = 2;
				dimensions=std::vector<int>({image->width(), image->height()});
			}
			else
			{
				rank = 3;
				dimensions=std::vector<int>({image->width(), image->height(), image->depth()});
			}
			channels = 1;
			if(image->spectrum()>1)
			{
				cimg_library::CImg<type> tmp = image->get_channel(0);
				flattened_length = tmp.size();
				data = std::unique_ptr<type[]>(new type[flattened_length]);;
				std::copy(tmp.data(), tmp.data() + tmp.size(), data.get());
			}
			else
			{
				flattened_length = image->size();
				data = std::unique_ptr<type[]>(new type[flattened_length]);;
				std::copy(image->data(), image->data() + image->size(), data.get());
			}
			if(image->max() > pow(2,16)-1)//32bit
				bit_depth = 32;
			if(image->max() > pow(2,8)-1)//16bit
				bit_depth = 16;
			else//8bit
				bit_depth = 8;
		}
		virtual ~Image()
		{
		}

		Image& operator=(const Image &other)
		{
			dimensions = other.dimensions;
			bit_depth = other.bit_depth,
			channels = other.channels,
			flattened_length = other.flattened_length,
			rank = other.rank;
			data = std::unique_ptr<type[]>(new type[flattened_length]);
			std::copy(other.data.get(), other.data.get()+flattened_length, data.get());
			return *this;
		}
		Image& operator=(Image &&other)
		{
			dimensions = std::move(other.dimensions);
			bit_depth = other.bit_depth;
			channels = other.channels;
			flattened_length = other.flattened_length;
			rank = other.rank;
			data = std::move(other.data);
			return *this;
		}
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
		void multiply(double value)
		{
			for(int i=0; i< flattened_length; ++i)
			{
				data[i] *= value;
			}
		}
		void displaceByVectorField(VectorArray2D &field)
		{
			Image tmp = Image<type>(this->getRank(),*(this->getDimensions()),this->getBitDepth(),this->getChannels());
			type *data = tmp.getData();

			if(rank==2)
			{
				int width = dimensions[0],
					height = dimensions[1],
					max_index_width = width-1,
					max_index_height = height-1,
					k, l;
				Vector2D v;
				for(int j=0; j<height; ++j)
				{
					for(int i=0; i<width; ++i)
					{
						v = field.get(i,j);
						l = i-round(v.x);
						k = j-round(v.y);
						if(0<=l && l<width && 0<=k && k<height)
						{
							data[i+j*width] = this->data[l+k*width];
						}
						else
						{
							using std::min;
							using std::max;
							data[i+j*width] = this->data[min(max_index_width,max(0,l))+min(max_index_height,max(0,k))*width];
//							data[i+j*width] = 0.;
						}
					}
				}
				*this = tmp;
			}
		}
		std::vector<type> pick(const std::vector<glm::ivec3> &points)
		{
			std::vector<type> intensities(points.size());
			int width = this->getWidth(),
				height = this->getHeight();
			for(auto i : points)
			{
				intensities.push_back(data[i.x + i.y*width + i.z*width*height]);
			}
			return intensities;
		}
		std::shared_ptr<Image<type>> imageTake(BoundingBox<glm::ivec2> box) const
		{
			glm::ivec2 	upperLeft = box.getUpperLeft(),
						bottomRight = box.getBottomRight();
			if(rank ==  2 && upperLeft.x <= bottomRight.x && upperLeft.y <= bottomRight.y && upperLeft.x >= 0 && upperLeft.y >=0 &&
					bottomRight.x < dimensions[0] && bottomRight.y < dimensions[1])
			{
				std::vector<int> new_dimensions;
				int new_width = bottomRight.x-upperLeft.x + 1;
				new_dimensions.push_back(new_width);
				new_dimensions.push_back(bottomRight.y-upperLeft.y + 1);
				std::shared_ptr<Image<type>> part = std::shared_ptr<Image<type>>(new Image<type>(this->rank, new_dimensions, this->bit_depth, this->channels));

				int pixel = upperLeft.x + upperLeft.y*this->getWidth();
				std::copy(this->data.get() + pixel, this->data.get() + pixel + new_width, part->data.get());
				for(int i=1; i<new_dimensions[1]; ++i)
				{
					pixel += this->getWidth();
					std::copy(this->data.get() + pixel, this->data.get() + pixel + new_width, part->data.get() + i*new_width);
				}
				return part;
			}
			else
			{
				return nullptr;
			}
		}
		static std::shared_ptr<Image<type>> openImage(std::string file_name)
		{
			try
			{
				cimg_library::CImg<type> img(file_name.c_str());
				return std::shared_ptr<Image<type>>(new Image<type>(&img));
			}
			catch(cimg_library::CImgException &e)
			{
				return nullptr;
			}
		}
		void saveImage(std::string file_name, int bit_depth=16) const
		{
			Image<type>::saveImage(file_name, this, bit_depth);
		}
		static void saveImage(std::string file_name, const Image<type> *image, int bit_depth)
		{
			try
			{
				if(image->getRank() == 2)
				{
					int width, height, depth, spectrum;

					width = image->getWidth();
					height = image->getHeight();
					depth = image->getDepth();
					spectrum = image->getChannels();
					cimg_library::CImg<type> img(width, height, depth, spectrum);
					std::copy(image->getData(), image->getData()+image->getFlattenedLength(), img.data());
					switch(bit_depth)
					{
						case 8:
							img.save_png(file_name.c_str(),1);
							break;
						case 16:
							img.save_png(file_name.c_str(),2);
							break;
						default:
							img.save_png(file_name.c_str(),2);
							break;
					}
				}
				else
				{
				}
			}
			catch(cimg_library::CImgException &e)
			{
				std::string message = std::string("ERROR: Failed to save image at: ") + file_name;
			}
		}
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
			return data.get();
		}
		const std::vector<int>* getDimensions() const
		{
			return &dimensions;
		}
		int getFlattenedLength() const
		{
			return flattened_length;
		}
		int getRank() const
		{
			return rank;
		}
		void setData(const type* data)
		{
			std::copy(data, data+flattened_length, this->data);
		}
		int getWidth() const
		{
			if(rank>0)
				return dimensions[0];
			else
				return 0;
		}
		int getHeight() const
		{
			if(rank>1)
				return dimensions[1];
			else
				return 0;
		}
		int getDepth() const
		{
			if(rank>2)
				return dimensions[2];
			else
				return 1;
		}

	private:
		std::vector<int> dimensions;
		int		bit_depth = 0,
				channels = 0,
				flattened_length = 0,
				rank = 0;
		std::unique_ptr<type[]> data = nullptr;

};

} /* namespace elib */

#endif /* IMAGE_HPP_ */
