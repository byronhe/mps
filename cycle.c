#include<stdio.h>
#include<stdlib.h>
#include<math.h>

int main(){
		double rad=4;
		int div=100;
		double x=10,y=5;
		int i=0;
		for(i=0;i<=div;i++){
				printf("%lf %lf\n",x + rad*cos(i*2*3.1415926/div),y+rad*sin(i*2*3.1415926/div));
		}
}
