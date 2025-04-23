#ifndef GAME_H
#define GAME_H

#include "glad/glad.h"
#include <math.h>
#include "Textures/MapChecker.h"
#include "Textures/walls.ppm"
#include "Textures/floors.h"
#include "Textures/ceiling.ppm"
#include "Textures/muffin.ppm"
#include "Queue.h"

#define M_PI 3.14159265358979323846 //180 degrees
#define PI2 M_PI/2.0f // 90 degrees
#define PI3 3.0*(M_PI/2.0f) // 270 degrees
#define D_PI 2.0*M_PI //0 - 360 degrees
#define DR M_PI/180.0f


#define mapWidth 16
#define mapHeight 8
#define CELLSIZE 64
#define PIXELSCALE 8
#define TEXTURE 32
#define NTEX TEXTURE-1
#define MAX 1000

typedef struct{
	int left,right,up,down;
}Keys;

typedef struct{
	float x,y;//position
	float dx,dy; //direction
	float a; //angle
}Player;

typedef struct{
	int width;
	int height;
	int* tex;
	int x,y,z;
}Sprite;

int walls[mapHeight][mapWidth]={
	{1,1,2,2,3,1,1,1,1,1,1,1,1,1,1,1},
	{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,1,0,0,1,0,0,1,0,0,0,1},
	{1,0,0,0,0,1,0,0,1,0,0,0,0,0,0,1},
	{1,0,0,0,0,1,0,0,1,1,1,1,0,0,0,1},
	{1,0,0,1,1,1,1,0,1,1,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
};

int floors[mapHeight][mapWidth]={
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
	{1,4,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,4,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,4,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,4,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,4,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,4,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
};

int ceiling[mapHeight][mapWidth]={
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
};

float radiansAdjust(float radians){
	float res = radians;
	if(radians < 0)  { res = radians + 2 * M_PI;}
	if(radians >= 2 * M_PI)  { res = radians - 2 * M_PI;}
	return res;
}

float distance(float x1, float x2, float y1, float y2){
	float deltaX = x2-x1; float deltaY = y2-y1;
	return sqrt( (deltaX * deltaX) + (deltaY * deltaY) );
}

void BFS(VECTOR2I* start,VECTOR2I* end){
	int dc[4] = {-1,1,0,0};
	int dr[4] = {0,0,-1,1};
	Queue q; initializeQueue(&q);
	bool reachedEnd = false;
	bool visited[mapHeight][mapWidth] = {false};
	VECTOR2I cameFrom[mapHeight][mapWidth]; 
	VECTOR2I neighbor={-1,-1}; VECTOR2I current = *start;
	if(start->x == end->x && start->y == end->y){return;}
	enqueue(&q,start); visited[start->y][start->x] = true;
	while(!isEmpty(&q)){
		current = peek(&q); dequeue(&q);
		reachedEnd = current.x == end->x && current.y == end->y;
		if(reachedEnd){break;}
		//Explore neighbors
		for(int i = 0; i < 4; i++){
			neighbor.x = current.x + dc[i]; neighbor.y = current.y + dr[i];
			if(neighbor.x < 0 || neighbor.x >= mapWidth || neighbor.y < 0 || neighbor.y >= mapHeight){continue;}
			if(walls[neighbor.y][neighbor.x] > 0 || visited[neighbor.y][neighbor.x] == true){ continue; }
			enqueue(&q,&neighbor); visited[neighbor.y][neighbor.x] = true; cameFrom[neighbor.y][neighbor.x] = current;
			//glColor3f(0.0f,0.0f,1.0f); glPointSize(3);
			//glBegin(GL_POINTS); glVertex3i((neighbor.x*CELLSIZE+CELLSIZE/2)/4,(neighbor.y*CELLSIZE+CELLSIZE/2)/4,2); glEnd();
		}
	}
	VECTOR2I actual = *end; VECTOR2I next;
	if(!reachedEnd){printf("ERROR:Unreachable location\n");return;}
	reachedEnd = false;
	while(!reachedEnd){
		next = cameFrom[actual.y][actual.x];
		reachedEnd = next.x == start->x && next.y == start->y;
		if(!reachedEnd)
			actual = next;
	}
};


#endif
