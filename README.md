# json-realize
基于C++11实现的JSON解析器

运行：
```
$ brew install cmake
$ cd github/json-tutorial/tutorial01
$ mkdir build
$ cd build
$ cmake -DCMAKE_BUILD_TYPE=Debug ..
$ make
```
内存泄漏：
```
valgrind --leak-check=full  ./leptjson_test
```


