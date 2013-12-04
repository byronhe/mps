#ifndef SCREEN_H
#define SCREEN_H
#include "comm.h"
#include "mesh.h"

//在屏幕上绘图，对应一个网格Mesh，单独一个线程。
//和Mesh通信方式是：
//1.Mesh可以调用Screen的set_need_refresh(),告诉Screen线程需要重画。
//2.Screen线程的IdleFunc在空闲时（绝大多数时候），检测是否需要重画，若是
//	调用draw_mesh()
//3.draw_mesh()遍历Mesh的点和边,重画
//
//注意：glut貌似只支持一个窗口。。。浪费。。。
class Screen{
		bool todo_refresh;
		Mesh * mesh;//记录这个屏幕上画哪个网格

		public:
		void set_need_refresh(){ todo_refresh=true; }
		bool is_need_refresh(){ return todo_refresh;}
		void set_refresh_ok(){ todo_refresh=false; }
		//初始化时，创建线程，新线程里
		Screen(Mesh* m);
		~Screen();
		//真正的绘图工作在这里。
		void draw_mesh();

		public:
		pthread_t thread_id;
		bool thread_do_exit;
		int window_id;
		unsigned long refreshcount;
		int x,y;//屏幕大小
};


#endif
