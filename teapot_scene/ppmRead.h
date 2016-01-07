#ifndef ppmOpenGL_H
#define ppmOpenGL_H

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <GL/glut.h>

void skipComment(FILE *fin);
GLubyte *readPPM(FILE *fin, GLsizei *wid, GLsizei *ht);

#endif
