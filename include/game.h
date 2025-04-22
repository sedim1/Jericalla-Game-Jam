#ifndef GAME_H
#define GAME_H

#include <math.h>
#include "glad/glad.h"

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

typedef struct{
	int left,right,up,down;
}Keys;

typedef struct{
	float x,y;//position
	float dx,dy; //direction
	float a; //angle
}Player;

typedef struct{
	int map;
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


#endif
