/*
 * maskList.hpp
 *
 *  Created on: Jun 20, 2013
 *      Author: kthierbach
 */

#ifndef MASKLIST_HPP_
#define MASKLIST_HPP_

#include <iostream>
#include <memory>
#include <set>
#include <sstream>
#include <unordered_map>
#include <utility>
#include <vector>

#include "boundingBox.hpp"
#include "mask.hpp"

namespace elib{

template <class Point>
class MaskList
{
	public:
		MaskList() noexcept
		{
		}
		MaskList(int rank, const std::vector<int> &dimensions) : rank(rank)
		{
			this->dimensions = std::vector<int>(dimensions.begin(), dimensions.end());
		}
		MaskList(const MaskList& other) : rank(other.rank), dimensions(other.dimensions), labels(other.labels), masks(other.masks)
		{
		}
		MaskList(MaskList&& other) : rank(other.rank), dimensions(std::move(other.dimensions)),
				labels(std::move(other.labels)), masks(std::move(other.masks))
		{
		}
		MaskList& operator=(const MaskList &other)
		{
			rank = other.rank;
			dimensions = other.dimensions;
			labels = other.labels;
			masks = std::unordered_map<int, std::shared_ptr<Mask<Point>>>(other.masks);
			return *this;
		}
		MaskList& operator=(MaskList &&other)
		{
			rank = other.rank;
			dimensions = std::move(other.dimensions);
			labels = std::move(other.labels);
			masks = std::move(other.masks);
			return *this;
		}
		virtual ~MaskList()
		{
		}

		void addMask(std::shared_ptr<Mask<Point> > &shared_mask_ptr, int id)
		{
			auto it = masks.find(id);
			if(it == masks.end())
			{
				labels.insert(id);
				shared_mask_ptr = std::shared_ptr<Mask<Point>>(new Mask<Point>(rank, dimensions));
				masks.insert(std::pair<int,std::shared_ptr<Mask<Point>>>(id, shared_mask_ptr));
			}
			else
			{
				shared_mask_ptr = it->second;
			}
		}
		void addMask(std::shared_ptr<Mask<Point> > &shared_mask_ptr, int id, Mask<Point> &m)
		{
			auto it = masks.find(id);
			if(it == masks.end())
			{
				labels.insert(id);
				shared_mask_ptr = new Mask<Point>(m);
				masks.insert(std::pair<int,std::shared_ptr<Mask<Point>>>(id, shared_mask_ptr));
			}
			else
			{
				shared_mask_ptr = it->second;
			}
		}
		void addMask(int id, Mask<Point> &m)
		{
			auto it = masks.find(id);
			if(it == masks.end())
			{
				labels.insert(id);
				std::shared_ptr<Mask<Point>> shared_mask_ptr = std::shared_ptr<Mask<Point>>(new Mask<Point>(m));
				masks.insert(std::pair<int,std::shared_ptr<Mask<Point>>>(id, shared_mask_ptr));
			}
		}
		void addMasks(MaskList list)
		{
			labels.insert(list.getints()->begin(), list.getints()->end());
			masks.insert(list.begin(),list.end());
		}
		std::shared_ptr<Mask<Point>> getMask(int id)
		{
			auto it = masks.find(id);
			if(it != masks.end())
			{
				return it->second;
			}
			else
			{
				return nullptr;
			}
		}
		void clear()
		{
			labels.clear();
			masks.clear();
		}
		void deleteMask(int id)
		{
			auto it = masks.find(id);
			if(it != masks.end())
			{
				labels.erase(id);
				masks.erase(it);
			}
		}
		void deleteMasks(std::vector<int> &for_deletion)
		{
			auto dit = for_deletion.begin();
			while(dit != for_deletion.end())
			{
				this->deleteMask(*dit);
				++dit;
			}
		}
		void deleteSparseRepresentations()
		{
			for(auto i = masks.begin(); i != masks.end(); ++i)
			{
				i->second->deleteSparseRepresentation();
			}
		}
		Mask<Point> fuse()
		{
			Mask<Point> mask(this->rank, this->dimensions);
			const std::vector<Point> *points;

			for(auto i = masks.begin(); i!= masks.end(); ++i)
			{
				points = i->second->getPoints();
				for(auto j = points->begin(); j != points->end(); ++j)
				{
					mask.addPoint(*j);
				}
			}
			return mask;
		}
		typename std::unordered_map<int, std::shared_ptr<Mask<Point>>>::iterator begin()
		{
			return masks.begin();
		}
		typename std::unordered_map<int, std::shared_ptr<Mask<Point>>>::iterator end()
		{
			return masks.end();
		}
		Image<int> toImage()
		{
			std::shared_ptr<Mask<Point>> mask_ptr;
			int *image_data, pixel;

			Image<int> image = Image<int>(rank, dimensions, bit_depth, 1);
			image_data = image.getData();
			for(auto& it : labels)
			{
				mask_ptr = this->getMask(it);
				if(mask_ptr!=nullptr)
				{
					for(auto& pt_it : *mask_ptr->getPoints())
					{
						for(int i=0; i<rank; ++i)
						{
							if(pt_it[i] < 0 || pt_it[i]>dimensions[i])
							{
								//throw GenericException("Mask doesn't fit into the image");
								return image;
							}
						}
						pixel = mask_ptr->getPixel(pt_it);
						image_data[pixel] = it;
					}
				}
				else
				{
					std::cerr << "mask with label " << it << " not found!";
				}
			}
			return image;
		}
		void relabel(int offset = 0)
		{
			typename std::unordered_map<int, std::shared_ptr<Mask<Point>>> tmp;
			int id = 1+offset;

			labels.clear();
			for(auto it = masks.begin(); it!=masks.end(); ++it)
			{
				tmp.insert(std::pair<int, std::shared_ptr<Mask<Point>>>(id, it->second));
				labels.insert(id);
				++id;
			}
			masks = tmp;
		}
		const std::set<int>* getints() const
		{
			return &labels;
		}
		int getSize() const
		{
			return masks.size();
		}
		int getRank() const
		{
			return rank;
		}
		const std::vector<int>* getDimensions() const
		{
			return &dimensions;
		}
		void setDimensions(const std::vector<int> &dimensions)
		{
			this->dimensions = std::vector<int>(dimensions.begin(), dimensions.end());
			for(auto i = masks.begin(); i != masks.end(); ++i)
			{
				i->second->setDimensions(dimensions);
			}
		}
		void setRank(int rank)
		{
			this->rank = rank;
			for(auto i = masks.begin(); i != masks.end(); ++i)
			{
				i->second->setRank(rank);
			}
		}
		void setOrigin(Point origin)
		{
			for(auto i = masks.begin(); i != masks.end(); ++i)
			{
				i->second->setOrigin(origin);
			}
		}
		std::string toString() const
		{
			std::stringstream ss;
			typename std::set<int>::iterator it;

			ss << "Size of list: " << labels.size() << std::endl;
			ss << "Rank: " << rank << std::endl;
			ss << "Dimensions: ";
			for(int i=0; i<rank; ++i)
			{
				ss << dimensions[i] << " ";
			}
			ss << std::endl;
			std::shared_ptr<Mask<Point>> mask;
			for(auto it: labels)
			{
				mask = masks.find(it)->second;
				ss << "\t label: " << it << " size: " << mask->getSize() << " rank: " << mask->getRank() << " width: " << mask->getWidth() << " height: " << mask->getHeight() << " depth: " << mask->getDepth() << std::endl;
			}
			return ss.str();
		}

private:
		int rank = 0;
		std::vector<int> dimensions;
		const static int bit_depth = 16;
		std::set<int> labels;
		std::unordered_map<int, std::shared_ptr<Mask<Point>>> masks;
};

} /* end namespace elib */


#endif /* MASKLIST_HPP_ */
