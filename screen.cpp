#include "screen.h"
#include "mesh.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <pthread.h>
#include <unistd.h>


bool glut_initialized=false;

//把所有的窗口都保存起来。
static vector<Screen*> all_screens;

void idle_screens_fun();//空闲时触发。
void reshape_screens_fun(int,int);//屏幕大小改变时触发。
void visibility_screen_fun(int);//当屏幕可见不可见转换时触发。

void * screen_handler(void* s){
		//cout<<"new screen."<<endl;
		Screen * screen=(Screen*) s;
		if(!glut_initialized){
				glut_initialized=true;
				int argc=0;
				char args[]="";
				glutInit(&argc,(char **)&args);
				glutInitDisplayMode(GLUT_RGBA);
		}
		screen->window_id=glutCreateWindow("Mesh");
		glutIdleFunc(idle_screens_fun);
		glutReshapeFunc(reshape_screens_fun);
		glutVisibilityFunc(visibility_screen_fun);
		glutMainLoop();
		return NULL;
}

Screen::Screen(Mesh* m){
		this->mesh=m;
		this->set_need_refresh();
		this->thread_do_exit=false;
		all_screens.push_back(this);
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		//pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
		pthread_create(&thread_id,&attr,screen_handler,(void * ) this);
		//cout<<"Screen Created!"<<endl;
}

Screen::~Screen(){
		//先告诉线程，该退出了，然后等线程退出。
		//函数再返回。
		this->thread_do_exit=true;
		pthread_join(thread_id,NULL);
}

void Screen::draw_mesh(){
		//static int count=0;
		//cout<<"draw_mesh"<<(count++)<<endl;
		//如果要支持多窗口，或者说一个Mesh一个显示区域，
		//那么此处就需要把window_id传给Mesh的绘图函数
		//但是glut不支持多窗口，所以...
		this->mesh->draw_opengl();
}

//所有的窗口线程空闲时都调用这个函数，这个函数检查所有的Screen，如果有哪个Screen的状态是需要重画，
//并且这个Screen的线程id是执行这个函数的线程id，则调用其draw_mesh()
//完成以后要睡眠一段时间，不然循环太快，占CPU的很。
void idle_screens_fun(){
		//cout<<"check idle screens."<<endl;
		auto it=all_screens.begin();
		auto e=all_screens.end();
		for(;it!=e;it++){
				if(pthread_self()==(*it)->thread_id){
						if((*it)->thread_do_exit==true){
								//本线程应该退出了。
								glutDestroyWindow((*it)->window_id);
								pthread_exit(0);
						}
						if((*it)->is_need_refresh()){
								//cout<<"found outdated screen."<<endl;
								(*it)->draw_mesh();
								(*it)->set_refresh_ok();
						}else{//窗口最大化，最小化的时候，也要重画。
								//if(((*it)->refreshcount++)%9==0)
								//		(*it)->draw_mesh();
						}
				}
		}
		usleep(400000);
}

void reshape_screens_fun(int x ,int y){
		//cout<<x<<"\t"<<y<<endl;
		auto it=all_screens.begin();
		auto e=all_screens.end();
		for(;it!=e;it++){
				if(pthread_self()==(*it)->thread_id){
						(*it)->set_need_refresh();
						(*it)->x=x;
						(*it)->y=y;
						glViewport(0,0,x,y);
						glMatrixMode(GL_PROJECTION);
						glLoadIdentity();
						gluOrtho2D(0.0,x,0.0,y);
				}
		}
}

void visibility_screen_fun(int visibility_state){
		//cout<<"visibility_state changed to:"<<visibility_state<<endl;
		auto it=all_screens.begin();
		auto e=all_screens.end();
		for(;it!=e;it++){
				if(pthread_self()==(*it)->thread_id){
						if(1==visibility_state){
								(*it)->draw_mesh();
								(*it)->set_refresh_ok();
						}
				}
		}
}
