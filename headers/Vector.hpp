#pragma once

#include <memory>

template <class T, class A = std::allocator<T>>
struct Vector_base {
  A alloc;
  T* elem;
  T* space;
  T* end;

  Vector_base(const A& a, typename A::size_type n)
      : alloc {a}, elem {alloc.allocate(n)}, space {elem + n}, end {elem + n} {}
  ~Vector_base() { alloc.deallocate(elem, end - elem); }

  Vector_base(const Vector_base&) = delete;
  Vector_base& operator=(const Vector_base&) = delete;

  Vector_base(Vector_base&& a) : alloc(std::move(a.alloc)), elem(a.elem), space(a.space), end(a.end) {
    a.elem = a.space = a.end = nullptr;
  }
  Vector_base& operator=(Vector_base&& other) {
    alloc = std::move(other.alloc);
    std::swap(elem, other.elem);
    std::swap(space, other.space);
    std::swap(end, other.end);
    return *this;
  }
};

template <class T, class A = std::allocator<T>>
class Vector {
 private:
  Vector_base<T, A> vb;
  void destroy_elements();

 public:
  explicit Vector(typename A::size_type n, const T& value = T(), const A& alloc = A());

  Vector(const Vector& other);
  Vector& operator=(const Vector& other);

  Vector(Vector&& other) noexcept;
  Vector& operator=(Vector&& other) noexcept;

  ~Vector() { destroy_elements(); }

  typename A::size_type size() const { return vb.space - vb.elem; }
  typename A::size_type capacity() const { return vb.end - vb.elem; }

  T& operator[](typename A::size_type i) { return vb.elem[i]; }
  const T& operator[](typename A::size_type i) const { return vb.elem[i]; }

  using iterator = T*;
  using const_iterator = const T*;

  iterator begin() { return vb.elem; }
  const_iterator begin() const { return vb.elem; }
  iterator end() { return vb.space; }
  const_iterator end() const { return vb.space; }

  void reserve(typename A::size_type new_capacity);

  void resize(typename A::size_type new_size, T = {});
  void clear() { resize(0); }

  void push_back(const T& value);
  void pop_back();
};

template <class T, class A>
void Vector<T, A>::destroy_elements() {
  for (T* p = vb.elem; p != vb.space; ++p) {
    p->~T();
  }
  vb.space = vb.elem;
}

template <class T, class A>
Vector<T, A>::Vector(typename A::size_type n, const T& value, const A& alloc) : vb {alloc, n} {
  std::uninitialized_fill(vb.elem, vb.space, value);
}

template <class T, class A>
Vector<T, A>::Vector(const Vector<T, A>& a) : vb {a.vb.alloc, a.size()} {
  std::uninitialized_copy(a.begin(), a.end(), vb.elem);
}

template <class T, class A>
Vector<T, A>::Vector(Vector&& a) noexcept : vb {a.vb.alloc, 0} {
  vb.elem = a.vb.elem;
  vb.space = a.vb.space;
  vb.end = a.vb.end;
  a.vb.elem = a.vb.space = a.vb.end = nullptr;
}

template <class T, class A>
Vector<T, A>& Vector<T, A>::operator=(Vector&& a) noexcept {
  if (this != &a) {
    clear();
    std::swap(vb, a.vb);
  }
  return *this;
}

template <class T, class A>
Vector<T, A>& Vector<T, A>::operator=(const Vector& a) {
  if (capacity() < a.size()) {
    Vector temp {a};
    std::swap(*this, temp);
    return *this;
  }
  if (this == &a) return *this;

  typename A::size_type sz = size();
  typename A::size_type asz = a.size();
  vb.alloc = a.vb.alloc;
  if (asz <= sz) {
    std::copy(a.begin(), a.end(), vb.elem);
    for (T* p = vb.elem + asz; p != vb.space; ++p) {
      p->~T();
    }
  } else {
    std::copy(a.begin(), a.begin() + sz, vb.elem);
    std::uninitialized_copy(a.begin() + sz, a.end(), vb.elem + sz);
  }
  vb.space = vb.elem + asz;
  return *this;
}

template <class T, class A>
void Vector<T, A>::reserve(typename A::size_type new_capacity) {
  if (new_capacity <= capacity()) return;

  Vector_base<T, A> new_vb {vb.alloc, new_capacity};
  new_vb.space = new_vb.elem + size();
  std::uninitialized_move(vb.elem, vb.space, new_vb.elem);
  destroy_elements();
  std::swap(vb, new_vb);
}

template <class T, class A>
void Vector<T, A>::resize(typename A::size_type new_size, T value) {
  reserve(new_size);
  if (size() < new_size) {
    std::uninitialized_fill(vb.elem + size(), vb.elem + new_size, value);
  } else {
    std::destroy(vb.elem + new_size, vb.space);
  }
  vb.space = vb.elem + new_size;
}

template <class T, class A>
void Vector<T, A>::push_back(const T& value) {
  if (capacity() == size()) {
    reserve(size() ? 2 * size() : 8);
  }
  std::allocator_traits<A>::construct(vb.alloc, &vb.elem[size()], value);
  ++vb.space;
}

template <class T, class A>
void Vector<T, A>::pop_back() {
  if (size() > 0) {
    --vb.space;
    std::allocator_traits<A>::destroy(vb.alloc, vb.space);
  }
}
