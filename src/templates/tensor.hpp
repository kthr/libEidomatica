/*
 * tensor.hpp
 *
 *  Created on: Oct 25, 2013
 *      Author: kthierbach
 */

#ifndef TENSOR_HPP_
#define TENSOR_HPP_

namespace elib{

template <typename T>
class Tensor
{
	public:
		Tensor();
		Tensor(int rank, int *dimensions) : rank(rank)
		{
			if(rank > 0)
			{
				flattened_length = dimensions[0];
				for(int i=1; i<rank; ++i)
				{
					flattened_length *= dimensions[i];
				}
				this->data = new T[flattened_length];
				this->dimensions = new int[rank];
				std::copy(dimensions, dimensions + rank, this->dimensions);
			}
		}
		Tensor(int rank, int *dimensions, T *data) : Tensor(rank, dimensions)
		{
			std::copy(data, data + this->flattened_length, this->data);
		}
		virtual ~Tensor()
		{
			delete[] data;
			delete[] dimensions;
		}

		int* getDimensions() const
		{
			return dimensions;
		}
		int getRank() const
		{
			return rank;
		}
		T* getData()
		{
			return data;
		}

	private:
		T *data = nullptr;
		int *dimensions = nullptr,
			flattened_length = 0,
			rank = 0;
};

} /* end namespace elib */

#endif /* TENSOR_HPP_ */
