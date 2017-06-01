#ifndef _ARRAY_H_
#define _ARRAY_H_

#include <iostream>
#include <numeric>
#include <cstddef>

inline
void check_multi_index(std::size_t rank,
		       const std::size_t* sizes,
		       const std::size_t* multi_index) {
  for (std::size_t i(0); i < rank; ++i)
    if (multi_index[i] >= sizes[i])
      throw std::string("multi index out of bound");
}

inline
void check_linear_index(std::size_t rank,
			const std::size_t* sizes,
			std::size_t linear_index) {
  if (linear_index >= std::accumulate(sizes,
				      sizes + rank,
				      1, std::multiplies<std::size_t>()))
    throw std::string("linear index out of bound");
}

inline
std::size_t to_linear_index(std::size_t rank,
			    const std::size_t* sizes,
			    const std::size_t* multi_index,
			    bool fortran_array_layout = false) {
  check_multi_index(rank, sizes, multi_index);
  
  std::size_t linear_index(0);
  std::size_t basis(1);

  for (std::size_t i(0); i < rank; ++i) {
    const std::size_t pos(fortran_array_layout ? i : rank - 1 - i);
    linear_index += basis * multi_index[pos];
    basis *= sizes[pos];
  }
  
  return linear_index;
}

inline
void to_multi_index(std::size_t rank,
		    std::size_t* multi_index,
		    const std::size_t* sizes,
		    std::size_t linear_index,
		    bool fortran_array_layout = false) {
  check_linear_index(rank, sizes, linear_index);
  
  for (std::size_t i(0); i < rank; ++i) {
    const std::size_t pos(fortran_array_layout ? i : rank - 1 - i);
    multi_index[i] = linear_index % sizes[pos];
    linear_index = (linear_index - multi_index[pos]) / sizes[pos];
  }
}

template<typename D, typename A, typename ... As>
void fill_array_with_parameters(D* dest, const A& a, const As&... as) {
  *dest = a;
  fill(++dest, as...);
}

template<typename D, typename A>
void fill_array_with_parameters(D* dest, const A& a) {
  *dest = a;
}

template<typename T>
class array {
public:
  typedef T value_type;

  array(std::size_t r, const std::size_t* s):
    rank(r), sizes(new std::size_t[rank]), data(nullptr) {

    std::copy(s, s + r, sizes);
    const std::size_t elements_number(std::accumulate(sizes,
						      sizes + rank,
						      1, std::multiplies<std::size_t>()));
    data = new T[elements_number];
  }

  array(std::initializer_list<std::size_t> s)
    : rank(s.size()), sizes(new std::size_t[rank]), data(nullptr) {

    std::copy(s.begin(), s.end(), sizes);
    const std::size_t elements_number(std::accumulate(sizes,
						      sizes + rank,
						      1, std::multiplies<std::size_t>()));
    data = new T[elements_number];
  }

  array(const array<T>& op): array(op.rank, op.sizes) {
    const std::size_t elements_number(std::accumulate(sizes,
						      sizes + rank,
						      1, std::multiplies<std::size_t>()));
    std::copy(op.data, op.data + elements_number, data);
  }

  array(array<T>&& op)
    : rank(op.rank),
      sizes(op.sizes),
      data(op.data) {
    op.sizes = nullptr;
    op.data = nullptr;
  }

  ~array() {
    delete [] sizes; sizes = nullptr;
    delete [] data; data = nullptr;
  }

  array<T>& operator=(const array<T>&) = delete;
  array<T>& operator=(array<T>&&) = delete;
  
  template<typename ... Is>
  const T& at(Is ... is) {
    if (sizeof...(Is) != rank)
      throw std::string("multi index rank mismatch");
    
    std::size_t indices[sizeof...(Is)] = {};
    fill_array_with_parameters(indices, is...);

    return data[to_linear_index(rank, sizes, indices, false)];
  }

  template<typename ... Is>
  T& at(Is ... is) {
    if (sizeof...(Is) != rank)
      throw std::string("multi index rank mismatch");
    
    std::size_t indices[sizeof...(Is)] = {};
    fill_array_with_parameters(indices, is...);

    return data[to_linear_index(rank, sizes, indices, false)];
  }

  std::size_t get_rank() const { return rank; }
  std::size_t get_size(std::size_t i) const { return sizes[i]; }
  const std::size_t* get_sizes() const { return sizes; }
  
private:
  std::size_t rank;
  std::size_t *sizes;
  value_type* data;
};

#endif /* _ARRAY_H_ */
