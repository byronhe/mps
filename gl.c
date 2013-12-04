#include <stdio.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

void display(){
		glClearColor(1,1,1,1);
		glClear(GL_COLOR_BUFFER_BIT);

		glColor3f(0,1.0,1.0);
		glBegin(GL_TRIANGLES);
		glVertex3f(0.25,0.25,0);
		glVertex3f(0.75,0.25,0);
		glVertex3f(0.75,0.75,0);
		glVertex3f(0.25,0.75,0);
		//glEnd();

		glColor4f(0,1,0,0.5);
		glRectf(0.8,0.05,0.01,0.03);

		GLUquadricObj *qobj;
		qobj=gluNewQuadric();
		gluQuadricDrawStyle(qobj,GLU_FILL);
		GLuint startList=glGenLists(1);
		glNewList(startList,GL_COMPILE);
		gluSphere(qobj,5.0,30,40);
		glEndList();
		glTranslatef(-1,-1,0);
		glCallList(startList);

		glutSolidSphere(0.08,300,2);
		//glutWireTeapot(0.3);
		glFlush();
}

void fun(){
		static int i=0;
		//printf("hello %d\n",i++);
		display();
}

void visibility(int i){
		printf("visibility %d\n",i);
}

int main(int argc,char ** argv){
		//glutInit(&argc,argv);
		int i=0;
		char *s="";
		glutInit(&i,&s);
		glutInitDisplayMode(GLUT_RGB);

		int wid=glutCreateWindow("hello");
		glutIdleFunc(fun);
		glutVisibilityFunc(visibility);
		//display();

		//glutDisplayFunc(display);
		//glutReshapeFunc(display);
		glutMainLoop();
		return 0;
}
