#ifndef FUNCTION_TRAITS_H
#define FUNCTION_TRAITS_H
// learned from this
// https://functionalcpp.wordpress.com/2013/08/05/function-traits/
#include <cstddef>
#include <tuple>

namespace srpc {
namespace common {

struct no_arg {};
struct have_arg {};
struct return_void {};
struct return_nonvoid {};

template <int N>
struct arg_count_trait {
  using type = have_arg;
};
template <>
struct arg_count_trait<0> {
  using type = no_arg;
};
template <typename T>
struct result_trait {
  using type = return_nonvoid;
};
template <>
struct result_trait<void> {
  using type = return_void;
};

template <class F>
struct function_traits
    : public function_traits<decltype(&F::type::operator())> {};

template <class F>
struct function_traits<F&> : public function_traits<F> {};

template <class F>
struct function_traits<F&&> : public function_traits<F> {};

template <class R, class... Args>
struct function_traits<R (*)(Args...)> {
  using return_type = R;

  static constexpr std::size_t arity = sizeof...(Args);

  using args_type = std::tuple<typename std::decay<Args>::type...>;

  typedef arg_count_trait<arity> args_tag;
  typedef result_trait<return_type> return_tag;

  template <std::size_t N>
  struct argument {
    static_assert(N < arity, "error: invalid parameter index.");
    using type = typename std::tuple_element<N, std::tuple<Args...>>::type;
  };
};

// member function pointer
template <class C, class R, class... Args>
struct function_traits<R (C::*)(Args...)>
    : public function_traits<R(C&, Args...)> {};

// const member function pointer
template <class C, class R, class... Args>
struct function_traits<R (C::*)(Args...) const>
    : public function_traits<R(C&, Args...)> {};

// member object pointer
template <class C, class R>
struct function_traits<R(C::*)> : public function_traits<R(C&)> {};

template <typename T>
struct func_kind_info : func_kind_info<decltype(&T::operator())> {};

template <typename C, typename R, typename... Args>
struct func_kind_info<R (C::*)(Args...)> : func_kind_info<R (*)(Args...)> {};

template <typename C, typename R, typename... Args>
struct func_kind_info<R (C::*)(Args...) const>
    : func_kind_info<R (*)(Args...)> {};

template <typename R, typename... Args>
struct func_kind_info<R (*)(Args...)> {
  typedef typename arg_count_trait<sizeof...(Args)>::type args_kind;
  typedef typename result_trait<R>::type result_kind;
};

}  // namespace common
}  // namespace srpc

#endif