/*
 * GraphCutImage.hpp
 *
 *  Created on: Jan 22, 2013
 *      Author: kthierbach
 */

#ifndef LABELING_HPP_
#define LABELING_HPP_

#include <vector>

#include "templates/image.hpp"
#include "utilities/parameters.hpp"

namespace elib{

class MultiLabelGraphcut
{
	public:
		MultiLabelGraphcut(){}
		~MultiLabelGraphcut(){}
		std::shared_ptr<Image<int>> multilabel_graphcut(Image<int> &label_image, Image<int> &input_image, Parameters &input_params);
		std::shared_ptr<Image<int>> adaptive_multilabel_graphcut(Image<int> &label_image, Image<int> &input_image, Parameters &input_params);

	private:
		int cycles = -1;
		void calculateIntensityDistributions(std::vector<float> &c0, std::vector<float> &c1, Image<int> &label_image, Image<int> &input_image);
};

struct ForSmoothFn
{
		int *image;
		float lambda;
		float mu;
		int dummyLabel;
};

inline float label_dist(int value);
float smoothFn(int p1, int p2, int l1, int l2, void *data);

} /* namespace elib */

#endif /* LABELING_HPP_ */
