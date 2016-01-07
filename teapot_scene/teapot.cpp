#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glut.h>
#include <GL/glx.h>
#include <GL/glext.h>
#include "fileParser.h"
#include "ppmRead.h"

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <fcntl.h>


#define BUFFER_OFFSET(i) ((char *)NULL + (i))
#define ESC_KEY '\033'
#define KEY_LIGHT 0
#define FILL_LIGHT 1
#define BACK_LIGHT 2

#define NULL_A (unsigned char(*)(unsigned char *))0

GLuint texName[7];
GLuint noLightFrag, phongFrag, vert, noLights, phong;
OBJObject *primaryOBJ;
GLsizei w,h;
FILE *fin;
GLubyte *texpat;
int rotation = 0;
const int verticesPerFace = 4;

const GLuint myBuffer = 1;
const GLuint vertBuffer = 2;
const GLuint normBuffer = 3;
const GLuint indexBuffer = 4;

struct point {
	float x, y, z;
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

int setTexture(char *s,unsigned char alpha) {
	GLubyte *tb_alpha, *tb_final, *tb_src, *tb_dst;
  if ( !(fin = fopen(s, "rb")) )  {  return 0; }
  texpat = readPPM(fin, &w, &h); /* w and h must be a power of 2 */
  int imageSize = w*h;
  if (texpat == NULL) return 0;
  glPixelStorei(GL_UNPACK_ALIGNMENT,1);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
  GLint format;
  if(alpha == 0){
  	format = GL_RGB;
  	tb_final = texpat;
  }
  else{
  		format = GL_RGBA;
		tb_alpha = (unsigned char *)calloc(4, imageSize);
		//RGBA original (source)
		tb_src = texpat;
		//RGBA destination
		tb_dst = tb_alpha;
		for(int i = 0; i < imageSize; i++){
			//Here use the per pixel alpha function to load new alpha components
			*tb_dst++ = *tb_src++;//R
			*tb_dst++ = *tb_src++;//G
			*tb_dst++ = *tb_src++;//B	
			*tb_dst++ = (alpha);//A
		}
		tb_final = tb_alpha;
  }
  glTexImage2D( GL_TEXTURE_2D, /* target */ 0, /* level */
  		3, /* components */
  		w, h, /* width, height */ 0, /* border */
  		format,  /* format */   GL_UNSIGNED_BYTE, /* type */
  		tb_final);
  free(texpat); /* free the texture memory */
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  return 1;
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
	glutInitDisplayMode(GLUT_RGBA|GLUT_DEPTH|GLUT_MULTISAMPLE|GLUT_STENCIL);

	glutInitWindowSize(1280,720);
	glutInitWindowPosition(100,50);

	glutCreateWindow("CPSC 4050 Final Project");
	glewInit();
	
	glEnableClientState(GL_NORMAL_ARRAY);

	// gray background 
	glClearColor(0.2,0.2,0.2,0.0);
	glGenTextures(7, texName); /* make 3 texture names */
  	glBindTexture(GL_TEXTURE_2D, texName[0]);
  	setTexture((char *)"orangetile.ppm",0);			//teapot image
  	glBindTexture(GL_TEXTURE_2D, texName[1]);
 	setTexture((char *)"marbleSurface.ppm",0);		//plate top
 	glBindTexture(GL_TEXTURE_2D, texName[2]);
 	setTexture((char *)"marbleSides.ppm",0);		//plate sides
 	glBindTexture(GL_TEXTURE_2D, texName[4]);
 	setTexture((char *)"floor.ppm", 0);				//cieling
 	glBindTexture(GL_TEXTURE_2D, texName[5]);
 	setTexture((char *)"orangetile.ppm", 0);		//floor
 	glBindTexture(GL_TEXTURE_2D, texName[6]);
 	setTexture((char *)"bluewall.ppm", 0);			//walls
}

//Sets up the viewing space and puts the eye and object view in a specific place
void setViewSpace(){
	struct point eye, view, up, vdir, utemp, vtemp;

	eye.x = -8.0; eye.y =3.5; eye.z = 0.0;
	view.x =  0; view.y = 0.8; view.z = 0.0;
	up.x = 0.0; up.y = 1.0; up.z = 0.0;
	vdir.x = view.x - eye.x; 
	vdir.y = view.y - eye.y; 
	vdir.z = view.z - eye.z;

	vtemp = cross(vdir,up);
	utemp = cross(vtemp,vdir);
	up = unit_length(utemp);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, 1.78, 0.1, 40.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(eye.x, eye.y, eye.z,
			  view.x, view.y, view.z,
			  up.x, up.y, up.z);
}

//Sets up our Key, Fill, and Back lights and enables them
void setLights() {
	float ambients[3][4] = {{0.0,0.0,0.0,0.0},{0.0,0.0,0.0,0.0}, {0.0,0.0,0.0,0.0}};
	float diffuses[3][4] = {{0.3,0.3,0.3,0.0},{0.1,0.1,0.1,0.0}, {0.4,0.4,0.4,0.0}};
	float speculars[3][4] = {{0.3,0.3,0.3,0.0},{0.1,0.1,0.1,0.0}, {0.4,0.4,0.4,0.0}};
	float positions[3][4] = {{-6.0, 6.0, 6.0, 1.0},{-6.0,1.5, -4.0,1.0}, {4.0, 3.0, 0.0, 1.0}};
	float directions[3][4] = {{6.0, -6.0, -6.0, 1.0},{6.0, -1.5, 4.0, 1.0}, {4.0, -3.0, 4.0, 1.0}};

	glLightModelfv(GL_LIGHT_MODEL_AMBIENT,ambients[KEY_LIGHT]);
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT,ambients[FILL_LIGHT]);
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT,ambients[BACK_LIGHT]);

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

//Calls in to read our shader programs and then compiles and sets our shaders
unsigned int setShaders() {
	char *vertshader, *fragshader1, *fragshader2;

	vert = glCreateShader(GL_VERTEX_SHADER);
	noLightFrag = glCreateShader(GL_FRAGMENT_SHADER);
	phongFrag = glCreateShader(GL_FRAGMENT_SHADER);

	vertshader = readShaderFiles((char*) "phong.vert");
	fragshader1 = readShaderFiles((char*) "noLights.frag");
	fragshader2 = readShaderFiles((char*) "phong.frag");
	glShaderSource(vert, 1, (const char **) &vertshader, NULL);
	glShaderSource(noLightFrag, 1, (const char **) &fragshader1, NULL);
	glShaderSource(phongFrag, 1, (const char **) &fragshader2, NULL);
	free(vertshader);
	free(fragshader1);
	free(fragshader2);

	glCompileShader(vert);
	glCompileShader(noLightFrag);
	glCompileShader(phongFrag);

	noLights = glCreateProgram();
	glAttachShader(noLights, noLightFrag);
	glAttachShader(noLights, vert);
	glLinkProgram(noLights);
	glDeleteShader(noLightFrag);
	phong = glCreateProgram();
	glAttachShader(phong, phongFrag);
	glAttachShader(phong, vert);
	glLinkProgram(phong);
	glDeleteShader(phongFrag);
	glDeleteShader(vert);
	return(phong);
}
void clearShaders() {
	glDeleteProgram(noLights);
	glDeleteProgram(phong);
}

//Our final draw function to draw the object
void drawObject() {
	glUseProgram(0);
	setShaders();
	GLdouble eqn1[4] = {0.0,1.0,0.0,0.0};
	int i = 0, j = 0;
	float th = 0.2;
	float rm = 12.0;
	float plt = 2.0;
	float flr = -2.0;
	float off= -1.0 * th;

	float nplt = -1.0*plt;
	float nrm = -1.0*rm;
	flr= flr + off;

	int vertIndex, faceIndex, normIndex, texIndex;

	struct point top[4]=	{{0.0,0.0,plt},{plt,0.0,0.0},{0.0,0.0,nplt},{nplt,0.0,0.0}};
	struct point front[4]=	{{0.0,0.0,plt},{plt,0.0,0.0},{plt,off,0.0},{0.0,off,plt}};
	struct point right[4]=	{{0.0,0.0,nplt},{plt,0.0,0.0},{plt,off,0.0},{0.0,off,nplt}};
	struct point back[4]=	{{nplt,0.0,0.0},{0.0,0.0,nplt},{0.0,off,nplt},{nplt,off,0.0}};
	struct point left[4]=	{{0.0,0.0,plt},{nplt,0.0,0.0},{nplt,off,0.0},{0.0,off,plt}};

	struct point roomTop[4]=	{{rm,rm+flr,rm},{rm,rm+flr,nrm},{nrm,rm+flr,nrm},{nrm,rm+flr,rm}};
	struct point roomFront[4]=	{{rm,rm+flr,nrm},{rm,rm+flr,rm},{rm,flr,rm},{rm,flr,nrm}};
	struct point roomRight[4]=	{{nrm,rm+flr,nrm},{rm,rm+flr,nrm},{rm,flr,nrm},{nrm,flr,nrm}};
	struct point roomBack[4]=	{{nrm,rm+flr,rm},{nrm,rm+flr,nrm},{nrm,flr,nrm},{nrm,flr,rm}};
	struct point roomLeft[4]=	{{rm,rm+flr,rm},{nrm,rm+flr,rm},{nrm,flr,rm},{rm,flr,rm}};
	struct point roomBottom[4]=	{{rm,flr,rm},{nrm,flr,rm},{nrm,flr,nrm},{rm,flr,nrm}};

	float mytexcoords[4][2] = {{0.0,1.0},{1.0,1.0},{1.0,0.0},{0.0,0.0}};
	
	GLint buffers = GL_NONE;
	glEnable(GL_MULTISAMPLE_ARB);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_NORMALIZE);
	glGetIntegerv(GL_DRAW_BUFFER, &buffers );

	glPushMatrix();
	glClearStencil(0x0);
	glClear(GL_STENCIL_BUFFER_BIT);
	glStencilFunc(GL_ALWAYS,0x1,0x1);
	glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
	glDrawBuffer(GL_NONE);
	glEnable(GL_STENCIL_TEST);
	glRotatef(rotation, 0,1,0);
	glBegin(GL_QUADS);
	for(i = 0; i < 4; i++) glVertex3f(top[i].x, top[i].y, top[i].z);
	glEnd();
	glDrawBuffer((GLenum) buffers);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glPopMatrix();

	glPushMatrix();
	glRotatef(rotation, 0,1,0);
	glScalef(1.0f, -1.0f, 1.0f);
	glTranslatef(0.0f, -0.04f, 0.0f);
	glClipPlane(GL_CLIP_PLANE0, eqn1);
	glEnable(GL_CLIP_PLANE0);
	glStencilFunc(GL_EQUAL, 0x1, 0x1);
	setLights();
	glBindTexture(GL_TEXTURE_2D, texName[6]);
	glUseProgram(noLights);
	glBegin(GL_QUADS);
	//
	for(i=0;i<4;i++){
		glNormal3f(1.0,0.0,0.0);
		glTexCoord2f(mytexcoords[i][0],mytexcoords[i][1]);
		glVertex3f(roomBack[i].x,roomBack[i].y,roomBack[i].z);
	}
	for(i=0;i<4;i++){
		glNormal3f(0.0,0.0,1.0);
		glTexCoord2f(mytexcoords[i][0],mytexcoords[i][1]);
		glVertex3f(roomRight[i].x,roomRight[i].y,roomRight[i].z);
	}
	for(i=0;i<4;i++){
		glNormal3f(0.0,0.0,-1.0);
		glTexCoord2f(mytexcoords[i][0],mytexcoords[i][1]);
		glVertex3f(roomLeft[i].x,roomLeft[i].y,roomLeft[i].z);
	}
	for(i=0;i<4;i++){
		glNormal3f(-1.0,0.0,0.0);
		glTexCoord2f(mytexcoords[i][0],mytexcoords[i][1]);
		glVertex3f(roomFront[i].x,roomFront[i].y,roomFront[i].z);
	}
	glEnd();
	glBindTexture(GL_TEXTURE_2D, texName[5]);
	glBegin(GL_QUADS);
	for(i=0;i<4;i++){
		glNormal3f(0.0,-1.0,0.0);
		glTexCoord2f(mytexcoords[i][0],mytexcoords[i][1]);
		glVertex3f(roomTop[i].x,roomTop[i].y,roomTop[i].z);
	}
	glEnd();
	glUseProgram(phong);
	glBindTexture(GL_TEXTURE_2D, texName[0]);
	glBegin(GL_QUADS);
	for(i = 0; i < primaryOBJ->fCount; i++){
		faceIndex = 4*i;
		for(j = 0; j < 4; j++){
			normIndex = 3*primaryOBJ->vNIndices[faceIndex + j];
			glNormal3f(primaryOBJ->vertNormals[normIndex],primaryOBJ->vertNormals[normIndex+1],primaryOBJ->vertNormals[normIndex+2]);

			texIndex = 2*primaryOBJ->texIndices[faceIndex+j];	
			glTexCoord2f(primaryOBJ->texCoords[texIndex], primaryOBJ->texCoords[texIndex+1]);

			vertIndex = 3*primaryOBJ->vIndices[faceIndex + j];	
			glVertex3f(primaryOBJ->vertices[vertIndex],primaryOBJ->vertices[vertIndex+1],primaryOBJ->vertices[vertIndex+2]);
		}
	}
	glEnd();
	glDisable(GL_CLIP_PLANE0);
	glPopMatrix();

	glPushMatrix();
	glRotatef(rotation, 0,1,0);
	setLights();
	glDisable(GL_STENCIL_TEST);
	glDrawBuffer(GL_NONE);
	glBindTexture(GL_TEXTURE_2D, texName[1]);
	glBegin(GL_QUADS);
	for(i=0;i<4;i++){
		glVertex3f(top[i].x, top[i].y, top[i].z);
	}
	glEnd();
	glDrawBuffer((GLenum) buffers);
	glPopMatrix();

	glPushMatrix();
	glRotatef(rotation, 0,1,0);
	glBindTexture(GL_TEXTURE_2D, texName[0]);
	glTranslatef(0.0f, -0.04f, 0.0f);
	glBegin(GL_QUADS);
	for(i = 0; i < primaryOBJ->fCount; i++){
		faceIndex = 4*i;
		for(j = 0; j < 4; j++){
			normIndex = 3*primaryOBJ->vNIndices[faceIndex + j];
			glNormal3f(primaryOBJ->vertNormals[normIndex],primaryOBJ->vertNormals[normIndex+1],primaryOBJ->vertNormals[normIndex+2]);

			texIndex = 2*primaryOBJ->texIndices[faceIndex+j];	
			glTexCoord2f(primaryOBJ->texCoords[texIndex], primaryOBJ->texCoords[texIndex+1]);

			vertIndex = 3*primaryOBJ->vIndices[faceIndex + j];	
			glVertex3f(primaryOBJ->vertices[vertIndex],primaryOBJ->vertices[vertIndex+1],primaryOBJ->vertices[vertIndex+2]);
		}
	}
	
	glEnd();
	glPopMatrix();

	//Draw Plate
	glPushMatrix();
	glRotatef(rotation, 0,1,0);
	glBindTexture(GL_TEXTURE_2D, texName[2]);
	glBegin(GL_QUADS);
	for(i=0;i<4;i++){
		glNormal3f(1.4142,0.0,1.4142);
		glTexCoord2f(mytexcoords[i][0], mytexcoords[i][1]);
		glVertex3f(front[i].x,front[i].y,front[i].z);
	}
	//;
	for(i=0;i<4;i++){
		glNormal3f(1.4142, 0.0, -1.4142);
		glTexCoord2f(mytexcoords[i][0], mytexcoords[i][1]);
		glVertex3f(right[i].x,right[i].y,right[i].z);
	}
	for(i=0;i<4;i++){
		glNormal3f(-1.4142, 0.0, -1.4142);
		glTexCoord2f(mytexcoords[i][0], mytexcoords[i][1]);
		glVertex3f(back[i].x,back[i].y,back[i].z);
	}
	for(i=0;i<4;i++){
		glNormal3f(-1.4142, 0.0, 1.4142);
		glTexCoord2f(mytexcoords[i][0], mytexcoords[i][1]);
		glVertex3f(left[i].x,left[i].y,left[i].z);
	}
	glEnd();
	glPopMatrix();

	//Draw Room
	glPushMatrix();
	glRotatef(rotation, 0,1,0);
	glUseProgram(0);
	clearShaders();
	setShaders();
	glUseProgram(noLights);
	glBindTexture(GL_TEXTURE_2D, texName[4]);
	glBegin(GL_QUADS);
	for(i=0;i<4;i++){
		glNormal3f(0.0, 1.0, 0.0);
		glTexCoord2f(mytexcoords[i][0],mytexcoords[i][1]);
		glVertex3f(roomBottom[i].x,roomBottom[i].y,roomBottom[i].z);
	}
	glEnd();
	glBindTexture(GL_TEXTURE_2D, texName[6]);
	glBegin(GL_QUADS);
	for(i=0;i<4;i++){
		glNormal3f(1.0, 0.0, 0.0);
		glTexCoord2f(mytexcoords[i][0],mytexcoords[i][1]);
		glVertex3f(roomBack[i].x,roomBack[i].y,roomBack[i].z);
	}
	for(i=0;i<4;i++){
		glNormal3f(0.0, 0.0, 1.0);
		glTexCoord2f(mytexcoords[i][0],mytexcoords[i][1]);
		glVertex3f(roomRight[i].x,roomRight[i].y,roomRight[i].z);
	}
	for(i=0;i<4;i++){
		glNormal3f(0.0, 0.0, -1.0);
		glTexCoord2f(mytexcoords[i][0],mytexcoords[i][1]);
		glVertex3f(roomLeft[i].x,roomLeft[i].y,roomLeft[i].z);
	}
	for(i=0;i<4;i++){
		glNormal3f(-1.0, 0.0, 0.0);
		glTexCoord2f(mytexcoords[i][0],mytexcoords[i][1]);
		glVertex3f(roomFront[i].x,roomFront[i].y,roomFront[i].z);
	}
	glEnd();
	glBindTexture(GL_TEXTURE_2D, texName[5]);
	glBegin(GL_QUADS);
		for(i=0;i<4;i++){
		glNormal3f(0.0, -1.0, 0.0);
		glTexCoord2f(mytexcoords[i][0],mytexcoords[i][1]);
		glVertex3f(roomTop[i].x,roomTop[i].y,roomTop[i].z);
	}
	glEnd();
	glPopMatrix();

	glPushMatrix();
	glEnable(GL_BLEND);
	glDepthMask(GL_FALSE);
	glDepthMask(GL_LEQUAL);
	glUseProgram(phong);
	glTranslatef(0.0f, -0.001f, 0.0f);
	glRotatef(rotation, 0,1,0);
	glBindTexture(GL_TEXTURE_2D, texName[1]);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBegin(GL_QUADS);
	for(i=0;i<4;i++){
		glNormal3f(0.0,1.0,0.0);
		glTexCoord2f(mytexcoords[i][0],mytexcoords[i][1]);
		glVertex3f(top[i].x, top[i].y+.002, top[i].z);
	}
	glEnd();
	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);
	glPopMatrix();

	glFlush();
	glutSwapBuffers();

}

//A quick function to allow easy quitting out of the program using the ESC key
void quitProg(unsigned char key, int x, int y) {
switch(key) {
        case ESC_KEY:           
                glDeleteBuffers(1,&myBuffer);
                glDeleteBuffers(1,&vertBuffer);
                glDeleteBuffers(1,&normBuffer);
                glDeleteBuffers(1,&indexBuffer);
				free(primaryOBJ->vertices);
				free(primaryOBJ->vertNormals);
				free(primaryOBJ->texCoords);
				free(primaryOBJ->vIndices);
				free(primaryOBJ->vNIndices);
				free(primaryOBJ->texIndices);
				free(primaryOBJ->tangents);
				free(primaryOBJ->bitangents);
				free(primaryOBJ);
                exit(0);
				break;
        default:
                break;
    }
}

void specialKeys(int key, int x, int y) {
	switch (key) {    
    	case 27:      
    		break;
       case 100: 
			rotation-=5;   
			break;
       case 102: 
			rotation+=5;
			break;
    }
    glutPostRedisplay();
}


/************************************************************************/

int main(int argc, char **argv) {
	printf("Press Left and Right arrow keys to rotate Scene\n");
	primaryOBJ = (OBJObject*)calloc(sizeof(OBJObject), 1);

	parseObj((char *)"teapot.605.obj", primaryOBJ, verticesPerFace);
	initOGL(argc,argv);
	glEnable(GL_DEPTH_TEST);

	setViewSpace();
	glutDisplayFunc(drawObject);
	glutKeyboardFunc(quitProg);
	glutSpecialFunc(specialKeys);
	glutMainLoop();

	return 0;
}