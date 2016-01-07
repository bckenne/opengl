#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string.h>

typedef struct v3f {
	GLfloat x, y, z;
} v3f;

typedef struct v3i {
	int x, y, z;
} v3i;

typedef struct face {
	v3i face_v, face_n;
} face;

std::vector<v3f> vertices;
std::vector<v3f> normals;
std::vector<v3f> corrected_v;
std::vector<v3f> corrected_n;
std::vector<v3f> vbo;
std::vector<face> faces;

GLfloat *array_buff;
int array_buff_size = 0;

int loadObject () {
	FILE *myfile = fopen("bunny.obj", "r");
	char *ret;
	int i, j;

	if(myfile == NULL) {
		printf("Error reading file\n");
		return 0;
	}
	
	char *first = (char *)malloc(sizeof(char) * 2);
	while(1) {
		v3f vert;
		face f;
		int line = fscanf(myfile, "%s", first);

		if(line == EOF) {
			break;
		}
		else if(strcmp("#", first) == 0) {
			char comment[256];
			ret = fgets(comment, 256, myfile);
			if(ret == NULL) {
				fprintf(stderr, "Error!\n");
			}
			continue;
		}
		else if(strcmp("v", first) == 0) {
			int retr = fscanf(myfile, "%f %f %f\n", &vert.x, &vert.y, &vert.z);
			if(retr < 0) {
				fprintf(stderr, "Error!\n");
			}
			vertices.push_back(vert);
		}
		else if(strcmp("vn", first) == 0) {
			int retr = fscanf(myfile, "%f %f %f\n", &vert.x, &vert.y, &vert.z);
			if(retr < 0) {
				fprintf(stderr, "Error!\n");
			}
			normals.push_back(vert);
		}
		else if(strcmp("f", first) == 0) {
			v3i face_v, face_n;
			int retr = fscanf(myfile, "%d//%d %d//%d %d//%d\n", &f.face_v.x, &f.face_n.x, &f.face_v.y, &f.face_n.y, &f.face_v.z, &f.face_n.z);
			if(retr < 0) {
				fprintf(stderr, "Error!\n");
			}
			faces.push_back(f);
		}
		else {
			printf("Unrecognised data type: %s\n", first);
			break;
		}
	}

	for(i = 0; i < faces.size(); i++) {
		corrected_v.push_back(vertices[faces[i].face_v.x-1]);
		corrected_v.push_back(vertices[faces[i].face_v.y-1]);
		corrected_v.push_back(vertices[faces[i].face_v.z-1]);
		
		corrected_n.push_back(normals[faces[i].face_n.x-1]);
		corrected_n.push_back(normals[faces[i].face_n.y-1]);
		corrected_n.push_back(normals[faces[i].face_n.z-1]);
	}
	
	array_buff_size = faces.size() * 2 * 3 * 3;
	array_buff = (GLfloat *)malloc(sizeof(GLfloat) * array_buff_size);

	j = 0;
	for(i = 0; i < faces.size()*3; i++) {
		array_buff[j] = corrected_v[i].x;	
		array_buff[j+1] = corrected_v[i].y;	
		array_buff[j+2] = corrected_v[i].z;	
		j += 3;
	}
	for(i = 0; i < faces.size()*3; i++) {
		array_buff[j] = corrected_n[i].x;	
		array_buff[j+1] = corrected_n[i].y;	
		array_buff[j+2] = corrected_n[i].z;	
		j += 3;
	}
	
	return 0;
}