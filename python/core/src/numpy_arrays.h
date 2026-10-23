#pragma once
#include <pybind11/numpy.h>
#include <cmath>
#include <algorithm>
#include <zspace/zCore/base/zDefinitions.h>
#include <limits>
#include <zspace/zCore/base/zTypeDef.h>
namespace zspace_python {
namespace arrays {
namespace py = pybind11;
using Floats = py::array_t<double, py::array::c_style | py::array::forcecast>;
inline zSpace::zPointArray points(const Floats &a) {
  if (a.ndim()!=2 || a.shape(1)!=3) throw py::value_error("positions must have shape (N, 3)");
  if (a.shape(0)>std::numeric_limits<int>::max()) throw py::value_error("Too many vertices");
  zSpace::zPointArray out; out.reserve(a.shape(0));
  auto v=a.unchecked<2>();
  for (py::ssize_t i=0;i<a.shape(0);++i) {
    for(int j=0;j<3;++j) if(!std::isfinite(v(i,j)) || std::abs(v(i,j))>std::numeric_limits<float>::max()) throw py::value_error("Invalid coordinate");
    out.emplace_back(v(i,0),v(i,1),v(i,2));
  }
  return out;
}
inline zSpace::zIntArray integers(const py::array &a) {
  if(a.ndim()!=1 || (a.dtype().kind()!='i' && a.dtype().kind()!='u')) throw py::value_error("indices/counts must be one-dimensional integer arrays");
  Floats values(a); zSpace::zIntArray out; out.reserve(a.size());
  for(py::ssize_t i=0;i<a.size();++i) {
    double v=values.data()[i];
    if(v<0 || v>std::numeric_limits<int>::max()) throw py::value_error("Index/count outside int range");
    out.push_back(static_cast<int>(v));
  }
  return out;
}
template<class V> py::array_t<float> xyz(const V &v) {
  py::array_t<float> out({static_cast<py::ssize_t>(v.size()),py::ssize_t(3)});
  auto a=out.mutable_unchecked<2>();
  for(size_t i=0;i<v.size();++i) { a(i,0)=v[i].x; a(i,1)=v[i].y; a(i,2)=v[i].z; }
  return out;
}
template<class T> py::array_t<T> scalar(const std::vector<T> &v) {
  py::array_t<T> out(v.size()); std::copy(v.begin(),v.end(),out.mutable_data()); return out;
}
inline void indices(const zSpace::zIntArray &ids,size_t n) {
  for(int i:ids) if(static_cast<size_t>(i)>=n) throw py::value_error("Vertex index out of range");
}
}
}
