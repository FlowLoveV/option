#include <cassert>
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include <optional>

#include "doctest.h"
#include "option.hpp"

using navp::None;
using navp::Option;
using navp::Some;

// from [https://github.com/TartanLlama/optional/tree/master/tests]
TEST_CASE("Triviality") {
  static_assert(!std::is_trivially_constructible_v<Option<int>>);
  static_assert(std::is_trivially_copy_constructible<Option<int>>::value);
  static_assert(std::is_trivially_copy_assignable<Option<int>>::value);
  static_assert(std::is_trivially_move_constructible<Option<int>>::value);
  static_assert(std::is_trivially_move_assignable<Option<int>>::value);
  static_assert(std::is_trivially_destructible<Option<int>>::value);

  {
    struct T {
      T(const T&) = default;
      T(T&&) = default;
      T& operator=(const T&) = default;
      T& operator=(T&&) = default;
      ~T() = default;
    };
    static_assert(std::is_trivially_copy_constructible<Option<T>>::value);
    static_assert(std::is_trivially_copy_assignable<Option<T>>::value);
    static_assert(std::is_trivially_move_constructible<Option<T>>::value);
    static_assert(std::is_trivially_move_assignable<Option<T>>::value);
    static_assert(std::is_trivially_destructible<Option<T>>::value);
  }

  {
    struct T {
      T(const T&) {}
      T(T&&){};
      T& operator=(const T&) { return *this; }
      T& operator=(T&&) { return *this; };
      ~T() {}
    };
    static_assert(!std::is_trivially_copy_constructible<Option<T>>::value);
    static_assert(!std::is_trivially_copy_assignable<Option<T>>::value);
    static_assert(!std::is_trivially_move_constructible<Option<T>>::value);
    static_assert(!std::is_trivially_move_assignable<Option<T>>::value);
    static_assert(!std::is_trivially_destructible<Option<T>>::value);
  }
}

TEST_CASE("Size") {
  static_assert(sizeof(Option<double>) == 16);
  static_assert(sizeof(std::optional<double>) == 16);
  struct T1 {
    double f[4];
  };
  static_assert(sizeof(Option<T1>) == 40);
  static_assert(sizeof(std::optional<T1>) == 40);
  struct ComplexT {
    char a[7];
    int b[3];
    float c[6];
    double d[9];
    std::string e[3];
  };
  static_assert(sizeof(Option<ComplexT>) == sizeof(ComplexT) + alignof(ComplexT));
  static_assert(sizeof(Option<ComplexT>) == sizeof(std::optional<ComplexT>));
}

// from [https://github.com/TartanLlama/optional/tree/master/tests]
TEST_CASE("Deletion") {
  static_assert(std::is_copy_constructible<Option<int>>::value);
  static_assert(std::is_copy_assignable<Option<int>>::value);
  static_assert(std::is_move_constructible<Option<int>>::value);
  static_assert(std::is_move_assignable<Option<int>>::value);
  static_assert(std::is_destructible<Option<int>>::value);

  {
    struct T {
      T(const T&) = default;
      T(T&&) = default;
      T& operator=(const T&) = default;
      T& operator=(T&&) = default;
      ~T() = default;
    };
    static_assert(std::is_copy_constructible<Option<T>>::value);
    static_assert(std::is_copy_assignable<Option<T>>::value);
    static_assert(std::is_move_constructible<Option<T>>::value);
    static_assert(std::is_move_assignable<Option<T>>::value);
    static_assert(std::is_destructible<Option<T>>::value);
  }

  {
    struct T {
      T(const T&) = delete;
      T(T&&) = delete;
      T& operator=(const T&) = delete;
      T& operator=(T&&) = delete;
    };
    static_assert(!std::is_copy_constructible<Option<T>>::value);
    static_assert(!std::is_copy_assignable<Option<T>>::value);
    static_assert(!std::is_move_constructible<Option<T>>::value);
    static_assert(!std::is_move_assignable<Option<T>>::value);
  }

  {
    struct T {
      T(const T&) = delete;
      T(T&&) = default;
      T& operator=(const T&) = delete;
      T& operator=(T&&) = default;
    };
    static_assert(!std::is_copy_constructible<Option<T>>::value);
    static_assert(!std::is_copy_assignable<Option<T>>::value);
    static_assert(std::is_move_constructible<Option<T>>::value);
    static_assert(std::is_move_assignable<Option<T>>::value);
  }

  {
    struct T {
      T(const T&) = default;
      T(T&&) = delete;
      T& operator=(const T&) = default;
      T& operator=(T&&) = delete;
    };
    static_assert(std::is_copy_constructible<Option<T>>::value);
    static_assert(std::is_copy_assignable<Option<T>>::value);
  }
}

// from [https://github.com/TartanLlama/optional/tree/master/tests]
TEST_CASE("Constexpr") {
  constexpr Option<int> o2{};
  constexpr Option<int> o3 = {};
  constexpr Option<int> o4 = None;
  constexpr Option<int> o5 = {None};
  constexpr Option<int> o6(None);

  CHECK(!o2);
  CHECK(!o3);
  CHECK(!o4);
  CHECK(!o5);
  CHECK(!o6);
}

// from [https://github.com/TartanLlama/optional/tree/master/tests]
TEST_CASE("Value Construct") {
  constexpr Option<int> o1 = 42;
  constexpr Option<int> o2{42};
  constexpr Option<int> o3(42);
  constexpr Option<int> o4 = {42};
  constexpr int i = 42;
  constexpr Option<int> o5 = std::move(i);
  constexpr Option<int> o6{std::move(i)};
  constexpr Option<int> o7(std::move(i));
  constexpr Option<int> o8 = {std::move(i)};

  CHECK(o1.unwrap() == 42);
  CHECK(o2.unwrap() == 42);
  CHECK(o3.unwrap() == 42);
  CHECK(o4.unwrap() == 42);
  CHECK(o5.unwrap() == 42);
  CHECK(o6.unwrap() == 42);
  CHECK(o7.unwrap() == 42);
  CHECK(o8.unwrap() == 42);
}

// from [https://github.com/TartanLlama/optional/tree/master/tests]
TEST_CASE("Constructor") {
  using T = std::vector<double>;
  Option<T> op1 = Some(std::vector<double>{1.0, 2.0});
  Option<T> op2 = None;
  Option<T> op3{std::in_place, {1.0, 2.0}};

  struct U {
    int a;
    float b;
    double c;
  };
  Option<U> op4(std::in_place, U{10, 10.0, 11.0});

  struct foo {
    foo() = default;
    foo(foo&) = delete;
    foo(foo&&){};
  };

  Option<int> o1;
  CHECK(!o1);

  Option<int> o2 = None;
  CHECK(!o2);

  Option<int> o3 = 42;
  CHECK(o3.unwrap() == 42);

  Option<int> o4 = o3;
  CHECK(o4.unwrap() == 42);

  Option<int> o5 = o1;
  CHECK(!o5);

  Option<int> o6 = std::move(o3);
  CHECK(o6.unwrap() == 42);

  Option<short> o7 = 42;
  CHECK(o7.unwrap() == 42);

  Option<int> o8 = o7;
  // CHECK(o8.unwrap() == 42);

  Option<int> o9 = std::move(o7);
  CHECK(o9.unwrap() == 42);

  {
    Option<int&> o;
    CHECK(!o);

    Option<int&> oo = o;
    CHECK(!oo);
  }

  {
    auto i = 42;
    Option<std::reference_wrapper<int>> o = i;
    CHECK(o.is_some());
    auto m = o.unwrap();
    auto val = m.get();
    CHECK(o.unwrap().get() == 42);

    Option<std::reference_wrapper<int>> oo = o;
    CHECK(oo);
    CHECK(oo.unwrap().get() == 42);
  }

  std::vector<foo> v;
  v.emplace_back();
  Option<std::vector<foo>> ov = std::move(v);
  CHECK(ov.unwrap().size() == 1);
}

// is_some(),is_none(),is_some_and(),is_none_or()
// unwrap(),unwrap_or(),unwrap_or_else(),unwrap_or_default()
// inspect(),expected()
TEST_CASE("Watcher") {
  auto o1 = Some(1.0);
  CHECK(o1.is_some());
  CHECK(!o1.is_none());
  auto f1 = [](const double& d) { return d > 0.0; };
  CHECK(o1.is_some_and(f1));
  CHECK(o1.is_none_or(f1));
  o1 = None;
  CHECK(!o1.is_some_and(f1));
  CHECK(o1.is_none_or(f1));

  auto o2 = Some(std::vector<int>{1, 2, 3, 4, 5});
  auto& ref_o2 = o2.unwrap();
  const auto& cref_o2 = o2.unwrap();
  auto&& rval_o2 = std::move(o2).unwrap();
  // const auto&& rval_o2 = std::move(o2).unwrap();

  auto o3 = Some(std::vector<int>{1, 2, 3, 4, 5, 6});
  CHECK(o3.unwrap().size() == 6);
  o3 = None;
  auto& v1 = o3.unwrap_or(rval_o2);
  CHECK(v1.size() == 5);
  o3 = None;
  auto v2 = o3.unwrap_or_default();
  CHECK(v2.size() == 0);
  auto make_vec = []() { return std::vector<int>{1, 2, 3}; };
  o3 = None;
  auto v3 = o3.unwrap_or_else(make_vec);
  CHECK(v3.size() == 3);
  o3 = None;
  CHECK_THROWS(o3.unwrap_unchecked());

  auto o4 = Some<std::vector<int>>({1, 2, 3, 4});
  auto& ref_o4 = o4.inspect([](const std::vector<int>& vec) { CHECK(vec.size() == 4); });

  Option<int> o5 = None;
  CHECK_THROWS(o5.expected("unwrap a none option"));
}

// from [https://github.com/TartanLlama/optional/tree/master/tests]
// replace()
TEST_CASE("Replace") {
  Option<std::pair<std::pair<int, int>, std::pair<double, double>>> i;
  auto& p = i.replace(std::pair(0, 2), std::pair(3, 4));
  REQUIRE(p.first.first == 0);
  REQUIRE(p.first.second == 2);
  REQUIRE(p.second.first == 3);
  REQUIRE(p.second.second == 4);

  struct A {
    A() { throw std::exception(); }
  };

  Option<A> a;
  REQUIRE_THROWS(a.replace());
}

// insert(), get_or_insert()
TEST_CASE("Insert") {
  auto o1 = Some<std::vector<double>>({1.0, 2.0, 3.0});
  o1 = o1.insert({2.0, 3.0, 4.0, 5.0, 6.0});
  CHECK(o1.unwrap().size() == 5);

  Option<std::vector<double>> o2 = None;
  const auto& vec = o2.get_or_insert({1.0, 2.0, 3.0, 4.0});
  CHECK(o2.unwrap().size() == 4);
}

// map(),map_or(),map_or_default(),map_or_else()
TEST_CASE("Map") {
  auto o1 = Some<std::string>("Hello Option!");
  auto get_size1 = [](const std::string& str) { return Some(str.size()); };
  auto s1 = o1.map(get_size1);
  CHECK(s1.unwrap() == 13);

  Option<std::string> o2 = None;
  auto get_size2 = [](const std::string& str) {return str.size();};
  auto s2 = o2.map_or(get_size2,9);
  CHECK(sizeof s2 == 8);
  CHECK(s2 == 9);

  Option<std::string> o3 = None;
  auto default_size = []() -> unsigned long {return 10;};
  auto get_size3  = [](const std::string& str) {return str.size();};
  auto s3 = o3.map_or_else(default_size, get_size3);
  CHECK(s3 == 10);
  o3.replace("Goodbye rust!");
  auto s4 = o3.map_or_else(default_size, get_size3);
  CHECK(s4 == 13);
}

// todo
// Member function testing
// unwrap_or_else()
// inspect()
// map() map_or() map_or_else()
TEST_CASE("Member Function Test") {
  struct T {
    std::string str;
    double num;


  };
}

// swap
TEST_CASE("swap") {
  using T = Option<std::string>;
  T o1("Hello World!");
  T o2("Goodbye Rust!");
  std::swap(o1,o2);
  CHECK(o1.unwrap() == "Goodbye Rust!");
  CHECK(o2.unwrap() == "Hello World!");
}


// as_ref
TEST_CASE("Ref") {
  auto o1 = Some<std::string>("Hello C++23!");
  auto ref = o1.as_ref();
  static_assert(std::is_same_v<std::remove_cvref_t<decltype(ref.unwrap())>, std::reference_wrapper<std::string>>);
}
