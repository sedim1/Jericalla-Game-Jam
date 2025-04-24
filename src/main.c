#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include "game.h"
#include "Quicksort.h"

//Main functions
bool init();
void end();
void Update();
void display();
void frameBufferSizeCallback(GLFWwindow* window,int w,int h);
void ProcessInput();

void initScene();

//Drawing functions
void drawMap2D();
void drawPlayer();
void drawRays3D();
void drawTextureMap();

void drawAllSprites(Sprite sprites[],int n);
void drawSprite(Sprite* s);
void drawSpriteOnMap(Sprite* s);

//Player functions
void playerUpdate();
void playerInput();
void rotateView();
void movePlayer();

//Enemy functions
void enemyUpdate();
void followPlayer();

Keys key = {0,0,0,0};
Player p;
float pSpeed = 2.5f, rotSpeed = 3.0f;

Sprite sprites[5]; int nSprites = 5;
//Enemy variables
Sprite* enemy; float te = 0.0f; VECTOR2I enemyNextPosition = {-1,-1}; float ewPos[2] = {-1.0f,};
float lastWait = 0.0f, cooldown = 1.0f;
enum EnemyAIStates enemyAiState; enum PlayerStates enemyState = IDLE; //0 IDLE, 1 MOVING


float angleTarget = 0.0f;
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
float aspectRatio;


//detaTime
float deltaTime;
float currentTime = 0.0f;
float lastTime = 0.0f;



int main(){
    if(!init())
        return -1;
    initScene();
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
    
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    SCREEN_WIDTH = mode->width;
    SCREEN_HEIGHT = mode->height;

    //Start projection attributes
    PROJECTION_WIDTH =  SCREEN_WIDTH;
    PROJECTION_HEIGHT = SCREEN_HEIGHT;
    PW2 = PROJECTION_WIDTH/2;
    PH2 = PROJECTION_HEIGHT/2;
    aspectRatio = PROJECTION_WIDTH/PROJECTION_HEIGHT;
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

    return true;
}

void end(){
    glfwDestroyWindow(window);
    glfwTerminate();
}

void initScene(){
    p.x =4.5*CELLSIZE; p.y = 4.5*CELLSIZE; p.a = 3*M_PI/2; p.dx = cos(p.a); p.dy = sin(p.a); p.state = IDLE;
    planeX = 0; planeY =  0.6;
    //Enemies
    sprites[0].x = 1.5*CELLSIZE; sprites[0].y = 1.5*CELLSIZE; sprites[0].tex = witch; sprites[0].visible = 1; sprites[0].width = 72; sprites[0].height = 89;
    sprites[1].x = 2.5*CELLSIZE; sprites[1].y = 1.5*CELLSIZE; sprites[1].tex = muffin; sprites[1].visible = 1; sprites[1].width = 40; sprites[1].height = 40;
    sprites[2].x = 3.5*CELLSIZE; sprites[2].y = 1.5*CELLSIZE; sprites[2].tex = muffin; sprites[2].visible = 1; sprites[2].width = 40; sprites[2].height = 40;
    sprites[3].x = 4.5*CELLSIZE; sprites[3].y = 1.5*CELLSIZE; sprites[3].tex = muffin; sprites[3].visible = 1; sprites[3].width = 40; sprites[3].height = 40;
    sprites[4].x = 5.5*CELLSIZE; sprites[4].y = 1.5*CELLSIZE; sprites[4].tex = muffin; sprites[4].visible = 1; sprites[4].width = 40; sprites[4].height = 40;
    enemy = &sprites[0]; enemyAiState = STILL; enemyState = IDLE;
}

void Update(){
    while(!glfwWindowShouldClose(window)){
	//calculate deltatime
	currentTime =  (float)glfwGetTime();
	deltaTime = currentTime - lastTime;
	lastTime = currentTime;
        ProcessInput();
        playerUpdate();
        enemyUpdate();
        //Draw on Screen
        display();
    }
}

void display(){
    glClearColor(0.0f,0.0f,0.0f,1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    drawPlayer();
    drawMap2D();
    drawAllSprites(sprites,nSprites);
    drawRays3D();
    glfwPollEvents();
    glfwSwapBuffers(window);
}

//Enemy functions
void enemyUpdate(){
    if(enemyAiState == FOLLOW_PLAYER){//Follow the player
        followPlayer();
    }
}

void followPlayer(){
    if(enemyState == IDLE){
        float currentWait = (float)glfwGetTime();
        if(currentWait-lastWait>=cooldown){
            lastWait = currentWait;
            VECTOR2I start = {(int)(enemy->x/CELLSIZE),(int)(enemy->y/CELLSIZE)};
            VECTOR2I end = {(int)(p.x/CELLSIZE),(int)(p.y/CELLSIZE)};
            //VECTOR2I end = {10,5};
            enemyNextPosition = BFS(&start,&end);
            te = 0.0f;
            ewPos[0] = (enemyNextPosition.x * CELLSIZE) + CELLSIZE/2; ewPos[1] = (enemyNextPosition.y * CELLSIZE) + CELLSIZE/2;
            enemyState = MOVING;
        }
    }
    else if(enemyState == MOVING){
        float speedFactor = 5.0f;
        enemy->x = enemy->x + (te * (ewPos[0] - enemy->x));
        enemy->y = enemy->y + (te * (ewPos[1] - enemy->y));
        te += 1.0f * deltaTime * speedFactor;
        if(te>=1.0f){ te = 0.0f; enemyState = IDLE;enemy->x = ewPos[0]; enemy->y = ewPos[1];}
    }
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
    int mx = 0, my = 0, dof = 0, r = 0, mapVal = 0; float dist = 0.0f; int maxDof = 200;
    ra = radiansAdjust(p.a - FOV/2);
    for(r = 0; r <= rays; r++){
        dof = 0;
        int hm = 0, vm = 0;
        //Horizontal lines 
        float hrx = p.x, hry = p.y;
        float distH = 10000.0f;
        if(ra == 0 || ra == M_PI){ ra+=0.00000001f;}
        if(ra > M_PI ){ hry = ((int)(p.y/CELLSIZE))*CELLSIZE - 0.0001f; hrx = p.x - (p.y - hry)/tan(ra); yo = -CELLSIZE; xo = -CELLSIZE/tan(ra); }//Looking up
        if(ra < M_PI ){ hry = ((int)(p.y/CELLSIZE))*CELLSIZE + CELLSIZE; hrx = p.x - (p.y - hry)/tan(ra); yo = CELLSIZE; xo = CELLSIZE/tan(ra); }//Looking down
            while(dof < maxDof){
                mx = (int)(hrx/CELLSIZE); my = (int)(hry/CELLSIZE); // get grid position o
                if(mx >= 0 && mx < mapWidth && my >= 0 && my < mapHeight){
                    if(walls[my][mx] > 0){ dof = maxDof; distH = distance(p.x,hrx,p.y,hry); hm = walls[my][mx]-1;}
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
        while(dof < maxDof){
                mx = (int)(vrx/CELLSIZE); my = (int)(vry/CELLSIZE); // get grid position o
                if(mx >= 0 && mx < mapWidth && my >= 0 && my < mapHeight){
                    if(walls[my][mx] > 0){ dof = maxDof; distV = distance(p.x,vrx,p.y,vry); vm = walls[my][mx]-1; }
                    else { vrx += xo; vry += yo; dof+=1;}
                }
                else { vrx += xo; vry += yo; dof+=1;}
        }
        float shade = 1.0f;
        //save the ray with the shortest distance
        if(distV < distH){dist=distV; rx = vrx; ry = vry; shade = 1.0f; mapVal = vm;}
        else{dist=distH; rx = hrx; ry = hry; shade = 0.5f; mapVal = hm;}
        float fog = 1;
        fog = fogFactor(dist);
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
                //if(pixel>=0 && pixel < TEXTURE*TEXTURE && pixel+1 < TEXTURE*TEXTURE && pixel+2 < TEXTURE*TEXTURE){
                    rgb[0] = wall[pixel+0] * shade; rgb[1] = wall[pixel+1] * shade; rgb[2] = wall[pixel+2] * shade;
                    glColor3ub(rgb[0] * fog,rgb[1] * fog,rgb[2] * fog); glPointSize(PIXELSCALE);
                    glBegin(GL_POINTS); glVertex3i(r*PIXELSCALE,y + lineOffset,0); glEnd();
                //}
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
            fog = fogFactor(distance(p.x,floorX,p.y,floorY));
            //get tex coords
                int rgb[3] = {0};
                if(iFloorX >= 0 && iFloorY >= 0 && iFloorX < mapWidth && iFloorY < mapHeight){
                    int fmapVal = floors[iFloorY][iFloorX];
                    int floorTx = (int)(floorX * TEXTURE  / (CELLSIZE))%TEXTURE; int floorTy = (int)(floorY * TEXTURE / (CELLSIZE))%TEXTURE;
                    int pixel = ((int)floorTy * TEXTURE + (int)floorTx) * 3 + (fmapVal*TEXTURE*TEXTURE*3);
                    //if(pixel>=0 && pixel < TEXTURE*TEXTURE && pixel+1 < TEXTURE*TEXTURE && pixel+2 < TEXTURE*TEXTURE){
                        rgb[0] = floorTex[pixel+0];
                        rgb[1] = floorTex[pixel+1] ;
                        rgb[2] = floorTex[pixel+2];
                        glColor3ub(rgb[0] * fog,rgb[1] * fog,rgb[2] * fog); glPointSize(PIXELSCALE);
                        glBegin(GL_POINTS); glVertex3i(r*PIXELSCALE,y,0); glEnd();
                    //}
                }
                //Draw ceiling
                int cmapVal = ceiling[iFloorY][iFloorX]-1;
                if(cmapVal >= 0 && iFloorX >= 0 && iFloorY >= 0 && iFloorX < mapWidth && iFloorY < mapHeight) { 
                    int floorTx = (int)(floorX * TEXTURE  / (CELLSIZE))%TEXTURE; int floorTy = (int)(floorY * TEXTURE / (CELLSIZE))%TEXTURE;
                    int pixel = ((int)floorTy * TEXTURE + (int)floorTx) * 3 + (cmapVal*TEXTURE*TEXTURE*3);
                    //if(pixel>=0 && pixel < TEXTURE*TEXTURE && pixel+1 < TEXTURE*TEXTURE && pixel+2 < TEXTURE*TEXTURE){
                        rgb[0] = texturedCeiling[pixel+0] * 0.7f;
                        rgb[1] = texturedCeiling[pixel+1] * 0.7f;
                        rgb[2] = texturedCeiling[pixel+2] * 0.7f;
                        glColor3ub(rgb[0] * fog,rgb[1] * fog,rgb[2] * fog); glPointSize(PIXELSCALE);
                        glBegin(GL_POINTS); glVertex3i(r*PIXELSCALE,topY - i,0); glEnd();
                    //}
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
    if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS){key.shift=1;}else{key.shift=0;}
}

void frameBufferSizeCallback(GLFWwindow* window,int w,int h){
    SCREEN_WIDTH = w;
    SCREEN_HEIGHT = h;
    PROJECTION_HEIGHT = SCREEN_HEIGHT;
    PH2 = PROJECTION_HEIGHT/2;
    glViewport(0,0,SCREEN_WIDTH,SCREEN_HEIGHT);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(0,SCREEN_WIDTH,SCREEN_HEIGHT,0,-2,2);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

}
//Player logic functions

void playerUpdate(){
    if(p.state == IDLE){
        playerInput(); //Detect player input 
    }
    else if(p.state == MOVING){
        movePlayer();
    }
    else if(p.state == ROTATING){
        rotateView();
    }
}
void playerInput(){
    if(key.shift==1){ pSpeed = 4.0f; rotSpeed = 6.0f;}
    else{ pSpeed = 2.0f; rotSpeed = 3.0f;}

    if(key.left == 1){ 
        p.t = 0.0f;
        p.state = ROTATING;
        angleTarget= p.a - 90.0f*M_PI/180.0f; //rotate 90 degrees to left
    }
    if(key.right == 1){ 
        p.t=0.0f;
        p.state = ROTATING;
        angleTarget = p.a + (90.0f*M_PI/180.0f); //rotate 90 degrees to right
    }
    int ipx = p.x/CELLSIZE, ipy = p.y/CELLSIZE;
    if(key.up == 1){
        float angle =p.a*(180.0f/M_PI);
        int dir[2] = {0,0};
        if((int)angle == 0) { dir[0] = 1;}
        else if((int)angle == 180) { dir[0] = -1;}
        else if((int)angle == 270) { dir[1] = -1;}
        else if((int)angle == 90) { dir[1] = 1;}
        if(walls[ipy + dir[1]][ipx+dir[0]] == 0){
            p.t = 0.0f;
            p.state = MOVING;
            p.wPos[0] = ((ipx + dir[0])*CELLSIZE) + (CELLSIZE/2); p.wPos[1] = ((ipy + dir[1])*CELLSIZE) + (CELLSIZE/2);
        }
    }
    if(key.down == 1){ 
        float angle =p.a*(180.0f/M_PI);
        int dir[2] = {0,0};
        if((int)angle == 0) { dir[0] = -1;}
        else if((int)angle == 180) { dir[0] = 1;}
        else if((int)angle == 270) { dir[1] = 1;}
        else if((int)angle == 90) { dir[1] = -1;}
        if(walls[ipy + dir[1]][ipx+dir[0]] == 0){
            p.t = 0.0f;
            p.state = MOVING;
            p.wPos[0] = ((ipx + dir[0])*CELLSIZE) + (CELLSIZE/2); p.wPos[1] = ((ipy + dir[1])*CELLSIZE) + (CELLSIZE/2);
        }
    }
}

void rotateView(){
    p.a = p.a + (p.t * (angleTarget - p.a));
    p.dx=cos(p.a); p.dy=sin(p.a);
    p.t += 1.0f * deltaTime * rotSpeed;
    if(p.t>=1.0f){ p.t = 0.0f; p.state = IDLE;p.a = angleTarget; p.dx=cos(p.a); p.dy=sin(p.a);p.a = radiansAdjust(p.a);}
}

void movePlayer(){
        p.x = p.x + (p.t * (p.wPos[0] - p.x));
        p.y = p.y + (p.t * (p.wPos[1] - p.y));
        p.t += 1.0f * deltaTime * pSpeed;
        if(p.t>=1.0f){ p.t = 0.0f; p.state = IDLE;p.x = p.wPos[0]; p.y = p.wPos[1];}
}

///Sprite drawing functions
void drawAllSprites(Sprite sprites[],int n){
    quickSort(sprites,&p,0,nSprites-1);
   //sort all sprites from mayor distance to less distance from player
   for(int i = 0; i < n; i++){
       if(sprites[i].visible == 1){//Draw if the sprite is on
                drawSprite(&sprites[i]);
                drawSpriteOnMap(&sprites[i]);
       }
   }
}
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
    int scalingFactor =  (708);
    int scaleX = (s->width*scalingFactor/ transformY);
    int spriteHeight = (s->height*scalingFactor/ transformY);
    int leftX = (spriteScreenX-scaleX/2), rightX = (spriteScreenX+scaleX/2);
    int topY   = (PH2 - spriteHeight / 2); int botY   = PH2 + spriteHeight / 2;
    float tx=0, ty=0, txStep = s->width/(float)(rightX-leftX), tyStep = s->height/(float)(botY-topY);
    if(transformY <= 45.5){return;}
    for(x = leftX; x < rightX; x+=PIXELSCALE){
        ty = 0;
        int sx = x;
        for(y = topY; y < botY; y++){
                if(sx>= 0 && sx <= PROJECTION_WIDTH && y < PROJECTION_HEIGHT){
                    int sy = topY;
                    float rgb[3] = {1.0};
                    int pixel = ((int)ty * s->width + (int)(tx)) * 3;
                    float fog = fogFactor(distance(p.x,s->x,p.y,s->y));
                    rgb[0] = s->tex[pixel+0]; rgb[1] = s->tex[pixel+1]; rgb[2] = s->tex[pixel+2]; 
                    ty+=tyStep; 
                    if(transformY > zDepth[sx/PIXELSCALE]){continue;}
                    if(rgb[0] == 255 && rgb[1] == 0 && rgb[2] == 255){continue;}
                    glColor3ub(rgb[0]*fog,rgb[1]*fog,rgb[2]*fog); 
                    glPointSize(PIXELSCALE);
                    glBegin(GL_POINTS); glVertex3i(x,y,1) ;glEnd();
                }
                
        }
        tx+=txStep*PIXELSCALE;
    }

}

