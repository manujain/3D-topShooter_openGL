#include <iostream>
#include <bits/stdc++.h>
#include <stdlib.h>
#include <string>
#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include "imageloader.h"
#include "vec3f.h"
#define ESC 27
#define PI 3.141592653589
#define DEG2RAD(deg) (deg * PI / 180)
#define PLAYER_VIEW 1
#define TOP_VIEW 2
#define OVERHEAD_VIEW 3
#define HELICOPTER_VIEW 4
#define FOLLOW_VIEW 5


using namespace std;

int win_width = 800, win_height = 800,view=1,helicopter_distance=0,move=0;
int pause=0;
float top_angle = 0;
float velz = 0.1f;
float z = 4.0f, x = -0.5f;
float beyblade_x =0.0;
float beyblade_y = 59.0;
int nishana_x = 50;
int nishana_y = 0;
float metervelocity=0.5f;
float vel_x=0,vel_y=0;
float theta=90;
float arrow_X=10.0f*cosf(DEG2RAD(theta)),arrow_Y=10.0f*sinf(DEG2RAD(theta));
int points =10;

class Power {
	int type;
	public:
	float len_x;
	float len_y;
	float r;
	float g;
	void set_coordinates(float x, float y, int t) {
		len_x=x;
		len_y=y;
		type=t;
		r=0.0f;
		g=0.0f;
	}
	void make() {
		if(type)
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		else
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glBegin(GL_QUADS);
		glVertex2f(-len_x / 2, 0);
		glVertex2f(len_x / 2, 0);
		glVertex2f(len_x / 2, len_y);
		glVertex2f(-len_x / 2, len_y);
		glEnd();
	}
};
Power powermeter;
Power powerlayer;

class line{
	public:
		void draw_line(float x , float y , float r, float g, float b,float x1,float x2) {
			glPushMatrix(); 
			glTranslatef(0.0f,0.0f,0.0f);
			glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
			glColor3f(r , g, b);
			glBegin(GL_LINES);
			glVertex2f(x , y);
			glVertex2f(x1, x2);
			glEnd();
			glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
			glPopMatrix(); 
		}
};
line arrow_direction;

class Terrain {
	private:
		int w; 
		int l; 
		float** hs; 
		Vec3f** normals;
		bool computedNormals; 
		public:
		Terrain(int w2, int l2) {
			w = w2;
			l = l2;
			hs = new float*[l];
			for(int i = 0; i < l; i++) {
				hs[i] = new float[w];
			}
			normals = new Vec3f*[l];
			for(int i = 0; i < l; i++) {
				normals[i] = new Vec3f[w];
			}
			computedNormals = false;
		}
		~Terrain() {
			for(int i = 0; i < l; i++) {
				delete[] hs[i];
			}
			delete[] hs;
			for(int i = 0; i < l; i++) {
				delete[] normals[i];
			}
			delete[] normals;
		}
		int width() {
			return w;
		}
		int length() {
			return l;
		}
		void setHeight(int x, int z, float y) {
			hs[z][x] = y;
			computedNormals = false;
		}
		float getHeight(int x, int z) {
			return hs[z][x];
		}
		void computeNormals() {
			if (computedNormals) {
				return;
			}
			Vec3f** normals2 = new Vec3f*[l];
			for(int i = 0; i < l; i++) {
				normals2[i] = new Vec3f[w];
			}
			for(int z = 0; z < l; z++) {
				for(int x = 0; x < w; x++) {
					Vec3f sum(0.0f, 0.0f, 0.0f);
					Vec3f out;
					if (z > 0) {
						out = Vec3f(0.0f, hs[z - 1][x] - hs[z][x], -1.0f);
					}
					Vec3f in;
					if (z < l - 1) {
						in = Vec3f(0.0f, hs[z + 1][x] - hs[z][x], 1.0f);
					}
					Vec3f left;
					if (x > 0) {
						left = Vec3f(-1.0f, hs[z][x - 1] - hs[z][x], 0.0f);
					}
					Vec3f right;
					if (x < w - 1) {
						right = Vec3f(1.0f, hs[z][x + 1] - hs[z][x], 0.0f);
					}
					if (x > 0 && z > 0) {
						sum += out.cross(left).normalize();
					}
					if (x > 0 && z < l - 1) {
						sum += left.cross(in).normalize();
					}
					if (x < w - 1 && z < l - 1) {
						sum += in.cross(right).normalize();
					}
					if (x < w - 1 && z > 0) {
						sum += right.cross(out).normalize();
					}
					normals2[z][x] = sum;
				}
			}
			const float FALLOUT_RATIO = 0.5f;
			for(int z = 0; z < l; z++) {
				for(int x = 0; x < w; x++) {
					Vec3f sum = normals2[z][x];
					if (x > 0) {
						sum += normals2[z][x - 1] * FALLOUT_RATIO;
					}
					if (x < w - 1) {
						sum += normals2[z][x + 1] * FALLOUT_RATIO;
					}
					if (z > 0) {
						sum += normals2[z - 1][x] * FALLOUT_RATIO;
					}
					if (z < l - 1) {
						sum += normals2[z + 1][x] * FALLOUT_RATIO;
					}
					if (sum.magnitude() == 0) {
						sum = Vec3f(0.0f, 1.0f, 0.0f);
					}
					normals[z][x] = sum;
				}
			}
			for(int i = 0; i < l; i++) {
				delete[] normals2[i];
			}
			delete[] normals2;
			computedNormals = true;
		}
		Vec3f getNormal(int x, int z) {
			if (!computedNormals) {
				computeNormals();
			}
			return normals[z][x];
		}
};

Terrain* loadTerrain(const char* filename, float height) {
	Image* image = loadBMP(filename);
	Terrain* t = new Terrain(image->width, image->height);
	for(int y = 0; y < image->height; y++) {
		for(int x = 0; x < image->width; x++) {
			unsigned char color =
				(unsigned char)image->pixels[3 * (y * image->width + x)];
			float h = height * ((color / 255.0f) - 0.5f);
			t->setHeight(x, y, h/3);
		}
	}
	delete image;
	t->computeNormals();
	return t;
}
float _angle = 60.0f;
Terrain* _pahad;

void cleanup() {
	delete _pahad;
}

void handleKeypress(unsigned char key, int x, int y) {
	switch (key) {
		case 27:
			cleanup();
			exit(0);
	}
}

void initRendering() {
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);
	glShadeModel(GL_SMOOTH);
}

void handleResize(int w, int h) {
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, (double)w / (double)h, 1.0, 200.0);
}

void draw_Target() {
	glPushMatrix();
	glTranslatef(nishana_x, _pahad->getHeight(nishana_x, nishana_y) + 3.0, nishana_y);
	glRotatef(180, 0, 1, 0);
	glColor3f(1.0, 0.0, 0.0);
	glutSolidTorus( 1.5, 2, 25, 30);
	glColor3f(0.0, 0.7, 1.0);
	glutWireTorus( 1.5, 2, 15, 60);
	glPopMatrix();
}

void draw_top() {

	glPushMatrix();
	glColor3f(1.0f, 0.0f, 0.0f);
	glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
	glTranslatef(beyblade_x, _pahad->getHeight((int)beyblade_x, (int)beyblade_y) + 12 * 0.4 -5.0f, beyblade_y);
	glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
	Vec3f yaxis(0.0f, 1.0f, 0.0f);
	Vec3f normal = _pahad->getNormal((int)beyblade_x, (int)beyblade_y);
	Vec3f rotation_axis = yaxis.cross(normal);
	normal = normal.normalize();
	rotation_axis = rotation_axis.normalize();
	float rotation_angle = acos(yaxis.dot(normal)) * 180.0f / PI ;
	glRotatef(rotation_angle ,rotation_axis[0], rotation_axis[1], rotation_axis[2]);
	if (vel_y>0.001)
		glRotatef(top_angle*2.5,-sinf(DEG2RAD(30*(1-vel_y/0.5))), 0,-cosf(DEG2RAD(30*(1-vel_y/0.5))) );
	glTranslatef(0.0, 0.0f, -5.0f);
	glutSolidCone(4, 5, 32, 20);
	glColor3f(0.0f, 0.0f, 1.0f); 
	glutSolidSphere(4.0f,32,2);
	glPopMatrix();
}

void drawScene() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0f, 0.0f, -10.0f);
	
	if(view == PLAYER_VIEW) {
	    glRotatef(30, 1.0f, 0.0f, 0.0f);
	} else if(view == TOP_VIEW) {
    	glRotatef(20, 1.0f, 0.0f, 0.0f);
  	} else if(view == OVERHEAD_VIEW) {
    	glRotatef(90, 1.0f, 0.0f, 0.0f);
  	} else if(view == HELICOPTER_VIEW) {
  		glRotatef(90, 1.0f, 0.0f, 0.0f);
    	glTranslatef(0, helicopter_distance, 0);
  	} else if(view == FOLLOW_VIEW) {
  		glRotatef(5, 1.0f, 0.0f, 0.0f);
  	}

	GLfloat ambientColor[] = {0.4f, 0.4f, 0.4f, 1.0f};
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientColor);

	GLfloat lightColor0[] = {0.6f, 0.6f, 0.6f, 1.0f};
	GLfloat lightPos0[] = {-0.5f, 0.8f, 0.1f, 0.0f};
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor0);
	glLightfv(GL_LIGHT0, GL_POSITION, lightPos0);

	float scale = 5.0f / max(_pahad->width() - 1, _pahad->length() - 1);
	glScalef(scale, scale , scale);
	glTranslatef(-(float)(_pahad->width() - 1) / 2,0.0f,-(float)(_pahad->length() - 1) / 2);
	if(view == TOP_VIEW) {
    	glTranslatef(30 - beyblade_x, 20 - _pahad->getHeight(beyblade_x, beyblade_y), 150 - beyblade_y);
  	} else if(view == FOLLOW_VIEW) {
  		glTranslatef(30 - beyblade_x, - _pahad->getHeight(beyblade_x, beyblade_y), 120 - beyblade_y);
  	}
	glColor3f(0.3f, 0.9f, 0.0f);

	for(int z = 0; z < _pahad->length() - 1; z++) {
		glBegin(GL_TRIANGLE_STRIP);
		for(int x = 0; x < _pahad->width(); x++) {
			Vec3f normal = _pahad->getNormal(x, z);
			float hcolor = abs(_pahad->getHeight(x, z+1));
			glColor3f(hcolor/4.0f-0.2, hcolor/4.0f, 0.0f);
			glNormal3f(normal[0], normal[1], normal[2]);
			glVertex3f(x, _pahad->getHeight(x, z), z);
			normal = _pahad->getNormal(x, z + 1);
			glNormal3f(normal[0], normal[1], normal[2]);
			glVertex3f(x, _pahad->getHeight(x, z + 1), z + 1);
		}
		glEnd();
	}
	glClearColor(1.0,1.0,1.0,1.0);
	draw_Target();
	draw_top();
	glTranslatef(-10.0f, 0.0f, 40.0f);

	if(view == PLAYER_VIEW) {
	    glRotatef(-30, 1.0f, 0.0f, 0.0f);
	} else if(view == TOP_VIEW) {
    	glRotatef(-20, 1.0f, 0.0f, 0.0f);
  	} else if(view == OVERHEAD_VIEW) {
    	glRotatef(-90, 1.0f, 0.0f, 0.0f);
  	} else if(view == HELICOPTER_VIEW) {
  		glRotatef(-90, 1.0f, 0.0f, 0.0f);
    	glTranslatef(0, helicopter_distance, 0);
  	} else if(view == FOLLOW_VIEW) {
  		glRotatef(-5, 1.0f, 0.0f, 0.0f);
  	}

	glTranslatef(-15.0f, -20.0f, 0.0f);
	arrow_X=10.0f*cosf(DEG2RAD(theta));
	arrow_Y=10.0f*sinf(DEG2RAD(theta));
	arrow_direction.draw_line(0.0f,0.0f,0.0f,0.0f,0.0f,arrow_X,arrow_Y);
	glTranslatef(15.0f, 20.0f, 0.0f);

	//Draw power layer
	glPushMatrix();
	glTranslatef(85.0f, -20.0f, 0.0f);
	glColor3f(powerlayer.r, powerlayer.g, 0.0f);
	powerlayer.make();
	glPopMatrix();

	//Draw power meter
	glPushMatrix();
	glTranslatef(85.0f, -20.0f, 0.0f);
	glColor3f(0.0f, 0.0f, 0.0f);
	powermeter.make();
	glPopMatrix();

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	char score[7]="SCORE:";
	glPushMatrix();
	glColor3f(1.0f,0.0f,0.0f);
	glRasterPos3f(15.0f, 20.0f, 0.0f);
	for(int i=0;i<6;i++)
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, score[i]);
	glPopMatrix();

	int temp=points;
	int i=0,a[10],m=0;
	if(temp<0)
		m=1;
	temp=fabs(temp);
	while(temp)
	{
		a[i++]=(temp%10)+48;
		temp/=10;
	}
	if(m)
		a[i++]='-';
	if(!i)
		a[i++]=48;
	glPushMatrix();
	glColor3f(1.0f,0.0f,0.0f);
	glRasterPos3f(26.0f, 20.0f, 0.0f);
	int len=i;
	for (int i=len-1;i>=0;i--) {
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, a[i]);
	}
	glPopMatrix();

	glutSwapBuffers();
}

void update(int value) {
	if(!pause)
	{
	top_angle += 5.0f*vel_y/0.5;
	if(beyblade_y>=0)
		beyblade_y=beyblade_y-vel_y;

	if(vel_y>0.001)
		vel_y=vel_y-0.001;
	else
		vel_y = 0.00;
	if(abs(vel_x)>0.001)
		vel_x = vel_x - 0.001*abs(vel_x)/vel_x;
	else
		vel_x=0;
	if(abs(beyblade_x-nishana_x)<4&&abs(beyblade_y-nishana_y)<4)
	{
		beyblade_x=30;
		beyblade_y=59;
		vel_y=0;
		vel_x=0;
		points+=10;
		nishana_x=rand()%60;
		nishana_y=0;
		theta=90;
		powerlayer.len_y=0;
	}
	if(beyblade_y<=0)
	{
		beyblade_x=30;
		beyblade_y=59;
		vel_y=0;
		vel_x=0;
		points-=1;
		nishana_x=rand()%60;
		nishana_y=0;
		theta=90;
		powerlayer.len_y=0;
	}
	if(beyblade_y>59)
		beyblade_y=59;
	if(beyblade_x<0||beyblade_x>59)
	{
		beyblade_x=30;
		beyblade_y=59;
		points-=1;
		vel_y=0;
		vel_x=0;
		nishana_x=rand()%60;
		nishana_y=0;
		theta=90;
		powerlayer.len_y=0;
	}
	if(beyblade_x>=0)
		beyblade_x = beyblade_x + vel_x;
	_angle += 1.0f;
	if (_angle > 360) {
		_angle -= 360;
	}
	if(vel_x==0&&vel_y==0)
	{
		move=0;
		beyblade_x=30;
		beyblade_y=59;
	}
	}
	glutPostRedisplay();
	glutTimerFunc(25, update, 0);
}

void processNormalKeys(unsigned char key, int xx, int yy) {
	if (key == ESC || key == 'q' || key == 'Q')
		exit(0);
	if(key <= '5' && key >= '1') {
    	view = key - '0';
  	}
	if(key==' ' && beyblade_y==59)
	{
		move=1;
		vel_y=powerlayer.len_y/36*0.8*sin(DEG2RAD(theta));
		vel_x=powerlayer.len_y/36*0.8*cos(DEG2RAD(theta));
	}
	if(key=='a'&&!move)
	{ 
		theta=theta+1;	
	}
	if(key=='c'&&!move)
	{
		theta=theta-1;	
	}
	if(key=='p')
		pause=(pause+1)%2;
}

void handleMouse(int button, int state, int x, int y) {
  if(button == 4) {
 		helicopter_distance--;
  } else if(button == 3) {
 		helicopter_distance++;
  }
}

void handleKeypress2(int key, int x, int y){
	if(key==GLUT_KEY_DOWN && powerlayer.len_y>0.00f&&!move)
	{
		powerlayer.len_y-=1;
		powerlayer.r=powerlayer.len_y/36;
		powerlayer.g=1.0f-(powerlayer.len_y/36);
	}
	if(key==GLUT_KEY_UP && powerlayer.len_y<36.00f&&!move)
	{
		powerlayer.len_y+=1;
		powerlayer.r=powerlayer.len_y/36;
		powerlayer.g=1.0f-(powerlayer.len_y/36);
	}
}

int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(win_height, win_width);
	glutCreateWindow("TopShooter_007");
	initRendering();

	powermeter.set_coordinates(5.0f, 36.0f, 0);
	powerlayer.set_coordinates(5.0f, 0.0f, 1);

	_pahad = loadTerrain("heightmap.bmp", 20);

	glutDisplayFunc(drawScene);
	glutKeyboardFunc(processNormalKeys);
	glutSpecialFunc(handleKeypress2);
	glutReshapeFunc(handleResize);
	glutMouseFunc(handleMouse);
	glutTimerFunc(25, update, 0);

	glutMainLoop();
	return 0;
}
