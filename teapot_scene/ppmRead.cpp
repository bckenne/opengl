#include "ppmRead.h"

void skipComment(FILE *fin) {
	char buf[120];
	
	while (1) {
	   fscanf(fin, "%1s", &buf);
	   if (buf[0] != '#') { ungetc(buf[0], fin); return; }
	   else fgets(buf, 120, fin);
	}
}

GLubyte *readPPM(FILE *fin, GLsizei *wid, GLsizei *ht) {
  GLubyte  *bytes;
  char cookie[3];
  int width, height, maxComp;
  int n, r,c;
  
	fscanf(fin, "%2s", &cookie);
	
	if (strcmp("P6", cookie)) return NULL;
	skipComment(fin);
	fscanf(fin, "%d", &width); *wid = width;
	skipComment(fin);	
	fscanf(fin, "%d", &height); *ht = height;
	skipComment(fin);		
	fscanf(fin, "%d", &maxComp);
	
	if (maxComp > 255) return NULL;
	fgetc(fin);
	
	n = width * height * 3;
	bytes = (GLubyte  *) malloc(n);
	if (bytes == NULL) return NULL;
	
	for (r=height-1; r>=0; r--)
	  for (c=0; c<width; c++) {
	     bytes[3*(r*width + c)] = fgetc(fin);
	     bytes[3*(r*width + c)+1] = fgetc(fin);
	     bytes[3*(r*width + c)+2] = fgetc(fin);
	  }
	
	return bytes;	
}