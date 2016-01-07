#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glut.h>
#include <GL/glx.h>
#include <GL/glext.h>
#include "loadobj.cpp"

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <fcntl.h>


#define BUFFER_OFFSET(i) ((char *)NULL + (i))
#define KEY_LIGHT 0
#define FILL_LIGHT 1
#define BACK_LIGHT 2
const GLuint mybuf = 1;


struct point {
	float x;
	float y;
	float z;
};

//Cross-Product
struct point cross(struct point u, struct point v) {
	struct point w;

	w.x = (u.y * v.z) - (u.z * v.y);
	w.y = -((u.x * v.z) - (u.z * v.x));
	w.z = ((u.x * v.y) - (u.y * v.x));

	return(w);
}

//Unit-Length
struct point unit_length(struct point u) {
	double length;
	struct point v;

	length = sqrt(u.x*u.x+u.y*u.y+u.z*u.z);
	v.x = u.x/length;
	v.y = u.y/length;
	v.z = u.z/length;

	return(v);
}

//Opens our shader files of ".vert" and ".frag" and loads them into a buffer
char *readShaderFiles(char *filename) {
	FILE *fp;
	char *content = NULL;
	int fd, count;

	fd = open(filename, O_RDONLY);
	count = lseek(fd, 0, SEEK_END);
	close(fd);

	content = (char *)calloc(1,(count+1));

	fp = fopen(filename, "r");
	count = fread(content, sizeof(char), count, fp);
	content[count] = '\0';
	fclose(fp);

	return content;
}

/************************************************************************/

//Initializes the main display and sets up the view area for the image and its background
void initOGL(int argc, char **argv) {
	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_RGBA|GLUT_DEPTH|GLUT_MULTISAMPLE);

	glutInitWindowSize(512,512);
	glutInitWindowPosition(100,50);

	glutCreateWindow("CPSC 4050 Project #2");
	glewInit();

	glBindBuffer(GL_ARRAY_BUFFER,mybuf);
	glBufferData(GL_ARRAY_BUFFER, array_buff_size * sizeof(GLfloat), array_buff, GL_STATIC_DRAW);
	glVertexPointer(3, GL_FLOAT, 3 * sizeof(GLfloat), BUFFER_OFFSET(0));
	glNormalPointer(GL_FLOAT, 3 * sizeof(GLfloat), BUFFER_OFFSET(array_buff_size/2 * sizeof(GLfloat)));
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);

	/* gray background */
	glClearColor(0.2,0.2,0.2,0.0);
	glClearAccum(0.0,0.0,0.0,0.0);
}

//Sets up the viewing space and puts the eye and object view in a specific place
void setViewSpace() {
	struct point eye, view, up, vdir, utemp, vtemp;

	eye.x = -1.5; eye.y = 2.5; eye.z = 6.0;
	view.x =  -0.5; view.y = 1.0; view.z = 0.0;
	up.x = 0.0; up.y = 1.0; up.z = 0.0;
	vdir.x = view.x - eye.x; 
	vdir.y = view.y - eye.y; 
	vdir.z = view.z - eye.z;

	vtemp = cross(vdir,up);
	utemp = cross(vtemp,vdir);
	up = unit_length(utemp);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0,1.0,1.0,20.0);

	// specify position for view volume
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(eye.x,eye.y,eye.z,view.x,view.y,view.z,up.x,up.y,up.z);
}

//Sets up our Key, Fill, and Back lights and enables them
void setLights() {
	//To simplify we create some 2D arrays and then assigned our lights these values
	float ambients[3][4] = {{0.0,0.0,0.0,0.0},{0.0,0.0,0.0,0.0}, {0.0,0.0,0.0,0.0}};
	float diffuses[3][4] = {{1.0,1.0,1.0,0.0},{1.0,1.0,1.0,0.0}, {1.0,1.0,1.0,0.0}};
	float speculars[3][4] = {{0.5,0.5,0.5,0.0},{0.5,0.5,0.5,0.0}, {1.0,1.0,1.0,0.0}};
	float positions[3][4] = {{-3.0, 4.5, 3.0, 1.0},{2.125,1.0, 6.5,1.0}, {1.5, 4.5, -4.5, 1.0}};
	float directions[3][4] = {{-1.5, -4.0, -2.0, 1.0},{1.5, -2.0, -2.0, 1.0}, {0.0, -2.0, 2.0, 1.0}};

	/* turn off scene default ambient */
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT,ambients[KEY_LIGHT]);
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT,ambients[FILL_LIGHT]);
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT,ambients[BACK_LIGHT]);

	/* make specular correct for spots */
	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,1);

	for(int i = 0; i < 3; i++) {
		glLightfv(GL_LIGHT0 + i,GL_AMBIENT,ambients[i]);
		glLightfv(GL_LIGHT0 + i,GL_DIFFUSE,diffuses[i]);
		glLightfv(GL_LIGHT0 + i,GL_SPECULAR,speculars[i]);
		glLightf(GL_LIGHT0 + i,GL_SPOT_EXPONENT,1.0);
		glLightf(GL_LIGHT0 + i,GL_SPOT_CUTOFF,180.0);
		glLightf(GL_LIGHT0 + i,GL_CONSTANT_ATTENUATION,0.5);
		glLightf(GL_LIGHT0 + i,GL_LINEAR_ATTENUATION,0.1);
		glLightf(GL_LIGHT0 + i,GL_QUADRATIC_ATTENUATION,0.01);
		glLightfv(GL_LIGHT0 + i,GL_POSITION,positions[i]);
		glLightfv(GL_LIGHT0 + i,GL_SPOT_DIRECTION,directions[i]);
	}

	glEnable(GL_LIGHTING);
	for(int j = 0; j < 3; j++) { glEnable(GL_LIGHT0 + j); }
}

//Sets up the material our object uses, in this case a shiny, orange bunny
void setMaterial() {
	float mat_ambient[] = {0.918,0.416,0.125,1.0};
	float mat_diffuse[] = {0.918,0.416,0.125,1.0};
	float mat_specular[] = {1.0,1.0,1.0,1.0};
	float mat_shininess[] = {38.4};

	glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT,mat_ambient);
	glMaterialfv(GL_FRONT_AND_BACK,GL_DIFFUSE,mat_diffuse);
	glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,mat_specular);
	glMaterialfv(GL_FRONT_AND_BACK,GL_SHININESS,mat_shininess);
}

//Calls in to read our shader programs and then compiles and sets our shaders
unsigned int setShaders() {
	char *vertshader, *fragshader;
	GLuint vertptr, fragptr, activeprog;

	vertptr = glCreateShader(GL_VERTEX_SHADER);
	fragptr = glCreateShader(GL_FRAGMENT_SHADER);

	vertshader = readShaderFiles((char*) "main.vert");
	fragshader = readShaderFiles((char*) "main.frag");
	glShaderSource(vertptr, 1, (const char **) &vertshader, NULL);
	glShaderSource(fragptr, 1, (const char **) &fragshader, NULL);
	free(vertshader);
	free(fragshader);

	glCompileShader(vertptr);
	glCompileShader(fragptr);

	activeprog = glCreateProgram();
	glAttachShader(activeprog, fragptr);
	glAttachShader(activeprog, vertptr);
	glLinkProgram(activeprog);
	glUseProgram(activeprog);

	return(activeprog);
}

//Our final draw function to draw the object
void drawObject() {
	glEnable(GL_MULTISAMPLE_ARB);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glDrawArrays(GL_TRIANGLES,0,corrected_v.size());
	glFlush();
}

//A quick function to allow easy quitting out of the program using the ESC key
void quitProg(unsigned char key, int x, int y) {
	switch(key) {
		case 27:
			glDeleteBuffers(1,&mybuf);
			exit(0);
			break;
		default:
			break;
	}
}

/************************************************************************/

int main(int argc, char **argv) {
	loadObject();
	initOGL(argc,argv);
	glEnable(GL_DEPTH_TEST);

	setViewSpace();
	setLights();
	setMaterial();
	setShaders();

	glutDisplayFunc(drawObject);
	glutKeyboardFunc(quitProg);
	glutMainLoop();

	return 0;
}
