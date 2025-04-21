#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include "game.h"
#include "Textures/MapChecker.h"
#include "glad/glad.h"
#include <GLFW/glfw3.h>

//Main functions
bool init();
void end();
void Update();
void display();
void frameBufferSizeCallback(GLFWwindow* window,int w,int h);
void ProcessInput();
//Drawing functions
void drawMap2D();
void drawPlayer();
void drawRays3D();

void movePlayer();

Keys key = {0,0,0,0};
Player p;
float pSpeed = 5.0f, rotSpeed = 5.0f;


GLFWwindow* window;

int SCREEN_WIDTH= 1024;
int SCREEN_HEIGHT= 640;

//PROJECTION ATTRIBUTES
const int PROJECTION_WIDTH =  450;
const int PROJECTION_HEIGHT = 550;
int PW2;
int PH2;
int rays;
float FOV = 60.0f * (M_PI/180.0f);
float distFromProjectionPlane;
float rayStep;

//detaTime
float deltaTime;
float currentTime = 0.0f;
float lastTime = 0.0f;


int main(){
    if(!init())
        return -1;
    Update();
    end();
    return 0;
}

bool init(){
    //Init glfw
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
    //Start window
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    window = glfwCreateWindow(SCREEN_WIDTH,SCREEN_HEIGHT,"RAYCASTER",NULL,NULL);
    if(!window){
        printf("ERROR::FAILED TO START GLFW::\n");
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(window);
    //StartGlad
    if(!gladLoadGLLoader ((GLADloadproc)glfwGetProcAddress)){
        printf("ERROR::FAILED TO LOAD GLAD::\n");
        return false;
    }
    //Start viewport and matrices
    glViewport(0,0,SCREEN_WIDTH,SCREEN_HEIGHT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(0,SCREEN_WIDTH,SCREEN_HEIGHT,0,-1,1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    

    glEnable(GL_DEPTH_TEST);

    //Set Callbacks
    glfwSetFramebufferSizeCallback(window, frameBufferSizeCallback); 

    p.x =300.0f; p.y = 300.0f; p.a = 3*M_PI/2; p.dx = cos(p.a); p.dy = sin(p.a);

    //Start Projection Attributes
    rayStep = (FOV/PROJECTION_WIDTH) * PIXELSCALE;
    //rayStep = (FOV/PIXELSCALE);
    distFromProjectionPlane = (PROJECTION_WIDTH/2)/tan((FOV/2));
    PW2 = PROJECTION_WIDTH/2;
    PH2 = PROJECTION_HEIGHT/2;
    rays = PROJECTION_WIDTH/PIXELSCALE;
    printf("rays to emit %d\n",rays);


    return true;
}

void end(){
    glfwDestroyWindow(window);
    glfwTerminate();
}

void Update(){
    while(!glfwWindowShouldClose(window)){
	//calculate deltatime
	currentTime =  (float)glfwGetTime();
	deltaTime = currentTime - lastTime;
	lastTime = currentTime;
        ProcessInput();
        movePlayer();
        //Draw on Screen
        display();
    }
}

void display(){
    glClearColor(0.0f,0.4f,0.3f,1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    drawPlayer();
    drawRays3D();
    drawMap2D();
    glfwPollEvents();
    glfwSwapBuffers(window);
}

//drawing functions
void drawMap2D(){
    float c; int p1[2], p2[2], p3[2], p4[2];
    for(int y = 0; y < mapHeight; y++){
        for(int x = 0; x < mapWidth; x++){
            if(walls[y][x] > 0) { c = 1.0f; } else { c = 0.5f;}
            glColor3f(c,c,c);
            p1[0] = (x * CELLSIZE) + 1; p1[1] = (y * CELLSIZE) + 1;
            p2[0] = (x * CELLSIZE) + 1; p2[1] = (y * CELLSIZE) + CELLSIZE - 1;
            p3[0] = (x * CELLSIZE) + CELLSIZE - 1; p3[1] = (y * CELLSIZE) + 1;
            p4[0] = (x * CELLSIZE) + CELLSIZE - 1; p4[1] = (y * CELLSIZE) + CELLSIZE - 1;
            glBegin(GL_TRIANGLES);
            glVertex2i(p1[0],p1[1]); glVertex2i(p2[0],p2[1]); glVertex2i(p3[0],p3[1]); 
            glVertex2i(p2[0],p2[1]); glVertex2i(p3[0],p3[1]); glVertex2i(p4[0],p4[1]); 
            glEnd();
        }
    }
}

void drawPlayer(){
    glPointSize(8); glLineWidth(2); glColor3f(0.0f,1.0f,1.0f);
    glBegin(GL_POINTS); glVertex2i(p.x,p.y); glEnd();
    glBegin(GL_LINES); glVertex2f(p.x,p.y); glVertex2f(p.x + p.dx * 20,p.y + p.dy * 20); glEnd();
}

void drawRays3D(){
    float rx = 0.0f,ry = 0.0f,xo = 0.0f,yo= 0.0f,ra = 0.0f;
    int mx = 0, my = 0, dof = 0, r = 0; float dist = 0.0f;
    ra = radiansAdjust(p.a - FOV/2);
    for(r = 0; r < rays; r++){
        dof = 0;
        //Horizontal lines 
        float hrx = p.x, hry = p.y;
        float distH = 10000.0f;
        if(ra == 0 || ra == M_PI){ ra+=0.00000001f;}
        if(ra > M_PI ){ hry = ((int)(p.y/CELLSIZE))*CELLSIZE - 0.0001f; hrx = p.x - (p.y - hry)/tan(ra); yo = -CELLSIZE; xo = -CELLSIZE/tan(ra); }//Looking up
        if(ra < M_PI ){ hry = ((int)(p.y/CELLSIZE))*CELLSIZE + CELLSIZE; hrx = p.x - (p.y - hry)/tan(ra); yo = CELLSIZE; xo = CELLSIZE/tan(ra); }//Looking down
            while(dof < 8){
                mx = (int)(hrx/CELLSIZE); my = (int)(hry/CELLSIZE); // get grid position o
                if(mx >= 0 && mx < mapWidth && my >= 0 && my < mapHeight){
                    if(walls[my][mx] > 0){ dof = 8; distH = distance(p.x,hrx,p.y,hry);}
                    else{ hrx += xo; hry += yo; dof+=1; }
                }
                else { hrx += xo; hry += yo; dof+=1;}
            }
        //Vertical lines
        float distV = 10000.0f;
        float vrx = p.x, vry = p.y;
        dof = 0;
        if(ra == M_PI/2 || ra == 3*M_PI/2){ ra+=0.00000001f;} //up or down
        if(ra > 3*M_PI/2 || ra < M_PI/2  ){//right
            vrx = ((int)(p.x/CELLSIZE))*CELLSIZE + CELLSIZE; vry = p.y - (p.x-vrx)*tan(ra); xo = CELLSIZE; yo= CELLSIZE*tan(ra);
        }
        if( ra < 3*M_PI/2 && ra > M_PI/2  ){ //left
            vrx = ((int)(p.x/CELLSIZE))*CELLSIZE - 0.0001f; vry = p.y - (p.x-vrx)*tan(ra); xo = -CELLSIZE; yo= -CELLSIZE*tan(ra);
        }
        while(dof < 8){
                mx = (int)(vrx/CELLSIZE); my = (int)(vry/CELLSIZE); // get grid position o
                if(mx >= 0 && mx < mapWidth && my >= 0 && my < mapHeight){
                    if(walls[my][mx] > 0){ dof = 8; distV = distance(p.x,vrx,p.y,vry); }
                    else { vrx += xo; vry += yo; dof+=1;}
                }
                else { vrx += xo; vry += yo; dof+=1;}
        }
        float shade = 1.0f;
        //save the ray with the shortest distance
        if(distV < distH){dist=distV; rx = vrx; ry = vry; shade = 1.0f;}
        else{dist=distH; rx = hrx; ry = hry; shade = 0.5f;}
        glLineWidth(2); glColor3f(1.0f,0.0f,0.0f);
        glBegin(GL_LINES); glVertex2i(p.x,p.y); glVertex2i(rx,ry); glEnd();

        //Start drawing the walls
        float correctedDist = dist * cos(radiansAdjust(ra - p.a));

        int projectedSliceHeight = (CELLSIZE/correctedDist) * distFromProjectionPlane;
        float ty = 0, tyStep = TEXTURE/(float)projectedSliceHeight,tyOffset = 0.0f;
        if(projectedSliceHeight > PROJECTION_HEIGHT){ tyOffset = (projectedSliceHeight-PROJECTION_HEIGHT)/2.0;projectedSliceHeight = PROJECTION_HEIGHT;}
        int lineOffset = PH2 - (projectedSliceHeight/2);
        ty = tyOffset * tyStep;
        float tx = 0;
        if(dist == distH) { tx = (int)(rx/2)%TEXTURE; if(ra < M_PI){tx = NTEX - tx;}}
        if(dist == distV) { tx = (int)(ry/2)%TEXTURE; if(ra > PI2 && ra < PI3){tx = NTEX - tx;}}
        for(int y = 0; y < projectedSliceHeight; y++){ 
            float c = 1.0f * shade;
            if(tx >= 0 && tx < TEXTURE && ty >=0 && ty < TEXTURE) {c = mapCheckerTexture[(int)ty * TEXTURE + (int)(tx)] * shade;}
            glColor3f(c,c,c); glPointSize(PIXELSCALE);
            glBegin(GL_POINTS); glVertex2i(550+r*PIXELSCALE,y + lineOffset); glEnd();
            ty+=tyStep;
        }
        ra += rayStep; ra = radiansAdjust(ra);
    }
}


//Input functions
void ProcessInput(){
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if(glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS){key.up=1;}else{key.up=0;}
    if(glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS){key.down=1;}else{key.down=0;}
    if(glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS){key.left=1;}else{key.left=0;}
    if(glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS){key.right=1;}else{key.right=0;}
}

void frameBufferSizeCallback(GLFWwindow* window,int w,int h){
    SCREEN_WIDTH = w;
    SCREEN_HEIGHT = h;
    glViewport(0,0,SCREEN_WIDTH,SCREEN_HEIGHT);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(0,SCREEN_WIDTH,SCREEN_HEIGHT,0,-1,1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

}

void movePlayer(){
    if(key.up == 1){ p.x += p.dx * pSpeed; p.y += p.dy * pSpeed; }
    if(key.down == 1){ p.x -= p.dx * pSpeed; p.y -= p.dy * pSpeed; }
    if(key.left == 1){ p.a -= rotSpeed * deltaTime; p.a=radiansAdjust(p.a); p.dx=cos(p.a); p.dy=sin(p.a);}
    if(key.right == 1){ p.a += rotSpeed * deltaTime; p.a=radiansAdjust(p.a); p.dx=cos(p.a); p.dy=sin(p.a);}
}
