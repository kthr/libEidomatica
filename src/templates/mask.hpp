/*
 * mask.hpp
 *
 *  Created on: Jun 12, 2013
 *      Author: kthierbach
 */

#ifndef MASK_HPP_
#define MASK_HPP_

#include <boost/numeric/ublas/matrix_sparse.hpp>
#include <functional>
#include <limits>
#include <memory>
#include <stdlib.h>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "circular_linked_list.hpp"
#include "glm/glm.hpp"
#include "image.hpp"

namespace elib{

template <class Point>
class Mask
{
	public:
		Mask() noexcept
		{
		}
		Mask(int rank, const std::vector<int> &dimensions) : rank(rank)
		{
			this->dimensions = std::vector<int>(dimensions.begin(), dimensions.end());
		}
		Mask(const Mask &other) : rank(other.rank), dimensions(other.dimensions), points(other.points)
		{
			if(other.mask != nullptr)
			{
				mask = std::unique_ptr<boost::numeric::ublas::compressed_matrix<int>>(new boost::numeric::ublas::compressed_matrix<int>(*other.mask));
			}
			if(other.bounding_box != nullptr)
			{
				bounding_box = std::unique_ptr<BoundingBox<Point>>(new BoundingBox<Point>(*other.bounding_box));
			}
		}
		Mask(Mask&& other) : rank(other.rank), dimensions(std::move(other.dimensions)), points(std::move(other.points)),
				mask(std::move(other.mask)), bounding_box(std::move(other.bounding_box))
		{
		}
		virtual ~Mask()
		{
		}
		Mask& operator=(const Mask &other)
		{
			rank = other.rank;
			dimensions = other.dimensions;
			points = other.points;
			if(other.mask != nullptr)
			{
				mask = std::unique_ptr<boost::numeric::ublas::compressed_matrix<int>>(new boost::numeric::ublas::compressed_matrix<int>(*other.mask));
			}
			if(other.bounding_box != nullptr)
			{
				bounding_box = std::unique_ptr<BoundingBox<Point>>(new BoundingBox<Point>(*bounding_box));
			}
			return *this;
		}
		Mask& operator=(Mask &&other)
		{
			rank = other.rank;
			dimensions = std::move(other.dimensions);
			points = std::move(other.points);
			mask = std::move(other.mask);
			bounding_box = std::move(other.bounding_box);
			return *this;
		}
		bool overlap(Mask &other)
		{
			namespace ub = boost::numeric::ublas;

			if(this->bounding_box == nullptr)
			{
				this->setBox(this->bounding_box);
			}
			if(other.bounding_box == nullptr)
			{
				other.setBox(other.bounding_box);
			}
			if(this->bounding_box->overlap(*other.bounding_box))
			{
				if(this->mask == nullptr)
				{
					this->createSparseRepresentation();
				}
				if(other.mask == nullptr)
				{
					other.createSparseRepresentation();
				}
				Mask mask(this->getRank(), *(this->getDimensions()));
				boost::numeric::ublas::compressed_matrix<int> result;
				try
				{
					result = boost::numeric::ublas::element_prod(*this->mask, *other.mask);
				}
				catch(const std::exception &e)
				{
					std::cout << "Can't get element product: " << e.what() << std::endl;
					abort();
				}
				if(mask.getRank()==2)
				{
					for(auto i = result.begin1(); i!= result.end1(); ++i)
					{
						for(auto j= i.begin(); j != i.end(); ++j)
							mask.addPoint(Point(j.index1(), j.index2()));
					}
				}
				return bool(mask.getSize());
			}
			return false;
		}
		void addPoint(Point p)
		{
			points.push_back(p);
			mask.reset();
			bounding_box.reset();
		}
		void deleteSparseRepresentation()
		{
			mask.reset();
		}
		const std::vector<Point>* getMask() const
		{
			return &points;
		}
		Image<int> toImage()
		{
			Image<int> image;
			int *image_data;
			int pixel;
			bool add_point;

			image = Image<int>(rank, dimensions, bit_depth, 1);
			image_data = image.getData();
			for(auto it=points.begin(); it!=points.end(); ++it)
			{
				add_point=true;
				for(int i=0; i<rank; ++i)
				{
					if((*it)[i] < 0 || (*it)[i]>=dimensions[i])
					{
						add_point=false;
					}
				}
				if(add_point)
				{
					pixel = Mask::getPixel(*it);
					image_data[pixel] = 1;
				}
			}
			return image;
		}
		BoundingBox<Point> getBoundingBox()
		{
			if(bounding_box == nullptr)
			{
				setBox(bounding_box);
			}
			return *bounding_box;
		}
		std::string getBoxMask()
		{
			if(bounding_box == nullptr)
			{
				setBox(bounding_box);
			}
			return boxMask(bounding_box);
		}
		const std::vector<Point>* getPoints() const
		{
			return &points;
		}
		int getSize() const
		{
			return int(points.size());
		}
		void getOutline(std::vector<Point> &polygon, Point &centroid) const
		{
			outline(polygon, centroid);
		}
		const std::vector<int>* getDimensions() const
		{
			return &dimensions;
		}
		int getPixel(Point p)
		{
			return pixel(p);
		}
		int getRank() const
		{
			return rank;
		}

		void setDimensions(const std::vector<int> &dimensions)
		{
			this->dimensions = std::vector<int>(dimensions.begin(), dimensions.end());
		}
		void setRank(int rank)
		{
			this->rank = rank;
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
		void setOrigin(Point origin)
		{
			for(auto i = points.begin(); i != points.end(); ++i)
			{
				*i += origin;
			}
		}

		static Mask<glm::ivec3> negate(const Mask<glm::ivec3> &mask)
		{
			Mask<glm::ivec3> negated = Mask(mask.rank, mask.dimensions);
			std::unordered_set<glm::ivec3, VectorHashFunctor> do_not_include;
            do_not_include.insert(mask.getPoints()->begin(), mask.getPoints()->end());
			glm::ivec3 p;
			for(int k=0; k<mask.getDepth(); ++k)
			{
				for(int j=0; j<mask.getHeight(); ++j)
				{
					for(int i=0; i<mask.getWidth(); ++i)
					{
						p = glm::ivec3(i,j,k);
						if(do_not_include.find(p) == do_not_include.end())
						{
							negated.addPoint(p);
						}
					}
				}
			}
			return negated;
		}

	private:
		int rank = 0;
		const static int bit_depth = 16;
		std::vector<int> dimensions;
		std::vector<Point> points;
		std::unique_ptr<boost::numeric::ublas::compressed_matrix<int>> mask = nullptr;
		std::unique_ptr<BoundingBox<Point>> bounding_box = nullptr;

		inline int pixel(glm::ivec2 p)
		{
			return p[1]*dimensions[0]+p[0];
		}
		inline int pixel(glm::ivec3 p)
		{
			return p[2]*dimensions[0]*dimensions[1] + p[1]*dimensions[0] + p[0];
		}
		void createSparseRepresentation()
		{
			namespace ub = boost::numeric::ublas;
			mask = std::unique_ptr<ub::compressed_matrix<int>>(new ub::compressed_matrix<int>(dimensions[0], dimensions[1], points.size()));
			if(!mask)
			{
				std::cerr << "Creation of sparse representation failed!";
				abort();
			}
			for(auto i=points.begin(); i!=points.end(); ++i)
			{
				mask->insert_element((*i).x, (*i).y, 1);
			}
		}
		void setBox(std::unique_ptr<BoundingBox<glm::ivec2>> &box)
		{
			int minX = std::numeric_limits<int>::max(),
				minY = std::numeric_limits<int>::max(),
				maxX = 0,
				maxY = 0;
			for(auto it=points.begin(); it!=points.end(); ++it)
			{
				minX = std::min(minX, it->x);
				minY = std::min(minY, it->y);
				maxX = std::max(maxX, it->x);
				maxY = std::max(maxY, it->y);
			}
			box = std::unique_ptr<BoundingBox<glm::ivec2>>(new BoundingBox<glm::ivec2>(glm::ivec2(minX, minY), glm::ivec2(maxX, maxY)));
		}
		void setBox(std::unique_ptr<BoundingBox<glm::ivec3>> &box)
		{
			int minX = std::numeric_limits<int32_t>::max(),
				minY = std::numeric_limits<int32_t>::max(),
				minZ = std::numeric_limits<int32_t>::max(),
				maxX = 0,
				maxY = 0,
				maxZ = 0;
			for(auto it=points.begin(); it!=points.end(); ++it)
			{
				minX = std::min(minX, it->x);
				minY = std::min(minY, it->y);
				minZ = std::min(minZ, it->z);
				maxX = std::max(maxX, it->x);
				maxY = std::max(maxY, it->y);
				maxZ = std::max(maxZ, it->z);
			}
			std::unique_ptr<BoundingBox<glm::ivec3>>(new BoundingBox<glm::ivec3>(glm::ivec3(minX, minY, minZ), glm::ivec3(maxX+1, maxY+1, maxZ+1)));
		}
		std::string boxMask(const std::unique_ptr<BoundingBox<glm::ivec2>> &boundingBox)
		{
			glm::ivec2 p, p2;
			std::vector<char> tmp;
			std::vector<char>::iterator cit;
			std::vector<glm::ivec2>::iterator it;
			int pixel;
			std::string mask = "";

			p = boundingBox->getBottomRight() - boundingBox->getUpperLeft() + glm::ivec2(1,1);
			dimensions[0] = p[0];
			dimensions[1] = p[1];
			tmp = std::vector<char>(p[0]*p[1], '0');
			for(it=points.begin(); it!=points.end(); ++it)
			{
				pixel = getPixel((*it)-boundingBox->getUpperLeft());
				tmp[pixel] = '1';
			}
			for(cit=tmp.begin(); cit!=tmp.end(); ++cit)
			{
				mask += *cit;
			}
			return mask;
		}
		void outline(std::vector<glm::ivec2> &outline, glm::ivec2 &centroid) const
		{
//			std::sort(points.begin(), points.end(), VectorSortComparator);
			if (!points.empty())
			{
				if(points.size() == 1)
				{
					//prevents function from infinite loop when the mask only contains one pixel
					centroid += points.front();
					outline.push_back(points.front());
					return;
				}
				std::unordered_set<glm::ivec2, VectorHashFunctor, VectorEqualFunctor> set;
				CircularLinkedList<glm::ivec2> neighbours;
				glm::ivec2 b, c, b0, b1, tmp, c1, *previous, *current;

				set.insert(points.begin(), points.end());

				neighbours.addLast(glm::ivec2(-1, 0));
				neighbours.addLast(glm::ivec2(-1, -1));
				neighbours.addLast(glm::ivec2(0, -1));
				neighbours.addLast(glm::ivec2(1, -1));
				neighbours.addLast(glm::ivec2(1, 0));
				neighbours.addLast(glm::ivec2(1, 1));
				neighbours.addLast(glm::ivec2(0, 1));
				neighbours.addLast(glm::ivec2(-1, 1));
				neighbours.setActualElement(glm::ivec2(-1, 0));

				b0 = points[0];
				for (int i = 1; i < neighbours.size(); i++)
				{
					previous = neighbours.getActualElementData();
					current = neighbours.getNext();
					tmp = glm::ivec2(b0.x + current->x, b0.y + current->y);
					if (set.find(tmp) != set.end())
					{
						b1 = tmp;
						c1 = glm::ivec2(b0.x + previous->x, b0.y + previous->y);
						outline.push_back(glm::ivec2(b0.x, b0.y));
						outline.push_back(glm::ivec2(b1.x, b1.y));
						break;
					}
				}
				b = b1;
				c = c1;
				while (true)
				{
					neighbours.setActualElement(glm::ivec2(c.x - b.x, c.y - b.y));
					for (int i = 1; i < 8; i++)
					{
						previous = neighbours.getActualElementData();
						current = neighbours.getNext();
						tmp = glm::ivec2(b.x + current->x, b.y + current->y);
						if (set.find(tmp) != set.end())
						{
							c = glm::ivec2(b.x + previous->x, b.y + previous->y);
							b = tmp;
							outline.push_back(glm::ivec2(b.x, b.y));
							break;
						}
					}
					if (b == b0)
						break;
				}
				for (unsigned int i = 0; i < outline.size(); ++i)
				{
					centroid += outline[i];
				}
				centroid /= outline.size();
			}
		}
		struct VectorHashFunctor
		{
			size_t operator()(const glm::ivec2 &v) const
			{
				return std::hash<int>()(v.x) ^ std::hash<int>()(v.y);
			}
            size_t operator()(const glm::ivec3 &v) const
			{
				return std::hash<int>()(v.x) ^ std::hash<int>()(v.y) ^ std::hash<int>()(v.z);
			}
		};
		struct VectorEqualFunctor
		{
			bool operator()(const glm::ivec2 &v1, const glm::ivec2 &v2) const
			{
				return (v1.x==v2.x && v1.y==v2.y);
			}
			bool operator()(const glm::ivec3 &v1, const glm::ivec3 &v2) const
			{
				return (v1.x==v2.x && v1.y==v2.y && v1.z==v2.z);
			}
		};
};

} /* namespace elib */

#endif /* MASK_HPP_ */

