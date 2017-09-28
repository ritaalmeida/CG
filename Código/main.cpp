// Standard Includes
#include "RgbImage.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>


#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#define PI 3.1415926
#define SIZE 20 //room size is 20x20
#define MAX_ARROWS 200 //the maximum number of arrows will be this
#define MAX_ARROW_SPEED	0.5 //the maximum speed of the arrow will be this

//--------------------------------Globals------------------------------------
char const *window_name;
float windowWidth;
float windowHeight;


//View Globals
int fov;
float meshSize;
float textX;
float textY;
float x,y,z;

//Arrow Structure
typedef struct {
	int active;
	GLfloat  x, y, z, dx, dy, dz, arrow_phi;
}  Arrow;

//Arrow variables
int shoot = 0; // So we know when we are shooting
int hits = 0, shots = 0; // So we know how many arrows we shot and how many hit the target
Arrow arrows[MAX_ARROWS]; //Array of arrows

//Cube Rotation
GLfloat cubeRotation;
int mSecDelay=10;

//Check what light is on
int lightIsOn[5];

//Check if fog is on
int fogison[2];

//Eye Globals
GLfloat xi, yi, zi;
GLfloat xf, yf, zf;

GLfloat step;
GLfloat radius;

GLfloat angleh;
GLfloat anglev;

int mouseX, mouseY;

//Textures
GLuint textures[6];
RgbImage imag;


//Arrow
GLUquadricObj *Cylinder; //Cylinder
GLUquadricObj *Disk; // Disck



//----------------------------Colors-----------------------------------------
//Black without Shininess
GLfloat roomDefaultDiffuse[]={1,1,1,1};
GLfloat roomDefaultSpecular[]={0,0,0};
GLint roomDefaultShininess=0;

GLfloat GlassDiffuse []={ 0.1, 0.1, 0.1, 0.3};
GLfloat GlassSpecular []={ 1, 1, 1};
GLint GlassShininess = 1;

GLfloat rubyDiffuse[]={0.61424,0.04136,0.04136,1};
GLfloat rubySpecular[]={0.727811,0.626959,0.626959};
GLfloat rubyShininess=0.6*128;

GLfloat bluePlasticDiffuse[]={0.0,0.2,1,1};
GLfloat bluePlasticSpecular[]={0.50196078,0.50196078,1};
GLfloat bluePlasticShininess=0.9*128;

GLfloat defaultLightEmission[]={0.0f,0.0f,0.0f,1.0f};

//Lights Color
GLfloat corL[]={2.0f,2.0f,2.0f,1.0f};
GLfloat corP[]={1.5f,1.5f,1.5f,1.0f};

GLfloat nevoeiroCor[] = {0.75, 0.75, 0.75, 1.0};

//-----------------------GLOBAL INITIALIZATION-------------------------------
void initializeGlobals(){
	window_name = "GC Project - Tiro ao alvo";
	windowWidth = glutGet(GLUT_SCREEN_WIDTH);
	windowHeight = glutGet(GLUT_SCREEN_HEIGHT);
	
	fov=60; //field of view
	
	radius = 3.0f;
	angleh = PI/2;
	
	xi = 0.0f;
	yi = 2.7f;
	zi = 20.0f;
	
	xf = 0;
	yf = yi;
	zf = -20;
	
	step=0.25f;
	
	mouseX = windowWidth / 2;
	mouseY = windowHeight / 2;
	
	lightIsOn[0]=1;
	lightIsOn[1]=1;
	lightIsOn[2]=1;
	lightIsOn[3]=1;
	lightIsOn[4]=1;
	fogison[0]=0;
	fogison[1]=0;
	
	cubeRotation=45.0;
}

//--------------------------TEXT FUNCTIONS-----------------------------------
void drawText(char *string, GLfloat x, GLfloat y, GLfloat z) {
	glRasterPos3f(x, y, z);
	while (*string)
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *string++);
}

//--------------------------FPS STYLE CONTROLS-------------------------------
void updatePos() {
	xf = xi + radius * cos(angleh);
	zf = zi - radius * sin(angleh);
	glutPostRedisplay();
}
int frontierCheck(GLfloat x, GLfloat z) {
	//Lobby left and right wall and room A and B back wall
	if((x<-9.5 || x>9.5) && (z<29.5 && z>19.5))
		return 0;
	//Lobby back wall
	if(z>29.5)
		return 0;
	//rooms B C and D front wall
	if(z<-19.5)
		return 0;
	//rooms A and B left wall
	if(x<-29.5)
		return 0;
	//rooms D and E right wall
	if(x>29.5)
		return 0;
	//middle
	if(x>-10.5 && x<10.5 && z>-0.5 && z<10.5)
		return 0;
	//wall AB
	if(x<-19.5 && z>-0.5 && z<1)
		return 0;
	//wall BC
	if(z<-9.5 && x<-9 && x>-10.5)
		return 0;
	//wall CD
	if(z<-9.5 && x>9 && x<10.5)
		return 0;
	//wall DE
	if(x>19.5 && z>-0.5 && z<1)
		return 0;
	return 1;
}

//--------------------------Target Collision Check-------------------------------

int targetCheck(GLfloat x, GLfloat z) {
	//target room A
	if (x<-19.8 && x>-20 && z>9 && z < 11)
		return 0;
	//target room B
	if (x<-20.8 && x>-21 && z<-8 && z>-12)
		return 0;
	//target room C
	if (x<1 && x>-1 && z<-11.5 && z>-12)
		return 0;
	//target room D
	if (x > 18.8 && x < 19 && z<-9 && z>-11)
		return 0;
	//target room E
	if (x>18.8 && x < 19 && z>9 && z < 11)
		return 0;
	return 1;
}


void keyboardMovement(unsigned char key, int x, int y){
	if(key == 27) exit(0);
	if((key == 'w' || key == 'W') && frontierCheck(xi + step * cos(angleh), zi - step * sin(angleh))) {
		xi += step * cos(angleh);
		zi -= step * sin(angleh);
	} else if((key == 's' || key == 'S') && frontierCheck(xi - step * cos(angleh), zi + step * sin(angleh))) {
		xi -= step * cos(angleh);
		zi += step * sin(angleh);
	} else if((key == 'a' || key == 'A') && frontierCheck(xi + step * sin(-angleh), zi - step * cos(-angleh))) {
		xi += step * sin(-angleh);
		zi -= step * cos(-angleh);
	} else if((key == 'd' || key == 'D') && frontierCheck(xi - step * sin(-angleh), zi + step * cos(-angleh))) {
		xi -= step * sin(-angleh);
		zi += step * cos(-angleh);
	} else if(key == '1'){
		if(lightIsOn[0]){
			glDisable(GL_LIGHT1);
			lightIsOn[0]=0;
		} else{
			glEnable(GL_LIGHT1);
			lightIsOn[0]=1;
		}
	} else if(key == '2'){
		if(lightIsOn[1]){
			glDisable(GL_LIGHT2);
			lightIsOn[1]=0;
		} else{
			glEnable(GL_LIGHT2);
			lightIsOn[1]=1;
		}
		
	} else if(key == '3'){
		if(lightIsOn[2]){
			glDisable(GL_LIGHT3);
			lightIsOn[2]=0;
		} else{
			glEnable(GL_LIGHT3);
			lightIsOn[2]=1;
		}
	} else if(key == '4'){
		if(lightIsOn[3]){
			glDisable(GL_LIGHT4);
			lightIsOn[3]=0;
		} else{
			glEnable(GL_LIGHT4);
			lightIsOn[3]=1;
		}
	} else if(key == '5'){
		if(lightIsOn[4]){
			glDisable(GL_LIGHT5);
			lightIsOn[4]=0;
		} else{
			glEnable(GL_LIGHT5);
			lightIsOn[4]=1;
		}
	} else if (key=='6'){
		if(fogison[0]){
			glDisable(GL_FOG);
			fogison[0]=0;
		}
		else{
			fogison[0]=1;
		}
	} else if (key=='7'){
		if(fogison[1]){
			glDisable(GL_FOG);
			fogison[1]=0;
		}
		else{
			fogison[1]=1;
		}
	} else if(key == 'q' || key == 'Q'){
		if(corL[0]<10){
			corL[0]+=0.25;
			corL[1]+=0.25;
			corL[2]+=0.25;
		}
	} else if(key == 'e' || key == 'E'){
		if(corL[0]>0.25){
			corL[0]-=0.25;
			corL[1]-=0.25;
			corL[2]-=0.25;
		}
	}
	else if (key == ' ') {
		shoot = 1;
	}
	else if (key == 'r' || key == 'R')
	{
		for (int i = 0; i < MAX_ARROWS; i++)
		{
			if (arrows[i].active == 1)
				arrows[i].active = 0;
		}
	}
	updatePos();
}

void mouseMovement(int x, int y) {
	if(x >= windowWidth - 1 || x == 0 || y >= windowHeight - 1 || y == 0) {
		mouseX = windowWidth / 2;
		mouseY = windowHeight / 2;
		glutWarpPointer(windowWidth / 2, windowHeight / 2);
	} else {
		int diffx = x - mouseX;
		int diffy = y - mouseY;
		
		mouseX = x;
		mouseY = y;
		
		angleh -= (GLfloat) 0.002f * diffx;
		yf -= (GLfloat) 0.002f * diffy;
		
		updatePos();
	}
}
//-------------------------------TEXTURES------------------------------------
void defineTextures(){
	//--------------------------------Floor-------------------------------
	glGenTextures(1, &textures[0]);
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	imag.LoadBmpFile("floor.bmp");
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, (int)imag.GetNumCols(), (int)imag.GetNumRows(), 0, GL_RGB, GL_UNSIGNED_BYTE, imag.ImageData());
	//-------------------------------Wall----------------------------------
	glGenTextures(1, &textures[1]);
	glBindTexture(GL_TEXTURE_2D, textures[1]);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	imag.LoadBmpFile("wall.bmp");
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, (int)imag.GetNumCols(), (int)imag.GetNumRows(), 0, GL_RGB, GL_UNSIGNED_BYTE, imag.ImageData());
	//--------------------------------Footer-------------------------------------
	glGenTextures(1, &textures[2]);
	glBindTexture(GL_TEXTURE_2D, textures[2]);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	imag.LoadBmpFile("footer.bmp");
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, (int)imag.GetNumCols(), (int)imag.GetNumRows(), 0, GL_RGB, GL_UNSIGNED_BYTE, imag.ImageData());
	//--------------------------------Ceilling-----------------------------------
	glGenTextures(1, &textures[3]);
	glBindTexture(GL_TEXTURE_2D, textures[3]);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	imag.LoadBmpFile("ceilling.bmp");
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, (int)imag.GetNumCols(), (int)imag.GetNumRows(), 0, GL_RGB, GL_UNSIGNED_BYTE, imag.ImageData());
	
	//--------------------------------Target-----------------------------------
	glGenTextures(1, &textures[4]);
	glBindTexture(GL_TEXTURE_2D, textures[4]);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	imag.LoadBmpFile("target.bmp");
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
				 (int)imag.GetNumCols(),
				 (int)imag.GetNumRows(),
				 0, GL_RGB,GL_UNSIGNED_BYTE,
				 imag.ImageData());
	
}
//---------------------------------SCENE-------------------------------------
void illuminate(){
	
	//FOG
	if (fogison[0]){
		glEnable(GL_FOG);
		glFogf(GL_FOG_MODE,GL_EXP2);
		glFogfv(GL_FOG_COLOR,nevoeiroCor);
		glFogf(GL_FOG_DENSITY,0.1);
		glHint(GL_FOG_HINT,GL_NICEST);
		glFogf(GL_FOG_START,0);
		glFogf(GL_FOG_END,1);
	}
	if(fogison[1]){
		glEnable(GL_FOG);
		glFogf(GL_FOG_MODE,GL_EXP2);
		glFogfv(GL_FOG_COLOR,nevoeiroCor);
		glFogf(GL_FOG_DENSITY,0.05);
		glHint(GL_FOG_HINT,GL_NICEST);
		glFogf(GL_FOG_START,0);
		glFogf(GL_FOG_END,1);
	}
	
	//Lights Definition
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	if(lightIsOn[0])
		glEnable(GL_LIGHT1);
	if(lightIsOn[1])
		glEnable(GL_LIGHT2);
	if(lightIsOn[2])
		glEnable(GL_LIGHT3);
	if(lightIsOn[3])
		glEnable(GL_LIGHT4);
	if(lightIsOn[4])
		glEnable(GL_LIGHT5);
	//Lights Position
	GLfloat posL[]={0.0f,7.1f,5.0f,1.0f};
	GLfloat posA[]={-20.0f,7.0f,10.0f,1.0f};
	GLfloat posB[]={-20.0f,7.0f,-10.0f,1.0f};
	GLfloat posC[]={0.0f,7.0f,-10.0f,1.0f};
	GLfloat posD[]={20.0f,7.0f,-10.0f,1.0f};
	GLfloat posE[]={20.0f,7.0f,10.0f,1.0f};
	//Lights Spotlight
	GLfloat direction[]={0.0f,-1.0f,0.0f};
	GLfloat concentration=10;
	GLfloat lightAngle=90.0;
	//Lobby Light
	glLightfv(GL_LIGHT0, GL_POSITION, posL);
	glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 1.0f);
	glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.05f);
	glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.0f);
	glLightfv(GL_LIGHT0, GL_SPECULAR, corL);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, corL);
	//Room A Light
	glLightfv(GL_LIGHT1, GL_POSITION, posA);
	glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, direction );
	glLightf (GL_LIGHT1, GL_SPOT_EXPONENT , concentration);
	glLightf (GL_LIGHT1, GL_SPOT_CUTOFF, lightAngle);
	glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, 1.0f);
	glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, 0.05f);
	glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0.0f);
	glLightfv(GL_LIGHT1, GL_SPECULAR, corP);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, corP);
	//Room B Light
	glLightfv(GL_LIGHT2, GL_POSITION, posB);
	glLightfv(GL_LIGHT2, GL_SPOT_DIRECTION, direction );
	glLightf (GL_LIGHT2, GL_SPOT_EXPONENT , concentration);
	glLightf (GL_LIGHT2, GL_SPOT_CUTOFF, lightAngle);
	glLightf(GL_LIGHT2, GL_CONSTANT_ATTENUATION, 1.0f);
	glLightf(GL_LIGHT2, GL_LINEAR_ATTENUATION, 0.05f);
	glLightf(GL_LIGHT2, GL_QUADRATIC_ATTENUATION, 0.0f);
	glLightfv(GL_LIGHT2, GL_SPECULAR, corP);
	glLightfv(GL_LIGHT2, GL_DIFFUSE, corP);
	//Room C Light
	glLightfv(GL_LIGHT3, GL_POSITION, posC);
	glLightfv(GL_LIGHT3, GL_SPOT_DIRECTION, direction );
	glLightf (GL_LIGHT3, GL_SPOT_EXPONENT , concentration);
	glLightf (GL_LIGHT3, GL_SPOT_CUTOFF, lightAngle);
	glLightf(GL_LIGHT3, GL_CONSTANT_ATTENUATION, 1.0f);
	glLightf(GL_LIGHT3, GL_LINEAR_ATTENUATION, 0.05f);
	glLightf(GL_LIGHT3, GL_QUADRATIC_ATTENUATION, 0.0f);
	glLightfv(GL_LIGHT3, GL_SPECULAR, corP);
	glLightfv(GL_LIGHT3, GL_DIFFUSE, corP);
	//Room D Light
	glLightfv(GL_LIGHT4, GL_POSITION, posD);
	glLightfv(GL_LIGHT4, GL_SPOT_DIRECTION, direction );
	glLightf (GL_LIGHT4, GL_SPOT_EXPONENT , concentration);
	glLightf (GL_LIGHT4, GL_SPOT_CUTOFF, lightAngle);
	glLightf(GL_LIGHT4, GL_CONSTANT_ATTENUATION, 1.0f);
	glLightf(GL_LIGHT4, GL_LINEAR_ATTENUATION, 0.05f);
	glLightf(GL_LIGHT4, GL_QUADRATIC_ATTENUATION, 0.0f);
	glLightfv(GL_LIGHT4, GL_SPECULAR, corP);
	glLightfv(GL_LIGHT4, GL_DIFFUSE, corP);
	//Room E Light
	glLightfv(GL_LIGHT5, GL_POSITION, posE);
	glLightfv(GL_LIGHT5, GL_SPOT_DIRECTION, direction );
	glLightf (GL_LIGHT5, GL_SPOT_EXPONENT , concentration);
	glLightf (GL_LIGHT5, GL_SPOT_CUTOFF, lightAngle);
	glLightf(GL_LIGHT5, GL_CONSTANT_ATTENUATION, 1.0f);
	glLightf(GL_LIGHT5, GL_LINEAR_ATTENUATION, 0.05f);
	glLightf(GL_LIGHT5, GL_QUADRATIC_ATTENUATION, 0.0f);
	glLightfv(GL_LIGHT5, GL_SPECULAR, corP);
	glLightfv(GL_LIGHT5, GL_DIFFUSE, corP);
}

void quad(GLfloat size) {
	
	GLfloat a[3] = { -size, size, size };
	GLfloat b[3] = { -size, -size, size };
	GLfloat e[3] = { -size, size, -size };
	GLfloat f[3] = { -size, -size, -size };
	
	// right quads, room A and B
	glPushMatrix();
	glBegin(GL_QUADS);
	glNormal3f(-1.0, 0.0, 0.0);
	glTexCoord2f(0.0f, 1.0f);  glVertex3fv(e);
	glTexCoord2f(0.0f, 0.0f);  glVertex3fv(f);
	glTexCoord2f(1.0f, 0.0f);  glVertex3fv(b);
	glTexCoord2f(1.0f, 1.0f);  glVertex3fv(a);
	glEnd();
	glPopMatrix();
	
	// left quads
	glBegin(GL_QUADS);
	glNormal3f(1.0, 0.0, 0.0);
	glTexCoord2f(1.0f, 0.0f);  glVertex3fv(a);
	glTexCoord2f(1.0f, 1.0f);  glVertex3fv(b);
	glTexCoord2f(0.0f, 1.0f);  glVertex3fv(f);
	glTexCoord2f(0.0f, 0.0f);  glVertex3fv(e);
	glEnd();
}

void cube(GLfloat size) {
	
	GLfloat a[3] = { -size, size, size };
	GLfloat b[3] = { -size, -size, size };
	GLfloat c[3] = { size, -size, size };
	GLfloat d[3] = { size, size, size };
	GLfloat e[3] = { -size, size, -size };
	GLfloat f[3] = { -size, -size, -size };
	GLfloat g[3] = { size, -size, -size };
	GLfloat h[3] = { size, size, -size };
	
	// Front Face
	glBegin(GL_QUADS);
	glNormal3f(0.0, 0.0, 1.0);
	glVertex3fv(a);
	glVertex3fv(b);
	glVertex3fv(c);
	glVertex3fv(d);
	glEnd();
	
	// Back Face
	glBegin(GL_QUADS);
	glNormal3f(0.0, 0.0, -1.0);
	glVertex3fv(e);
	glVertex3fv(h);
	glVertex3fv(g);
	glVertex3fv(f);
	glEnd();
	
	// Left Face
	glBegin(GL_QUADS);
	glNormal3f(-1.0, 0.0, 0.0);
	glVertex3fv(e);
	glVertex3fv(f);
	glVertex3fv(b);
	glVertex3fv(a);
	glEnd();
	
	
	// Right Face
	glBegin(GL_QUADS);
	glNormal3f(1.0, 0.0, 0.0);
	glVertex3fv(d);
	glVertex3fv(c);
	glVertex3fv(g);
	glVertex3fv(h);
	glEnd();
	
	// Top Face
	glBegin(GL_QUADS);
	glNormal3f(0.0, 1.0, 0.0);
	glVertex3fv(a);
	glVertex3fv(d);
	glVertex3fv(h);
	glVertex3fv(e);
	glEnd();
	
	// Bottom Face
	glBegin(GL_QUADS);
	glNormal3f(0.0, -1.0, 0.0);
	glVertex3fv(b);
	glVertex3fv(f);
	glVertex3fv(g);
	glVertex3fv(c);
	glEnd();
}

void middle(){
	//Floor CCW
	glColor3f(1.0, 1.0, 1.0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,textures[0]);
	glBegin(GL_QUADS);
	glNormal3f(0.0, 1.0, 0.0);
	textX=2.0/SIZE; //replicate 2 times on the x axis
	textY=2.0/SIZE; //replicate 2 times on the y axis
	meshSize=1;
	y=0.0;
	for(z=10.0;z>0.0;z-=meshSize){
		for(x=-10.0;x<10.0;x+=meshSize){
			glTexCoord2f(textX*x,textY*z);                          glVertex3f(x,y,z);
			glTexCoord2f(textX*(x+meshSize),textY*z);               glVertex3f(x+meshSize,y,z);
			glTexCoord2f(textX*(x+meshSize),textY*(z-meshSize));    glVertex3f(x+meshSize,y,z-meshSize);
			glTexCoord2f(textX*x,textY*(z-meshSize));               glVertex3f(x,y,z-meshSize);
		}
	}
	glEnd();
	glDisable(GL_TEXTURE_2D);

	glPushMatrix();
	glTranslatef(5,0,5);
	glScalef(0.25,0.25,0.25);
	glPopMatrix();
	glMaterialfv(GL_FRONT, GL_DIFFUSE, bluePlasticDiffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, bluePlasticSpecular);
	glMaterialf(GL_FRONT, GL_SHININESS, bluePlasticShininess);
	glPushMatrix();
	
	glTranslatef(-5, 2.5, 5);
	glRotatef(cubeRotation,1,1,1);
	cube(0.5);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, GlassDiffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, GlassSpecular);
	cube(1.0);
 
	glPopMatrix();
	glMaterialfv(GL_FRONT, GL_DIFFUSE, rubyDiffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, rubySpecular);
	glMaterialf(GL_FRONT, GL_SHININESS, rubyShininess);
	glPushMatrix();
	glTranslatef(5, 2.5, 5);
	glRotatef(cubeRotation,1,1,1);
	cube(0.5);
	
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, GlassDiffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, GlassSpecular);
	
	cube(1.0);
	glPopMatrix();
	glMaterialfv(GL_FRONT, GL_DIFFUSE,  roomDefaultDiffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, roomDefaultSpecular);
	glMaterialf(GL_FRONT, GL_SHININESS, roomDefaultShininess);
	//Ceilling CW
	glColor3f(1.0, 1.0, 1.0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,textures[3]);
	glBegin(GL_QUADS);
	glNormal3f(0.0, -1.0, 0.0);
	textX=4.0/SIZE; //replicate 4 times on the x axis
	textY=4.0/SIZE; //replicate 4 times on the y axis
	meshSize=1;
	y=7.5;
	for(z=10.0;z>0.0;z-=meshSize){
		for(x=-10.0;x<10.0;x+=meshSize){
			glTexCoord2f(textX*x,textY*z); glVertex3f(x,y,z);
			glTexCoord2f(textX*x,textY*(z-meshSize)); glVertex3f(x,y,z-meshSize);
			glTexCoord2f(textX*(x+meshSize),textY*(z-meshSize)); glVertex3f(x+meshSize,y,z-meshSize);
			glTexCoord2f(textX*(x+meshSize),textY*z); glVertex3f(x+meshSize,y,z);
		}
	}
	glEnd();
	glDisable(GL_TEXTURE_2D);
	glPushMatrix();
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, corL);
	glTranslatef(0,7.5,5);
	glutSolidSphere(0.5,15,15);
	
	
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, defaultLightEmission);
	glPopMatrix();
	
	//Back Wall CCW
	glDisable(GL_CULL_FACE);
	glDepthMask(GL_FALSE);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, GlassDiffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, GlassSpecular);
	glMaterialf (GL_FRONT_AND_BACK, GL_SHININESS, GlassShininess);
	glBegin(GL_QUADS);
	glNormal3f(0.0, 0.0, 1.0);
	textX=20.0/SIZE;
	textY=40.0/SIZE;
	meshSize=1;
	z=10.0;
	for(y=0.0;y<7.5;y+=meshSize){
		for(x=-10.0;x<10.0;x+=meshSize){
			glVertex3f(x,y,z);
			glVertex3f(x,y+meshSize,z);
			glVertex3f(x+meshSize,y+meshSize,z);
			glVertex3f(x+meshSize,y,z);
		}
	}
	glEnd();
	//Left Wall
	glBegin(GL_QUADS);
	glNormal3f(-1.0, 0.0, 0.0);
	textX=20.0/SIZE;
	textY=40.0/SIZE;
	meshSize=1;
	x=-10.0;
	for(y=0.0;y<7.5;y+=meshSize){
		for(z=0.0;z<10.0;z+=meshSize){
			glVertex3f(x,y,z);
			glVertex3f(x,y,z+meshSize);
			glVertex3f(x,y+meshSize,z+meshSize);
			glVertex3f(x,y+meshSize,z);
		}
	}
	glEnd();
	//Front Wall
	glBegin(GL_QUADS);
	glNormal3f(0.0, 0.0, -1.0);
	textX=40.0/SIZE;
	textY=40.0/SIZE;
	meshSize=1;
	z=0.0;
	for(y=0.0;y<7.5;y+=meshSize){
		for(x=-10.0;x<10.0;x+=meshSize){
			glVertex3f(x,y,z);
			glVertex3f(x,y+meshSize,z);
			glVertex3f(x+meshSize,y+meshSize,z);
			glVertex3f(x+meshSize,y,z);
		}
	}
	glEnd();
	//Right Wall
	glBegin(GL_QUADS);
	glNormal3f(1.0, 0.0, 0.0);
	textX=20.0/SIZE;
	textY=40.0/SIZE;
	meshSize=1;
	x=10.0;
	for(y=0.0;y<7.5;y+=meshSize){
		for(z=0.0;z<10.0;z+=meshSize){
			glVertex3f(x,y,z);
			glVertex3f(x,y+meshSize,z);
			glVertex3f(x,y+meshSize,z+meshSize);
			glVertex3f(x,y,z+meshSize);
		}
	}
	glEnd();
	glDepthMask(GL_TRUE);
	glEnable(GL_CULL_FACE);
	glMaterialfv(GL_FRONT, GL_DIFFUSE,  roomDefaultDiffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, roomDefaultSpecular);
	glMaterialf(GL_FRONT, GL_SHININESS, roomDefaultShininess);
}

void lobby(){
	//Floor CCW
	glColor3f(1.0, 1.0, 1.0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,textures[0]);
	glBegin(GL_QUADS);
	glNormal3f(0.0, 1.0, 0.0);
	textX=2.0/SIZE; //replicate 2 times on the x axis
	textY=2.0/SIZE; //replicate 2 times on the y axis
	meshSize=1;
	y=0.0;
	for(z=30.0;z>10.0;z-=meshSize){
		for(x=-10.0;x<10.0;x+=meshSize){
			glTexCoord2f(textX*x,textY*z); glVertex3f(x,y,z);
			glTexCoord2f(textX*(x+meshSize),textY*z); glVertex3f(x+meshSize,y,z);
			glTexCoord2f(textX*(x+meshSize),textY*(z-meshSize)); glVertex3f(x+meshSize,y,z-meshSize);
			glTexCoord2f(textX*x,textY*(z-meshSize)); glVertex3f(x,y,z-meshSize);
		}
	}
	glEnd();
	glDisable(GL_TEXTURE_2D);
	//Ceilling CW
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,textures[3]);
	glBegin(GL_QUADS);
	glNormal3f(0.0, -1.0, 0.0);
	textX=4.0/SIZE; //replicate 4 times on the x axis
	textY=4.0/SIZE; //replicate 4 times on the y axis
	meshSize=1;
	y=7.5;
	for(z=30.0;z>10.0;z-=meshSize){
		for(x=-10.0;x<10.0;x+=meshSize){
			glTexCoord2f(textX*x,textY*z); glVertex3f(x,y,z);
			glTexCoord2f(textX*x,textY*(z-meshSize)); glVertex3f(x,y,z-meshSize);
			glTexCoord2f(textX*(x+meshSize),textY*(z-meshSize)); glVertex3f(x+meshSize,y,z-meshSize);
			glTexCoord2f(textX*(x+meshSize),textY*z); glVertex3f(x+meshSize,y,z);
		}
	}
	glEnd();
	glDisable(GL_TEXTURE_2D);
	//Back Wall CW
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,textures[1]);
	glBegin(GL_QUADS);
	glNormal3f(0.0, 0.0, -1.0);
	textX=40.0/SIZE;
	textY=40.0/SIZE;
	meshSize=1;
	z=30.0;
	for(y=0.50;y<7.0;y+=meshSize){
		for(x=-10.0;x<10.0;x+=meshSize){
			glTexCoord2f(textX*x,textY*y); glVertex3f(x,y,z);
			glTexCoord2f(textX*x,textY*(y+meshSize)); glVertex3f(x,y+meshSize,z);
			glTexCoord2f(textX*(x+meshSize),textY*(y+meshSize)); glVertex3f(x+meshSize,y+meshSize,z);
			glTexCoord2f(textX*(x+meshSize),textY*y); glVertex3f(x+meshSize,y,z);
		}
	}
	glEnd();
	glDisable(GL_TEXTURE_2D);
	//Back Footer CW
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,textures[2]);
	glBegin(GL_QUADS);
	glNormal3f(0.0, 0.0, -1.0);
	textX=20.0/SIZE;
	textY=40.0/SIZE;
	meshSize=1;
	z=30.0;
	y=0.0;
	for(x=-10.0;x<10.0;x+=meshSize){
		glTexCoord2f(textX*x,textY*y); glVertex3f(x,y,z);
		glTexCoord2f(textX*x,textY*(y+0.5)); glVertex3f(x,y+0.5,z);
		glTexCoord2f(textX*(x+meshSize),textY*(y+0.5)); glVertex3f(x+meshSize,y+0.5,z);
		glTexCoord2f(textX*(x+meshSize),textY*y); glVertex3f(x+meshSize,y,z);
	}
	glEnd();
	glDisable(GL_TEXTURE_2D);
	//Left Wall CW
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,textures[1]);
	glBegin(GL_QUADS);
	glNormal3f(1.0, 0.0, 0.0);
	textX=20.0/SIZE;
	textY=40.0/SIZE;
	meshSize=1;
	x=-10.0;
	for(y=0.5;y<7.0;y+=meshSize){
		for(z=20.0;z<30.0;z+=meshSize){
			glTexCoord2f(textX*z,textY*y); glVertex3f(x,y,z);
			glTexCoord2f(textX*z,textY*(y+meshSize)); glVertex3f(x,y+meshSize,z);
			glTexCoord2f(textX*(z+meshSize),textY*(y+meshSize)); glVertex3f(x,y+meshSize,z+meshSize);
			glTexCoord2f(textX*(z+meshSize),textY*y); glVertex3f(x,y,z+meshSize);
		}
	}
	glEnd();
	glDisable(GL_TEXTURE_2D);
	//Left Footer CW
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,textures[2]);
	glBegin(GL_QUADS);
	glNormal3f(1.0, 0.0, 0.0);
	textX=20.0/SIZE;
	textY=40.0/SIZE;
	meshSize=1;
	x=-10.0;
	y=0.0;
	for(z=20.0;z<30.0;z+=meshSize){
		glTexCoord2f(textX*z,textY*y); glVertex3f(x,y,z);
		glTexCoord2f(textX*z,textY*(y+0.5)); glVertex3f(x,y+0.5,z);
		glTexCoord2f(textX*(z+meshSize),textY*(y+0.5)); glVertex3f(x,y+0.5,z+meshSize);
		glTexCoord2f(textX*(z+meshSize),textY*y); glVertex3f(x,y,z+meshSize);
	}
	glEnd();
	glDisable(GL_TEXTURE_2D);
	//Right Wall CCW
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,textures[1]);
	glBegin(GL_QUADS);
	glNormal3f(-1.0, 0.0, 0.0);
	textX=20.0/SIZE;
	textY=40.0/SIZE;
	meshSize=1;
	x=10.0;
	for(y=0.5;y<7.0;y+=meshSize){
		for(z=20.0;z<30.0;z+=meshSize){
			glTexCoord2f(textX*z,textY*y); glVertex3f(x,y,z);
			glTexCoord2f(textX*(z+meshSize),textY*y); glVertex3f(x,y,z+meshSize);
			glTexCoord2f(textX*(z+meshSize),textY*(y+meshSize)); glVertex3f(x,y+meshSize,z+meshSize);
			glTexCoord2f(textX*z,textY*(y+meshSize)); glVertex3f(x,y+meshSize,z);
		}
	}
	glEnd();
	glDisable(GL_TEXTURE_2D);
	//Right Footer CCW
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,textures[2]);
	glBegin(GL_QUADS);
	glNormal3f(-1.0, 0.0, 0.0);
	textX=20.0/SIZE;
	textY=40.0/SIZE;
	meshSize=1;
	x=10.0;
	y=0.0;
	for(z=20.0;z<30.0;z+=meshSize){
		glTexCoord2f(textX*z,textY*y); glVertex3f(x,y,z);
		glTexCoord2f(textX*(z+meshSize),textY*y); glVertex3f(x,y,z+meshSize);
		glTexCoord2f(textX*(z+meshSize),textY*(y+0.5)); glVertex3f(x,y+0.5,z+meshSize);
		glTexCoord2f(textX*z,textY*(y+0.5)); glVertex3f(x,y+0.5,z);
	}
	glEnd();
	glDisable(GL_TEXTURE_2D);
}

void roomA(){
	//Floor
	glColor3f(1.0, 1.0, 1.0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,textures[0]);
	glBegin(GL_QUADS);
	glNormal3f(0.0, 1.0, 0.0);
	textX=2.0/SIZE; //replicate 2 times on the x axis
	textY=2.0/SIZE; //replicate 2 times on the y axis
	meshSize=1;
	y=0.0;
	for(z=20.0;z>0.0;z-=meshSize){
		for(x=-30.0;x<-10.0;x+=meshSize){
			glTexCoord2f(textX*x,textY*z); glVertex3f(x,y,z);
			glTexCoord2f(textX*(x+meshSize),textY*z); glVertex3f(x+meshSize,y,z);
			glTexCoord2f(textX*(x+meshSize),textY*(z-meshSize)); glVertex3f(x+meshSize,y,z-meshSize);
			glTexCoord2f(textX*x,textY*(z-meshSize)); glVertex3f(x,y,z-meshSize);
		}
	}
	glEnd();
	glDisable(GL_TEXTURE_2D);
	
	glPushMatrix();
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textures[4]);
	glTranslatef(-19,2.5,10.0);
	glRotatef(180, 1.0, 0.0, 0.0);
	quad(1.0);
	glPopMatrix();
	glDisable(GL_TEXTURE_2D);
	glMaterialfv(GL_FRONT, GL_DIFFUSE,  roomDefaultDiffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, roomDefaultSpecular);
	glMaterialf(GL_FRONT, GL_SHININESS, roomDefaultShininess);
	glDisable(GL_TEXTURE_2D);
	//Ceilling
	glColor3f(1.0, 1.0, 1.0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,textures[3]);
	glBegin(GL_QUADS);
	glNormal3f(0.0, -1.0, 0.0);
	textX=4.0/SIZE; //replicate 4 times on the x axis
	textY=4.0/SIZE; //replicate 4 times on the y axis
	meshSize=1;
	y=7.5;
	for(z=20.0;z>0.0;z-=meshSize){
		for(x=-30.0;x<-10.0;x+=meshSize){
			glTexCoord2f(textX*x,textY*z); glVertex3f(x,y,z);
			glTexCoord2f(textX*x,textY*(z-meshSize)); glVertex3f(x,y,z-meshSize);
			glTexCoord2f(textX*(x+meshSize),textY*(z-meshSize)); glVertex3f(x+meshSize,y,z-meshSize);
			glTexCoord2f(textX*(x+meshSize),textY*z); glVertex3f(x+meshSize,y,z);
		}
	}
	glEnd();
	glDisable(GL_TEXTURE_2D);
	glPushMatrix();
	if(lightIsOn[0])
		glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, corP);
	glTranslatef(-20,7.5,10);
	glutSolidSphere(0.5,15,15);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, defaultLightEmission);
	glPopMatrix();
	//Back Wall
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,textures[1]);
	glBegin(GL_QUADS);
	glNormal3f(0.0, 0.0, -1.0);
	textX=40.0/SIZE;
	textY=40.0/SIZE;
	meshSize=1;
	z=20.0;
	for(y=0.50;y<7.0;y+=meshSize){
		for(x=-30.0;x<-10.0;x+=meshSize){
			glTexCoord2f(textX*x,textY*y); glVertex3f(x,y,z);
			glTexCoord2f(textX*x,textY*(y+meshSize)); glVertex3f(x,y+meshSize,z);
			glTexCoord2f(textX*(x+meshSize),textY*(y+meshSize)); glVertex3f(x+meshSize,y+meshSize,z);
			glTexCoord2f(textX*(x+meshSize),textY*y); glVertex3f(x+meshSize,y,z);
		}
	}
	glEnd();
	glDisable(GL_TEXTURE_2D);
	//Back Footer
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,textures[2]);
	glBegin(GL_QUADS);
	glNormal3f(0.0, 0.0, -1.0);
	textX=20.0/SIZE;
	textY=40.0/SIZE;
	meshSize=1;
	z=20.0;
	y=0.0;
	for(x=-30.0;x<-10.0;x+=meshSize){
		glTexCoord2f(textX*x,textY*y); glVertex3f(x,y,z);
		glTexCoord2f(textX*x,textY*(y+0.5)); glVertex3f(x,y+0.5,z);
		glTexCoord2f(textX*(x+meshSize),textY*(y+0.5)); glVertex3f(x+meshSize,y+0.5,z);
		glTexCoord2f(textX*(x+meshSize),textY*y); glVertex3f(x+meshSize,y,z);
	}
	glEnd();
	glDisable(GL_TEXTURE_2D);
	//Left Wall
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,textures[1]);
	glBegin(GL_QUADS);
	glNormal3f(1.0, 0.0, 0.0);
	textX=20.0/SIZE;
	textY=40.0/SIZE;
	meshSize=1;
	x=-30.0;
	for(y=0.5;y<7.0;y+=meshSize){
		for(z=0.0;z<20.0;z+=meshSize){
			glTexCoord2f(textX*z,textY*y); glVertex3f(x,y,z);
			glTexCoord2f(textX*z,textY*(y+meshSize)); glVertex3f(x,y+meshSize,z);
			glTexCoord2f(textX*(z+meshSize),textY*(y+meshSize)); glVertex3f(x,y+meshSize,z+meshSize);
			glTexCoord2f(textX*(z+meshSize),textY*y); glVertex3f(x,y,z+meshSize);
		}
	}
	glEnd();
	glDisable(GL_TEXTURE_2D);
	//Left Footer
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,textures[2]);
	glBegin(GL_QUADS);
	glNormal3f(1.0, 0.0, 0.0);
	textX=20.0/SIZE;
	textY=40.0/SIZE;
	meshSize=1;
	x=-30.0;
	y=0.0;
	for(z=0.0;z<20.0;z+=meshSize){
		glTexCoord2f(textX*z,textY*y); glVertex3f(x,y,z);
		glTexCoord2f(textX*z,textY*(y+0.5)); glVertex3f(x,y+0.5,z);
		glTexCoord2f(textX*(z+meshSize),textY*(y+0.5)); glVertex3f(x,y+0.5,z+meshSize);
		glTexCoord2f(textX*(z+meshSize),textY*y); glVertex3f(x,y,z+meshSize);
	}
	glEnd();
	glDisable(GL_TEXTURE_2D);
	
}

void roomB(){
	//Floor
	glColor3f(1.0, 1.0, 1.0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,textures[0]);
	glBegin(GL_QUADS);
	glNormal3f(0.0, 1.0, 0.0);
	textX=2.0/SIZE; //replicate 2 times on the x axis
	textY=2.0/SIZE; //replicate 2 times on the y axis
	meshSize=1;
	y=0.0;
	for(z=0.0;z>-20.0;z-=meshSize){
		for(x=-30.0;x<-10.0;x+=meshSize){
			glTexCoord2f(textX*x,textY*z); glVertex3f(x,y,z);
			glTexCoord2f(textX*(x+meshSize),textY*z); glVertex3f(x+meshSize,y,z);
			glTexCoord2f(textX*(x+meshSize),textY*(z-meshSize)); glVertex3f(x+meshSize,y,z-meshSize);
			glTexCoord2f(textX*x,textY*(z-meshSize)); glVertex3f(x,y,z-meshSize);
		}
	}
	glEnd();
	
	glPushMatrix();
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textures[4]);
	glTranslatef(-20.0,2.5,-10.0);
	glRotatef(180, 1.0, 0.0, 0.0);
	
	quad(1.0);
	glPopMatrix();
	glMaterialfv(GL_FRONT, GL_DIFFUSE,  roomDefaultDiffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, roomDefaultSpecular);
	glMaterialf(GL_FRONT, GL_SHININESS, roomDefaultShininess);
	glDisable(GL_TEXTURE_2D);
	//Ceilling
	glColor3f(1.0, 1.0, 1.0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,textures[3]);
	glBegin(GL_QUADS);
	glNormal3f(0.0, -1.0, 0.0);
	textX=4.0/SIZE; //replicate 4 times on the x axis
	textY=4.0/SIZE; //replicate 4 times on the y axis
	meshSize=1;
	y=7.5;
	for(z=0.0;z>-20.0;z-=meshSize){
		for(x=-30.0;x<-10.0;x+=meshSize){
			glTexCoord2f(textX*x,textY*z); glVertex3f(x,y,z);
			glTexCoord2f(textX*x,textY*(z-meshSize)); glVertex3f(x,y,z-meshSize);
			glTexCoord2f(textX*(x+meshSize),textY*(z-meshSize)); glVertex3f(x+meshSize,y,z-meshSize);
			glTexCoord2f(textX*(x+meshSize),textY*z); glVertex3f(x+meshSize,y,z);
		}
	}
	glEnd();
	glDisable(GL_TEXTURE_2D);
	glPushMatrix();
	if(lightIsOn[1])
		glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, corP);
	glTranslatef(-20,7.5,-10);
	glutSolidSphere(0.5,15,15);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, defaultLightEmission);
	glPopMatrix();
	//Left Wall
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,textures[1]);
	glBegin(GL_QUADS);
	glNormal3f(1.0, 0.0, 0.0);
	textX=20.0/SIZE;
	textY=40.0/SIZE;
	meshSize=1;
	x=-30.0;
	for(y=0.5;y<7.0;y+=meshSize){
		for(z=-20.0;z<0.0;z+=meshSize){
			glTexCoord2f(textX*z,textY*y); glVertex3f(x,y,z);
			glTexCoord2f(textX*z,textY*(y+meshSize)); glVertex3f(x,y+meshSize,z);
			glTexCoord2f(textX*(z+meshSize),textY*(y+meshSize)); glVertex3f(x,y+meshSize,z+meshSize);
			glTexCoord2f(textX*(z+meshSize),textY*y); glVertex3f(x,y,z+meshSize);
		}
	}
	glEnd();
	glDisable(GL_TEXTURE_2D);
	//Left Footer
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,textures[2]);
	glBegin(GL_QUADS);
	glNormal3f(1.0, 0.0, 0.0);
	textX=20.0/SIZE;
	textY=40.0/SIZE;
	meshSize=1;
	x=-30.0;
	y=0.0;
	for(z=-20.0;z<0.0;z+=meshSize){
		glTexCoord2f(textX*z,textY*y); glVertex3f(x,y,z);
		glTexCoord2f(textX*z,textY*(y+0.5)); glVertex3f(x,y+0.5,z);
		glTexCoord2f(textX*(z+meshSize),textY*(y+0.5)); glVertex3f(x,y+0.5,z+meshSize);
		glTexCoord2f(textX*(z+meshSize),textY*y); glVertex3f(x,y,z+meshSize);
	}
	glEnd();
	glDisable(GL_TEXTURE_2D);
	//Front Wall
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,textures[1]);
	glBegin(GL_QUADS);
	glNormal3f(0.0, 0.0, 1.0);
	textX=20.0/SIZE;
	textY=40.0/SIZE;
	meshSize=1;
	z=-20.0;
	for(y=0.5;y<7.5;y+=meshSize){
		for(x=-30.0;x<-10.0;x+=meshSize){
			glTexCoord2f(textX*x,textY*y); glVertex3f(x,y,z);
			glTexCoord2f(textX*(x+meshSize),textY*y); glVertex3f(x+meshSize,y,z);
			glTexCoord2f(textX*(x+meshSize),textY*(y+meshSize)); glVertex3f(x+meshSize,y+meshSize,z);
			glTexCoord2f(textX*x,textY*(y+meshSize)); glVertex3f(x,y+meshSize,z);
		}
	}
	glEnd();
	glDisable(GL_TEXTURE_2D);
	//Front Footer
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,textures[2]);
	glBegin(GL_QUADS);
	glNormal3f(0.0, 0.0, 1.0);
	textX=20.0/SIZE;
	textY=40.0/SIZE;
	meshSize=1;
	z=-20.0;
	y=0.0;
	for(x=-30.0;x<-10.0;x+=meshSize){
		glTexCoord2f(textX*x,textY*y); glVertex3f(x,y,z);
		glTexCoord2f(textX*(x+meshSize),textY*y); glVertex3f(x+meshSize,y,z);
		glTexCoord2f(textX*(x+meshSize),textY*(y+0.5)); glVertex3f(x+meshSize,y+0.5,z);
		glTexCoord2f(textX*x,textY*(y+0.5)); glVertex3f(x,y+0.5,z);
	}
	glEnd();
	glDisable(GL_TEXTURE_2D);
}

void roomC(){
	//Floor
	glColor3f(1.0, 1.0, 1.0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,textures[0]);
	glBegin(GL_QUADS);
	glNormal3f(0.0, 1.0, 0.0);
	textX=2.0/SIZE; //replicate 2 times on the x axis
	textY=2.0/SIZE; //replicate 2 times on the y axis
	meshSize=1;
	y=0.0;
	for(z=0.0;z>-20.0;z-=meshSize){
		for(x=-10.0;x<10.0;x+=meshSize){
			glTexCoord2f(textX*x,textY*z); glVertex3f(x,y,z);
			glTexCoord2f(textX*(x+meshSize),textY*z); glVertex3f(x+meshSize,y,z);
			glTexCoord2f(textX*(x+meshSize),textY*(z-meshSize)); glVertex3f(x+meshSize,y,z-meshSize);
			glTexCoord2f(textX*x,textY*(z-meshSize)); glVertex3f(x,y,z-meshSize);
		}
	}
	glEnd();
	glDisable(GL_TEXTURE_2D);
	//TARGET
	
	glPushMatrix();

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textures[4]);
	glTranslatef(0.0,2.5,-12.5);
	glRotatef(90.0, 0.0, 1.0, 0.0);
	quad(1.0);
	glPopMatrix();
	
	glMaterialfv(GL_FRONT, GL_DIFFUSE,  roomDefaultDiffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, roomDefaultSpecular);
	glMaterialf(GL_FRONT, GL_SHININESS, roomDefaultShininess);
	glDisable(GL_TEXTURE_2D);
	//Ceilling
	glColor3f(1.0, 1.0, 1.0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,textures[3]);
	glBegin(GL_QUADS);
	glNormal3f(0.0, -1.0, 0.0);
	textX=4.0/SIZE; //replicate 4 times on the x axis
	textY=4.0/SIZE; //replicate 4 times on the y axis
	meshSize=1;
	y=7.5;
	for(z=0.0;z>-20.0;z-=meshSize){
		for(x=-10.0;x<10.0;x+=meshSize){
			glTexCoord2f(textX*x,textY*z); glVertex3f(x,y,z);
			glTexCoord2f(textX*x,textY*(z-meshSize)); glVertex3f(x,y,z-meshSize);
			glTexCoord2f(textX*(x+meshSize),textY*(z-meshSize)); glVertex3f(x+meshSize,y,z-meshSize);
			glTexCoord2f(textX*(x+meshSize),textY*z); glVertex3f(x+meshSize,y,z);
		}
	}
	glEnd();
	glDisable(GL_TEXTURE_2D);
	glPushMatrix();
	if(lightIsOn[2])
		glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, corP);
	glTranslatef(0,7.5,-10);
	glutSolidSphere(0.5,15,15);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, defaultLightEmission);
	glPopMatrix();
	//Front Wall
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,textures[1]);
	glBegin(GL_QUADS);
	glNormal3f(0.0, 0.0, 1.0);
	textX=20.0/SIZE;
	textY=40.0/SIZE;
	meshSize=1;
	z=-20.0;
	for(y=0.5;y<7.5;y+=meshSize){
		for(x=-10.0;x<10.0;x+=meshSize){
			glTexCoord2f(textX*x,textY*y); glVertex3f(x,y,z);
			glTexCoord2f(textX*(x+meshSize),textY*y); glVertex3f(x+meshSize,y,z);
			glTexCoord2f(textX*(x+meshSize),textY*(y+meshSize)); glVertex3f(x+meshSize,y+meshSize,z);
			glTexCoord2f(textX*x,textY*(y+meshSize)); glVertex3f(x,y+meshSize,z);
		}
	}
	glEnd();
	glDisable(GL_TEXTURE_2D);
	//Front Footer
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,textures[2]);
	glBegin(GL_QUADS);
	glNormal3f(0.0, 0.0, 1.0);
	textX=20.0/SIZE;
	textY=40.0/SIZE;
	meshSize=1;
	z=-20.0;
	y=0.0;
	for(x=-10.0;x<10.0;x+=meshSize){
		glTexCoord2f(textX*x,textY*y); glVertex3f(x,y,z);
		glTexCoord2f(textX*(x+meshSize),textY*y); glVertex3f(x+meshSize,y,z);
		glTexCoord2f(textX*(x+meshSize),textY*(y+0.5)); glVertex3f(x+meshSize,y+0.5,z);
		glTexCoord2f(textX*x,textY*(y+0.5)); glVertex3f(x,y+0.5,z);
	}
	glEnd();
	glDisable(GL_TEXTURE_2D);
}

void roomD(){
	//Floor
	glColor3f(1.0, 1.0, 1.0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,textures[0]);
	glBegin(GL_QUADS);
	glNormal3f(0.0, 1.0, 0.0);
	textX=2.0/SIZE; //replicate 2 times on the x axis
	textY=2.0/SIZE; //replicate 2 times on the y axis
	meshSize=1;
	y=0.0;
	for(z=0.0;z>-20.0;z-=meshSize){
		for(x=10.0;x<30.0;x+=meshSize){
			glTexCoord2f(textX*x,textY*z); glVertex3f(x,y,z);
			glTexCoord2f(textX*(x+meshSize),textY*z); glVertex3f(x+meshSize,y,z);
			glTexCoord2f(textX*(x+meshSize),textY*(z-meshSize)); glVertex3f(x+meshSize,y,z-meshSize);
			glTexCoord2f(textX*x,textY*(z-meshSize)); glVertex3f(x,y,z-meshSize);
		}
	}
	glEnd();

	glPushMatrix();
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textures[4]);

	glTranslatef(20.0,2.5,-10.0);
	quad(1.0);
	glPopMatrix();
	glMaterialfv(GL_FRONT, GL_DIFFUSE,  roomDefaultDiffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, roomDefaultSpecular);
	glMaterialf(GL_FRONT, GL_SHININESS, roomDefaultShininess);
	glDisable(GL_TEXTURE_2D);
	//Ceilling
	glColor3f(1.0, 1.0, 1.0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,textures[3]);
	glBegin(GL_QUADS);
	glNormal3f(0.0, -1.0, 0.0);
	textX=4.0/SIZE; //replicate 4 times on the x axis
	textY=4.0/SIZE; //replicate 4 times on the y axis
	meshSize=1;
	y=7.5;
	for(z=0.0;z>-20.0;z-=meshSize){
		for(x=10.0;x<30.0;x+=meshSize){
			glTexCoord2f(textX*x,textY*z); glVertex3f(x,y,z);
			glTexCoord2f(textX*x,textY*(z-meshSize)); glVertex3f(x,y,z-meshSize);
			glTexCoord2f(textX*(x+meshSize),textY*(z-meshSize)); glVertex3f(x+meshSize,y,z-meshSize);
			glTexCoord2f(textX*(x+meshSize),textY*z); glVertex3f(x+meshSize,y,z);
		}
	}
	glEnd();
	glDisable(GL_TEXTURE_2D);
	glPushMatrix();
	if(lightIsOn[3])
		glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, corP);
	glTranslatef(20,7.5,-10);
	glutSolidSphere(0.5,15,15);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, defaultLightEmission);
	glPopMatrix();
	//Front Wall
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,textures[1]);
	glBegin(GL_QUADS);
	glNormal3f(0.0, 0.0, 1.0);
	textX=20.0/SIZE;
	textY=40.0/SIZE;
	meshSize=1;
	z=-20.0;
	for(y=0.5;y<7.5;y+=meshSize){
		for(x=10.0;x<30.0;x+=meshSize){
			glTexCoord2f(textX*x,textY*y); glVertex3f(x,y,z);
			glTexCoord2f(textX*(x+meshSize),textY*y); glVertex3f(x+meshSize,y,z);
			glTexCoord2f(textX*(x+meshSize),textY*(y+meshSize)); glVertex3f(x+meshSize,y+meshSize,z);
			glTexCoord2f(textX*x,textY*(y+meshSize)); glVertex3f(x,y+meshSize,z);
		}
	}
	glEnd();
	glDisable(GL_TEXTURE_2D);
	//Front Footer
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,textures[2]);
	glBegin(GL_QUADS);
	glNormal3f(0.0, 0.0, 1.0);
	textX=20.0/SIZE;
	textY=40.0/SIZE;
	meshSize=1;
	z=-20.0;
	y=0.0;
	for(x=10.0;x<30.0;x+=meshSize){
		glTexCoord2f(textX*x,textY*y); glVertex3f(x,y,z);
		glTexCoord2f(textX*(x+meshSize),textY*y); glVertex3f(x+meshSize,y,z);
		glTexCoord2f(textX*(x+meshSize),textY*(y+0.5)); glVertex3f(x+meshSize,y+0.5,z);
		glTexCoord2f(textX*x,textY*(y+0.5)); glVertex3f(x,y+0.5,z);
	}
	glEnd();
	glDisable(GL_TEXTURE_2D);
	//Right Wall
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,textures[1]);
	glBegin(GL_QUADS);
	glNormal3f(-1.0, 0.0, 0.0);
	textX=20.0/SIZE;
	textY=40.0/SIZE;
	meshSize=1;
	x=30.0;
	for(y=0.5;y<7.0;y+=meshSize){
		for(z=-20.0;z<0.0;z+=meshSize){
			glTexCoord2f(textX*z,textY*y); glVertex3f(x,y,z);
			glTexCoord2f(textX*(z+meshSize),textY*y); glVertex3f(x,y,z+meshSize);
			glTexCoord2f(textX*(z+meshSize),textY*(y+meshSize)); glVertex3f(x,y+meshSize,z+meshSize);
			glTexCoord2f(textX*z,textY*(y+meshSize)); glVertex3f(x,y+meshSize,z);
		}
	}
	glEnd();
	glDisable(GL_TEXTURE_2D);
	//Right Footer
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,textures[2]);
	glBegin(GL_QUADS);
	glNormal3f(-1.0, 0.0, 0.0);
	textX=20.0/SIZE;
	textY=40.0/SIZE;
	meshSize=1;
	x=30.0;
	y=0.0;
	for(z=-20.0;z<0.0;z+=meshSize){
		glTexCoord2f(textX*z,textY*y); glVertex3f(x,y,z);
		glTexCoord2f(textX*(z+meshSize),textY*y); glVertex3f(x,y,z+meshSize);
		glTexCoord2f(textX*(z+meshSize),textY*(y+0.5)); glVertex3f(x,y+0.5,z+meshSize);
		glTexCoord2f(textX*z,textY*(y+0.5)); glVertex3f(x,y+0.5,z);
	}
	glEnd();
	glDisable(GL_TEXTURE_2D);
}

void roomE(){
	//Floor
	glColor3f(1.0, 1.0, 1.0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,textures[0]);
	glBegin(GL_QUADS);
	glNormal3f(0.0, 1.0, 0.0);
	textX=2.0/SIZE; //replicate 2 times on the x axis
	textY=2.0/SIZE; //replicate 2 times on the y axis
	meshSize=1;
	y=0.0;
	for(z=20.0;z>0.0;z-=meshSize){
		for(x=10.0;x<30.0;x+=meshSize){
			glTexCoord2f(textX*x,textY*z); glVertex3f(x,y,z);
			glTexCoord2f(textX*(x+meshSize),textY*z); glVertex3f(x+meshSize,y,z);
			glTexCoord2f(textX*(x+meshSize),textY*(z-meshSize)); glVertex3f(x+meshSize,y,z-meshSize);
			glTexCoord2f(textX*x,textY*(z-meshSize)); glVertex3f(x,y,z-meshSize);
		}
	}
	glEnd();
	
	glPushMatrix();
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textures[4]);
	glTranslatef(20.0,2.5,10.0);

	quad(1.0);
	
	glPopMatrix();
	glMaterialfv(GL_FRONT, GL_DIFFUSE,  roomDefaultDiffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, roomDefaultSpecular);
	glMaterialf(GL_FRONT, GL_SHININESS, roomDefaultShininess);
	glDisable(GL_TEXTURE_2D);
	//Ceilling
	glColor3f(1.0, 1.0, 1.0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,textures[3]);
	glBegin(GL_QUADS);
	glNormal3f(0.0, -1.0, 0.0);
	textX=4.0/SIZE; //replicate 4 times on the x axis
	textY=4.0/SIZE; //replicate 4 times on the y axis
	meshSize=1;
	y=7.5;
	for(z=20.0;z>0.0;z-=meshSize){
		for(x=10.0;x<30.0;x+=meshSize){
			glTexCoord2f(textX*x,textY*z); glVertex3f(x,y,z);
			glTexCoord2f(textX*x,textY*(z-meshSize)); glVertex3f(x,y,z-meshSize);
			glTexCoord2f(textX*(x+meshSize),textY*(z-meshSize)); glVertex3f(x+meshSize,y,z-meshSize);
			glTexCoord2f(textX*(x+meshSize),textY*z); glVertex3f(x+meshSize,y,z);
		}
	}
	glEnd();
	glDisable(GL_TEXTURE_2D);
	glPushMatrix();
	if(lightIsOn[4])
		glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, corP);
	glTranslatef(20,7.5,10);
	glutSolidSphere(0.5,15,15);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, defaultLightEmission);
	glPopMatrix();
	//Back Wall
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,textures[1]);
	glBegin(GL_QUADS);
	glNormal3f(0.0, 0.0, -1.0);
	textX=40.0/SIZE;
	textY=40.0/SIZE;
	meshSize=1;
	z=20.0;
	for(y=0.50;y<7.0;y+=meshSize){
		for(x=10.0;x<30.0;x+=meshSize){
			glTexCoord2f(textX*x,textY*y); glVertex3f(x,y,z);
			glTexCoord2f(textX*x,textY*(y+meshSize)); glVertex3f(x,y+meshSize,z);
			glTexCoord2f(textX*(x+meshSize),textY*(y+meshSize)); glVertex3f(x+meshSize,y+meshSize,z);
			glTexCoord2f(textX*(x+meshSize),textY*y); glVertex3f(x+meshSize,y,z);
		}
	}
	glEnd();
	glDisable(GL_TEXTURE_2D);
	//Back Footer
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,textures[2]);
	glBegin(GL_QUADS);
	glNormal3f(0.0, 0.0, -1.0);
	textX=20.0/SIZE;
	textY=40.0/SIZE;
	meshSize=1;
	z=20.0;
	y=0.0;
	for(x=10.0;x<30.0;x+=meshSize){
		glTexCoord2f(textX*x,textY*y); glVertex3f(x,y,z);
		glTexCoord2f(textX*x,textY*(y+0.5)); glVertex3f(x,y+0.5,z);
		glTexCoord2f(textX*(x+meshSize),textY*(y+0.5)); glVertex3f(x+meshSize,y+0.5,z);
		glTexCoord2f(textX*(x+meshSize),textY*y); glVertex3f(x+meshSize,y,z);
	}
	glEnd();
	glDisable(GL_TEXTURE_2D);
	//Right Wall
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,textures[1]);
	glBegin(GL_QUADS);
	glNormal3f(-1.0, 0.0, 0.0);
	textX=20.0/SIZE;
	textY=40.0/SIZE;
	meshSize=1;
	x=30.0;
	for(y=0.5;y<7.0;y+=meshSize){
		for(z=0.0;z<20.0;z+=meshSize){
			glTexCoord2f(textX*z,textY*y); glVertex3f(x,y,z);
			glTexCoord2f(textX*(z+meshSize),textY*y); glVertex3f(x,y,z+meshSize);
			glTexCoord2f(textX*(z+meshSize),textY*(y+meshSize)); glVertex3f(x,y+meshSize,z+meshSize);
			glTexCoord2f(textX*z,textY*(y+meshSize)); glVertex3f(x,y+meshSize,z);
		}
	}
	glEnd();
	glDisable(GL_TEXTURE_2D);
	//Right Footer
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,textures[2]);
	glBegin(GL_QUADS);
	glNormal3f(-1.0, 0.0, 0.0);
	textX=20.0/SIZE;
	textY=40.0/SIZE;
	meshSize=1;
	x=30.0;
	y=0.0;
	for(z=0.0;z<20.0;z+=meshSize){
		glTexCoord2f(textX*z,textY*y); glVertex3f(x,y,z);
		glTexCoord2f(textX*(z+meshSize),textY*y); glVertex3f(x,y,z+meshSize);
		glTexCoord2f(textX*(z+meshSize),textY*(y+0.5)); glVertex3f(x,y+0.5,z+meshSize);
		glTexCoord2f(textX*z,textY*(y+0.5)); glVertex3f(x,y+0.5,z);
	}
	glEnd();
	glDisable(GL_TEXTURE_2D);
}

void wallAB(){
	//Back Wall
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,textures[1]);
	glBegin(GL_QUADS);
	glNormal3f(0.0, 0.0, 1.0);
	textX=20.0/SIZE;
	textY=40.0/SIZE;
	meshSize=1;
	z=0.5;
	for(y=0.5;y<7.5;y+=meshSize){
		for(x=-30.0;x<-20.0;x+=meshSize){
			glTexCoord2f(textX*x,textY*y); glVertex3f(x,y,z);
			glTexCoord2f(textX*(x+meshSize),textY*y); glVertex3f(x+meshSize,y,z);
			glTexCoord2f(textX*(x+meshSize),textY*(y+meshSize)); glVertex3f(x+meshSize,y+meshSize,z);
			glTexCoord2f(textX*x,textY*(y+meshSize)); glVertex3f(x,y+meshSize,z);
		}
	}
	glEnd();
	glDisable(GL_TEXTURE_2D);
	//Back Footer
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,textures[2]);
	glBegin(GL_QUADS);
	glNormal3f(0.0, 0.0, 1.0);
	textX=20.0/SIZE;
	textY=40.0/SIZE;
	meshSize=1;
	z=0.5;
	y=0.0;
	for(x=-30.0;x<-20.0;x+=meshSize){
		glTexCoord2f(textX*x,textY*y); glVertex3f(x,y,z);
		glTexCoord2f(textX*(x+meshSize),textY*y); glVertex3f(x+meshSize,y,z);
		glTexCoord2f(textX*(x+meshSize),textY*(y+0.5)); glVertex3f(x+meshSize,y+0.5,z);
		glTexCoord2f(textX*x,textY*(y+0.5)); glVertex3f(x,y+0.5,z);
	}
	glEnd();
	glDisable(GL_TEXTURE_2D);
	//Front Wall
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,textures[1]);
	glBegin(GL_QUADS);
	glNormal3f(0.0, 0.0, -1.0);
	textX=40.0/SIZE;
	textY=40.0/SIZE;
	meshSize=1;
	z=0.0;
	for(y=0.50;y<7.0;y+=meshSize){
		for(x=-30.0;x<-20.0;x+=meshSize){
			glTexCoord2f(textX*x,textY*y); glVertex3f(x,y,z);
			glTexCoord2f(textX*x,textY*(y+meshSize)); glVertex3f(x,y+meshSize,z);
			glTexCoord2f(textX*(x+meshSize),textY*(y+meshSize)); glVertex3f(x+meshSize,y+meshSize,z);
			glTexCoord2f(textX*(x+meshSize),textY*y); glVertex3f(x+meshSize,y,z);
		}
	}
	glEnd();
	glDisable(GL_TEXTURE_2D);
	//Front Footer
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,textures[2]);
	glBegin(GL_QUADS);
	glNormal3f(0.0, 0.0, -1.0);
	textX=20.0/SIZE;
	textY=40.0/SIZE;
	meshSize=1;
	z=0.0;
	y=0.0;
	for(x=-30.0;x<-20;x+=meshSize){
		glTexCoord2f(textX*x,textY*y); glVertex3f(x,y,z);
		glTexCoord2f(textX*x,textY*(y+0.5)); glVertex3f(x,y+0.5,z);
		glTexCoord2f(textX*(x+meshSize),textY*(y+0.5)); glVertex3f(x+meshSize,y+0.5,z);
		glTexCoord2f(textX*(x+meshSize),textY*y); glVertex3f(x+meshSize,y,z);
	}
	glEnd();
	glDisable(GL_TEXTURE_2D);
	//Right Wall
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,textures[1]);
	glBegin(GL_QUADS);
	glNormal3f(1.0, 0.0, 0.0);
	textX=20.0/SIZE;
	textY=40.0/SIZE;
	meshSize=1;
	x=-20.0;
	z=0.0;
	for(y=0.5;y<7.0;y+=meshSize){
		glTexCoord2f(textX*z,textY*y); glVertex3f(x,y,z);
		glTexCoord2f(textX*z,textY*(y+meshSize)); glVertex3f(x,y+meshSize,z);
		glTexCoord2f(textX*(z+0.5),textY*(y+meshSize)); glVertex3f(x,y+meshSize,z+0.5);
		glTexCoord2f(textX*(z+0.5),textY*y); glVertex3f(x,y,z+0.5);
	}
	glEnd();
	glDisable(GL_TEXTURE_2D);
	//Right Footer
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,textures[2]);
	glBegin(GL_QUADS);
	glNormal3f(1.0f,0.0f,0.0f);
	glTexCoord2f(0.0f,0.0f); glVertex3f( -20.0, 0.0, 0.0 );
	glTexCoord2f(0.0f,1.0f); glVertex3f( -20.0, 0.5, 0.0);
	glTexCoord2f(1.0f,1.0f); glVertex3f( -20.0, 0.5, 0.5);
	glTexCoord2f(1.0f,0.0f); glVertex3f( -20.0, 0.0, 0.5 );
	
	
	glEnd();
	glDisable(GL_TEXTURE_2D);
}

void wallBC(){
	//Left Wall
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,textures[1]);
	glBegin(GL_QUADS);
	glNormal3f(-1.0, 0.0, 0.0);
	textX=20.0/SIZE;
	textY=40.0/SIZE;
	meshSize=1;
	x=-10.0;
	for(y=0.5;y<7.0;y+=meshSize){
		for(z=-20.0;z<-10.0;z+=meshSize){
			glTexCoord2f(textX*z,textY*y); glVertex3f(x,y,z);
			glTexCoord2f(textX*(z+meshSize),textY*y); glVertex3f(x,y,z+meshSize);
			glTexCoord2f(textX*(z+meshSize),textY*(y+meshSize)); glVertex3f(x,y+meshSize,z+meshSize);
			glTexCoord2f(textX*z,textY*(y+meshSize)); glVertex3f(x,y+meshSize,z);
		}
	}
	glEnd();
	glDisable(GL_TEXTURE_2D);
	//Left Footer
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,textures[2]);
	glBegin(GL_QUADS);
	glNormal3f(-1.0, 0.0, 0.0);
	textX=20.0/SIZE;
	textY=40.0/SIZE;
	meshSize=1;
	x=-10.0;
	y=0.0;
	for(z=-20.0;z<-10.0;z+=meshSize){
		glTexCoord2f(textX*z,textY*y); glVertex3f(x,y,z);
		glTexCoord2f(textX*(z+meshSize),textY*y); glVertex3f(x,y,z+meshSize);
		glTexCoord2f(textX*(z+meshSize),textY*(y+0.5)); glVertex3f(x,y+0.5,z+meshSize);
		glTexCoord2f(textX*z,textY*(y+0.5)); glVertex3f(x,y+0.5,z);
	}
	glEnd();
	glDisable(GL_TEXTURE_2D);
	//Right Wall
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,textures[1]);
	glBegin(GL_QUADS);
	glNormal3f(1.0, 0.0, 0.0);
	textX=20.0/SIZE;
	textY=40.0/SIZE;
	meshSize=1;
	x=-9.5;
	for(y=0.5;y<7.0;y+=meshSize){
		for(z=-20.0;z<-10.0;z+=meshSize){
			glTexCoord2f(textX*z,textY*y); glVertex3f(x,y,z);
			glTexCoord2f(textX*z,textY*(y+meshSize)); glVertex3f(x,y+meshSize,z);
			glTexCoord2f(textX*(z+meshSize),textY*(y+meshSize)); glVertex3f(x,y+meshSize,z+meshSize);
			glTexCoord2f(textX*(z+meshSize),textY*y); glVertex3f(x,y,z+meshSize);
		}
	}
	glEnd();
	glDisable(GL_TEXTURE_2D);
	//Right Footer
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,textures[2]);
	glBegin(GL_QUADS);
	glNormal3f(1.0, 0.0, 0.0);
	textX=20.0/SIZE;
	textY=40.0/SIZE;
	meshSize=1;
	x=-9.5;
	y=0.0;
	for(z=-20.0;z<-10.0;z+=meshSize){
		glTexCoord2f(textX*z,textY*y); glVertex3f(x,y,z);
		glTexCoord2f(textX*z,textY*(y+0.5)); glVertex3f(x,y+0.5,z);
		glTexCoord2f(textX*(z+meshSize),textY*(y+0.5)); glVertex3f(x,y+0.5,z+meshSize);
		glTexCoord2f(textX*(z+meshSize),textY*y); glVertex3f(x,y,z+meshSize);
	}
	glEnd();
	glDisable(GL_TEXTURE_2D);
	//Back Wall
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,textures[1]);
	glBegin(GL_QUADS);
	glNormal3f(0.0, 0.0, 1.0);
	textX=20.0/SIZE;
	textY=40.0/SIZE;
	meshSize=1;
	z=-10.0;
	x=-10.0;
	for(y=0.5;y<7.5;y+=meshSize){
		glTexCoord2f(textX*x,textY*y); glVertex3f(x,y,z);
		glTexCoord2f(textX*(x+0.5),textY*y); glVertex3f(x+0.5,y,z);
		glTexCoord2f(textX*(x+0.5),textY*(y+meshSize)); glVertex3f(x+0.5,y+meshSize,z);
		glTexCoord2f(textX*x,textY*(y+meshSize)); glVertex3f(x,y+meshSize,z);
	}
	glEnd();
	glDisable(GL_TEXTURE_2D);
	//Back Footer
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,textures[2]);
	glBegin(GL_QUADS);
	glNormal3f(0.0f,0.0f,1.0f);
	glTexCoord2f(0.0f,0.0f); glVertex3f( -10.0, 0.0, -10.0 );
	glTexCoord2f(.25f,0.0f); glVertex3f( -9.5, 0.0, -10.0 );
	glTexCoord2f(.25f,1.0f); glVertex3f( -9.5, 0.5, -10.0);
	glTexCoord2f(0.0f,1.0f); glVertex3f( -10.0, 0.5, -10.0);
	glEnd();
	glDisable(GL_TEXTURE_2D);
}

void wallCD(){
	//Left Wall
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,textures[1]);
	glBegin(GL_QUADS);
	glNormal3f(-1.0, 0.0, 0.0);
	textX=20.0/SIZE;
	textY=40.0/SIZE;
	meshSize=1;
	x=9.5;
	for(y=0.5;y<7.0;y+=meshSize){
		for(z=-20.0;z<-10.0;z+=meshSize){
			glTexCoord2f(textX*z,textY*y); glVertex3f(x,y,z);
			glTexCoord2f(textX*(z+meshSize),textY*y); glVertex3f(x,y,z+meshSize);
			glTexCoord2f(textX*(z+meshSize),textY*(y+meshSize)); glVertex3f(x,y+meshSize,z+meshSize);
			glTexCoord2f(textX*z,textY*(y+meshSize)); glVertex3f(x,y+meshSize,z);
		}
	}
	glEnd();
	glDisable(GL_TEXTURE_2D);
	//Left Footer
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,textures[2]);
	glBegin(GL_QUADS);
	glNormal3f(-1.0, 0.0, 0.0);
	textX=20.0/SIZE;
	textY=40.0/SIZE;
	meshSize=1;
	x=9.5;
	y=0.0;
	for(z=-20.0;z<-10.0;z+=meshSize){
		glTexCoord2f(textX*z,textY*y); glVertex3f(x,y,z);
		glTexCoord2f(textX*(z+meshSize),textY*y); glVertex3f(x,y,z+meshSize);
		glTexCoord2f(textX*(z+meshSize),textY*(y+0.5)); glVertex3f(x,y+0.5,z+meshSize);
		glTexCoord2f(textX*z,textY*(y+0.5)); glVertex3f(x,y+0.5,z);
	}
	glEnd();
	glDisable(GL_TEXTURE_2D);
	//Right Wall
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,textures[1]);
	glBegin(GL_QUADS);
	glNormal3f(1.0, 0.0, 0.0);
	textX=20.0/SIZE;
	textY=40.0/SIZE;
	meshSize=1;
	x=10;
	for(y=0.5;y<7.0;y+=meshSize){
		for(z=-20.0;z<-10.0;z+=meshSize){
			glTexCoord2f(textX*z,textY*y); glVertex3f(x,y,z);
			glTexCoord2f(textX*z,textY*(y+meshSize)); glVertex3f(x,y+meshSize,z);
			glTexCoord2f(textX*(z+meshSize),textY*(y+meshSize)); glVertex3f(x,y+meshSize,z+meshSize);
			glTexCoord2f(textX*(z+meshSize),textY*y); glVertex3f(x,y,z+meshSize);
		}
	}
	glEnd();
	glDisable(GL_TEXTURE_2D);
	//Right Footer
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,textures[2]);
	glBegin(GL_QUADS);
	glNormal3f(1.0, 0.0, 0.0);
	textX=20.0/SIZE;
	textY=40.0/SIZE;
	meshSize=1;
	x=10;
	y=0.0;
	for(z=-20.0;z<-10.0;z+=meshSize){
		glTexCoord2f(textX*z,textY*y); glVertex3f(x,y,z);
		glTexCoord2f(textX*z,textY*(y+0.5)); glVertex3f(x,y+0.5,z);
		glTexCoord2f(textX*(z+meshSize),textY*(y+0.5)); glVertex3f(x,y+0.5,z+meshSize);
		glTexCoord2f(textX*(z+meshSize),textY*y); glVertex3f(x,y,z+meshSize);
	}
	glEnd();
	glDisable(GL_TEXTURE_2D);
	//Back Wall
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,textures[1]);
	glBegin(GL_QUADS);
	glNormal3f(0.0, 0.0, 1.0);
	textX=20.0/SIZE;
	textY=40.0/SIZE;
	meshSize=1;
	z=-10.0;
	x=9.5;
	for(y=0.5;y<7.5;y+=meshSize){
		glTexCoord2f(textX*x,textY*y); glVertex3f(x,y,z);
		glTexCoord2f(textX*(x+0.5),textY*y); glVertex3f(x+0.5,y,z);
		glTexCoord2f(textX*(x+0.5),textY*(y+meshSize)); glVertex3f(x+0.5,y+meshSize,z);
		glTexCoord2f(textX*x,textY*(y+meshSize)); glVertex3f(x,y+meshSize,z);
	}
	glEnd();
	glDisable(GL_TEXTURE_2D);
	//Back Footer
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,textures[2]);
	glBegin(GL_QUADS);
	glNormal3f(0.0f,0.0f,1.0f);
	glTexCoord2f(0.0f,0.0f); glVertex3f( 9.5, 0.0, -10.0 );
	glTexCoord2f(.25f,0.0f); glVertex3f( 10.0, 0.0, -10.0 );
	glTexCoord2f(.25f,1.0f); glVertex3f( 10.0, 0.5, -10.0);
	glTexCoord2f(0.0f,1.0f); glVertex3f( 9.5, 0.5, -10.0);
	glEnd();
	glDisable(GL_TEXTURE_2D);
}

void wallDE(){
	//Back Wall
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,textures[1]);
	glBegin(GL_QUADS);
	glNormal3f(0.0, 0.0, 1.0);
	textX=20.0/SIZE;
	textY=40.0/SIZE;
	meshSize=1;
	z=0.5;
	for(y=0.5;y<7.5;y+=meshSize){
		for(x=20.0;x<30.0;x+=meshSize){
			glTexCoord2f(textX*x,textY*y); glVertex3f(x,y,z);
			glTexCoord2f(textX*(x+meshSize),textY*y); glVertex3f(x+meshSize,y,z);
			glTexCoord2f(textX*(x+meshSize),textY*(y+meshSize)); glVertex3f(x+meshSize,y+meshSize,z);
			glTexCoord2f(textX*x,textY*(y+meshSize)); glVertex3f(x,y+meshSize,z);
		}
	}
	glEnd();
	glDisable(GL_TEXTURE_2D);
	//Back Footer
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,textures[2]);
	glBegin(GL_QUADS);
	glNormal3f(0.0, 0.0, 1.0);
	textX=20.0/SIZE;
	textY=40.0/SIZE;
	meshSize=1;
	z=0.5;
	y=0.0;
	for(x=20.0;x<30.0;x+=meshSize){
		glTexCoord2f(textX*x,textY*y); glVertex3f(x,y,z);
		glTexCoord2f(textX*(x+meshSize),textY*y); glVertex3f(x+meshSize,y,z);
		glTexCoord2f(textX*(x+meshSize),textY*(y+0.5)); glVertex3f(x+meshSize,y+0.5,z);
		glTexCoord2f(textX*x,textY*(y+0.5)); glVertex3f(x,y+0.5,z);
	}
	glEnd();
	glDisable(GL_TEXTURE_2D);
	//Front Wall
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,textures[1]);
	glBegin(GL_QUADS);
	glNormal3f(0.0, 0.0, -1.0);
	textX=40.0/SIZE;
	textY=40.0/SIZE;
	meshSize=1;
	z=0.0;
	for(y=0.50;y<7.0;y+=meshSize){
		for(x=20.0;x<30.0;x+=meshSize){
			glTexCoord2f(textX*x,textY*y); glVertex3f(x,y,z);
			glTexCoord2f(textX*x,textY*(y+meshSize)); glVertex3f(x,y+meshSize,z);
			glTexCoord2f(textX*(x+meshSize),textY*(y+meshSize)); glVertex3f(x+meshSize,y+meshSize,z);
			glTexCoord2f(textX*(x+meshSize),textY*y); glVertex3f(x+meshSize,y,z);
		}
	}
	glEnd();
	glDisable(GL_TEXTURE_2D);
	//Front Footer
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,textures[2]);
	glBegin(GL_QUADS);
	glNormal3f(0.0, 0.0, -1.0);
	textX=20.0/SIZE;
	textY=40.0/SIZE;
	meshSize=1;
	z=0.0;
	y=0.0;
	for(x=20.0;x<30;x+=meshSize){
		glTexCoord2f(textX*x,textY*y); glVertex3f(x,y,z);
		glTexCoord2f(textX*x,textY*(y+0.5)); glVertex3f(x,y+0.5,z);
		glTexCoord2f(textX*(x+meshSize),textY*(y+0.5)); glVertex3f(x+meshSize,y+0.5,z);
		glTexCoord2f(textX*(x+meshSize),textY*y); glVertex3f(x+meshSize,y,z);
	}
	glEnd();
	glDisable(GL_TEXTURE_2D);
	//Left Wall
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,textures[1]);
	glBegin(GL_QUADS);
	glNormal3f(-1.0, 0.0, 0.0);
	textX=20.0/SIZE;
	textY=40.0/SIZE;
	meshSize=1;
	x=20.0;
	z=0.0;
	for(y=0.5;y<7.0;y+=meshSize){
		glTexCoord2f(textX*z,textY*y); glVertex3f(x,y,z);
		glTexCoord2f(textX*(z+0.5),textY*y); glVertex3f(x,y,z+0.5);
		glTexCoord2f(textX*(z+0.5),textY*(y+meshSize)); glVertex3f(x,y+meshSize,z+0.5);
		glTexCoord2f(textX*z,textY*(y+meshSize)); glVertex3f(x,y+meshSize,z);
	}
	glEnd();
	glDisable(GL_TEXTURE_2D);
	//Left Footer
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,textures[2]);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f,0.0f); glVertex3f( 20.0, 0.0, 0.0 );
	glTexCoord2f(.25f,0.0f); glVertex3f( 20.0, 0.0, 0.5 );
	glTexCoord2f(.25f,1.0f); glVertex3f( 20.0, 0.5, 0.5);
	glTexCoord2f(0.0f,1.0f); glVertex3f( 20.0, 0.5, 0.0);
	glEnd();
	glDisable(GL_TEXTURE_2D);
}

void drawScene(){
	glPushMatrix();
	lobby();
	roomA();
	wallAB();
	roomB();
	wallBC();
	roomC();
	wallCD();
	roomD();
	wallDE();
	roomE();
	middle();
	glPopMatrix();
}

//----------------------DRAW THE ARROW ------------------------
void drawArrow(Arrow *a) {
	
	glPushMatrix();
	
	glTranslatef(a->x, a->y, a->z);
	//glRotatef(xf, 0.0, 1.0, 0.0);
	//gluPerspective(fov, windowWidth/windowHeight, 0.01f, 200.0f);
	
	//glRotatef(fmod((-fov*180)/PI), 360.0), 0.0, 1.0, 0.0);
	
	
	Cylinder=gluNewQuadric();      // Create A Pointer To The Quadric Object ( NEW )
	gluQuadricNormals(Cylinder, GLU_SMOOTH);  // Create Smooth Normals ( NEW )
	gluQuadricTexture(Cylinder, GL_TRUE);    // Create Texture Coords ( NEW )
	gluCylinder(Cylinder, 0.0, 0.05 ,0.15 ,15 ,15);
	
	glTranslatef(0.0f, 0.0f, 0.15f);
	glRotatef(90.0, 0.0f, 0.0f, 1.0f);
	Disk = gluNewQuadric();
	gluQuadricNormals(Disk, GLU_SMOOTH);  // Create Smooth Normals ( NEW )
	gluQuadricTexture(Disk, GL_TRUE);    // Create Texture Coords ( NEW )
	gluDisk(Disk, 0.0, 0.05, 64, 1.0);
	
	Cylinder=gluNewQuadric();      // Create A Pointer To The Quadric Object ( NEW )
	gluQuadricNormals(Cylinder, GLU_SMOOTH);  // Create Smooth Normals ( NEW )
	gluQuadricTexture(Cylinder, GL_TRUE);    // Create Texture Coords ( NEW )
	gluCylinder(Cylinder, 0.025, 0.025 ,1.0 ,32 ,32);
	
	glTranslatef(0.0f, 0.0f, 1.0f);
	glRotatef(90.0, 0.0f, 0.0f, 1.0f);
	Disk = gluNewQuadric();
	gluQuadricNormals(Disk, GLU_SMOOTH);  // Create Smooth Normals ( NEW )
	gluQuadricTexture(Disk, GL_TRUE);    // Create Texture Coords ( NEW )
	gluDisk(Disk, 0.0, 0.025, 64, 1.0);
	glPopMatrix();
	
}

//---------------------------------ARROW-----------------------------------
void moveArrow() {
	int i = 0;
	
	// Give the arrows velocity if shoot is true.
	if (shoot == 1) {
		
		for (i = 0; i < MAX_ARROWS; i++) {
			
			if (arrows[i].active == 0) {
				arrows[i].active = 1;
				arrows[i].x = xf;
				arrows[i].y = yf;
				arrows[i].z = zf;
				arrows[i].arrow_phi = angleh;
				arrows[i].dx += 1 * MAX_ARROW_SPEED * cos(arrows[i].arrow_phi);
				arrows[i].dy = yi * MAX_ARROW_SPEED;
				arrows[i].dz += -1 * MAX_ARROW_SPEED * sin(arrows[i].arrow_phi);
				break;
			}
		}
		
		// Resets shoot key to prevent rapid fire
		shoot = 0;
	}
	
	/* Advance arrows and eliminating those that have gone past
	 * the window boundaries
	 */
	for (i = 0; i < MAX_ARROWS; i++) {
		
		if (arrows[i].active == 1) {
			arrows[i].x += arrows[i].dx;
			arrows[i].z += arrows[i].dz;
		}
		//Arrow Boundries/ Destroy arrows outside bounderies´
		if (arrows[i].active == 1 && (!targetCheck(arrows[i].x, arrows[i].z))) {
			arrows[i].dx = 0;
			arrows[i].dy = 0;
			arrows[i].dz = 0;
		}
		else if (arrows[i].active == 1 && !frontierCheck(arrows[i].x, arrows[i].z))
		{
			arrows[i].dx = 0;
			arrows[i].dy = 0;
			arrows[i].dz = 0;
		}
	}
}

void Timer(int value) {
	cubeRotation+=1;
	if(cubeRotation==360)
		cubeRotation=0;
	
	// Moves the arrow
	shots = 0, hits = 0;
	for (int i = 0; i < MAX_ARROWS; i++)
	{
		if (arrows[i].active == 1)
		{
			if (!targetCheck(arrows[i].x, arrows[i].z))
			{
				hits += 1;
			}
			shots += 1;
		}
	}
	moveArrow();
	
	glutPostRedisplay();
	glutTimerFunc(mSecDelay, Timer, 1);
}

//---------------------------------DYSPLAY-----------------------------------
void display(){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	//set viewport
	glViewport(0, 0, windowWidth, windowHeight);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	//set perspective
	gluPerspective(fov, windowWidth/windowHeight, 0.01f, 200.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	//set the eye
	gluLookAt(xi, yi, zi, xf, yf, zf, 0, 1, 0);
	
	drawScene();
	illuminate();
	
	// Draws text
	char buffer[1024];
	snprintf(buffer, sizeof(buffer), "Accuracy is %d/%d", hits, shots);
	drawText(buffer, xf, yf, zf);
	
	// Draws the arrows on screen when they are active
	for (int i = 0; i < MAX_ARROWS; i++) {
		if (arrows[i].active) {
			drawArrow(&arrows[i]);
		}
	}
	glutSwapBuffers();
}

void init(){
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glShadeModel(GL_SMOOTH);
	defineTextures();
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	
	glViewport(0, 0, windowWidth, windowHeight);
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
}

//--------------------------------------MAIN----------------------------------
int main(int argc, char * argv[]) {
	
	glutInit(&argc, argv);
	initializeGlobals();
	
	glutInitDisplayMode(GLUT_DOUBLE  | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(windowWidth,windowHeight);
	glutCreateWindow(window_name);
	
	glutFullScreen();
	glutSetCursor(GLUT_CURSOR_NONE);
	
	init();
	glutWarpPointer(mouseX,mouseY);
	
	glutDisplayFunc(display);
	glutTimerFunc(mSecDelay, Timer, 1);
	// Callback function that controls movement, mouse and keyboard
	glutKeyboardFunc(keyboardMovement);
	glutPassiveMotionFunc(mouseMovement);
	
	glutMainLoop();
	
	
	return 0;
}
