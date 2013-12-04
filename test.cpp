#include "comm.h"
#include <thread>
#include "point.h"

void say_hello(){
		cout<<"hello"<<endl;
}

int main(){
		Point a1,a2,a3,a4;
		a1.x=14;
		a1.y=10;
		
		a2.x=14.5;
		a2.y=10.5;

		a3.x=10;
		a3.y=6;

		a4.x=6;
		a4.y=2;

		cout<<get_lines_relation(&a1,&a2,&a3,&a4)<<endl;

		Line l;
		l.start=&a1;
		l.end=&a2;
		a1.x=0;
		a1.y=0;

		a2.x=1;
		a2.y=0;

		Point b1;
		b1.x=0.5;
		b1.y=1;
		cout<<point2line_dist(&b1,&l)<<endl;
		std::thread th(&say_hello);
		std::thread::id id=th.get_id();
		th.join();
		cout<<id;
		return 0;
}
