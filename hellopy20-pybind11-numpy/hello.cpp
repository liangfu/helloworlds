#include <pybind11/embed.h>
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>
#include <iostream>

namespace py = pybind11;

int hello_add(int i, int j) {
  return i + j;
}

py::array hello_vstack(py::array a, py::array b) {
  py::object np_vstack = py::module::import("numpy").attr("vstack");
  py::object cv2_imshow = py::module::import("cv2").attr("imshow");
  py::object cv2_waitKey = py::module::import("cv2").attr("waitKey");
  py::object cv2_resize = py::module::import("cv2").attr("resize");
  py::object res = np_vstack(py::make_tuple(a, b));
  res = cv2_resize(res, py::make_tuple(200, 100));
  cv2_imshow(py::str("result"), res);
  cv2_waitKey();
  return res;
}

PYBIND11_MODULE(hello, m) {
  m.doc() = "pybind11 example plugin"; // optional module docstring

  m.def("add", &hello_add, "A function which adds two numbers");
  m.def("vstack", &hello_vstack, "A function which vertically stack two numpy arrays");
}

int main()
{
  py::scoped_interpreter guard{};
  py::object hello = py::module::import("hello");
  py::object cv2 = py::module::import("cv2");
  py::object np = py::module::import("numpy");
  py::object hello_vstack = hello.attr("vstack");
  py::object np_array = np.attr("array");
  hello_vstack(np_array(std::vector<float>(2,0)),np_array(std::vector<float>(2,255)));
}



