/*
 * connectedComponents.cpp
 *
 *  Created on: Jun 10, 2013
 *      Author: kthierbach
 */

#include "connected_components.hpp"

#include "glm/glm.hpp"

using elib::ConnectedComponents;
using elib::Image;

namespace elib{

std::vector<glm::ivec3> ConnectedComponents::SMALL_2D({glm::ivec3(-1,0,0), glm::ivec3(0,-1,0), glm::ivec3(1,0,0), glm::ivec3(0,1,0)});
std::vector<glm::ivec3> ConnectedComponents::LARGE_2D({glm::ivec3(-1,0,0), glm::ivec3(-1,-1,0), glm::ivec3(0,-1,0), glm::ivec3(1,-1,0), glm::ivec3(1,0,0), glm::ivec3(1,1,0), glm::ivec3(0,1,0), glm::ivec3(-1,1,0)});

std::vector<glm::ivec3> ConnectedComponents::SMALL_3D({glm::ivec3(-1,0,0), glm::ivec3(0,0,-1), glm::ivec3(1,0,0), glm::ivec3(0,0,1), glm::ivec3(0,1,0), glm::ivec3(0,-1,0)});
std::vector<glm::ivec3> ConnectedComponents::LARGE_3D({glm::ivec3(-1,0,0), glm::ivec3(0,0,-1), glm::ivec3(1,0,0), glm::ivec3(0,0,1), glm::ivec3(0,1,0), glm::ivec3(0,-1,0), glm::ivec3(-1,1,-1), glm::ivec3(1,1,-1), glm::ivec3(-1,1,1), glm::ivec3(1,1,1), glm::ivec3(-1,-1,-1), glm::ivec3(1,-1,-1), glm::ivec3(-1,-1,1), glm::ivec3(1,-1,1)});

ConnectedComponents::ConnectedComponents()
{
}

ConnectedComponents::~ConnectedComponents()
{
	// TODO Auto-generated destructor stub
}

Image<int> ConnectedComponents::getComponents(Image<int> image)
{
	Image<int> tmp_image = Image<int>(image);
	int *tmp_data = tmp_image.getData();
	Image<int> label_image = Image<int>(image.getRank(), *image.getDimensions(), 16, 1);
	int *label_data = label_image.getData();
	int width = image.getWidth(),
		height = image.getHeight(),
		depth = image.getDepth();
	std::queue<glm::ivec3 > indices;
	std::vector<glm::ivec3 > *neighbours;
	glm::ivec3 index;
	int pixel;

	if(connectivity == SMALL_CONNECTIVITY)
	{
		if(image.getRank() == 2)
			neighbours = &(ConnectedComponents::SMALL_2D);
		else
			neighbours = &(ConnectedComponents::SMALL_3D);
	}
	else
	{
		if(image.getRank() == 2)
			neighbours = &(ConnectedComponents::LARGE_2D);
		else
			neighbours = &(ConnectedComponents::LARGE_3D);
	}
	for(int k=0; k<depth; ++k)
	{
		for(int j=0; j<height; ++j)
		{
			for(int i=0; i<width; ++i)
			{
				pixel = k*width*height + j*width + i;
				if(tmp_data[pixel] > 0)
				{
					tmp_data[pixel] = 0;
					label_data[pixel] = label;
					index = glm::ivec3(i,j,k);
					addNeigbours(&indices, neighbours, index, image.getRank(), *image.getDimensions());
					while(!indices.empty())
					{
						index = indices.front();
						indices.pop();
						pixel = index.y*width + index.x;
						if(tmp_data[pixel] > 0)
						{
							tmp_data[pixel] = 0;
							label_data[pixel] = label;
							addNeigbours(&indices, neighbours, index,  image.getRank(), *image.getDimensions());
						}
					}
					label++;
				}

			}
		}
	}
	return label_image;

}

} /* end namespace elib */
