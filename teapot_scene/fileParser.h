#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glut.h>
typedef struct {
	GLfloat *vertices;
	GLfloat *vertNormals;
	GLfloat *texCoords;
	GLfloat *tangents;
	GLfloat *bitangents;

	GLuint *vIndices;
	GLuint *vNIndices;
	GLuint *texIndices;

	int vCount, fCount;

}OBJObject;

char *readFile(char* fileName);
int parseObj(char *fileName, OBJObject *container, int vPerF);