#ifndef MESH_H
#define MESH_H
#include "comm.h"
#include "point.h"
#include "screen.h"

enum GridState{
		GRID_INVALID=0,
		GRID_OUT=1, 	//在区域外面
		GRID_IN=2,	//在区域里面
		GRID_LINE=3	//有区域边界经过的格子
};

struct Grid{
		public:
				list<Point*> points;//包含在这个格子里的点
				list<Line*> lines;//经过这个格子的线段
				GridState state;
				Grid(){state=GRID_INVALID;}
		private:
				Grid& operator=(Grid &);
};

typedef void (*grid_function) (Grid&, void*);

class Mesh{
		private:
				double x_min,x_max,y_min,y_max,step;//step既是格子的边长，也是小球的直径
				double step_squ;//作用范围半径的平方。
				Grid * mesh;
				long x_num,y_num;
				//用于内存管理
				list<Line*> all_lines;
				list<Point*> all_points;
				list<Point*> cycle_points;//边界上的点。

				Screen *screen;
		public:
				Mesh();
				~Mesh();
				bool add_point(Point * p);
				bool delete_point(Point *);
				bool move_point(Point *,double x_new,double y_new);

				//foreach_point不能在遍历的同时修改，移动。
				void foreach_point(point_function,void *);
				//forall_points可以。
				template <typename T>
						void forall_points(T f,void * data){
								auto it=all_points.begin();
								auto end=all_points.end();
								for(;it!=end;it++){
										f(*it,data);
								}
								return;
						}
				template<typename grid_function>
						void foreach_neighbour_grid(Point*,grid_function,void *);
				template <typename T>
						void foreach_neighbour(Point*,T,void *);
				//遍历所有网格，按照指定的方向遍历。
				template<typename T>
						void foreach_grid_from_direction(T,double,double);
				void foreach_points_neighbour(point2_function,void *);
				void point_neighbour_direction(Point *p,double & x,double &y);

				void init_bound(std::istream&);
				bool is_point_in_bound(Point *p);
				void mark_grid_state();//标记所有的格子的状态。
				void fill_blank_grid();//有的内部格子没有装点，填充之。
				void fill_border_blank(double,double);//填充边界区域的空白部分。
				void drop_all_points(double x,double y);//把所有的点往一个方向落。
				bool can_move_point(Point*,Point&);
				void foreach_line(line_function,void* data);
				template <typename T>
						void add_point_cycle(T& c);

		private:
				void init(double x,double y,double x_max,double x_min,double step);
				bool add_line(Line *);
				void coord2index(double,double,int &,int &);
				void index2coord(int,int,double&,double&);
				Grid& index2grid(const int &,const int&);
				//判断一条线段是否和现有的边界相交。
				bool is_cross_bound(Point*,Point*);
		public:
				void verify_points();
				void write_points_to(FILE*);
				//这个函数会立即返回，它只是告诉绘图线程应该重画，就返回。
				void plot_points();
				//这个函数把本网格的所有点和线绘制在一个Screen里，使用了opengl
				//是在screen线程里执行的
				void draw_opengl();
				void coord2glcoord(const double&,const double&,double&,double&);
				void write_lines_to(FILE *);
				void write_grids_to(FILE*);
				double get_step(){return step;}
};

#endif
