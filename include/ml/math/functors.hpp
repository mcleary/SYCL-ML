#ifndef INCLUDE_ML_MATH_FUNCTORS_HPP
#define INCLUDE_ML_MATH_FUNCTORS_HPP

#include "ml/utils/common.hpp"

namespace ml
{

namespace functors
{

template<class T>
struct positive {
  constexpr T operator()(T x) const {
    return x > 0;
  }
};

template<class T>
struct negative {
  constexpr T operator()(T x) const {
    return x < 0;
  }
};

template<class T>
struct identity {
  constexpr T operator()(T x) const {
    return x;
  }
};

template<class T>
struct sqrt {
  constexpr T operator()(T x) const {
    return cl::sycl::sqrt(x);
  }
};

template <class T, class BinaryOp>
class partial_binary_op {
public:
  partial_binary_op(T c, BinaryOp binary_op = BinaryOp()) : _c(c), _binary_op(binary_op) {}

  inline constexpr T operator()(T x) const {
    return _binary_op(_c, x);
  }

private:
  T _c;
  BinaryOp _binary_op;
};

template <class T>
struct sum_log_abs {
  inline constexpr T operator()(T x1, T x2) const {
    return x1 + cl::sycl::log(cl::sycl::fabs(x2));
  }
};

template <class T>
struct exp_diff {
  template <class T1, class T2>
  constexpr T operator()(T1&& x1, T2&& x2) const {
    return cl::sycl::exp(std::forward<T1>(x1) - std::forward<T2>(x2));
  }
};

template <class T>
struct prod_diff {
  template <class T1, class T2>
  constexpr T operator()(T1&& x1, T2&& x2) const {
    return (std::forward<T2>(x2) - std::forward<T1>(x1)) * (std::forward<T2>(x2) - std::forward<T1>(x1));
  }
};

template <class T>
struct prod_sum {
  template <class T1, class T2>
  constexpr T operator()(T1&& x1, T2&& x2) const {
    return (std::forward<T2>(x2) + std::forward<T1>(x1)) * (std::forward<T2>(x2) + std::forward<T1>(x1));
  }
};

}

}

#endif //INCLUDE_ML_MATH_FUNCTORS_HPP
