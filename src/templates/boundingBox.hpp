/*
 * boundingBox.hpp
 *
 *  Created on: Jan 28, 2014
 *      Author: kthierbach
 */

#ifndef BOUNDINGBOX_HPP_
#define BOUNDINGBOX_HPP_

#include "glm/glm.hpp"

namespace elib
{

template <class Point>
class BoundingBox
{
	public:
		BoundingBox(const BoundingBox &other) : upper_left(other.upper_left), bottom_right(other.bottom_right)
		{
		}
		BoundingBox(BoundingBox &&other) : upper_left(std::move(upper_left)), bottom_right(std::move(other.bottom_right))
		{
		}

		BoundingBox& operator=(const BoundingBox &other)
		{
			upper_left = other.upper_left;
			bottom_right = other.bottom_right;
		}
		BoundingBox& operator=(BoundingBox &&other)
		{
			upper_left = std::move(upper_left);
			bottom_right = std::move(other.bottom_right);
		}
		BoundingBox(Point upper_left, Point bottom_right) : upper_left(upper_left), bottom_right(bottom_right)
		{
		}
		virtual ~BoundingBox()
		{
		}

		bool inside(Point other)
		{
			if(smallerThan(upper_left,other) && smallerThan(other,bottom_right))
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		bool overlap(BoundingBox<glm::ivec2> box)
		{
			glm::ivec2 	other_upper_left = box.getUpperLeft(),
						other_bottom_right = box.getBottomRight();
			if(upper_left.x > other_bottom_right.x || other_upper_left.x > bottom_right.x ||
				upper_left.y > other_bottom_right.y || other_upper_left.y > bottom_right.y
			)
			{
				return false;
			}
			return true;
		}
		Point getUpperLeft() const
		{
			return upper_left;
		}
		Point getBottomRight() const
		{
			return bottom_right;
		}

	private:
		Point upper_left, bottom_right;

		static bool smallerThan(glm::ivec2 p1, glm::ivec2 p2)
		{
			return p1.x <= p2.x && p1.y <= p2.y;
		}
		static bool smallerThan(glm::ivec3 p1, glm::ivec3 p2)
		{
			return p1.x <= p2.x && p1.y <= p2.y && p1.z <= p2.z;
		}

};

} /* namespace elib */

#endif /* BOUNDINGBOX_HPP_ */
