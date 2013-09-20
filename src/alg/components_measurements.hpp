/*
 * componentsMeasurements.hpp
 *
 *  Created on: Jun 10, 2013
 *      Author: kthierbach
 */

#ifndef COMPONENTSMEASUREMENTS_HPP_
#define COMPONENTSMEASUREMENTS_HPP_

#include <queue>
#include <set>

#include "templates/masklist.hpp"

namespace elib {

template <typename Point>
class ComponentsMeasurements
{
	public:

		ComponentsMeasurements()
		{
			labels = new std::set<int>();
			masks = new MaskList<Point>();
		}
		ComponentsMeasurements(const ComponentsMeasurements& other)
		{
			operator=(other);
		}
		ComponentsMeasurements(ComponentsMeasurements&& other)
		: ComponentsMeasurements::ComponentsMeasurements()
		{
			swap(*this,other);
		}
		explicit ComponentsMeasurements(Image<int> label_image)
		{
			this->label_image = label_image;
			labels = new std::set<int>();
			masks = new MaskList<Point>();
			init();
		}
		virtual ~ComponentsMeasurements()
		{
			delete labels;
			delete masks;
		}

		ComponentsMeasurements& operator=(ComponentsMeasurements other)
		{
			swap(*this, other);
			return *this;
		}

		MaskList<Point> getMasks()
		{
			return *masks;
		}
		int getNumberOfObjects() const
		{
			return masks->getSize();
		}
		const std::set<int>* getLabels() const
		{
			return labels;
		}

	private:

		Image<int> label_image;
		int num_labels = 0;
		std::set<int> *labels;
		MaskList<Point> *masks;

		friend void swap(ComponentsMeasurements& first, ComponentsMeasurements& second)
		{
			// enable ADL (not necessary in our case, but good practice)
			using std::swap;
			// by swapping the members of two classes,
			// the two classes are effectively swapped
			swap(first.connectivity, second.connectivity);
			swap(first.label_image, second.label_image);
			swap(first.labels, second.labels);
			swap(first.masks, second.masks);
			swap(first.num_labels, second.num_labels);
		}
		void init()
		{
			int *label_data = label_image.getData();

			int pixel;
			int width = label_image.getWidth(),
				height = label_image.getHeight(),
				depth = label_image.getDepth(),
				label;
			Mask<Point>* mask_ptr;
			for(int k=0; k<depth; ++k)
			{
				for (int j = 0; j < height; ++j)
				{
					for (int i = 0; i < width; ++i)
					{
						pixel = k*width*height + j*width + i;
						if ((label=label_data[pixel]) > 0)
						{
							if((mask_ptr=masks->getMask(label)) == nullptr)
							{
								labels->insert(label);
								mask_ptr = masks->addMask(label);
								mask_ptr->addPoint(Point(i, j, k));
							}
							else
							{
								mask_ptr->addPoint(Point(i, j, k));
							}
						}

					}
				}
			}
		}
};

} /* namespace elib */

#endif /* COMPONENTSMEASUREMENTS_HPP_ */
