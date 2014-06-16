/*
 * tensor.hpp
 *
 *  Created on: Oct 25, 2013
 *      Author: kthierbach
 */

#ifndef TENSOR_HPP_
#define TENSOR_HPP_

#include <string>
#include <vector>

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
			dimensions = std::vector<int>(rank);
			data = std::unique_ptr<T>(new T[flattened_length]);

			std::copy(other.dimensions.begin(), other.dimensions.end(), dimensions.begin());
			std::copy(other.data.get(), other.data.get()+flattened_length, data.get());
		}
		Tensor(const Tensor &&other)
		: Tensor()
		{
			this->operator=(other);
		}
		Tensor(int rank, const int *dimensions) : rank(rank)
		{
			if(rank > 0)
			{
				flattened_length = dimensions[0];
				for(int i=1; i<rank; ++i)
				{
					flattened_length *= dimensions[i];
				}
				this->data = std::unique_ptr<T>(new T[flattened_length]);
				this->dimensions = std::vector<int>(rank);
				std::copy(dimensions, dimensions + flattened_length, this->dimensions.begin());
				std::fill_n(this->data.get(), flattened_length, 0);
			}
		}
		Tensor(int rank, const std::vector<int> &dimensions) : rank(rank)
		{
			if(rank > 0)
			{
				flattened_length = dimensions[0];
				for(int i=1; i<rank; ++i)
				{
					flattened_length *= dimensions[i];
				}
				this->data = std::unique_ptr<T>(new T[flattened_length]);
				this->dimensions = std::vector<int>(rank);
				std::copy(dimensions.begin(), dimensions.end(), this->dimensions.begin());
				std::fill_n(this->data.get(), flattened_length, 0);
			}
		}
		Tensor(int rank, const std::vector<int> &dimensions, const T *data) : Tensor(rank, dimensions)
		{
			std::copy(data, data + this->flattened_length, this->data);
		}
		virtual ~Tensor()
		{
		}

		Tensor& operator=(Tensor other)
		{
			swap(*this,other);
			return *this;
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
		T* getData() const
		{
			return data.get();
		}
		T get(int i) const
		{
			return data.get()[i];
		}

	private:
		std::unique_ptr<T> data = nullptr;
		std::vector<int> dimensions;
		int flattened_length = 0,
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
