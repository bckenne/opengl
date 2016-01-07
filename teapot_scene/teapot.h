#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>

//Glew is needed for C++, but not C, to avoid errors with implicit declaration
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glut.h>
#include "fileParser.h"
#include "readTexture.h"