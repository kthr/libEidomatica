/*
 * tensor.hpp
 *
 *  Created on: Oct 25, 2013
 *      Author: kthierbach
 */

#ifndef TENSOR_HPP_
#define TENSOR_HPP_

#include <string>

namespace elib{

template <typename T>
class Tensor
{
	public:
		Tensor()
		{

		}
		Tensor(const Tensor &other)
		: flattened_length(other.flattened_length), rank(other.rank)
		{
			dimensions = new int[rank];
			data = new T[flattened_length];

			std::copy(other.dimensions, other.dimensions+rank, dimensions);
			std::copy(other.data, other.data+flattened_length, data);
		}
		Tensor(const Tensor &&other)
		: Tensor()
		{
			this->operator=(other);
		}
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
				std::fill_n(this->data, flattened_length, 0);
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

		Tensor& operator=(Tensor other)
		{
			swap(*this,other);
			return *this;
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
		T* getData()
		{
			return data;
		}

	private:
		T *data = nullptr;
		int *dimensions = nullptr,
			flattened_length = 0,
			rank = 0;

		friend void swap(Tensor& first,Tensor& second)
		{
			using std::swap;

			swap(first.dimensions, second.dimensions);
			swap(first.flattened_length, second.flattened_length);
			swap(first.rank, second.rank);
			swap(first.data, second.data);
		}
};

} /* end namespace elib */

#endif /* TENSOR_HPP_ */
