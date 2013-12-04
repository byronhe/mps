#ifndef COMM_H
#define COMM_H

#include <list>
#include <vector>
#include <queue>
#include <map>
#include <string>
#include <algorithm>
#include <limits>
#include <iosfwd>
#include <iostream>
#include <fstream>
#include <sstream>

#include <cstdio>
#include <cassert>
#include <cstdlib>
#include <cmath>
#include <unistd.h>
using std::list;
using std::vector;
using std::queue;
using std::map;
using std::cin;
using std::cout;
using std::fstream;
using std::ofstream;
using std::endl;
using std::max;
using std::min;
using std::string;
using std::stringstream;
using std::copy;
using std::abs;

//用于回调函数传参
template <typename T1,typename T2>
struct Cons{
		T1 car;
		T2 cdr;
};

const double FLOAT_MIN=std::numeric_limits<double>::min();


const double pi=3.1415926;
class Mesh;
class Screen;
#endif
