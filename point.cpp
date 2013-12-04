#include "point.h"

Line::~Line(){
		//线不负责管理其包含的点的内存。
		start=end=NULL;
}

bool Point::operator==(const Point& p){
		return x==p.x && y==p.y;
}

void print_point(Point *p,void* f){
		//fprintf((FILE*)f,"[(%f,%f) (%f,%f)]\n",p->x,p->y,p->u,p->v);
		fprintf((FILE*)f,"[(%f,%f)]\n",p->x,p->y);
}

void print_line(Line * l,void * f){
		fprintf((FILE*)f,"\n|");
		print_point(l->start,f);
		fprintf((FILE*)f,"|");
		print_point(l->end,f);
}

double points_dist_squ(Point * p1,Point * p2){
		double delta_x=p1->x - p2->x;
		double delta_y=p1->y - p2->y;
		return delta_x*delta_x+delta_y*delta_y;
}

//三点确定的两条线段,p1->p2,p1->p3 的叉积
double line_cross_prod(Point* p1,Point* p2,Point* p3){
		return (p2->x - p1->x)*(p3->y - p1->y) - (p3->x - p1->x)*(p2->y - p1->y);
}

//三点确定的两条线段,p1->p2,p1->p3 的点积
double line_dot_prod(Point* p1,Point* p2,Point* p3){
		return (p2->x - p1->x)*(p3->x - p1->x) + (p2->y - p1->y)*(p3->y - p1->y);
}

//点到线段的距离,注意，不是直线，所以要考虑端点
//因为此处是用高来算，可能出现的情况是：
//1.高与直线的交点在线段上，返回高，
//2.交点不在线段上，返回较短的边长。
//交点是否在线段上，用点积正负来判断。
double point2line_dist(Point* p,Line* l){
		if(line_dot_prod(l->end,l->start,p) <= FLOAT_MIN)
				return sqrt(points_dist_squ(p,l->end));
		if(line_dot_prod(l->start,l->end,p) <= FLOAT_MIN)
				return sqrt(points_dist_squ(p,l->start));

		double area=fabs(line_cross_prod(l->start,l->end,p));
		double dist=area/sqrt(points_dist_squ(l->start,l->end));

#if 0
		double proj=fabs(line_dot_prod(l->start,p,l->end))/sqrt(points_dist_squ(l->start,l->end));
		double dist2=sqrt(points_dist_squ(p,l->start) - proj*proj);
		if(fabs(dist2-dist) < FLOAT_MIN){
				std::cerr<<"algorithm error."<<dist<<"!="<<dist2<<endl;
		}
#endif

		return dist;
}

/*
   判断线段a1--a2 和 b1--b2的位置关系
   叉积可以用来判断一点在一条线段的哪一侧，
   TWO_LINE_NOTCROSS=0,		//无公共点
   TWO_LINE_CROSS_MID,		//有一个公共点，公共点在两条线段的中间。
   TWO_LINE_CROSS_END_MID,	//一个公共点，公共点在一条线段的端点上，在另一条的中间。
   TWO_LINE_CROSS_END_END,	//一个公共点，公共点在两条线段的端点上。
   TWO_LINE_PUB_LINE 		//无穷多公共点，也意味着共线。
   */
LineRelation get_lines_relation(Point* a1,Point* a2,Point* b1,Point* b2){
		double a1_cross,a2_cross,b1_cross,b2_cross;
		a1_cross=line_cross_prod(b1,b2,a1);
		a2_cross=line_cross_prod(b1,b2,a2);
		b1_cross=line_cross_prod(a1,a2,b1);
		b2_cross=line_cross_prod(a1,a2,b2);
		double a_cross=a1_cross*a2_cross;
		double b_cross=b1_cross*b2_cross;
		//叉积同号，即有两个点在另一条直线的同一侧,且不在直线上，故不可能相交
		if(a_cross>FLOAT_MIN || b_cross>FLOAT_MIN)
				return TWO_LINE_NOTCROSS;
		if(a_cross< -FLOAT_MIN && b_cross< -FLOAT_MIN)
				return TWO_LINE_CROSS_MID;
		if(fabs(a_cross)<=FLOAT_MIN && fabs(b_cross)<=FLOAT_MIN){
				//假设a1--a2,b1共线，然后
				//有两种可能：
				//1.若b1在a1--a2上（不包括端点），则a1或a2不可能在b1--b2上，不成立。
				//2.b1在a1--a2外，包括端点上，不妨假设b1在a1及其外侧，
				//	则出现两种可能：
				//	(1).a1与b1--b2共线，则
				//		((1. a1与b1重合
				//		((2. 四点共线
				//	(2).a2与b1--b2共线，则四点重合
				//总之：有两种情况：
				//1.共一个端点
				//2.四点共线。
				if(fabs(a1_cross)>FLOAT_MIN || fabs(a2_cross)>FLOAT_MIN 
								||fabs(b1_cross)>FLOAT_MIN||fabs(b2_cross)>FLOAT_MIN)
						return TWO_LINE_CROSS_END_END;
				//四点共线有：
				//1.共一个端点
				//2.不相交
				//3.无穷多公共点
				double a_max,a_min,b_max,b_min;
				//在共线的条件下，挑选一个差异显著的坐标，因为可能会垂直或水平。
				if(fabs(a1->x - a2->x) > fabs(a1->y - a2->y)){
						a_max=max(a1->x,a2->x);
						a_min=min(a1->x,a2->x);
						b_max=max(b1->x,b2->x);
						b_min=min(b1->x,b2->x);
				}else{
						a_max=max(a1->y,a2->y);
						a_min=min(a1->y,a2->y);
						b_max=max(b1->y,b2->y);
						b_min=min(b1->y,b2->y);
				}
				double max_in_min,min_in_max;
				max_in_min=max(a_min,b_min);//小中的大
				min_in_max=min(a_max,b_max);//大中的小
				if(max_in_min<min_in_max)
						return TWO_LINE_PUB_LINE;
				if(max_in_min>min_in_max)
						return TWO_LINE_NOTCROSS;
				//if(fabs(max_in_min - min_in_max) < FLOAT_MIN)
				return TWO_LINE_CROSS_END_END;
		}
		//一个小于0，一个等于零。
		return TWO_LINE_CROSS_END_MID;
}

bool rotate_vector(double& x,double& y,double angle){
		double xx,yy,sin_a,cos_a;
		xx=x;
		yy=y;
		sin_a=sin(angle);
		cos_a=cos(angle);
		x=xx*cos_a + yy*sin_a;
		y=yy*cos_a - xx*sin_a;
		return true;
}
