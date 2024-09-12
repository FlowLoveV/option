#pragma once

#include <cpptrace/cpptrace.hpp>
#include <variant>

namespace navp {

class option_error : public std::runtime_error {
  using std::runtime_error::runtime_error;
};

template <typename T>
class Option;

namespace details {

struct NoneType {
  explicit NoneType() = default;
};

template <typename T>
concept is_tag_t = std::is_same_v<std::remove_cv_t<T>, NoneType>;
template <typename T>
concept not_tag_t = !std::is_same_v<std::remove_cv_t<T>, NoneType>;

}  // namespace details

template <details::not_tag_t T>
struct Some;

constexpr details::NoneType None{};

// The T type cannot be a tag type, which violates its original intent
template <details::not_tag_t T>
struct Some {
 private:
  template <typename _Up>
  using __not_self = std::__not_<std::is_same<Some, std::__remove_cvref_t<_Up>>>;

 public:
  typedef T value_type;

  Some() = default;
  Some(const Some&) = default;
  Some(Some&&) = default;
  Some& operator=(const Some&) = default;
  Some& operator=(Some&&) = default;

  template <details::not_tag_t U = T>
    requires __not_self<U>::value
  constexpr Some(const U& _val) noexcept(
      std::conjunction_v<std::is_nothrow_copy_constructible<T>, std::is_nothrow_constructible<T, const U&>>)
      : val(static_cast<T>(_val)) {
    static_assert(std::is_copy_constructible_v<T>);
    static_assert(std::is_constructible_v<T, const U&>);
  }

  template <details::not_tag_t U = T>
    requires __not_self<U>::value
  constexpr Some(U&& _val) noexcept(
      std::conjunction_v<std::is_nothrow_move_constructible<T>, std::is_nothrow_constructible<T, U&&>>)
      : val(static_cast<T>(std::forward<U>(_val))) {
    static_assert(std::is_move_constructible_v<T>);
    static_assert(std::is_constructible_v<T, U&&>);
  }

  template <details::not_tag_t U = T>
    requires __not_self<U>::value
  constexpr Some& operator=(const U& _val) noexcept(
      std::conjunction_v<std::is_nothrow_copy_assignable<T>, std::is_nothrow_convertible<const U&, T>>) {
    static_assert(std::is_copy_assignable_v<T>);
    static_assert(std::is_convertible_v<const U&, T>);
    if (this != &_val) {
      val = static_cast<T>(_val);
    }
    return *this;
  }

  template <details::not_tag_t U = T>
    requires __not_self<U>::value
  constexpr Some& operator=(U&& _val) noexcept(
      std::conjunction_v<std::is_nothrow_move_assignable<T>, std::is_nothrow_convertible<U&&, T>>) {
    static_assert(std::is_move_assignable_v<T>);
    static_assert(std::is_convertible_v<U&&, T>);
    if (this != &_val) {
      val = static_cast<T>(std::forward<U>(_val));
    }
    return *this;
  }

  constexpr inline operator T&() & noexcept { return val; }
  constexpr inline operator const T&() const& noexcept { return val; }
  constexpr inline operator T&&() && noexcept {
    static_assert(std::is_move_assignable_v<T>);
    return std::move(val);
  }
  constexpr inline operator const T&&() const&& noexcept {
    static_assert(std::is_move_assignable_v<T>);
    return std::move(val);
  }

  constexpr operator Option<T>() & noexcept { return Option<T>(std::forward<T>(val)); }
  constexpr operator Option<T>() && noexcept { return Option<T>(std::forward<T>(val)); }

  ~Some() = default;

  T val;
};
// deduce helper
template <typename U>
Some(U) -> Some<U>;

template <typename _Tp, typename _Up>
using __converts_from_option =
    std::__or_<std::is_constructible<_Tp, const Option<_Up>&>, std::is_constructible<_Tp, Option<_Up>&>,
               std::is_constructible<_Tp, const Option<_Up>&&>, std::is_constructible<_Tp, Option<_Up>&&>,
               std::is_convertible<const Option<_Up>&, _Tp>, std::is_convertible<Option<_Up>&, _Tp>,
               std::is_convertible<const Option<_Up>&&, _Tp>, std::is_convertible<Option<_Up>&&, _Tp>>;

template <typename T>
class Option : private std::variant<Some<T>, details::NoneType> {
 private:
  template <typename _Up>
  using __not_self = std::__not_<std::is_same<Option, std::__remove_cvref_t<_Up>>>;
  template <typename _Up>
  using __not_some = std::__not_<std::is_same<Some<T>, std::__remove_cvref_t<_Up>>>;
  template <typename... _Cond>
  using _Requires = std::enable_if_t<std::__and_v<_Cond...>, bool>;

 public:
  // operator ()
  constexpr operator bool() const noexcept { return is_some(); }

  // operator ==
  constexpr bool operator==(const Some<T>& some) const noexcept {
    if (is_some()) {
      return some.val == _m_get_some_value();
    }
    return false;
  }
  constexpr bool operator==(details::NoneType) const noexcept { return is_none(); }

  // operator |
  template <typename U>
  constexpr Option<U> operator|(const Option<U>& rhs) const noexcept {
    return is_some() ? rhs : None;
  }
  template <typename U>
  constexpr Option<U> operator|(Option<U>&& rhs) const noexcept {
    return is_some() ? std::move(rhs) : None;
  }

  // default
  // using std::variant<Some<T>, details::NoneType>::variant;
  // Option() = default;
  constexpr Option() noexcept : std::variant<Some<T>, details::NoneType>(details::NoneType{}) {}

  // copy/move constructor from T
  template <typename U = T,
            _Requires<__not_self<U>, __not_some<U>, std::is_constructible<T, U>, std::is_convertible<U, T>> = true>
  constexpr Option(U&& val) noexcept(std::is_nothrow_constructible_v<T, U>)
      : std::variant<Some<T>, details::NoneType>(Some(std::forward<U>(val))) {}

  template <typename U = T, _Requires<__not_self<U>, __not_some<U>, std::is_constructible<T, U>,
                                      std::__not_<std::is_convertible<U, T>>> = false>
  explicit constexpr Option(U&& val) noexcept(std::is_nothrow_constructible_v<T, U>)
      : std::variant<Some<T>, details::NoneType>(Some(std::forward<U>(val))) {}

  // copy/move constructor form Some<T>
  template <typename U = T,
            _Requires<__not_self<U>, std::__not_<__not_some<U>>, std::is_constructible<T, const U&>,
                      std::is_convertible<const U&, T>, std::__not_<__converts_from_option<T, U>>> = true>
  constexpr Option(const Some<U>& some) noexcept(std::is_nothrow_convertible_v<T, const U&>)
      : std::variant<Some<T>, details::NoneType>(some.val) {}

  template <typename U = T,
            _Requires<__not_self<U>, std::__not_<__not_some<U>>, std::is_constructible<T, const U&>,
                      std::__not_<std::is_convertible<const U&, T>>, std::__not_<__converts_from_option<T, U>>> = false>
  explicit constexpr Option(const Some<U>& some) noexcept(std::is_nothrow_convertible_v<T, const U&>)
      : std::variant<Some<T>, details::NoneType>(some.val) {}

  template <typename U = T, _Requires<__not_self<U>, std::__not_<__not_some<U>>, std::is_constructible<T, U>,
                                      std::is_convertible<U, T>, std::__not_<__converts_from_option<T, U>>> = true>
  constexpr Option(Some<T>&& some) noexcept(std::is_nothrow_convertible_v<T, U>)
      : std::variant<Some<T>, details::NoneType>(std::move(some)) {}

  template <typename U = T,
            _Requires<__not_self<U>, std::__not_<__not_some<U>>, std::is_constructible<T, U>,
                      std::__not_<std::is_convertible<U, T>>, std::__not_<__converts_from_option<T, U>>> = false>
  explicit constexpr Option(Some<T>&& some) noexcept(std::is_nothrow_convertible_v<T, U>)
      : std::variant<Some<T>, details::NoneType>(std::move(some)) {}

  // construct in_place
  template <typename... Args, _Requires<std::is_constructible<T, Args...>> = false>
  explicit constexpr Option(std::in_place_t, Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
      : std::variant<Some<T>, details::NoneType>(std::in_place_index_t<0>{}, std::forward<Args>(args)...) {}

  template <typename U, typename... Args,
            _Requires<std::is_constructible<T, std::initializer_list<U>&, Args...>> = false>
  explicit constexpr Option(std::in_place_t, std::initializer_list<U> list, Args&&... args) noexcept(
      std::is_nothrow_constructible_v<T, std::initializer_list<U>&, Args...>)
      : std::variant<Some<T>, details::NoneType>(std::in_place_index_t<0>{}, list, std::forward<Args>(args)...) {}

  // from NoneType
  constexpr Option(details::NoneType) noexcept : std::variant<Some<T>, details::NoneType>(details::NoneType{}) {}

  // is_some
  constexpr bool is_some() const noexcept { return this->index() == 0; }

  // is_none
  constexpr bool is_none() const noexcept { return this->index() == 1; }

  // is_some_and
  template <typename F>
    requires std::is_invocable_r_v<bool, F, const T&>
  constexpr bool is_some_and(F&& f) const& noexcept(std::is_nothrow_invocable_r_v<bool, F, const T&> &&
                                                    std::is_nothrow_invocable_v<F, const T&>) {
    if (is_some()) {
      return f(_m_get_some_value());
    }
    return false;
  }
  template <typename F>
    requires std::is_invocable_r_v<bool, F, T&&>
  constexpr bool is_some_and(F&& f) && noexcept(std::is_nothrow_invocable_r_v<bool, F, T&&> &&
                                                std::is_nothrow_invocable_v<F, T&&>) {
    if (is_some()) {
      return f(_m_get_some_value());
    }
    return false;
  }

  // is_none_or
  template <typename F>
    requires std::is_invocable_r_v<bool, F, const T&>
  constexpr bool is_none_or(F&& f) const& noexcept(std::is_nothrow_invocable_v<F, const T&>) {
    if (is_some()) {
      return F(_m_get_some_value());
    }
    return true;
  }
  template <typename F>
    requires std::is_invocable_r_v<bool, F, const T&&>
  constexpr bool is_none_or(F&& f) const&& noexcept(std::is_nothrow_invocable_v<F, T&&>) {
    if (is_some()) {
      return F(_m_get_some_value());
    }
    return true;
  }

  // insert
  constexpr auto insert(const T& _val) & noexcept -> std::add_lvalue_reference_t<T> {
    this->template emplace<0>(_val);
    return _m_get_some_value();
  }
  constexpr auto insert(T&& _val) && noexcept -> std::add_rvalue_reference_t<T> {
    this->template emplace<0>(std::forward<T>(_val));
    return std::move(_m_get_some_value());
  }

  // get_or_insert
  constexpr auto get_or_insert(const T& _val) & noexcept -> std::add_lvalue_reference_t<T> {
    if (is_none()) {
      this->template emplace<0>(_val);
    }
    return _m_get_some_value();
  }
  constexpr auto get_or_insert(T&& _val) && noexcept -> std::add_rvalue_reference_t<T> {
    if (is_none()) {
      this->template emplace<0>(_val);
    }
    return std::move(_m_get_some_value());
  }

  // inspect
  template <typename F>
    requires std::is_invocable_r_v<void, F, const T&>
  [[nodiscard]] constexpr Option inspect(F&& f) & noexcept(std::is_nothrow_invocable_v<F, const T&>) {
    if (is_some()) {
      f(_m_get_some_value());
    }
    return *this;
  }
  template <typename F>
    requires std::is_invocable_r_v<void, F, T&&>
  [[nodiscard]] constexpr Option inspect(F&& f) && noexcept(std::is_nothrow_invocable_v<F, T&&>) {
    if (is_some()) {
      f(_m_get_some_value());
    }
    return std::move(*this);
  }

  // replace (usually called `emplace` in stardand libiary)
  constexpr Option replace(const T& _val) const& noexcept {
    this->template emplace<0>(_val);
    return *this;
  }
  constexpr Option replace(const T& _val) & noexcept {
    this->template emplace<0>(_val);
    return *this;
  }
  constexpr Option replace(T&& _val) && noexcept {
    this->template emplace<0>(std::forward<T>(_val));
    return std::move(*this);
  }
  constexpr Option replace(const T&& _val) const&& noexcept {
    this->template emplace<0>(std::forward<T>(_val));
    return std::move(*this);
  }

  // unwrap
  constexpr auto unwrap() & -> std::add_lvalue_reference_t<T> {
    if (is_some()) {
      return _m_get_some_value();
    } else {
      cpptrace::generate_trace(1).print_with_snippets();
      throw option_error("unwrap a none option!");
    }
  }
  constexpr auto unwrap() && -> std::add_rvalue_reference_t<T> {
    if (is_some()) {
      return std::move(_m_get_some_value());
    } else {
      cpptrace::generate_trace(1).print_with_snippets();
      throw option_error("unwrap a none option!");
    }
  }

  // unwrap_or
  constexpr auto unwrap_or(const T& _val) & noexcept -> std::add_lvalue_reference_t<T> {
    return is_some() ? _m_get_some_value() : const_cast<T&>(_val);
  }
  constexpr auto unwrap_or(T&& _val) && noexcept -> std::add_rvalue_reference_t<T> {
    return is_some() ? std::move(_m_get_some_value()) : std::forward<T>(_val);
  }

  // unwrap_or_default
  template <typename U = T>
    requires std::is_default_constructible_v<U>
  constexpr U unwrap_or_default() noexcept(std::is_nothrow_default_constructible_v<T>) {
    return is_some() ? _m_get_some_value() : T();
  }

  // unwrap_or_else
  template <typename F>
    requires std::is_invocable_r_v<T, F>
  constexpr T unwrap_or_else(F&& f) noexcept(std::is_nothrow_invocable_r_v<T, F>) {
    return is_some() ? _m_get_some_value() : f();
  }

  // unwrap_unchecked
  constexpr auto unwrap_unchecked() { return _m_get_some_value(); }

  // template <typename F>
  //   requires std::is_invocable_v<F, const T&>
  // constexpr auto and_then(F&& f) const& noexcept(std::is_nothrow_invocable_v<F, const T&>) {
  //   return is_some() ? f(_m_get_some_value()) : None;
  // }
  // template <typename F>
  //   requires std::is_invocable_v<F, T&&>
  // constexpr auto and_then(F&& f) && noexcept(std::is_nothrow_invocable_v<F, T&&>) {
  //   return is_some() ? f(_m_get_some_value()) : None;
  // }

  // expected
  constexpr auto expected(const char* msg) & -> std::add_lvalue_reference_t<T> {
    if (is_some()) {
      return _m_get_some_value();
    }
    cpptrace::generate_trace(1).print_with_snippets();
    throw option_error(msg);
  }

  // map
  template <typename F>
    requires std::is_invocable_v<F, const T&>
  constexpr auto map(F&& f) const& noexcept(std::is_nothrow_invocable_v<F, const T&>) {
    return is_some() ? f(_m_get_some_value()) : None;
  }
  template <typename F>
    requires std::is_invocable_v<F, T&&>
  constexpr auto map(F&& f) && noexcept(std::is_nothrow_invocable_v<F, T&&>) {
    return is_some() ? f(_m_get_some_value()) : None;
  }

  // map_or
  template <typename F, typename U = std::invoke_result_t<F, const T&>>
  constexpr U map_or(F&& f, const U& _default) const& noexcept(std::is_nothrow_invocable_v<F, const T&>) {
    return is_some() ? f(_m_get_some_value()) : _default;
  }
  template <typename F, typename U = std::invoke_result_t<F, T&&>>
  constexpr U map_or(F&& f, U&& _default) && noexcept(std::is_nothrow_invocable_v<F, T&&>) {
    return is_some() ? f(std::move(_m_get_some_value())) : std::move(_default);
  }

  // map_or_else
  template <typename D, typename F, typename U = std::invoke_result_t<D>>
    requires std::is_same_v<U, std::invoke_result_t<F, const T&>>
  constexpr U map_or_else(D&& _default, F&& f) const& noexcept(std::is_nothrow_invocable_v<F, const T&> &&
                                                               std::is_nothrow_invocable_v<D>) {
    return is_some() ? f(_m_get_some_value()) : _default();
  }
  template <typename D, typename F, typename U = std::invoke_result_t<D>>
    requires std::is_same_v<U, std::invoke_result_t<F, T&&>>
  constexpr U map_or_else(D&& _default,
                          F&& f) && noexcept(std::is_nothrow_invocable_v<F, T&&> && std::is_nothrow_invocable_v<D>) {
    return is_some() ? f(std::move(_m_get_some_value())) : _default();
  }

  // as_ref
  constexpr Option<std::reference_wrapper<T>> as_ref() & noexcept {
    using rt_type = Option<std::reference_wrapper<T>>;
    return is_some() ? rt_type{Some(ref())} : Option<std::reference_wrapper<T>>{};
  }
  constexpr Option<std::reference_wrapper<T>> as_ref() const& noexcept {
    using rt_type = Option<std::reference_wrapper<T>>;
    return is_some() ? rt_type{Some(ref())} : Option<std::reference_wrapper<T>>{};
  }

  // todo list
  // ok_or
  // ok_or_else
  // or
  // or_else

 private:
  // unchecked get reference_wrapper
  constexpr std::reference_wrapper<T> ref() & noexcept { return std::ref(std::get<Some<T>>(*this).val); }
  constexpr std::reference_wrapper<T> ref() const& noexcept { return std::cref(std::get<Some<T>>(*this).val); }
  // unchecked get value
  constexpr inline const T& _m_get_some_value() const& { return std::get<Some<T>>(*this).val; }
  constexpr inline T& _m_get_some_value() & { return std::get<Some<T>>(*this).val; }
  constexpr inline T&& _m_get_some_value() && { return std::move(std::get<Some<T>>(*this)).val; }
  constexpr inline const T&& _m_get_some_value() const&& { return std::move(std::get<Some<T>>(*this)).val; }
};

}  // namespace navp
