/*
 * @Brief: 
 * @Author: Baozhe ZHANG 
 * @Date: 2024-03-29 12:39:59 
 * @Last Modified by: Baozhe ZHANG
 * @Last Modified time: 2024-04-18 14:58:12
 */

#pragma once

#include "dsm/type.h"

#include <vector>
#include <string>
#include <gsl-lite.hpp>
#include <atomic>

namespace dsm {

using Byte = char;

class Manager;

/**
 * @brief Object may be served as a parent class 
 * 
 */
class Object {
 public:
  Object() : m_name(nullptr), m_manager_ptr(nullptr) {}
  Object(std::string name, Manager *manager_ptr) : 
    m_name(name), m_manager_ptr(manager_ptr) {}
  

  // Explicitly disable copy constructor and copy assignment operator
  // to avoid implicitly registering the object in `Manager`.
  // Users should use `Manager::mmap` to create an object
  Object(const Object &) = delete;
  Object &operator=(const Object &) = delete;

  // However uses can transfer the ownership of the object
  // this does not affect the underlying manager's database
  Object(Object &&);
  Object &operator=(Object &&) noexcept;

  virtual ~Object() = default;
  
  std::string get_name() const { return m_name; };


 protected: 
  gsl::span<Byte> read(size_t offset, size_t length) const;

  void write(size_t offset, gsl::span<Byte> data);

 private: 
  std::string m_name;
  // `data` is stored in local memory in `std::vector`
  // with STL's allocator. It does not care about what 
  // to be stored in the memory. All represented as `char`.
  std::vector<Byte> m_data;

  friend Manager; 
  // Object's read & write operations are notified to `Manager`
  Manager *m_manager_ptr;
}; // class Object


template <typename T>
class Element final : public Object {
  static_assert(std::is_floating_point<T>::value || std::is_integral<T>::value,
                "T must be a floating point or integral type");

 public: 
  using value_type = T;

  Element() = delete;

  /**
   * @brief Explicitly construct a new Element object
   * from a rvalue. Copy constructor is deleted because 
   * of avoiding implicitly copy the underlying managed 
   * object and hence another allocating memory for all
   * peers. 
   * Memory allocation should be done by the user explicitly.
   * 
   * @param rhs 
   */
  explicit Element(Object &&rhs) : Object(std::move(rhs)) {
    *this = static_cast<T>(0);
  }

  ~Element() override = default;

  Element(const Element &) = delete;
  Element &operator=(const Element &) = delete;
  Element(Element &&) = delete;
  Element &operator=(Element &&) = delete;

  Element &operator=(T t) {
    std::array<Byte, sizeof(T)> tmp;
    const Byte *byte_ptr = reinterpret_cast<const Byte *>(&t);
    for (size_t i = 0; i < tmp.size(); i++) {
      tmp[i] = byte_ptr[i];
    }
    write(0, tmp);
    return *this;
  }

  template <typename U>
  Element &operator+=(U t) {
    static_assert(std::is_floating_point<U>::value || std::is_integral<U>::value,
                  "U must be a floating point or integral type");
    T tmp = static_cast<T>(*this) + t;
    *this = tmp;
    return *this;
  }

  template <typename U>
  Element &operator-=(U t) {
    static_assert(std::is_floating_point<U>::value || std::is_integral<U>::value,
                  "U must be a floating point or integral type");
    T tmp = static_cast<T>(*this) - t;
    *this = tmp;
    return *this;
  }

  template <typename U>
  Element &operator*=(U t) {
    static_assert(std::is_floating_point<U>::value || std::is_integral<U>::value,
                  "U must be a floating point or integral type");
    T tmp = static_cast<T>(*this) * t;
    *this = tmp;
    return *this;
  }

  template <typename U>
  Element &operator/=(U t) {
    static_assert(std::is_floating_point<U>::value || std::is_integral<U>::value,
                  "U must be a floating point or integral type");
    T tmp = static_cast<T>(*this) / t;
    *this = tmp;
    return *this;
  }

  template <typename U>
  Element &operator%=(U t) {
    static_assert(std::is_integral<T>::value && std::is_integral<U>::value,
                  "T and U must be integral types");
    T tmp = static_cast<T>(*this) % t;
    *this = tmp;
    return *this;
  }

  template <typename U>
  T operator+(U t) {
    static_assert(std::is_same<U, Element<int32_t>>::value || 
                  std::is_same<U, Element<uint32_t>>::value || 
                  std::is_same<U, Element<int64_t>>::value || 
                  std::is_same<U, Element<uint64_t>>::value || 
                  std::is_same<U, Element<float>>::value || 
                  std::is_same<U, Element<double>>::value || 
                  std::is_integral<U>::value || std::is_floating_point<U>::value,
                  "U must be an Element type or an integral or floating point type");
    return static_cast<T>(*this) + t;
  }

  template <typename U>
  T operator-(U t) {
    static_assert(std::is_same<U, Element<int32_t>>::value || 
                  std::is_same<U, Element<uint32_t>>::value || 
                  std::is_same<U, Element<int64_t>>::value || 
                  std::is_same<U, Element<uint64_t>>::value || 
                  std::is_same<U, Element<float>>::value || 
                  std::is_same<U, Element<double>>::value || 
                  std::is_integral<U>::value || std::is_floating_point<U>::value,
                  "U must be an Element type or an integral or floating point type");
    return static_cast<T>(*this) - t;
  }

  T operator-() {
    return -static_cast<T>(*this);
  }

  template <typename U>
  T operator*(U t) {
    static_assert(std::is_same<U, Element<int32_t>>::value || 
                  std::is_same<U, Element<uint32_t>>::value || 
                  std::is_same<U, Element<int64_t>>::value || 
                  std::is_same<U, Element<uint64_t>>::value || 
                  std::is_same<U, Element<float>>::value || 
                  std::is_same<U, Element<double>>::value || 
                  std::is_integral<U>::value || std::is_floating_point<U>::value,
                  "U must be an Element type or an integral or floating point type");
    return static_cast<T>(*this) * t;
  }

  template <typename U>
  T operator/(U t) {
    static_assert(std::is_same<U, Element<int32_t>>::value || 
                  std::is_same<U, Element<uint32_t>>::value || 
                  std::is_same<U, Element<int64_t>>::value || 
                  std::is_same<U, Element<uint64_t>>::value || 
                  std::is_same<U, Element<float>>::value || 
                  std::is_same<U, Element<double>>::value || 
                  std::is_integral<U>::value || std::is_floating_point<U>::value,
                  "U must be an Element type or an integral or floating point type");
    return static_cast<T>(*this) / t;
  }

  template <typename U>
  T operator%(U t) {
    static_assert(std::is_integral<U>::value || 
                  std::is_same<U, Element<int32_t>>::value ||
                  std::is_same<U, Element<uint32_t>>::value ||
                  std::is_same<U, Element<int64_t>>::value ||
                  std::is_same<U, Element<uint64_t>>::value, 
                  "T and U must be integral types");
    return static_cast<T>(*this) % t;
  }

  operator T() {
    auto tmp = read(0, sizeof(T));
    T t = 0;
    Byte *byte_ptr = reinterpret_cast<Byte *>(&t);
    for (size_t i = 0; i < tmp.size(); i++) {
      byte_ptr[i] = tmp[i];
    }
    return t;
  }

  operator T() const {
    auto tmp = read(0, sizeof(T));
    T t = 0;
    Byte *byte_ptr = reinterpret_cast<Byte *>(&t);
    for (size_t i = 0; i < tmp.size(); i++) {
      byte_ptr[i] = tmp[i];
    }
    return t;
  }
}; // class Element

template <>
class Element<char> final : public Object {
 public: 
  Element() = delete;

  explicit Element(Object &&rhs) : Object(std::move(rhs)) {}

  ~Element() override = default;

  Element(const Element &) = delete;
  Element &operator=(const Element &) = delete;
  Element(Element &&) = delete;
  Element &operator=(Element &&) = delete;

  Element &operator=(char c) {
    write(0, {c});
    return *this;
  }

  Element &operator+=(char c) {
    char tmp = static_cast<char>(*this) + c;
    *this = tmp;
    return *this;
  }

  Element &operator-=(char c) {
    char tmp = static_cast<char>(*this) - c;
    *this = tmp;
    return *this;
  }

  char operator+(char c) {
    return static_cast<char>(*this) + c;
  }

  char operator-(char c) {
    return static_cast<char>(*this) - c;
  }

  char operator-() {
    return -static_cast<char>(*this);
  }

  operator char() {
    auto tmp = read(0, 1);
    return tmp[0];
  }
}; 

template <>
class Element<bool> final : public Object {
 public: 
  Element() = delete;

  explicit Element(Object &&rhs) : Object(std::move(rhs)) {}

  ~Element() override = default;

  Element(const Element &) = delete;
  Element &operator=(const Element &) = delete;
  Element(Element &&) = delete;
  Element &operator=(Element &&) = delete;

  Element &operator=(bool b) {
    Byte tmp = b ? 1 : 0;
    write(0, {tmp});
    return *this;
  }

  Element &operator&=(bool b) {
    bool tmp = static_cast<bool>(*this) & b;
    *this = tmp;
    return *this;
  }

  Element &operator|=(bool b) {
    bool tmp = static_cast<bool>(*this) | b;
    *this = tmp;
    return *this;
  }

  Element &operator^=(bool b) {
    bool tmp = static_cast<bool>(*this) ^ b;
    *this = tmp;
    return *this;
  }

  bool operator&(bool b) {
    return static_cast<bool>(*this) & b;
  }

  bool operator|(bool b) {
    return static_cast<bool>(*this) | b;
  }

  bool operator^(bool b) {
    return static_cast<bool>(*this) ^ b;
  }

  bool operator!() {
    return !static_cast<bool>(*this);
  }

  operator bool() {
    auto tmp = read(0, 1);
    return tmp[0];
  }
};



using Int32 = Element<int32_t>;
using UInt32 = Element<uint32_t>;
using Int64 = Element<int64_t>;
using UInt64 = Element<uint64_t>;
using Float = Element<float>;
using Double = Element<double>;
using Char = Element<char>;
using Bool = Element<bool>;




template <typename T, size_t N>
class ArrayCompact final : public Object {
  static_assert(std::is_floating_point<T>::value || 
                std::is_integral<T>::value ||
                std::is_same<T, Char>::value ||
                "T must be a floating point or integral or char type");

 public: 
  ArrayCompact() = delete;

  explicit ArrayCompact(Object &&rhs) : Object(std::move(rhs)) {}

  ~ArrayCompact() override = default;

  ArrayCompact(const ArrayCompact &) = delete;
  ArrayCompact &operator=(const ArrayCompact &) = delete;
  ArrayCompact(ArrayCompact &&) = delete;
  ArrayCompact &operator=(ArrayCompact &&) = delete;

  T operator[](size_t i) {
    if (i >= N) {
      throw std::out_of_range("Index out of range");
    }
    gsl::span<Byte> tmp = read(i * sizeof(T), sizeof(T));
    T t;
    Byte *byte_ptr = reinterpret_cast<Byte *>(&t);
    for (size_t j = 0; j < tmp.size(); j++) {
      byte_ptr[j] = tmp[j];
    }
    return t;
  }

  ArrayCompact &operator=(const std::array<T, N> &arr) {
    write(0, {arr.begin(), arr.end()});
    return *this;
  }

  ArrayCompact &set(size_t i, T t) {
    if (i >= N) {
      throw std::out_of_range("Index out of range");
    }
    std::array<Byte, sizeof(T)> tmp;
    const Byte *byte_ptr = reinterpret_cast<const Byte *>(&t);
    for (size_t j = 0; j < tmp.size(); j++) {
      tmp[j] = byte_ptr[j];
    }
    write(i * sizeof(T), tmp);
    return *this;
  }

}; // class ArrayCompact

template <typename T, size_t N>
class Array final {
  static_assert(std::is_same<T, Int32>::value || 
                std::is_same<T, UInt32>::value || 
                std::is_same<T, Int64>::value || 
                std::is_same<T, UInt64>::value || 
                std::is_same<T, Float>::value || 
                std::is_same<T, Double>::value || 
                std::is_same<T, Char>::value || 
                std::is_same<T, Bool>::value || 
                "T must be an Element type ");
  using value_type = T;
 public: 
  Array() = delete;
  explicit Array(gsl::span<Object *> data) {
    if (data.size() != N) {
      throw std::invalid_argument("Data size does not match");
    }
    for (size_t i = 0; i < N; i++) {
      m_data[i] = static_cast<T*>(data[i]);
      m_names[i] = data[i]->get_name();
    }
  }

  Array(const Array &) = delete;
  Array(Array &&) = delete;
  Array &operator=(const Array &) = delete;
  Array &operator=(Array &&) = delete;
  
  ~Array() = default;

  T &operator[](size_t i) {
    if (i >= N) {
      throw std::out_of_range("Index out of range");
    }
    return *m_data[i];
  }

  const T &operator[](size_t i) const {
    if (i >= N) {
      throw std::out_of_range("Index out of range");
    }
    return *m_data[i];
  }

  const std::string &name(size_t i) const {
    if (i >= N) {
      throw std::out_of_range("Index out of range");
    }
    return m_names[i];
  }

  const std::array<std::string, N> &names() const {
    return m_names;
  }

 private:
  std::array<T*, N> m_data;
  std::array<std::string, N> m_names;
}; // class Array

using Vector2F = Array<Float, 2>;
using Vector3F = Array<Float, 3>;
using Vector4F = Array<Float, 4>;
using Vector2D = Array<Double, 2>;
using Vector3D = Array<Double, 3>;
using Vector4D = Array<Double, 4>;

template <typename T, size_t M, size_t N>
class Matrix final {
  static_assert(std::is_same<T, Int32>::value || 
                std::is_same<T, UInt32>::value || 
                std::is_same<T, Int64>::value || 
                std::is_same<T, UInt64>::value || 
                std::is_same<T, Float>::value || 
                std::is_same<T, Double>::value || 
                std::is_same<T, Char>::value || 
                std::is_same<T, Bool>::value || 
                "T must be an Element type");
  using value_type = T;
 public: 
  Matrix() = delete;
  explicit Matrix(gsl::span<Object *> data) {
    if (data.size() != M * N) {
      throw std::invalid_argument("Data size does not match");
    }
    for (size_t i = 0; i < M; i++) {
      for (size_t j = 0; j < N; j++) {
        m_data[i][j] = static_cast<T*>(data[i * N + j]);
        m_data[i][j] = data[i * N + j]->get_name();
      }
    }
  }

  ~Matrix() = default;

  Matrix(const Matrix &) = delete;
  Matrix(Matrix &&) = delete;
  Matrix &operator=(const Matrix &) = delete;
  Matrix &operator=(Matrix &&) = delete;

  Array<T, N> &operator[](size_t i) {
    if (i >= M) {
      throw std::out_of_range("Index out of range");
    }
    return m_data[i];
  }

  const Array<T, N> &operator[](size_t i) const {
    if (i >= M) {
      throw std::out_of_range("Index out of range");
    }
    return m_data[i];
  }

  T &operator()(size_t i, size_t j) {
    if (i >= M || j >= N) {
      throw std::out_of_range("Index out of range");
    }
    return *m_data[i][j];
  }

  const T &operator()(size_t i, size_t j) const {
    if (i >= M || j >= N) {
      throw std::out_of_range("Index out of range");
    }
    return *m_data[i][j];
  }

  const std::array<std::array<std::string, N>, M> &names() const {
    return m_names;
  }

  const std::array<std::string, N> &names(size_t i) const {
    if (i >= M) {
      throw std::out_of_range("Index out of range");
    }
    return m_names[i];
  }

  const std::string &name(size_t i, size_t j) const {
    if (i >= M || j >= N) {
      throw std::out_of_range("Index out of range");
    }
    return m_names[i][j];
  }

 private:
  std::array<std::array<T*, N>, M> m_data;
  std::array<std::array<std::string, N>, M> m_names;
}; // class Matrix


using Matrix2F = Matrix<Float, 2, 2>;
using Matrix3F = Matrix<Float, 3, 3>;
using Matrix4F = Matrix<Float, 4, 4>;
using Matrix2D = Matrix<Double, 2, 2>;
using Matrix3D = Matrix<Double, 3, 3>;
using Matrix4D = Matrix<Double, 4, 4>;


} // namespace dsm

template <typename T>
std::ostream &operator<<(std::ostream &os, const dsm::Element<T> &e) {
  os << static_cast<T>(e);
  return os;
}
