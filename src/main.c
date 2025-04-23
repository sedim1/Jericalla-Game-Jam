#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include "game.h"

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
void drawTextureMap();

void drawSprite(Sprite* s);
void drawSpriteOnMap(Sprite* s);


void movePlayer();

Keys key = {0,0,0,0};
Player p;
float pSpeed = 5.0f, rotSpeed = 1.0f;
Sprite obj,obj2;
float planeX, planeY;

GLFWwindow* window;

int SCREEN_WIDTH= 800;
int SCREEN_HEIGHT= 640;

//PROJECTION ATTRIBUTES
int PROJECTION_WIDTH =  800;
int PROJECTION_HEIGHT = 640;
int PW2;
int PH2;
int rays;
float FOV = 60.0f * (M_PI/180.0f);
float distFromProjectionPlane;
float rayStep;
float zDepth[MAX];
int xOffset;

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
    //glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    SCREEN_WIDTH = mode->width;
    SCREEN_HEIGHT = mode->height;

    //Start projection attributes
    PROJECTION_WIDTH =  SCREEN_WIDTH;
    PROJECTION_HEIGHT = SCREEN_HEIGHT;
    PW2 = PROJECTION_WIDTH/2;
    PH2 = PROJECTION_HEIGHT/2;
    xOffset = (SCREEN_WIDTH-PROJECTION_WIDTH)/2;
    rayStep = (FOV/PROJECTION_WIDTH) * PIXELSCALE;
    distFromProjectionPlane = (PROJECTION_WIDTH/2)/tan((FOV/2));
    rays = PROJECTION_WIDTH/PIXELSCALE;


    window = glfwCreateWindow(SCREEN_WIDTH,SCREEN_HEIGHT,"RAYCASTER",monitor,NULL);
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

    glOrtho(0,SCREEN_WIDTH,SCREEN_HEIGHT,0,-2,2);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    

    glEnable(GL_DEPTH_TEST);

    //Set Callbacks
    glfwSetFramebufferSizeCallback(window, frameBufferSizeCallback); 

    p.x =4.5*CELLSIZE; p.y = 4.5*CELLSIZE; p.a = 3*M_PI/2; p.dx = cos(p.a); p.dy = sin(p.a);
    planeX = 0; planeY =  0.6;
    obj.x = 1.5*CELLSIZE; obj.y = 2.5 * CELLSIZE; obj.z = 20; obj.tex = muffin; obj.width = 32; obj.height = 32;
    obj2 = obj;
    obj2.x = 4.5*CELLSIZE; obj2.y = 3.5*CELLSIZE; obj.z = 40;
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
    glClearColor(0.0f,0.0f,0.0f,1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    drawPlayer();
    //drawSprite(&obj);
    drawSprite(&obj2);
    //drawSpriteOnMap(&obj);
    drawSpriteOnMap(&obj2);
    drawMap2D();
    drawRays3D();
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
            p1[0] = (x * (CELLSIZE/4)) + 1; p1[1] = (y * (CELLSIZE)/4) + 1;
            p2[0] = (x * (CELLSIZE/4)) + 1; p2[1] = (y * (CELLSIZE/4)) + (CELLSIZE/4) - 1;
            p3[0] = (x * CELLSIZE/4) + (CELLSIZE/4) - 1; p3[1] = (y * (CELLSIZE/4)) + 1;
            p4[0] = (x * (CELLSIZE/4)) + (CELLSIZE/4) - 1; p4[1] = (y * (CELLSIZE/4)) + (CELLSIZE/4) - 1;
            glBegin(GL_TRIANGLES);
            glVertex2i(p1[0],p1[1]); glVertex2i(p2[0],p2[1]); glVertex2i(p3[0],p3[1]); 
            glVertex2i(p2[0],p2[1]); glVertex2i(p3[0],p3[1]); glVertex2i(p4[0],p4[1]); 
            glEnd();
        }
    }
}

void drawPlayer(){
    glPointSize(4); glLineWidth(2); glColor3f(0.0f,1.0f,1.0f);
    glBegin(GL_POINTS); glVertex3i(p.x/4,p.y/4,2); glEnd();
    glBegin(GL_LINES); glVertex3i(p.x/4,p.y/4,2); glVertex3i((p.x/4 + p.dx * 10),(p.y/4 + p.dy * 10),2); glEnd();
}

void drawSpriteOnMap(Sprite* p){
    glPointSize(4); glLineWidth(2); glColor3f(1.0f,0.0f,0.0f);
    glBegin(GL_POINTS); glVertex3i(p->x/4,p->y/4,2); glEnd();
    glBegin(GL_LINES); glVertex3i(p->x/4,p->y/4,2); glVertex3i((p->x/4),(p->y/4),2); glEnd();
}

void drawRays3D(){
    float rx = 0.0f,ry = 0.0f,xo = 0.0f,yo= 0.0f,ra = 0.0f;
    int mx = 0, my = 0, dof = 0, r = 0, mapVal = 0; float dist = 0.0f;
    ra = radiansAdjust(p.a - FOV/2);
    for(r = 0; r < rays; r++){
        dof = 0;
        int hm = 0, vm = 0;
        //Horizontal lines 
        float hrx = p.x, hry = p.y;
        float distH = 10000.0f;
        if(ra == 0 || ra == M_PI){ ra+=0.00000001f;}
        if(ra > M_PI ){ hry = ((int)(p.y/CELLSIZE))*CELLSIZE - 0.0001f; hrx = p.x - (p.y - hry)/tan(ra); yo = -CELLSIZE; xo = -CELLSIZE/tan(ra); }//Looking up
        if(ra < M_PI ){ hry = ((int)(p.y/CELLSIZE))*CELLSIZE + CELLSIZE; hrx = p.x - (p.y - hry)/tan(ra); yo = CELLSIZE; xo = CELLSIZE/tan(ra); }//Looking down
            while(dof < 15){
                mx = (int)(hrx/CELLSIZE); my = (int)(hry/CELLSIZE); // get grid position o
                if(mx >= 0 && mx < mapWidth && my >= 0 && my < mapHeight){
                    if(walls[my][mx] > 0){ dof = 15; distH = distance(p.x,hrx,p.y,hry); hm = walls[my][mx]-1;}
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
        while(dof < 15){
                mx = (int)(vrx/CELLSIZE); my = (int)(vry/CELLSIZE); // get grid position o
                if(mx >= 0 && mx < mapWidth && my >= 0 && my < mapHeight){
                    if(walls[my][mx] > 0){ dof = 15; distV = distance(p.x,vrx,p.y,vry); vm = walls[my][mx]-1; }
                    else { vrx += xo; vry += yo; dof+=1;}
                }
                else { vrx += xo; vry += yo; dof+=1;}
        }
        float shade = 1.0f;
        //save the ray with the shortest distance
        if(distV < distH){dist=distV; rx = vrx; ry = vry; shade = 1.0f; mapVal = vm;}
        else{dist=distH; rx = hrx; ry = hry; shade = 0.5f; mapVal = hm;}
        //Start drawing the walls
        float correctedDist = dist * cos(radiansAdjust(p.a - ra));
        zDepth[r] = correctedDist;
        int projectedSliceHeight = (CELLSIZE/correctedDist) * distFromProjectionPlane;
        float ty = 0, tyStep = TEXTURE/(float)projectedSliceHeight,tyOffset = 0.0f;
        if(projectedSliceHeight > PROJECTION_HEIGHT){ tyOffset = (projectedSliceHeight-PROJECTION_HEIGHT)/2.0;projectedSliceHeight = PROJECTION_HEIGHT;}
        int lineOffset = PH2 - (projectedSliceHeight/2);
        int lineOffsetEnd = PH2 + (projectedSliceHeight/2);
        ty = (tyOffset * tyStep);
        float tx = 0;
        if(dist == distH) { tx = (int)(rx/2)%TEXTURE; if(ra < M_PI){tx = NTEX - tx;}}
        if(dist == distV) { tx = (int)(ry/2)%TEXTURE; if(ra > PI2 && ra < PI3){tx = NTEX - tx;}}
        for(int y = 0; y < projectedSliceHeight; y++){ 
            if(mapVal >= 0){
                float rgb[3] = {1.0};
                int pixel = ((int)ty * TEXTURE + (int)tx) * 3 + (mapVal*32*32*3);
                rgb[0] = wallTex[pixel+0] * shade; rgb[1] = wallTex[pixel+1] * shade; rgb[2] = wallTex[pixel+2] * shade;
                glColor3ub(rgb[0],rgb[1],rgb[2]); glPointSize(PIXELSCALE);
                glBegin(GL_POINTS); glVertex3i(xOffset + r*PIXELSCALE,y + lineOffset,0); glEnd();
                ty+=tyStep;
            }
        }
        int i = 0, topY = PH2 - (projectedSliceHeight/2);
        //draw floors and ceiling
        for(int y = lineOffset + projectedSliceHeight; y < PROJECTION_HEIGHT; y++){
            float straightDistToP = (TEXTURE * distFromProjectionPlane)/((y)-PH2); //sttraight distance from player to floor point
            float beta = ra - p.a; beta = radiansAdjust(beta);
            float actualDistanceToFloor = straightDistToP / cos(beta); //actual distance from player to floor point
            float floorX = p.x + cos(ra) * actualDistanceToFloor;
            float floorY = p.y + sin(ra) * actualDistanceToFloor;
            int iFloorX = (int)(floorX/CELLSIZE); int iFloorY = (int)(floorY/CELLSIZE);
            //get tex coords
                int rgb[3] = {0};
                if(iFloorX >= 0 && iFloorY >= 0 && iFloorX < mapWidth && iFloorY < mapHeight){
                    int fmapVal = floors[iFloorY][iFloorX];
                    int floorTx = (int)(floorX * TEXTURE  / (CELLSIZE))%TEXTURE; int floorTy = (int)(floorY * TEXTURE / (CELLSIZE))%TEXTURE;
                    int pixel = ((int)floorTy * TEXTURE + (int)floorTx) * 3 + (fmapVal*TEXTURE*TEXTURE*3);
                    rgb[0] = texturedFloors[pixel+0] * 0.7f;
                    rgb[1] = texturedFloors[pixel+1] * 0.7f;
                    rgb[2] = texturedFloors[pixel+2] * 0.7f;
                    glColor3ub(rgb[0],rgb[1],rgb[2]); glPointSize(PIXELSCALE);
                    glBegin(GL_POINTS); glVertex3i(xOffset + r*PIXELSCALE,y,0); glEnd();
                }
                //Draw ceiling
                int cmapVal = ceiling[iFloorY][iFloorX]-1;
                if(cmapVal >= 0 && iFloorX >= 0 && iFloorY >= 0 && iFloorX < mapWidth && iFloorY < mapHeight) { 
                    int floorTx = (int)(floorX * TEXTURE  / (CELLSIZE))%TEXTURE; int floorTy = (int)(floorY * TEXTURE / (CELLSIZE))%TEXTURE;
                    int pixel = ((int)floorTy * TEXTURE + (int)floorTx) * 3 + (cmapVal*TEXTURE*TEXTURE*3);
                    rgb[0] = texturedCeiling[pixel+0] * 0.7f;
                    rgb[1] = texturedCeiling[pixel+1] * 0.7f;
                    rgb[2] = texturedCeiling[pixel+2] * 0.7f;
                    glColor3ub(rgb[0],rgb[1],rgb[2]); glPointSize(PIXELSCALE);
                    glBegin(GL_POINTS); glVertex3i(xOffset + r*PIXELSCALE,topY - i,0); glEnd();
                }
                i++;
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
    PROJECTION_HEIGHT = SCREEN_HEIGHT;
    PH2 = PROJECTION_HEIGHT/2;
    xOffset = (SCREEN_WIDTH-PROJECTION_WIDTH)/2;
    glViewport(0,0,SCREEN_WIDTH,SCREEN_HEIGHT);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(0,SCREEN_WIDTH,SCREEN_HEIGHT,0,-2,2);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

}

void movePlayer(){
    if(key.up == 1){ p.x += p.dx * pSpeed; p.y += p.dy * pSpeed; }
    if(key.down == 1){ p.x -= p.dx * pSpeed; p.y -= p.dy * pSpeed; }
    if(key.left == 1){ 
        p.a -= rotSpeed * deltaTime; p.a=radiansAdjust(p.a); 
        p.dx=cos(p.a); p.dy=sin(p.a);
    }
    if(key.right == 1){ 
        p.a += rotSpeed * deltaTime; p.a=radiansAdjust(p.a); p.dx=cos(p.a); p.dy=sin(p.a);
        float oldPlaneX = planeX;
    }
}

///Sprite drawing functions
void drawSprite(Sprite* s){
    float spriteX = s->x - p.x;
    float spriteY = s->y - p.y;
    planeX = -p.dy * 0.6;
    planeY =  p.dx * 0.6;
    float invDet = 1.0 / (planeX * p.dy - p.dx * planeY);
    float transformX = (invDet * (p.dy * spriteX - p.dx * spriteY));
    float transformY = invDet * (-planeY * spriteX + planeX * spriteY); //this is actually the depth inside the screen, that what Z is in 3D
    int spriteScreenX = (int)((PW2) * (1 + transformX / transformY));
    int x,y, sx = 0;
    int scalingFactor = 1054;
    int scaleX = (s->width*scalingFactor/ transformY);
    int spriteHeight = (s->height*scalingFactor/ transformY);
    int leftX = (spriteScreenX-scaleX/2), rightX = (spriteScreenX+scaleX/2);
    int topY   = (PH2 - spriteHeight / 2); int botY   = PH2 + spriteHeight / 2;

    float tx=0, ty=0, txStep = s->width/(float)(rightX-leftX), tyStep = s->height/(float)(botY-topY);
    if(transformY <= 35.5){return;}
    for(x = leftX; x < rightX; x++){
        ty = 0;
        int sx = x;
        for(y = topY; y < botY; y++){
                if(sx>= 0 && sx <= PROJECTION_WIDTH && y <= PROJECTION_HEIGHT){
                    int sy = topY;
                    float rgb[3] = {1.0};
                    int pixel = ((int)ty * s->width + (int)(tx)) * 3;
                    rgb[0] = s->tex[pixel+0]; rgb[1] = s->tex[pixel+1]; rgb[2] = s->tex[pixel+2]; ty+=tyStep; 
                    if(transformY > zDepth[sx/PIXELSCALE]){continue;}
                    if(rgb[0] == 255 && rgb[1] == 0 && rgb[2] == 255){continue;}
                    glColor3ub(rgb[0],rgb[1],rgb[2]); 
                    glPointSize(1);
                    glBegin(GL_POINTS); glVertex3i(x,y+s->z,1) ;glEnd();
                }
                
        }
        tx+=txStep;
    }
    /*for(x = 0; x < rightX - leftX; x++){
        ty = 0;
        int sx = ((leftX+x*PIXELSCALE));
        //int sx = (leftX);
        for(y = 0; y < botY - topY; y++){
            if( transformY >  0 && topY + y * PIXELSCALE <= PROJECTION_HEIGHT && 
                    leftX+x*PIXELSCALE >= 0 && leftX+x*PIXELSCALE < PROJECTION_WIDTH && transformY < zDepth[(int)(x)/PIXELSCALE]){
                int sy = topY+y*PIXELSCALE;
                float rgb[3] = {1.0};
                int pixel = ((int)ty * s->width + (int)tx) * 3;
                rgb[0] = s->tex[pixel+0]; rgb[1] = s->tex[pixel+1]; rgb[2] = s->tex[pixel+2];
                ty+=tyStep; 
                //if(rgb[0] == 255 && rgb[1] == 0 && rgb[2] == 255){continue;}
                glColor3ub(rgb[0],rgb[1],rgb[2]); 
                glPointSize(PIXELSCALE);
                //glBegin(GL_POINTS); glVertex3i(xOffset + leftX + x*PIXELSCALE,topY + y * PIXELSCALE,1) ;glEnd();
                glBegin(GL_POINTS); glVertex3i(sx,sy,1) ;glEnd();
            }
        }
        tx+=txStep;
    }*/
}

