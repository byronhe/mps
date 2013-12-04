#include "mesh.h"

bool most_advance(Point* p,double direc_x,double direc_y,Mesh *mesh,double &x,double &y,double &best);

Mesh::Mesh(){
				x_min=x_max=y_min=y_max=step=0;
				step_squ=0;
				mesh=NULL;
				x_num=y_num=0;
				screen=NULL;
}

void Mesh::init(double x,double y,double x_max,double y_max,double step){
		x_min=x;
		y_min=y;
		this->x_max=x_max;
		this->y_max=y_max;
		this->step=step;
		step_squ=step*step;//作用范围半径的平方。

		x_num=(x_max-x_min)/step+1;
		y_num=(y_max-y_min)/step+1;
		mesh=new Grid[x_num*y_num];
		screen=new Screen(this);
}

//释放一个存放了指针的容器里的对象
template <typename T>
void delete_ptr(T& obj){
		delete obj;
		obj=NULL;
}

Mesh::~Mesh(){
		//要先停掉screen线程，因为screen会访问mesh数据。
		delete screen;
		delete [] mesh;
		mesh=NULL;
		//释放all_points,all_lines里存的结构
		for_each(all_points.begin(),all_points.end(),delete_ptr<Point*>);
		for_each(all_lines.begin(), all_lines.end() ,delete_ptr<Line* >);
		for_each(cycle_points.begin(), cycle_points.end() ,delete_ptr<Point* >);
}

void Mesh::coord2index(double x_d,double y_d,int &x,int &y){
		//最边上一个格子有可能只有一部分在x_max,y_max里
		//assert(x_d >= x_min && x_d <=x_max && y_d >=y_min && y_d <=y_max);
		x=(x_d-x_min)/step;
		y=(y_d-y_min)/step;
}

void Mesh::index2coord(int x,int y,double& x_d,double& y_d){
		assert(x>= 0 && x<= x_num && y>=0&& y<=y_num);
		x_d=x_min+x*step;
		y_d=y_min+y*step;
}

Grid& Mesh::index2grid(const int &x,const int&y){
		return mesh[y*x_num+x];
}

bool Mesh::add_point(Point * p){
		int x,y;
		coord2index(p->x,p->y,x,y);
		index2grid(x,y).points.push_front(p);
		all_points.push_front(p);
		return true;
}

bool Mesh::delete_point(Point *p){
		int x,y;
		coord2index(p->x,p->y,x,y);
		index2grid(x,y).points.remove(p);
		//内存在all_points被释放时释放
		return true;
}

bool Mesh::move_point(Point *p,double x_new,double y_new){
		int x_i,y_i,x_new_i,y_new_i;
		coord2index(p->x,p->y,x_i,y_i);
		coord2index(x_new,y_new,x_new_i,y_new_i);
		p->x=x_new;
		p->y=y_new;
		if(x_i==x_new_i && y_i==y_new_i){//优化，同一个格子，不用移动
				return true;
		}

		Grid & g=index2grid(x_new_i,y_new_i);
		g.points.push_front(p);

		Grid &gg=index2grid(x_i,y_i);
		gg.points.remove(p);

		return true;
}

template <typename T>
void Mesh::foreach_neighbour(Point*p,T  f,void * data){
		int x,y;
		coord2index(p->x,p->y,x,y);
		for(int i=-1;i<2;i++){
				if(i+y<0 || i+y>=y_num)
						continue;
				for(int j=-1;j<2;j++){
						if(j+x<0 || j+x>=x_num)
								continue;
						Grid & g=index2grid(x+j,y+i);
						auto it =g.points.begin();
						auto end=g.points.end();
						for(;it!=end;it++){
								if(step_squ>points_dist_squ(*it,p))
										f(*it,data);
						}
				}
		}
}

void Mesh::foreach_point(point_function f,void* data){
		for(int j=0;j<y_num;j++){
				for(int i=0;i<x_num;i++){
						Grid& g=index2grid(i,j);
						auto it=g.points.begin();
						auto end=g.points.end();
						for(;it!=end;it++){
								//f不能更改当前网格的points，不然会崩溃
								f(*it,data);
						}
				}
		}
}

//遍历所有网格，按照指定的方向遍历。
template<typename T>
void Mesh::foreach_grid_from_direction(T fun,double x,double y){
}

#if 0
void Mesh::forall_points(point_function f,void * data){
		auto it=all_points.begin();
		auto end=all_points.end();
		for(;it!=end;it++){
				f(*it,data);
		}
		return;
		int count=0;
		for_each(all_points.begin(),all_points.end(),
						[&count](Point* p){count++;}
				);
		//cout<<"all_points num="<<count<<endl;
		vector<Point*> grid_points;
		for(int i=0;i<y_num;i++){
				for(int j=0;j<x_num;j++){
						Grid & g=index2grid(j,i);
						if(GRID_OUT==g.state)
								continue;
						grid_points.reserve(g.points.size());
						copy(g.points.begin(),g.points.end(),grid_points.begin());
						for(unsigned int i=0;i<grid_points.size();i++){
								print_point(grid_points[i],stdout);
								f(grid_points[i],data);
						}
				}
		}
		return;
}
#endif

const char * grid_state_str(GridState s){
		const static char* grid_state_str[]={"?","x","+","."};
		return grid_state_str[s];
}

void Mesh::write_points_to(FILE* f){
		foreach_point(print_point,f);
}

void Mesh::write_lines_to(FILE * f){
		foreach_line(print_line,f);
}

void Mesh::write_grids_to(FILE* f){
		for(int i=y_num-1;i>=0;i--){
				fprintf(f,"%.4f ",x_min+i*step);
				for(int j=0;j<x_num;j++){
						Grid & g=index2grid(j,i);
						fprintf(f,"%s ",grid_state_str(g.state));
				}
				fprintf(f,"\n");
		}
}

//标记出所有的格子的状态。
//所有线上的格子都已经标出。
void Mesh::mark_grid_state(){
		//一行一行地从左到右扫描，经过了奇数个有线格子的格子，是内部格子，
		//否则是外部格子。
		for(int i=0;i<y_num;i++){
				int passed_lines=0;
				for(int j=0;j<x_num;j++){
						Grid & g=index2grid(j,i);
						assert(g.state==GRID_INVALID||g.state==GRID_LINE);
						if(g.state==GRID_LINE){
								auto it=g.lines.begin(),e=g.lines.end();
								for(;it!=e;it++){
										/* 因为浮点特性，
										 * step*(j+1)!=step*j+step
										 * 算法要鲁棒，如果无法鲁棒，
										 * 那么相同的数值一定要通过同样的指令计算出来。
										 */
										double x,y,right_x;
										index2coord(j+1,i,right_x,y);
										index2coord(j,i,x,y);
										y+=0.5*step;
										//边界线在格子中线附近的x坐标。
										double line_x=(y - (*it)->start->y)/
												((*it)->end->y - (*it)->start->y) *
												((*it)->end->x - (*it)->start->x) +
												(*it)->start->x;
										if(line_x < x || line_x >= right_x)
												continue;
										double x_min=min((*it)->start->x,(*it)->end->x);
										double x_max=max((*it)->start->x,(*it)->end->x);
										if(line_x < x_min || line_x > x_max)
												continue;
										//交点要在格子中线上，同时要在边界线上。
										passed_lines++;
								}
						}else{
								if(passed_lines%2==0)
										g.state=GRID_OUT;
								else
										g.state=GRID_IN;
						}
				}
		}
#if 0
		for(int j=0;j<x_num;j++){
				double test1=step*(j+1);
				double test2=step*j+step;
				if(test1!=test2){
						cout<<"test1 "<<test1<<"!=test2 "<<test2<<" "<<endl;
				}
		}
#endif
}

#if 0
//判断点是否在多边形内部，使用角度和的办法。
bool Mesh::is_point_in_bound_angle(Point *p){
		double angle=0.0;
		auto all_lines::iterator it,end;
		end=all_lines.end();
		for(;it!=end;it++){
		}
}
#endif

//这个函数是在格子已经标记好之后发挥作用的。
bool Mesh::is_point_in_bound(Point *p){
		int x_i,y_i;
		coord2index(p->x,p->y,x_i,y_i);
		if(index2grid(x_i,y_i).state==GRID_IN)
				return true;
		if(index2grid(x_i,y_i).state==GRID_OUT)
				return false;
		int passed_lines=0;
		for(int j=0;j<=x_i;j++){
				Grid & g=index2grid(j,y_i);
				if(g.state==GRID_LINE){
						auto it=g.lines.begin(),e=g.lines.end();
						for(;it!=e;it++){
								double x,y,right_x;
								index2coord(j+1,y_i,right_x,y);
								index2coord(j,y_i,x,y);
								right_x=min(right_x,p->x);
								y=p->y;
								//边界线在p点水平线附近的x坐标。
								double line_x=(y - (*it)->start->y)/
										((*it)->end->y - (*it)->start->y) *
										((*it)->end->x - (*it)->start->x) +
										(*it)->start->x;
								if(line_x < x || line_x >= right_x)
										continue;
								//排除在边界线的延长线上的情况
								double x_min=min((*it)->start->x,(*it)->end->x);
								double x_max=max((*it)->start->x,(*it)->end->x);
								if(line_x < x_min || line_x > x_max)
										continue;
								passed_lines++;
						}
				}
		}
		return (passed_lines%2!=0);
}

typedef Cons<point2_function,Cons<Point*,void*> > Paras2;
typedef Cons<Mesh*,Paras2> Paras;

static void neigh_pair_visit(Point* p,void* data){
		Paras2* e=(Paras2*) data;
		(e->car)(e->cdr.car,p,e->cdr.cdr);
}

static void visit_neigh(Point*p,void*data){
		Paras *e=(Paras*) data;
		e->cdr.cdr.car=p;
		e->car->foreach_neighbour(p,neigh_pair_visit,&(e->cdr));
}

void Mesh::foreach_points_neighbour(point2_function f,void * data){
		Paras e;
		e.car=this;
		e.cdr.car=f;
		e.cdr.cdr.cdr=data;
		foreach_point(visit_neigh,&e);
}

//简单，可靠
bool Mesh::add_line(Line *l){
		all_lines.push_front(l);
		Point p1,p2;
		for(int i=0;i<y_num;i++){
				for(int j=0;j<x_num;j++){
						bool cross_grid=false;

						index2coord(j,i,p1.x,p1.y);
						index2coord(j+1,i,p2.x,p2.y);
						if(get_lines_relation(l->start,l->end,&p1,&p2)!=TWO_LINE_NOTCROSS)
						{
								cross_grid=true;
						}
						index2coord(j,i,p1.x,p1.y);
						index2coord(j,i+1,p2.x,p2.y);
						if(get_lines_relation(l->start,l->end,&p1,&p2)!=TWO_LINE_NOTCROSS)
						{
								cross_grid=true;
						}
						index2coord(j+1,i,p1.x,p1.y);
						index2coord(j+1,i+1,p2.x,p2.y);
						if(get_lines_relation(l->start,l->end,&p1,&p2)!=TWO_LINE_NOTCROSS)
						{
								cross_grid=true;
						}
						index2coord(j,i+1,p1.x,p1.y);
						index2coord(j+1,i+1,p2.x,p2.y);
						if(get_lines_relation(l->start,l->end,&p1,&p2)!=TWO_LINE_NOTCROSS)
						{
								cross_grid=true;
						}
						if(cross_grid){
								index2grid(j,i).lines.push_front(l);
								index2grid(j,i).state=GRID_LINE;
						}
				}
		}
		return true;
}

void Mesh::foreach_line(line_function f,void* data){
		auto it=all_lines.begin();
		auto e=all_lines.end();
		for(;it!=e;it++){
				f(*it,data);
		}
}

typedef Cons<Point*,Cons<Point*,bool*> > LinePara;
void cross_bound(Line* l,void * data){
		LinePara* p=(LinePara*) data;
		LineRelation r=get_lines_relation(l->start,l->end,p->car,p->cdr.car);
		if(r==TWO_LINE_CROSS_MID || r==TWO_LINE_CROSS_END_MID || r==TWO_LINE_PUB_LINE){
				//如果传入的线(p里面的),和某条边(l)共点，则不认为相交。
				*(p->cdr.cdr)=true;
		}
}

bool Mesh::is_cross_bound(Point* p1,Point* p2){
		bool f;
		LinePara p;
		p.car=p1;
		p.cdr.car=p2;
		p.cdr.cdr=&f;
		foreach_line(cross_bound,&p);
		return f;
}

template <typename T>
void Mesh::add_point_cycle(T& c){
		typename T::iterator it=c.begin();
		typename T::iterator e =c.end();
		if(it==e)
				return;
		Point* last=*(e-1);
		for(;it!=e;it++){
				Line* l=new Line();
				l->start=last;
				l->end=*it;
				cycle_points.push_front(*it);
				add_line(l);
				last=*it;
		}
		cout<<std::endl;
}


void Mesh::init_bound(std::istream& f){
		string buff;
		vector<Point*> pv;
		list<vector<Point*> > pvl;
		while(getline(f,buff)){
				bool insert=true;
				if(buff[buff.find_first_not_of(' ')]=='#')
						insert=false;
				if(0==buff.length()){
						insert=false;
						pvl.push_back(pv);
						pv.clear();
				}
				if(insert){
						stringstream line;
						line<<buff;
						double x,y;
						line>>x>>y;
						Point * p=new Point();
						p->x=x;
						p->y=y;
						pv.push_back(p);

						x_max=max(x,x_max);
						x_min=min(x,x_min);
						y_max=max(y,y_max);
						y_min=min(y,y_min);

				}
				buff.clear();
		}
		pvl.push_back(pv);

		double t=x_max-x_min;
		x_min-=0.05*t;
		x_max+=0.05*t;
		t=y_max-y_min;
		y_min-=0.1*t;
		y_max+=0.1*t;
		init(x_min,y_min,x_max,y_max,0.4);
		for(auto it=pvl.begin(),e=pvl.end();
						it!=e;it++){
				add_point_cycle(*it);
		}
}

typedef Cons<Mesh *,Cons<Point *,Cons<bool,bool> > > DropPara;
//			Mesh	要下落的点，该点是否被移动，是否允许偏转。

void drop_point(Point* p,void *data);

//有的内部格子没有装点，填充之。
void Mesh::fill_blank_grid(){
		for(int j=0;j<y_num;j++){
				double d=-1.0;
				for(int i=0;i<x_num;i++){
						Grid& g=index2grid(i,j);
						if(g.state!=GRID_IN)
								continue;
						if(0==g.points.size()){
								Point* p=new Point();
								double x_r,y_h;
								index2coord(i,j,p->x,p->y);
								index2coord(i+1,j+1,x_r,y_h);
								p->x=(p->x + x_r)/2;
								p->y=(p->y + y_h)/2;
								if(can_move_point(NULL,*p)){
										add_point(p);

										Point direc;
										direc.x=d;
										d=-d;
										direc.y=0;
										DropPara para;
										para.car=this;
										para.cdr.car=&direc;
										para.cdr.cdr.cdr=false;//不允许偏转。
										//bool & droped=para.cdr.cdr.car;
										drop_point(p,&para);
										i--;
										i=0;

										cout<<"add new Point:"<<d;
										print_point(p,stdout);
								}else{
										delete p;
								}
						}
				}
				for(int i=0;i<x_num;i++){
						Grid& g=index2grid(i,j);
						if(g.state!=GRID_IN)
								continue;
						if(0==g.points.size()){
								Point* p=new Point();
								double x_r,y_h;
								index2coord(i,j,p->x,p->y);
								index2coord(i+1,j+1,x_r,y_h);
								p->x=(p->x + x_r)/2;
								p->y=(p->y + y_h)/2;

								cout<<"fill blank,going to add new Point:";
								print_point(p,stdout);

								Point direc_new;
								point_neighbour_direction(p,direc_new.x,direc_new.y);
								print_point(&direc_new,stdout);

								double best;
								if(!most_advance(p,-1,0,this,p->x,p->y,best)){
										delete p;
								}else{
										assert(can_move_point(NULL,*p));
										cout<<"succeed fill mid blank."<<endl;
										print_point(p,stdout);

										add_point(p);
										Point direc;
										direc.x=-1;
										direc.y=0;
										DropPara para;
										para.car=this;
										para.cdr.car=&direc;
										para.cdr.cdr.cdr=false;//不允许偏转。
										//bool & droped=para.cdr.cdr.car;
										drop_point(p,&para);
								}
						}
				}
		}
}

//填充边界区域的空白部分。给出了前一次移动的方向，(x,y)
//应该首先找出(x,y)的反方向，所对应的边界，
//然后从边界外往经过边界往里塞入粒子。
void Mesh::fill_border_blank(double x,double y){
		//cout<<"fill_border_blank,direction=("<<x<<","<<y<<")"<<endl;
}

template<typename grid_function>
void Mesh::foreach_neighbour_grid(Point* p,grid_function fun,void * data){
		int x,y;
		coord2index(p->x,p->y,x,y);
		for(int i=-1;i<2;i++){
				if(i+y<0 || i+y>=y_num)
						continue;
				for(int j=-1;j<2;j++){
						if(j+x<0 || j+x>=x_num)
								continue;
						fun(index2grid(x+j,y+i),data);
				}
		}
}

typedef Cons<Point*,Cons<Point* ,Cons<bool,Cons<double,double> > > > PlacePara;
//			新的点，     旧点         结果    距离半径，半径平方

//遍历点的所有邻居，判断是否可以在某个坐标放置一个点，而不与其它点相碰撞，不与边界线相交。
void can_place_point_here(Grid & g,void * data){
		//data里要有:新点，保存的结果, step
		PlacePara * para=(PlacePara*) data;
		Point * new_point =para->car;
		Point * old_point =para->cdr.car;
		bool & result=para->cdr.cdr.car;
		double dist=0.5*(para->cdr.cdr.cdr.car);
		double dist_squ=para->cdr.cdr.cdr.cdr;

		if(!result)
				return;

		if(g.state==GRID_OUT){
				//不能出界，
				//这个检查不完全，可能存在点到了边界线的外侧，
				//而且跟边界线不接触，在GRID_LINE的格子上的情况。
				//这种情况应该使用is_point_in_bound()来检测。
				return;
		}

		auto it =g.points.begin();
		auto end=g.points.end();
		for(;it!=end;it++){
				if(points_dist_squ(*it,new_point) < dist_squ && (*it)!=old_point){
						//cout<<"touch point "<<(*it)->x<<" "<<(*it)->y<<endl;
						result=false;
						return;
				}
		}

		if(g.state==GRID_IN)
				return;

		auto lit=g.lines.begin();
		auto lend=g.lines.end();
		for(;lit!=lend;lit++){
				if(point2line_dist(new_point,*lit) < dist){
						result=false;
						return;
				}
		}
}

//判断能不能移动一个点。
bool Mesh::can_move_point(Point* p,Point& new_point){
		//typedef Cons<Point*,Cons<Point* ,Cons<bool,Cons<double,double> > > > PlacePara;
		//			新的点，     旧点         结果    距离半径，半径平方
		PlacePara para;
		para.car=&new_point;
		para.cdr.car=p;
		bool & result=para.cdr.cdr.car;
		result=true;
		para.cdr.cdr.cdr.car=step;
		para.cdr.cdr.cdr.cdr=step*step;

		if(new_point.x < x_min || new_point.x > x_max 
						|| new_point.y < y_min || new_point.y > y_max){
				return false;
		}

		result=true;
		foreach_neighbour_grid(&new_point,can_place_point_here,&para);
		result=(result && is_point_in_bound(&new_point));
		return result;
}

//typedef Cons<Mesh *,Cons<Point *,Cons<bool,bool> > > DropPara;
//			Mesh	要下落的点，该点是否被移动，是否允许偏转。

//找出一个点，在一个方向上最远可以前进多远，限于一步之内，该点可以不属于这个mesh，如果不能前进，返回false
bool most_advance(Point* p,double direc_x,double direc_y,Mesh *mesh,double &x,double &y,double &best){
		//二分法求根。
		double low_radio=0.0;
		double high_radio=1.0;//mesh->x_num + mesh->y_num;
		best=low_radio;
		bool at_least_one_success=false;
		double mid;
		double step=mesh->get_step();
		Point new_point;
		while(fabs(low_radio-high_radio)>0.000001){
				mid=(low_radio+high_radio)/2;
				new_point.x=p->x + direc_x * step * mid;
				new_point.y=p->y + direc_y * step * mid;
				bool result=mesh->can_move_point(p,new_point);
				if(result){
						low_radio=mid;
						best=max(best,mid);
						at_least_one_success=true;
				}else{
						high_radio=mid;
				}
				//cout<<"mid="<<mid<<" best="<<best<<" result="<<result<<endl;
		}

		if(!at_least_one_success)
				return false;

		x=p->x + direc_x * step * best;
		y=p->y + direc_y * step * best;
		return true;
}

//下落一个点。
void drop_point(Point* p,void *data){
		//data里面有：Mesh,落的方向。
		DropPara * drop_para=(DropPara*) data;
		Mesh * mesh=drop_para->car;
		Point * direc= drop_para->cdr.car;
		bool & droped=drop_para->cdr.cdr.car;
		bool& allow_redirec=drop_para->cdr.cdr.cdr;
		Point new_point;
		double best;

		if(!most_advance(p,direc->x,direc->y,mesh,new_point.x,new_point.y,best))
				return;
		assert(mesh->can_move_point(p,new_point));

		if(best>0.3){//如果本次可以移动的距离大于一个给定值，那么很有可能可以继续移动。
				mesh->move_point(p,new_point.x,new_point.y);
				mesh->plot_points();
				droped=true;
				usleep(10000);
				drop_point(p,data);//能移动就继续移动，直到不能再移动为止。
				return;
		}else{//当本次移动距离小于一定值时，先不移动，尝试根据临近点合斥力重定向。
				//当正前方不能移动时，加上斜方向自动寻路。
				//类似于认为小球有光滑的表面,可以偏转,可以自动寻找势能最低点。
				//根据是否已经被移动决定是否结束。
				if(allow_redirec){//若允许重定向。是为了防止无限递归。
						Point direc_new;
						mesh->point_neighbour_direction(p,direc_new.x,direc_new.y);
						if((direc_new.x * direc_new.x) + (direc_new.y * direc_new.y)>0.9){//临近点合力不为零。

								//防止反向运动。
								//用点积判断是否逆向。
								Point zero;
								zero.x=zero.y=0.0;
								double angle_delta=pi/180;
								if(line_cross_prod(&zero,direc,&direc_new)<0)
										angle_delta*=-1.0;
								int rotate_count=0;
								while(line_dot_prod(&zero,direc,&direc_new)<0.2){
										rotate_vector(direc_new.x,direc_new.y,angle_delta);
										rotate_count++;
								}
								//cout<<"rotate "<<rotate_count<<"times."<<endl;
								DropPara para_rec;
								para_rec.car=mesh;
								para_rec.cdr.car=&direc_new;
								para_rec.cdr.cdr.cdr=false;//不允许偏转
								bool & droped_rec=para_rec.cdr.cdr.car;
								droped_rec=false;

								drop_point(p,&para_rec);

								if(droped_rec){//偏转后的方向可以移动并且已经移动
										cout<<"successful drop towards:("<<direc_new.x<<","<<direc_new.y<<")"<<endl;
										print_point(p,stdout);
										mesh->plot_points();
										usleep(10000);
										droped=true;
										return;
								}else{
										//当能移动的距离较小，允许偏转，但偏转没有成功时，尽可能多移动。
										mesh->move_point(p,new_point.x,new_point.y);
										mesh->plot_points();
										usleep(10000);
										droped=true;
										return;
								}
						}else{//附近有的点小于小于一个，合力太小。
								cout<<"unexpected force:("<<direc_new.x<<","<<direc_new.y<<")"<<endl;
								return;
						}
				}else{//不允许偏转，同时可以移动的距离又极小。
						//cout<<"best="<<best<<".cant redirect."<<endl;
						return;
				}
		}
}

//给定一个点，算出该点周围的点施加给该点的距离向量之和。
void Mesh::point_neighbour_direction(Point *p,double & x,double &y){
		x=y=0.0;
		foreach_neighbour_grid(p,[&x,&y,&p](Grid& g,void *data)
						{
						auto it =g.points.begin();
						auto end=g.points.end();
						for(;it!=end;it++){
						Point *pp=*it;
						if(p!=pp){
						x+= pp->x - p->x;
						y+= pp->y - p->y;
						//cout<<"push "<<points_dist_squ(p,pp)<<"("<<x<<","<<y<<")"<<endl;
						}
						}
						},NULL);

		double dist=sqrt(x*x+y*y);
		if(fabs(dist)>FLOAT_MIN){
				x/=dist;
				y/=dist;
		}
}

//把所有的点往一个方向落。
void Mesh::drop_all_points(double x,double y){
		//先把它弄成单位向量。
		double dist=sqrt(x*x+y*y);
		x/=dist;
		y/=dist;
		Point direc;
		direc.x=x;
		direc.y=y;

		DropPara para;
		para.car=this;
		para.cdr.car=&direc;
		bool & droped=para.cdr.cdr.car;
		para.cdr.cdr.cdr=true;//允许偏转
		do{
				droped=false;
				forall_points(drop_point,&para);
				fill_blank_grid();
				fill_border_blank(direc.x,direc.y);
		}while(droped);//持续移动直到所有的点都没办法移动。

		verify_points();

		//for(int i=0;i<max(x_num,y_num);i++){
		//		forall_points(drop_point,&para);
		//		fill_blank_grid();
		//}
		//foreach_point(print_point,stdout);
		//foreach_point(drop_point,&para);
		//verify_points();
}

#if 0
static void print_point_corrd(Point* p,void * data){
		ofstream *f=(ofstream*) data;
		(*f)<<p->x<<"\t"<<p->y<<endl;
}
#endif

void Mesh::plot_points(){
		//ofstream f("points.txt");
		//foreach_point(print_point_corrd,&f);
		//f.close();

		this->screen->set_need_refresh();
}

static void check_distance_nt_re(Point* self,Point*p,void* data){
		double re_s=(*(double*)data);
		re_s*=re_s;
		double dis_s=((self->x-p->x)*(self->x-p->x)+(self->y-p->y)*(self->y-p->y));
		//printf("%f",re_s);
		assert(dis_s<re_s);
		if(dis_s > re_s){
				printf("\nmeet not neighbour points!\n");
				print_point(self,stdout);
				print_point(p,stdout);
				printf("\n");
		}
}


bool wrong_point_found=false;
//检查:
//1.是否所有的点都在与其位置对应的格子里，
//2.是否没有点与其它点相重叠，
//3.是否没有点与边界线相交，
//4.是否没有点超出边界。
void Mesh::verify_points(){
		//cout<<"------------begin--to--verify--all--points-----------"<<endl;

		foreach_points_neighbour(check_distance_nt_re,&step);

		for(int j=0;j<y_num;j++){
				for(int i=0;i<x_num;i++){
						//cout<<"****************************"<<endl;
						//cout<<"verify grid:"<<endl;

						Grid& g=index2grid(i,j);
						if(GRID_OUT==g.state){
								assert(0==g.points.size());
								assert(0==g.lines.size());
						}
						if(GRID_INVALID==g.state){
								assert(0);
						}
						if(GRID_LINE==g.state){
								assert(g.lines.size()>0);
						}
						if(GRID_IN==g.state){
								auto it=g.points.begin();
								auto end=g.points.end();
								for(;it!=end;it++){
										int x,y;
										coord2index((*it)->x,(*it)->y,x,y);
										assert(x==i && y==j);//点在它所应该在的格子里
										assert(is_point_in_bound(*it));

										//typedef Cons<Point*,Cons<Point* ,Cons<bool,Cons<double,double> > > > PlacePara;
										//			新的点，     旧点         结果    距离半径，半径平方
										PlacePara para;
										para.car=*it;
										para.cdr.car=*it;
										bool & result=para.cdr.cdr.car;
										result=true;
										para.cdr.cdr.cdr.car=step;
										para.cdr.cdr.cdr.cdr=step*step;
										foreach_neighbour_grid(*it,can_place_point_here,&para);
										if(!result){
												cout<<"wrong point touched neighbour:"<<endl;
												print_point(*it,stdout);
												wrong_point_found=true;
												result=true;
												foreach_neighbour_grid(*it,can_place_point_here,&para);
												assert(result);
												//double x_p,y_p;
												//index2coord(i,j,x_p,y_p);
												//cout<<i<<","<<j<<"| "<<x_p<<"-"<<x_p+step<<","<<y_p<<
												//		"-"<<y_p+step<<endl;

										}
								}
						}
				}
		}
		//cout<<"---------------end--verify--all--points----------------"<<endl;
}

