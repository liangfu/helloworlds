
# hello: hello.cpp
# 	g++ -Wall -g -shared -std=c++11 -fPIC -I/usr/include/python2.7 -I/usr/local/include/pybind11 hello.cpp -o hello.so
# 	python -c 'import hello; import cv2; import numpy as np; \
# print(hello.vstack(np.array([[1,2],[3,4]]), np.array([[5,6],[7,8]]))); cv2.waitKey();'

hello: hello.cpp
	g++ -Wall -g -O2 -std=c++11 -I/usr/include/python2.7 -I/usr/local/include/pybind11 -shared -fPIC hello.cpp -o hello.so
	g++ -Wall -g -O2 -std=c++11 -I/usr/include/python2.7 -I/usr/local/include/pybind11 hello.cpp -o hello -lpython2.7
	./hello	


