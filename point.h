#ifndef POINT_H
#define POINT_H
#include "comm.h"

struct Point{
		double x,y;//,u,v,p;
		Point(){x=y=0;}//u=v=p=0.0;}
		//点相等的意义是它们的位置相同。
		bool operator==(const Point& p);
};

void print_point(Point *p,void* f);

struct Line{
		public:
				Point* start;
				Point* end;
				~Line();
};

//callback interface;
typedef void (*point_function) (Point *p, void *data);
typedef void (*point2_function) (Point* self,Point *p, void *data);
typedef void (*line_function) (Line*,void*data);

void print_line(Line*,void*);

//两点距离平方
double points_dist_squ(Point * ,Point * );

//点到线段的距离,注意，不是直线，所以要考虑端点
double point2line_dist(Point*,Line*);

//三点确定的两条线段,p1->p2,p1->p3 的叉积,顺序很重要！
double line_cross_prod(Point* p1,Point* p2,Point* p3);

//三点确定的两条线段,p1->p2,p1->p3 的点积
double line_dot_prod(Point* p1,Point* p2,Point* p3);

//把向量(x,y)旋转角度angle，弧度制。
bool rotate_vector(double& x,double& y,double angle);

//两条线之间的关系
enum LineRelation{
		TWO_LINE_NOTCROSS=0,//无公共点
		TWO_LINE_CROSS_MID,//有一个公共点，公共点在两条线段的中间。
		TWO_LINE_CROSS_END_MID,//一个公共点，公共点在一条线段的端点上，在另一条的中间。
		TWO_LINE_CROSS_END_END,//一个公共点，公共点在两条线段的端点上。
		TWO_LINE_PUB_LINE //无穷多公共点，也意味着共线。
};

//判断线段a1--a2 和 b1--b2的位置关系
LineRelation get_lines_relation(Point* a1,Point* a2,Point* b1,Point* b2);

#endif
