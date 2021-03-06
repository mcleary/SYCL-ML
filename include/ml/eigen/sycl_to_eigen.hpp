#ifndef INCLUDE_ML_EIGEN_SYCL_TO_EIGEN_HPP
#define INCLUDE_ML_EIGEN_SYCL_TO_EIGEN_HPP

#include "ml/utils/buffer_t.hpp"
#include "ml/utils/access.hpp"

namespace ml
{

namespace detail
{

template <int IN_DIM, int OUT_DIM=IN_DIM>
eig_dsize_t<OUT_DIM> range_to_dsize(const range<IN_DIM>& r) {
  static_assert(IN_DIM <= OUT_DIM, "");

  eig_dsize_t<OUT_DIM> dim;
  int i = 0;
  for (; i < IN_DIM; ++i)
    dim[i] = static_cast<eig_index_t>(r[i]);
  for (; i < OUT_DIM; ++i)
    dim[i] = 1;
  return dim;
}

} // detail

/**
 * @brief Convert a SYCL buffer to an Eigen Tensor.
 *
 * The class holds the host pointer and makes sure that the Tensor is destroyed at the end.\n
 * Thus this object must stay alive as long as the Tensor is used.
 *
 * @todo Because of the way Eigen works if 2 \p sycl_to_eigen_t objects are created with the same buffer and one is
 *       destroyed, the 2 Tensors become invalid. The fix would require to either count the number of references for
 *       each buffer or to create a different pointer if one already exist.
 *
 * @tparam T Only the type defined by \p cl::sycl::codeplay::buffer_data_type is possible for now
 * @tparam IN_DIM dimension of the SYCL buffer
 * @tparam OUT_DIM dimension of the Eigen Tensor
 * @tparam DataLayout Eigen::RowMajor or Eigen::ColMajor
 */
template <class T, int IN_DIM, int OUT_DIM = IN_DIM, Eigen::StorageOptions DataLayout = Eigen::RowMajor>
class sycl_to_eigen_t {
private:
  using Self = sycl_to_eigen_t<T, IN_DIM, OUT_DIM, DataLayout>;

public:
  sycl_to_eigen_t() : _host_ptr(nullptr), _tensor(nullptr, eig_dsize_t<OUT_DIM>()) {}

  sycl_to_eigen_t(buffer_t<T, IN_DIM>& b, const eig_dsize_t<OUT_DIM>& sizes) :
      _host_ptr(static_cast<T*>(get_eigen_device().attach_buffer(b))),
      _tensor(_host_ptr, sizes) {}

  ~sycl_to_eigen_t() {
    if (_host_ptr)
      get_eigen_device().detach_buffer(_host_ptr);
  }

  /**
   * @return the Eigen Tensor
   */
  inline auto& tensor() { return _tensor; }

  /**
   * @return the Eigen TensorDevice (for assignment)
   */
  inline auto device() { return tensor().device(get_eigen_device()); }

  // No copy, only move
  sycl_to_eigen_t(const Self&) = delete;
  sycl_to_eigen_t(Self&&) = default;
  Self& operator=(const Self&) = delete;
  Self& operator=(Self&&) = default;

private:
  T* _host_ptr;
  tensor_map_t<T, OUT_DIM, DataLayout> _tensor;
};

/**
 * @brief Create a Tensor of dimension 0 from a SYCL buffer.
 *
 * Only the first value of the buffer is used.
 *
 * @tparam IN_DIM
 * @tparam DataLayout
 * @tparam T
 * @param b
 * @return the \p sycl_to_eigen_t associated to \p b
 */
template <int IN_DIM, Eigen::StorageOptions DataLayout = Eigen::RowMajor, class T>
inline auto sycl_to_scalar_eigen(buffer_t<T, IN_DIM>& b) {
  return sycl_to_eigen_t<T, IN_DIM, 0, DataLayout>(b, eig_dsize_t<0>());
}

/**
 * @brief Create a Tensor of any dimensions from a SYCL buffer.
 *
 * @tparam IN_DIM dimension of the input buffer
 * @tparam OUT_DIM dimension of the output Tensor
 * @tparam R_DIM dimension of the range
 * @tparam DataLayout
 * @tparam T
 * @param b
 * @param r range defining the size of the tensor
 * @return the \p sycl_to_eigen_t associated to \p b
 */
template <int IN_DIM, int OUT_DIM=IN_DIM, Eigen::StorageOptions DataLayout = Eigen::RowMajor, int R_DIM, class T>
inline auto sycl_to_eigen(buffer_t<T, IN_DIM>& b, const range<R_DIM>& r) {
  static_assert(R_DIM >= IN_DIM && R_DIM <= OUT_DIM, "");
  return sycl_to_eigen_t<T, IN_DIM, OUT_DIM, DataLayout>(b, detail::range_to_dsize<R_DIM, OUT_DIM>(r));
}

/// @see sycl_to_eigen(buffer_t<T, IN_DIM>&, const range<IN_DIM>&)
template <int IN_DIM, int OUT_DIM=IN_DIM, Eigen::StorageOptions DataLayout = Eigen::RowMajor, class T>
inline auto sycl_to_eigen(buffer_t<T, IN_DIM>& b) {
  return sycl_to_eigen<IN_DIM, OUT_DIM, DataLayout>(b, b.get_kernel_range());
}

/**
 * @brief Force a buffer of dimension 1 to be converted to a Tensor of dimension 2.
 *
 * @tparam D whether to build the Tensor as a column (by default) or a row.
 * @tparam DataLayout
 * @tparam T
 * @param v
 * @return the \p sycl_to_eigen_t associated to \p b
 */
template <data_dim D = COL, Eigen::StorageOptions DataLayout = Eigen::RowMajor, class T>
inline auto sycl_to_eigen_2d(vector_t<T>& v) {
  return sycl_to_eigen<1, 2, DataLayout>(v, build_lin_or_tr<opp<D>(), range<2>>(v.get_kernel_range()[0], 1));
}

} // ml

#endif //INCLUDE_ML_EIGEN_SYCL_TO_EIGEN_HPP
