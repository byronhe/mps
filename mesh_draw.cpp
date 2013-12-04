#include "mesh.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

//用于把坐标变换为opengl屏幕上的坐标
inline void Mesh::coord2glcoord(const double &x,const double &y,double& x_out,double& y_out){
		x_out=(x - x_min)/(x_max-x_min)*(screen->x);
		y_out=(y - y_min)/(y_max-y_min)*(screen->y);
}

static void draw_line(Line *l,void * m){
		Mesh * mesh=(Mesh *) m;
		double x,y,x_e,y_e;
		mesh->coord2glcoord(l->start->x,l->start->y,x,y);
		mesh->coord2glcoord(l->end->x,l->end->y,x_e,y_e);
		glVertex3i(int(x),int(y),0.0);
		glVertex3i(int(x_e),int(y_e),0.0);
}

typedef Cons<Mesh*,Cons<int,Point *> > DrawPointPara;

static void draw_point(Point* p,void* m){
		DrawPointPara *dp=(DrawPointPara*) m;
		Mesh * mesh=dp->car;

		//glBegin(GL_LINE_LOOP);
		glBegin(GL_POLYGON);
		Point *pl=dp->cdr.cdr;
		for(int i=0;i<dp->cdr.car;i++){
				double x,y;
				mesh->coord2glcoord((p->x+pl[i].x),(p->y+pl[i].y),x,y);
				glVertex3i(int(x),int(y),0);
		}
		glEnd();
		//cout<<x<<"\t"<<y<<"\t"<<endl;
}

void Mesh::draw_opengl(){
		//先把边界线画出来，再把边界内部区域画出来，再把小球画出来。

		//cout<<"draw_opengl"<<endl;

		glClearColor(1,1,1,1);
		glClear(GL_COLOR_BUFFER_BIT);

		glColor3f(1.0,0,0);
		glBegin(GL_LINES);
		//glLineWidth(1.0);
		foreach_line(draw_line,this);
		glEnd();

		for(int j=0;j<y_num;j++){
				for(int i=0;i<x_num;i++){
						GridState state=index2grid(i,j).state;

						if(state==GRID_IN)
								glColor3f(0.9,0.4,0.0);
						if(state==GRID_OUT)
								glColor3f(0.0,0.4,0.1);
						if(state==GRID_LINE)
								continue;

						glBegin(GL_POLYGON);

						double x,y;
						index2coord(i,j,x,y);
						coord2glcoord(x,y,x,y);
						glVertex3i(int(x),int(y),0);

						index2coord(i+1,j,x,y);
						coord2glcoord(x,y,x,y);
						glVertex3i(int(x),int(y),0);

						index2coord(i+1,j+1,x,y);
						coord2glcoord(x,y,x,y);
						glVertex3i(int(x),int(y),0);

						index2coord(i,j+1,x,y);
						coord2glcoord(x,y,x,y);
						glVertex3i(int(x),int(y),0);

						glEnd();
				}
		}

		//算好一个圆圈，每个点把它整体移动即可。
		DrawPointPara para;
		const int div=40;
		double rad=0.5*step;
		Point cycle[div];

		for(int i=0;i<div;i++){
				cycle[i].x=rad*cos(i*2*pi/div);
				cycle[i].y=rad*sin(i*2*pi/div);
			//	coord2glcoord(cycle[i].x,cycle[i].y,cycle[i].x,cycle[i].y);
		}

		para.car=this;
		para.cdr.car=div;
		para.cdr.cdr=&cycle[0];

		glColor3f(0,1.0,0);
		//foreach_point(draw_point,&para);
		forall_points(draw_point,&para);

		glFlush();
}
