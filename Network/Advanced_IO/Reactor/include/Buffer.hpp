#include <cstring>
#include <limits>
#include <stdexcept>
#include <vector>

// 以vector<char>为底层容器临时搭建的缓冲区类
class Buffer {
public:
  static const size_t npos = std::numeric_limits<size_t>::max();
  ;
  typedef std::vector<char>::iterator iterator;
  typedef std::vector<char>::const_iterator const_iterator;

  Buffer() : _base() {}

  template <class InputInterator>
  Buffer(InputInterator first, InputInterator last) : _base(first, last) {}

  Buffer(const char *begin, std::size_t n) {
    _base.resize(n);
    memcpy(_base.data(), begin, n);
  }

  Buffer &operator=(const Buffer &x) {
    _base = x._base;
    return *this;
  }

  Buffer(Buffer &&other) noexcept : _base(std::move(other._base)) {}

  Buffer &operator=(Buffer &&other) noexcept {
    if (this != &other) {
      _base = std::move(other._base);
    }
    return *this;
  }

  iterator begin() { return _base.begin(); }

  const_iterator begin() const { return _base.begin(); }

  iterator end() { return _base.end(); }

  const_iterator end() const { return _base.end(); }

  size_t size() const { return _base.size(); }

  bool empty() const { return _base.empty(); }

  char &operator[](size_t n) { return _base[n]; }

  const char &operator[](size_t n) const { return _base[n]; }

  char &at(size_t n) {
    if (n >= size()) {
      throw std::out_of_range("Index out of range");
    }
    return _base[n];
  }

  const char &at(size_t n) const {
    if (n >= size()) {
      throw std::out_of_range("Index out of range");
    }
    return _base[n];
  }

  Buffer &append(const char *begin, size_t n) {
    if (n == 0)
      return *this; // 如果 n 为 0，直接返回
    size_t i = size();
    _base.resize(i + n);
    memcpy(_base.data() + i, begin, n);
    return *this;
  }

  Buffer subBuffer(size_t pos, size_t len = npos) {
    if (pos > size()) {
      throw std::out_of_range("Position out of range");
    }
    if (len == npos || pos + len > size()) {
      len = size() - pos;
    }
    Buffer temp(_base.data() + pos, len);
    return temp;
  }

  Buffer &erase(size_t pos, size_t len = npos) {
    if (pos >= size()) {
      throw std::out_of_range("Position out of range");
    }
    if (len == npos || pos + len > size()) {
      len = size() - pos; // 如果 len 超出范围，则调整为从 pos 到末尾
    }
    _base.erase(_base.begin() + pos, _base.begin() + pos + len);
    return *this;
  }

  const char* data() {return _base.data();}

  void clear() { _base.clear(); }

private:
  std::vector<char> _base;
};