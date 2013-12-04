#include "point.h"
#include "mesh.h"


int main(){
		/*
		 * 1.构造一个网格，指定格子边长,原点坐标
		 * 2.把边界线段读入网格中
		 * 3.指定重力方向
		 * 4.遍历网格，给所有在边界里，又没有边界经过的格子插入点。
		 * 5.沿重力方向，撴实。
		 * 6.重复4，补点直到无点可补
		 */
		Mesh m;
		fstream f("input.txt");
		m.init_bound(f);
		//m.foreach_line(print_line,stdout);
		m.mark_grid_state();
		m.fill_blank_grid();
		//m.write_points_to(stdout);
		m.write_grids_to(stdout);
		m.plot_points();

		double x_dir,y_dir;
		while(cin){
				cout<<"move towards which direction?\n:";
				cin>>x_dir>>y_dir;
				cout<<"got it. moving towards ("<<x_dir<<","<<y_dir<<")."<<endl;
				cout<<"please wait..."<<endl;
				m.drop_all_points(x_dir,y_dir);
		}
		cout<<"\n\nGAME OVER!"<<endl;

		//const int div=0;
		//for(int i=0;i<div;i++){
		//m.drop_all_points(cos(i*2*pi/div), sin(i*2*pi/div));
		//		system("sleep 6");
		//}

		(void)system("sleep 300");
		return 0;
}
