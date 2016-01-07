#include "fileParser.h"

char *readFile(char* fileName) {
	FILE *fp;
	char *content = NULL;
	int fd, count;
	fd = open(fileName, O_RDONLY);
	count = lseek(fd, 0, SEEK_END);
	close(fd);
	content = (char *)calloc(1,(count+1));
	fp = fopen(fileName, "r");
	count = fread(content,sizeof(char),count,fp);
	content[count] = '\0';
	fclose(fp);
	return content;
}

int parseObj(char *fileName, OBJObject *container, int vPerF) {
	char *content = readFile(fileName);
	int vtCount = 0;	
	container->vCount = 0;
	container->fCount = 0;
	char *contentDup = strdup(content);
	char *line = strtok(contentDup, "\n");

	while(line){
		int linelength = strlen(line) + 1;
		char *word = strtok(line, " ");	

		if(!strcmp(word, "v"))		container->vCount++;
		else if(!strcmp(word,"vt"))	vtCount++;
		else if(!strcmp(word,"f"))	container->fCount++;

		line = strtok(line+linelength, "\n");	
	}
	free(contentDup);

	container->vertices = (GLfloat *)calloc(sizeof(GLfloat), 3*container->vCount);
	container->vertNormals = (GLfloat *)calloc(sizeof(GLfloat), 3*container->vCount);
	container->tangents = (GLfloat *)calloc(sizeof(GLfloat), 3*container->vCount);
	container->bitangents = (GLfloat *)calloc(sizeof(GLfloat), 3*container->vCount);
	container->texCoords = (GLfloat *)calloc(sizeof(GLfloat), 2*vtCount);
	
	container->vIndices= (GLuint *)calloc(sizeof(GLuint), vPerF*container->fCount);
	container->vNIndices= (GLuint *)calloc(sizeof(GLuint), vPerF*container->fCount);
	container->texIndices= (GLuint *)calloc(sizeof(GLuint), vPerF*container->fCount);

	contentDup = strdup(content);	
	line = strtok(contentDup, "\n");	
	unsigned int vIndex = 0;
	unsigned int vNIndex = 0;
	unsigned int fIndex = 0;
	unsigned int vtIndex = 0;
	unsigned int vxIndex = 0;
	unsigned int vyIndex = 0;

	while(line){
		int lineLength = strlen(line) + 1;
		char *word = strtok(line, " ");	
		if(!strcmp(word, "v")){
			for(int j = 0; j < 3; vIndex++, j++) {
				word = strtok(NULL, " ");
				container->vertices[vIndex] = (GLfloat)atof(word);
			}
		}
		else if(!strcmp(word,"vn")){
			for(int j = 0; j < 3; vNIndex++, j++) {
				word = strtok(NULL, " ");
				container->vertNormals[vNIndex] = (GLfloat)atof(word);
			}
		}
		else if(!strcmp(word,"vx")){
			for(int j = 0; j < 3; vxIndex++, j++) {
				word = strtok(NULL, " ");
				container->tangents[vxIndex] = (GLfloat)atof(word);
			}
		}
		else if(!strcmp(word,"vy")){
			for(int j = 0; j < 3; vyIndex++, j++) {
				word = strtok(NULL, " ");
				container->bitangents[vyIndex] = (GLfloat)atof(word);
			}
		}
		else if(!strcmp(word,"vt")){
			for(int j = 0; j < 2; vtIndex++, j++) {
				word = strtok(NULL, " ");
				container->texCoords[vtIndex] = (GLfloat)atof(word);
			}
		}
		else if(!strcmp(word,"f")){
			for(int j = 0; j < vPerF; fIndex++, j++){
				word = strtok(NULL, "/");
				container->vIndices[fIndex] = (((GLuint)atoi(word))-1);	
				word = strtok(NULL, "/");
				container->texIndices[fIndex] = (((GLuint)atoi(word))-1);
				word = strtok(NULL, " /");
				container->vNIndices[fIndex] = (((GLuint)atoi(word))-1); 
			}
		}		
		line = strtok(line+lineLength, "\n");	
	}
	free(contentDup);
	free(content);
	return 0;
}