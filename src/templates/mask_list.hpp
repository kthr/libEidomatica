/*
 * maskList.hpp
 *
 *  Created on: Jun 20, 2013
 *      Author: kthierbach
 */

#ifndef MASKLIST_HPP_
#define MASKLIST_HPP_

#include <unordered_map>
#include <sstream>
#include <vector>

#include "mask.hpp"

namespace elib{

template <typename Point>
class MaskList
{
	public:
		MaskList()
		{
			labels = new std::set<int>();
			masks = new std::unordered_map<int, Mask<Point>*>();
		}
		MaskList(const MaskList& other)
		{
			typename std::set<int>::iterator sit;
			typename std::unordered_map<int, Mask<Point>*>::iterator it;

			this->labels = new std::set<int>();
			for(sit=other.labels->begin(); sit!=other.labels->end(); ++sit)
			{
				this->labels->insert(*sit);
			}
			this->masks = new std::unordered_map<int, Mask<Point>*>(other.masks->size());
			for(it=other.masks->begin(); it!=other.masks->end(); ++it)
			{
				this->masks->insert(std::pair<int, Mask<Point>* >(it->first, new Mask<Point>(*(it->second))));
			}
		}
		MaskList(MaskList&& other)
		: MaskList()
		{
			swap(*this, other);
		}
		elib::MaskList<Mask<Point>>& operator=(MaskList<Mask<Point>> other)
		{
			swap(*this, other);
			return *this;
		}
		virtual ~MaskList()
		{
			typename std::unordered_map<int, Mask<Point>*>::iterator it;
			for(it=masks->begin(); it!=masks->end(); ++it)
			{
				delete it->second;
			}
			delete labels;
			delete masks;
		}
		Mask<Point>* operator[](int label)
		{
			return (*masks)[label];
		}

		Mask<Point>* addMask(int id)
		{
			Mask<Point>* mask_ptr;
			typename std::unordered_map<int, Mask<Point>*>::iterator it;

			it = masks->find(id);
			if(it == masks->end())
			{
				labels->insert(id);
				mask_ptr = new Mask<Point>();
				masks->insert(std::pair<int,Mask<Point>*>(id, mask_ptr));
			}
			else
			{
				mask_ptr = it->second;
			}
			return mask_ptr;
		}
		Mask<Point>* addMask(int id, Mask<Point> &m)
		{
			Mask<Point>* mask_ptr;
			typename std::unordered_map<int, Mask<Point>*>::iterator it;

			it = masks->find(id);
			if(it == masks->end())
			{
				labels->insert(id);
				mask_ptr = new Mask<Point>(m);
				masks->insert(std::pair<int,Mask<Point>*>(id, mask_ptr));
				return mask_ptr;
			}
			else
			{
				return nullptr;
			}
		}
		void addMasks(MaskList<Mask<Point>> *list)
		{
			typename std::unordered_map<int, Mask<Point>*>::iterator it= list->begin();
			while(it!=list->end())
			{
				this->addMask(it->first, *(it->second));
				++it;
			}
		}
		Mask<Point>* getMask(int id)
		{
			typename std::unordered_map<int, Mask<Point>*>::iterator it;

			it = masks->find(id);
			if(it != masks->end())
			{
				return it->second;
			}
			else
			{
				return nullptr;
			}
		}
		void deleteMask(int id)
		{
			typename std::unordered_map<int, Mask<Point>*>::iterator it;

			it = masks->find(id);
			if(it != masks->end())
			{
				labels->erase(id);
				delete it->second;
				masks->erase(it);
			}
		}
		void deleteMasks(std::vector<int> &for_deletion)
		{
			typename std::vector<int>::iterator dit;

			dit = for_deletion.begin();
			while(dit != for_deletion.end())
			{
				this->deleteMask(*dit);
				++dit;
			}
		}
		typename std::unordered_map<int, Mask<Point>*>::iterator begin()
		{
			return masks->begin();
		}
		typename std::unordered_map<int, Mask<Point>*>::iterator end()
		{
			return masks->end();
		}
		Image<int> toImage(int rank, int *dimensions)
		{
			Image<int> image;
			int *image_data, label, pixel;
			typename std::unordered_map<int, Mask<Point>* >::iterator it;
			typename std::vector<Point> *points;
			typename std::vector<Point>::iterator pt_it;

			image = Image<int>(rank, dimensions, bit_depth, 1);
			image_data = image.getData();
			for(it = masks->begin(); it!=masks->end(); ++it)
			{
				label = it->first;
				points = it->second->getPoints();
				for(pt_it=points->begin(); pt_it!=points->end(); ++pt_it)
				{
					for(int i=0; i<rank; ++i)
					{
						if((*pt_it)[i] < 0 || (*pt_it)[i]>dimensions[i])
						{
							//throw GenericException("Mask<Point> doesn't fit into the image");
							return Image<int>();
						}
					}
					pixel = Mask<Point>::getPixel(*pt_it, dimensions);
					image_data[pixel] = label;
				}
			}
			return image;
		}
		void relabel(int offset = 0)
		{
			typename std::unordered_map<int, Mask<Point>*> *tmp = new std::unordered_map<int, Mask<Point>*>();
			typename std::unordered_map<int, Mask<Point>*>::iterator it;
			int id = 1+offset;

			labels->clear();
			for(it = masks->begin(); it!=masks->end(); ++it)
			{
				tmp->insert(std::pair<int,Mask<Point>*>(id, it->second));
				labels->insert(id);
				++id;
			}
			delete masks;
			masks = tmp;
		}
		std::set<int>* getLabels()
		{
			return labels;
		}
		int getSize()
		{
			return masks->size();
		}
		std::string toString()
		{
			std::stringstream ss;
			typename std::set<int>::iterator it;

			ss << "Size of list: " << labels->size() << "\n";
			for(it=labels->begin(); it!=labels->end(); ++it)
			{
				ss << "\t label: " << *it << " size: " << masks->find(*it)->second->getSize() << "\n";
			}
			return ss.str();
		}

	private:
		std::set<int> *labels;
		std::unordered_map<int, Mask<Point>*> *masks;
		const static int bit_depth = 16;

		friend void swap(MaskList<Point> &first, MaskList<Point> &second) // nothrow
		{
			// enable ADL (not necessary in our case, but good practice)
			using std::swap;

			// by swapping the members of two classes,
			// the two classes are effectively swapped
			swap(first.labels, second.labels);
			swap(first.masks, second.masks);
		}
};

} /* end namespace elib */


#endif /* MASKLIST_HPP_ */
