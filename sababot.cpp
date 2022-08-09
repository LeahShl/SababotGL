#include "sababot.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <iostream>
#include <vector>

int mvstate = MOV_ROBOT;        // What am I moving right now?
int first_person = false;       // Am I in first person mode?
int adjust_ambient = false;     // Should I show adjust ambient box?
int show_help = false;          // Should I show the help menu?

GLuint textures[4]; // Texture buffer

GLint win_width = START_WIDTH,
      win_height = START_HEIGHT;

// THIRD PERSON CAMERA
GLfloat cam_dist = 50.0, world_rot = 0.0,
        xref = 0.0, yref = 0.0, zref = 0.0,
        Vx = 0.0, Vy = 1.0, Vz = 0.0;

// FIRST PERSON CAMERA
GLfloat fpx0 = ROBOT_BODY_WIDTH * 0.5, fpy0 = ROBOT_BODY_HEIGHT + ROBOT_NECK_LENGTH, fpz0 = ROBOT_BODY_DEPTH + 0.9,
        fpxref = 0.0, fpyref = ROBOT_BODY_HEIGHT + ROBOT_NECK_LENGTH, fpzref = 100.0,
        fpVx = 0.0, fpVy = 1.0, fpVz = 0.0;

// PERSPECTIVE PROJECTION
GLfloat fov = 30.0, aspect = 1.0 * win_width / win_height, zNear = 1.0, zFar = 400.0;

// ROBOT MOVEMENT
GLfloat robotx = 0.0, robotz = 0.0, robot_forward = 0.0, robot_y_rotate = 0.0,
        head_x_rotate = 0.0, head_y_rotate = 0.0,
        shoulder_x_rotate = 90.0, shoulder_y_rotate = -10.0,
        elbow_x_rotate = -20.0, hand_y_rotate = 0.0;

// LIGHT VARIABLES
GLfloat red = 0.5, green = 0.5, blue = 0.5,
        light_x = 0.0, light_y = 60.0, light_z = 0.0,
        light_xref = 0.0, light_yref = -1.0, light_zref = 0.0;

// Used for saving user input when in adjust ambient box
std::vector <char> user_input = {}; 

// The aspect is used to update frustum in initCamera()
void updateWindowAspect(float new_aspect)
{
    aspect = new_aspect;
}

// Loads a 1D/2D image texture to buffer using stb_image.h.
// May fail for other reasons other than a file reading error. 
// On a file reading error, the texture will not load and the 
// objects which this texture has been applied to will be displayed 
// with a solid color. Other errors may cause unexpected behaviour.
// 
// For 3 channel images use format=GL_RGB and nchannels=3.
// For 4 channel images use format=GL_RGBA and nchannels=4.
void loadTexture(const char *filename, GLenum target, GLenum format, int nchannels)
{
    int width, height, nrChannels;
    unsigned char *image = stbi_load(filename, &width, &height, &nrChannels, nchannels);
    if (image)
    {
        if(target == GL_TEXTURE_2D)
        {
            glTexImage2D(target, 8, format, width, height, 1, format, GL_UNSIGNED_BYTE, image);
            gluBuild2DMipmaps(target, format, width, height, format, GL_UNSIGNED_BYTE, image);
        }
        if(target == GL_TEXTURE_1D)
        {
            glTexImage1D(target, 8, format, width, 0, format, GL_UNSIGNED_BYTE, image);
            gluBuild1DMipmaps(target, format, width, format, GL_UNSIGNED_BYTE, image);
        }
    }
    else
        std::cout << "ERROR reading texture file \"" << filename << "\"\n";

    stbi_image_free(image);
}

// Loads 4 textures into the texture buffer using loadTexture().
// Loading more than 4 textures will fail on some architectures,
// delete previous textures or use a fragment shader in that case.
void initTextures()
{
    glGenTextures(4, textures);

    glBindTexture(GL_TEXTURE_2D, textures[TEXTURE_MARBLE]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_MIPMAP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    loadTexture("textures/marble.png", GL_TEXTURE_2D, GL_RGB, 3);

    glBindTexture(GL_TEXTURE_1D, textures[TEXTURE_WALL]);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_MIPMAP);
    loadTexture("textures/wall1px.png", GL_TEXTURE_1D, GL_RGB, 3);

    glBindTexture(GL_TEXTURE_2D, textures[TEXTURE_WOOD]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_MIPMAP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    loadTexture("textures/wood.png", GL_TEXTURE_2D, GL_RGB, 3);

    glBindTexture(GL_TEXTURE_2D, textures[TEXTURE_HELP]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_MIPMAP);
    loadTexture("textures/help.png", GL_TEXTURE_2D, GL_RGBA, 4);
}

// Initializes frustum, camera and applies world transformations.
//
// In first person mode, the transformations applied are the reverse
// transformation to the robot's movement, giving the user the feeling
// that they see the world from the robot's eyes.
// 
// In third person mode, the camera is set at an equal distance from
// each axis and rotated around the Y axis.
void initCamera()
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(fov, aspect, zNear, zFar);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    if(first_person)
    {
        gluLookAt(fpx0, fpy0, fpz0, fpxref, fpyref, fpzref, fpVx, fpVy, fpVz);
        glRotatef(head_x_rotate, -1.0, 0.0, 0.0);
        glRotatef(robot_y_rotate + head_y_rotate, 0.0, -1.0, 0.0);
        glTranslatef(-robotx, 0.0, -robotz);
    }
    else
    {
        gluLookAt(cam_dist, cam_dist, cam_dist, xref, yref, zref, Vx, Vy, Vz);
        glRotatef(world_rot, 0.0, 1.0, 0.0);
    }
}

// Initializes a single positional light source.
// All objects have a baseline of ambient and diffuse lighting, but some
// objects have special material properties that are declared seperately.
// The light source has a spotlight cutoff in order to see the effect
// of changing the light's direction. 
void initLight()
{
    GLfloat white_light[] = {1.0, 1.0, 1.0, 0.0};
    GLfloat light_position[] = {light_x, light_y, light_z, 1.0};
    GLfloat lmodel_ambient[] = {red, green, blue, 1.0};
    GLfloat light_direction[] = {light_xref, light_yref, light_zref};
    glLightfv(GL_LIGHT0, GL_DIFFUSE, white_light);
    glLightfv(GL_LIGHT0, GL_SPECULAR, white_light);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 90.0);
    glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, light_direction);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
}

void displayfloor(float size)
{
    // Set material properties
    glMateriali(GL_FRONT, GL_SHININESS, 128);
    glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR); // makes it even shinier

    // Draw floor
    glPushMatrix();
    glTranslatef(-size / 2, 0.0, -size / 2);
    glColor3f(1.0, 1.0, 1.0);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textures[TEXTURE_MARBLE]);
    glBegin(GL_QUADS);
    glNormal3f(0.0, 1.0, 0.0);
    // Dividing the floor into smaller quads lets the floor REALLY shine.
    int subdiv = 42;
    float texrepeat = subdiv / 4.0;
    for(int i=0; i<subdiv; i++)
    {
        for(int j=0; j<subdiv; j++)
        {
            glTexCoord2f(i / texrepeat, j / texrepeat);
            glVertex3f(size * i / subdiv, 0.0, size * j / subdiv);
            glTexCoord2f(i / texrepeat, (j + 1.0) / texrepeat);
            glVertex3f(size * i / subdiv, 0.0, size * (j + 1.0) / subdiv);
            glTexCoord2f((i + 1.0) / texrepeat, (j + 1.0) / texrepeat);
            glVertex3f(size * (i + 1.0) / subdiv, 0.0, size * (j + 1.0) / subdiv);
            glTexCoord2f((i + 1.0) / texrepeat, j / texrepeat);
            glVertex3f(size * (i + 1.0) / subdiv, 0.0, size * j / subdiv);
        }
    }
    glEnd();
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();
    glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SINGLE_COLOR); // back to default
}

// The walls are displayed based on camera position.
// 
// In first person mode, all walls are displayed.
// In third person mode, only the two walls opposite to the camera are
// displayed. The walls to be displayed can be easily determined by the
// camera's angle of rotation around the Y axis.
void displayWalls(float size, float height)
{
    // Set material properties
    glColor3f(1.0, 1.0, 1.0);
    glEnable(GL_TEXTURE_1D);
    glBindTexture(GL_TEXTURE_1D, textures[TEXTURE_WALL]);

    // This is the actual position of the camera, given that the initial
    // camera position is at equal spaces from each axis.
    float camera_pos = world_rot - 45.0;

    if(first_person || cos(camera_pos * PI / 180.0) >= 0.0)
    {
        // draw -Z wall (fridge side)
        glPushMatrix();
        glBegin(GL_QUADS);
        glTexCoord1f(0.0);
        glVertex3f(-size * 0.5, height, -size * 0.5);
        glTexCoord1f(1.0);
        glVertex3f(size * 0.5, height, -size * 0.5);
        glTexCoord1f(1.0);
        glVertex3f(size * 0.5, 0.0, -size * 0.5);
        glTexCoord1f(0.0);
        glVertex3f(-size * 0.5, 0.0, -size * 0.5);
        glEnd();
        glPopMatrix();
    }
    if(first_person || cos(camera_pos * PI / 180.0) <= 0.0)
    {
        // draw +Z wall (table side)
        glPushMatrix();
        glBegin(GL_QUADS);
        glTexCoord1f(0.0);
        glVertex3f(-size * 0.5, height, size * 0.5);
        glTexCoord1f(1.0);
        glVertex3f(size * 0.5, height, size * 0.5);
        glTexCoord1f(1.0);
        glVertex3f(size * 0.5, 0.0, size * 0.5);
        glTexCoord1f(0.0);
        glVertex3f(-size * 0.5, 0.0, size * 0.5);
        glEnd();
        glPopMatrix();
    }
    if(first_person || sin(camera_pos * PI / 180.0) >= 0.0)
    {
        // draw +X wall (no arm side)
        glPushMatrix();
        glBegin(GL_QUADS);
        glTexCoord1f(0.0);
        glVertex3f(size * 0.5, height, size * 0.5);
        glTexCoord1f(1.0);
        glVertex3f(size * 0.5, height, -size * 0.5);
        glTexCoord1f(1.0);
        glVertex3f(size * 0.5, 0.0, -size * 0.5);
        glTexCoord1f(0.0);
        glVertex3f(size * 0.5, 0.0, size * 0.5);
        glEnd();
        glPopMatrix();
    }
    if(first_person || sin(camera_pos * PI / 180.0) <= 0.0)
    {
        // draw -X wall (arm side)
        glPushMatrix();
        glBegin(GL_QUADS);
        glTexCoord1f(0.0);
        glVertex3f(-size * 0.5, height, size * 0.5);
        glTexCoord1f(1.0);
        glVertex3f(-size * 0.5, height, -size * 0.5);
        glTexCoord1f(1.0);
        glVertex3f(-size * 0.5, 0.0, -size * 0.5);
        glTexCoord1f(0.0);
        glVertex3f(-size * 0.5, 0.0, size * 0.5);
        glEnd();
        glPopMatrix();
    }
    glDisable(GL_TEXTURE_1D);
}

// Displays a subdivided rectangular cuboid where each face is subdivided
// into subdiv*subdiv quads.
// This allows for more accurate illumination calculations for each face.
// The more subdivitions, the more fine grained and realistic the final result.
void rectCuboidDiv(float x, float y, float z, int subdiv)
{
    // Top face
    glBegin(GL_QUADS);
    glNormal3f(0.0, 1.0, 0.0);
    for(int i=0; i<subdiv; i++)
    {
        for(int j=0; j<subdiv; j++)
        {
            glTexCoord2f(i, j);
            glVertex3f(x * i / subdiv, y, z * j / subdiv);
            glTexCoord2f(i + 1.0, j);
            glVertex3f(x * (i + 1.0) / subdiv, y, z * j / subdiv);
            glTexCoord2f(i + 1.0, j + 1.0);
            glVertex3f(x * (i + 1.0) / subdiv, y, z * (j + 1.0) / subdiv);
            glTexCoord2f(i, j + 1.0);
            glVertex3f(x * i / subdiv, y, z * (j + 1.0) / subdiv);
        }
    }
    glEnd();

    // Side face 1
    glBegin(GL_QUADS);
    glNormal3f(-1.0, 0.0, 0.0);
    for(int i=0; i<subdiv; i++)
    {
        for(int j=0; j<subdiv; j++)
        {
            glTexCoord2f(i, j);
            glVertex3f(0.0, y * i / subdiv, z * j / subdiv);
            glTexCoord2f(i + 1.0, j);
            glVertex3f(0.0, y * (i + 1.0) / subdiv, z * j / subdiv);
            glTexCoord2f(i + 1.0, j + 1.0);
            glVertex3f(0.0, y * (i + 1.0) / subdiv, z * (j + 1.0) / subdiv);
            glTexCoord2f(i, j + 1.0);
            glVertex3f(0.0, y * i / subdiv, z * (j + 1.0) / subdiv);
        }
    }
    glEnd();

    // Side face 2
    glBegin(GL_QUADS);
    glNormal3f(0.0, 0.0, -1.0);
    for(int i=0; i<subdiv; i++)
    {
        for(int j=0; j<subdiv; j++)
        {
            glTexCoord2f(i, j);
            glVertex3f(x * i / subdiv, y * j / subdiv, 0.0);
            glTexCoord2f(i, j + 1.0);
            glVertex3f(x * i / subdiv, y * (j + 1.0) / subdiv, 0.0);
            glTexCoord2f(i + 1.0, j + 1.0);
            glVertex3f(x * (i + 1.0) / subdiv, y * (j + 1.0) / subdiv, 0.0);
            glTexCoord2f(i + 1.0, j);
            glVertex3f(x * (i + 1.0) / subdiv, y * j / subdiv, 0.0);
        }
    }
    glEnd();

    // Side face 3
    glBegin(GL_QUADS);
    glNormal3f(1.0, 0.0, 0.0);
    for(int i=0; i<subdiv; i++)
    {
        for(int j=0; j<subdiv; j++)
        {
            glTexCoord2f(i, j);
            glVertex3f(x * i / subdiv, y * j / subdiv, z);
            glTexCoord2f(i, j + 1.0);
            glVertex3f(x * i / subdiv, y * (j + 1.0) / subdiv, z);
            glTexCoord2f(i + 1.0, j + 1.0);
            glVertex3f(x * (i + 1.0) / subdiv, y * (j + 1.0) / subdiv, z);
            glTexCoord2f(i + 1.0, j);
            glVertex3f(x * (i + 1.0) / subdiv, y * j / subdiv, z);
        }
    }
    glEnd();

    // Side face 4
    glBegin(GL_QUADS);
    glNormal3f(0.0, 0.0, 1.0);
    for(int i=0; i<subdiv; i++)
    {
        for(int j=0; j<subdiv; j++)
        {
            glTexCoord2f(i, j);
            glVertex3f(x, y * i / subdiv, z * j / subdiv);
            glTexCoord2f(i + 1.0, j);
            glVertex3f(x, y * (i + 1.0) / subdiv, z * j / subdiv);
            glTexCoord2f(i + 1.0, j + 1.0);
            glVertex3f(x, y * (i + 1.0) / subdiv, z * (j + 1.0) / subdiv);
            glTexCoord2f(i, j + 1.0);
            glVertex3f(x, y * i / subdiv, z * (j + 1.0) / subdiv);
        }
    }
    glEnd();

    // Bottom face
    glBegin(GL_QUADS);
    glNormal3f(0.0, -1.0, 0.0);
    for(int i=0; i<subdiv; i++)
    {
        for(int j=0; j<subdiv; j++)
        {
            glTexCoord2f(i, j);
            glVertex3f(x * i / subdiv, 0.0, z * j / subdiv);
            glTexCoord2f(i, j + 1.0);
            glVertex3f(x * i / subdiv, 0.0, z * (j + 1.0) / subdiv);
            glTexCoord2f(i + 1.0, j + 1.0);
            glVertex3f(x * (i + 1.0) / subdiv, 0.0, z * (j + 1.0) / subdiv);
            glTexCoord2f(i + 1.0, j);
            glVertex3f(x * (i + 1.0) / subdiv, 0.0, z * j / subdiv);
        }
    }
    glEnd();
}

void rectCuboid(float x, float y, float z)
{
    rectCuboidDiv(x, y, z, 1);
}

void displayTeapot(float x, float y, float z, float rot)
{
    glPushMatrix();
    glTranslatef(x, y, z);
    glRotatef(rot, 0.0, 0.1, 0.0);
    glColor3f(0.97, 0.97, 0.85);
    glutSolidTeapot(2.0);
    glPopMatrix();
}

void displayTable(float posx, float posz, float height, float sizex, float sizez, float thickness)
{
    // Set material properties
    glColor3f(0.96, 0.91, 0.82);
    glMateriali(GL_FRONT, GL_SHININESS, 5);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textures[TEXTURE_WOOD]);

    // Start drawing table
    glPushMatrix();
    glTranslatef(posx, height, posz);
    rectCuboid(sizex, thickness, sizez);

    // Leg 1
    glRotatef(90.0, 1.0, 0.0, 0.0);
    glPushMatrix();
    glTranslatef(1.0, 1.0, 0.0);
    gluCylinder(gluNewQuadric(), 0.5, 0.2, height, 10.0, 10.0);
    glPopMatrix();

    // Leg 2
    glPushMatrix();
    glTranslatef(1.0, sizez - 1.0, 0.0);
    gluCylinder(gluNewQuadric(), 0.5, 0.2, height, 10.0, 10.0);
    glPopMatrix();

    // Leg 3
    glPushMatrix();
    glTranslatef(sizex - 1.0, sizez - 1.0, 0.0);
    gluCylinder(gluNewQuadric(), 0.5, 0.2, height, 10.0, 10.0);
    glPopMatrix();

    // Leg 4
    glPushMatrix();
    glTranslatef(sizex - 1.0, 1.0, 0.0);
    gluCylinder(gluNewQuadric(), 0.5, 0.2, height, 10.0, 10.0);
    glPopMatrix();

    glPopMatrix();

    glDisable(GL_TEXTURE_2D);
    glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SINGLE_COLOR);
}

void displayChair(float posx, float posz, float height, float seat_width, float seat_depth, float thickness)
{
    float chair_back_total_height = 7.0;
    float chair_back_height = seat_width / 2.0;
    float chair_back_width = seat_width - 1.0;
    glPushMatrix();
    displayTable(posx, posz, height, seat_width, seat_depth, thickness);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textures[TEXTURE_WOOD]);
    glTranslatef(posx, height, posz);
    glRotatef(-90.0, 1.0, 0.0, 0.0);
    glPushMatrix();
    glTranslatef(0.5, -thickness / 2.0, 0.5 * chair_back_total_height);
    rectCuboid(chair_back_width, thickness, chair_back_height);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.5, 0.0, 0.0);
    gluCylinder(gluNewQuadric(), 0.3, 0.3, chair_back_total_height, 10.0, 10.0);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(seat_width - 0.5, 0.0, 0.0);
    gluCylinder(gluNewQuadric(), 0.3, 0.3, chair_back_total_height, 10.0, 10.0);
    glPopMatrix();
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
    glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SINGLE_COLOR);
}

void displayFridge(float posx, float posz, float width, float height, float depth)
{
    float door_split = height * 0.7;
    int subdiv = 80;

    // Set material properties (chrome)
    GLfloat chrome_amb[] = {0.25, 0.25, 0.25, 1.0};
    GLfloat chrome_diff[] = {0.4, 0.4, 0.4};
    GLfloat chrome_spec[] = {0.774597, 0.774597, 0.774597};
    glMaterialfv(GL_FRONT, GL_AMBIENT, chrome_amb);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, chrome_diff);
    glMaterialfv(GL_FRONT, GL_SPECULAR, chrome_spec);
    glMaterialf(GL_FRONT, GL_SHININESS, 100.0);
    glColor3f(0.7, 0.7, 0.7);

    glPushMatrix();
    glTranslatef(posx, 0.0, posz);
    rectCuboidDiv(width, height, depth, subdiv); // fridge body

    glTranslatef(0.4, 0.0, depth);
    rectCuboid(width - 0.8, height, 0.2); // fridge rubber
    glTranslatef(-0.4, 0.0, 0.2);
    rectCuboidDiv(width, door_split - 0.1, 1.0, subdiv); // lower door
    glTranslatef(0.0, door_split + 0.1, 0.0);
    rectCuboidDiv(width, height - door_split - 0.1, 1.0, subdiv); // upper door
    glPopMatrix();
}

void displayBin(float posx, float posz, float width, float height)
{
    glPushMatrix();
    glTranslatef(posx, 0.0, posz);
    glRotatef(-90.0, 1.0, 0.0, 0.0);
    gluCylinder(gluNewQuadric(), width * 0.4, width * 0.5, height, 30.0, 30.0);
    glPopMatrix();
}

void drawHand()
{
    glEnable(GL_AUTO_NORMAL);
    glBegin(GL_POLYGON);
    glVertex3d(-0.358546, 0.658333, 0.121825);
    glVertex3d(-0.070166, 0.314921, -0.003107);
    glVertex3d(-0.345402, 0.168081, 0.022856);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex3d(-0.358546, 0.658333, 0.121825);
    glVertex3d(-0.345402, 0.168081, 0.022856);
    glVertex3d(-0.353634, -0.13327, -0.004367);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex3d(-0.363394, -0.543997, 0.205104);
    glVertex3d(-0.358546, 0.658333, 0.121825);
    glVertex3d(-0.353634, -0.13327, -0.004367);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex3d(-0.363394, -0.543997, 0.205104);
    glVertex3d(-0.353634, -0.13327, -0.004367);
    glVertex3d(-0.086638, -0.287777, -0.057556);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex3d(0.620158, -0.442214, 1.799144);
    glVertex3d(0.784323, -0.487413, 1.156149);
    glVertex3d(1.118022, -0.494098, 1.079097);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex3d(0.789202, 0.722864, 1.072322);
    glVertex3d(0.622418, 0.11785, 1.76035);
    glVertex3d(1.122906, 0.716177, 0.995272);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex3d(-0.584306, -0.469127, 1.34049);
    glVertex3d(-0.320341, -0.429321, 1.930518);
    glVertex3d(-0.854429, -0.467321, 1.35077);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex3d(-0.579426, 0.741151, 1.256662);
    glVertex3d(-0.849546, 0.742953, 1.266945);
    glVertex3d(-0.318086, 0.130742, 1.891729);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex3d(-0.58149, -0.498269, 0.919934);
    glVertex3d(-0.584306, -0.469127, 1.34049);
    glVertex3d(-0.854429, -0.467321, 1.35077);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex3d(-0.58149, -0.498269, 0.919934);
    glVertex3d(-0.854429, -0.467321, 1.35077);
    glVertex3d(-0.838866, -0.517187, 0.631746);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex3d(-0.579426, 0.741151, 1.256662);
    glVertex3d(-0.576606, 0.712011, 0.836104);
    glVertex3d(-0.849546, 0.742953, 1.266945);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex3d(-0.576606, 0.712011, 0.836104);
    glVertex3d(-0.833982, 0.693091, 0.547918);
    glVertex3d(-0.849546, 0.742953, 1.266945);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex3d(-0.201358, -0.525545, 0.548242);
    glVertex3d(-0.58149, -0.498269, 0.919934);
    glVertex3d(-0.838866, -0.517187, 0.631746);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex3d(-0.201358, -0.525545, 0.548242);
    glVertex3d(-0.838866, -0.517187, 0.631746);
    glVertex3d(-0.363394, -0.543997, 0.205104);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex3d(-0.576606, 0.712011, 0.836104);
    glVertex3d(-0.196478, 0.684733, 0.464414);
    glVertex3d(-0.833982, 0.693091, 0.547918);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex3d(-0.196478, 0.684733, 0.464414);
    glVertex3d(-0.358546, 0.658333, 0.121825);
    glVertex3d(-0.833982, 0.693091, 0.547918);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex3d(0.245558, -0.532001, 0.481058);
    glVertex3d(-0.201358, -0.525545, 0.548242);
    glVertex3d(-0.363394, -0.543997, 0.205104);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex3d(0.245558, -0.532001, 0.481058);
    glVertex3d(-0.363394, -0.543997, 0.205104);
    glVertex3d(0.262338, -0.551247, 0.114564);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex3d(-0.196478, 0.684733, 0.464414);
    glVertex3d(0.250442, 0.678277, 0.39723);
    glVertex3d(-0.358546, 0.658333, 0.121825);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex3d(0.250442, 0.678277, 0.39723);
    glVertex3d(0.267178, 0.648007, 0.031501);
    glVertex3d(-0.358546, 0.658333, 0.121825);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex3d(0.657842, -0.515621, 0.741518);
    glVertex3d(0.245558, -0.532001, 0.481058);
    glVertex3d(0.262338, -0.551247, 0.114564);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex3d(0.657842, -0.515621, 0.741518);
    glVertex3d(0.262338, -0.551247, 0.114564);
    glVertex3d(0.817018, -0.540276, 0.394886);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex3d(0.250442, 0.678277, 0.39723);
    glVertex3d(0.662726, 0.694653, 0.657693);
    glVertex3d(0.267178, 0.648007, 0.031501);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex3d(0.662726, 0.694653, 0.657693);
    glVertex3d(0.821906, 0.670001, 0.311058);
    glVertex3d(0.267178, 0.648007, 0.031501);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex3d(0.784323, -0.487413, 1.156149);
    glVertex3d(0.657842, -0.515621, 0.741518);
    glVertex3d(0.817018, -0.540276, 0.394886);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex3d(0.784323, -0.487413, 1.156149);
    glVertex3d(0.817018, -0.540276, 0.394886);
    glVertex3d(1.118022, -0.494098, 1.079097);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex3d(0.662726, 0.694653, 0.657693);
    glVertex3d(0.789202, 0.722864, 1.072322);
    glVertex3d(0.821906, 0.670001, 0.311058);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex3d(0.789202, 0.722864, 1.072322);
    glVertex3d(1.122906, 0.716177, 0.995272);
    glVertex3d(0.821906, 0.670001, 0.311058);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex3d(0.784323, -0.487413, 1.156149);
    glVertex3d(0.620158, -0.442214, 1.799144);
    glVertex3d(0.789202, 0.722864, 1.072322);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex3d(0.620158, -0.442214, 1.799144);
    glVertex3d(0.622418, 0.11785, 1.76035);
    glVertex3d(0.789202, 0.722864, 1.072322);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex3d(-0.320341, -0.429321, 1.930518);
    glVertex3d(-0.584306, -0.469127, 1.34049);
    glVertex3d(-0.318086, 0.130742, 1.891729);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex3d(-0.584306, -0.469127, 1.34049);
    glVertex3d(-0.579426, 0.741151, 1.256662);
    glVertex3d(-0.318086, 0.130742, 1.891729);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex3d(-0.584306, -0.469127, 1.34049);
    glVertex3d(-0.58149, -0.498269, 0.919934);
    glVertex3d(-0.579426, 0.741151, 1.256662);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex3d(-0.58149, -0.498269, 0.919934);
    glVertex3d(-0.576606, 0.712011, 0.836104);
    glVertex3d(-0.579426, 0.741151, 1.256662);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex3d(-0.58149, -0.498269, 0.919934);
    glVertex3d(-0.201358, -0.525545, 0.548242);
    glVertex3d(-0.576606, 0.712011, 0.836104);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex3d(-0.201358, -0.525545, 0.548242);
    glVertex3d(-0.196478, 0.684733, 0.464414);
    glVertex3d(-0.576606, 0.712011, 0.836104);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex3d(-0.201358, -0.525545, 0.548242);
    glVertex3d(0.245558, -0.532001, 0.481058);
    glVertex3d(-0.196478, 0.684733, 0.464414);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex3d(0.245558, -0.532001, 0.481058);
    glVertex3d(0.250442, 0.678277, 0.39723);
    glVertex3d(-0.196478, 0.684733, 0.464414);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex3d(0.245558, -0.532001, 0.481058);
    glVertex3d(0.657842, -0.515621, 0.741518);
    glVertex3d(0.250442, 0.678277, 0.39723);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex3d(0.657842, -0.515621, 0.741518);
    glVertex3d(0.662726, 0.694653, 0.657693);
    glVertex3d(0.250442, 0.678277, 0.39723);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex3d(0.657842, -0.515621, 0.741518);
    glVertex3d(0.784323, -0.487413, 1.156149);
    glVertex3d(0.662726, 0.694653, 0.657693);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex3d(0.784323, -0.487413, 1.156149);
    glVertex3d(0.789202, 0.722864, 1.072322);
    glVertex3d(0.662726, 0.694653, 0.657693);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex3d(0.620158, -0.442214, 1.799144);
    glVertex3d(1.118022, -0.494098, 1.079097);
    glVertex3d(1.122906, 0.716177, 0.995272);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex3d(0.620158, -0.442214, 1.799144);
    glVertex3d(1.122906, 0.716177, 0.995272);
    glVertex3d(0.622418, 0.11785, 1.76035);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex3d(-0.854429, -0.467321, 1.35077);
    glVertex3d(-0.320341, -0.429321, 1.930518);
    glVertex3d(-0.318086, 0.130742, 1.891729);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex3d(-0.854429, -0.467321, 1.35077);
    glVertex3d(-0.318086, 0.130742, 1.891729);
    glVertex3d(-0.849546, 0.742953, 1.266945);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex3d(-0.838866, -0.517187, 0.631746);
    glVertex3d(-0.854429, -0.467321, 1.35077);
    glVertex3d(-0.849546, 0.742953, 1.266945);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex3d(-0.838866, -0.517187, 0.631746);
    glVertex3d(-0.849546, 0.742953, 1.266945);
    glVertex3d(-0.833982, 0.693091, 0.547918);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex3d(-0.363394, -0.543997, 0.205104);
    glVertex3d(-0.838866, -0.517187, 0.631746);
    glVertex3d(-0.833982, 0.693091, 0.547918);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex3d(-0.363394, -0.543997, 0.205104);
    glVertex3d(-0.833982, 0.693091, 0.547918);
    glVertex3d(-0.358546, 0.658333, 0.121825);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex3d(0.817018, -0.540276, 0.394886);
    glVertex3d(0.262338, -0.551247, 0.114564);
    glVertex3d(0.267178, 0.648007, 0.031501);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex3d(0.817018, -0.540276, 0.394886);
    glVertex3d(0.267178, 0.648007, 0.031501);
    glVertex3d(0.821906, 0.670001, 0.311058);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex3d(1.118022, -0.494098, 1.079097);
    glVertex3d(0.817018, -0.540276, 0.394886);
    glVertex3d(0.821906, 0.670001, 0.311058);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex3d(1.118022, -0.494098, 1.079097);
    glVertex3d(0.821906, 0.670001, 0.311058);
    glVertex3d(1.122906, 0.716177, 0.995272);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex3d(-0.358546, 0.658333, 0.121825);
    glVertex3d(0.267178, 0.648007, 0.031501);
    glVertex3d(-0.070166, 0.314921, -0.003107);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex3d(0.267178, 0.648007, 0.031501);
    glVertex3d(0.19683, 0.160408, -0.05629);
    glVertex3d(-0.070166, 0.314921, -0.003107);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex3d(0.267178, 0.648007, 0.031501);
    glVertex3d(0.262338, -0.551247, 0.114564);
    glVertex3d(0.19683, 0.160408, -0.05629);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex3d(0.262338, -0.551247, 0.114564);
    glVertex3d(0.188598, -0.140939, -0.083516);
    glVertex3d(0.19683, 0.160408, -0.05629);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex3d(0.262338, -0.551247, 0.114564);
    glVertex3d(-0.363394, -0.543997, 0.205104);
    glVertex3d(0.188598, -0.140939, -0.083516);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex3d(-0.363394, -0.543997, 0.205104);
    glVertex3d(-0.086638, -0.287777, -0.057556);
    glVertex3d(0.188598, -0.140939, -0.083516);
    glEnd();
    glDisable(GL_AUTO_NORMAL);
}

void displayRobot()
{
    glMaterialf(GL_FRONT, GL_SHININESS, 70.0);
    GLfloat arm_length = 3.0;
    GLfloat arm_pos[3] = {0.0, ROBOT_BODY_HEIGHT * (float)0.8, ROBOT_BODY_DEPTH * (float)0.5};
    glColor3f(0.6, 0.7, 1.0);

    glPushMatrix();
    glTranslatef(robotx, ROBOT_BODY_DEPTH * 0.5, robotz);
    glTranslatef(ROBOT_BODY_WIDTH * 0.5, 0.0, ROBOT_BODY_DEPTH * 0.5);
    glRotatef(robot_y_rotate, 0.0, 1.0, 0.0);
    glTranslatef(-ROBOT_BODY_WIDTH * 0.5, 0.0, -ROBOT_BODY_DEPTH * 0.5);
    

    /* BODY */
    glPushMatrix();
    rectCuboid(ROBOT_BODY_WIDTH, ROBOT_BODY_HEIGHT, ROBOT_BODY_DEPTH);
    glPopMatrix();

    /* NECK */
    glPushMatrix();
    glTranslatef(ROBOT_BODY_WIDTH / 2.0, ROBOT_BODY_HEIGHT, ROBOT_BODY_DEPTH / 2.0);
    glRotatef(-90.0, 1.0, 0.0, 0.0);
    gluCylinder(gluNewQuadric(), 0.9, 0.9, ROBOT_NECK_LENGTH * 1.5, 10.0, 10.0);
    glPopMatrix();

    /* HEAD */
    glPushMatrix();
    glTranslatef(ROBOT_BODY_WIDTH * 0.625, ROBOT_BODY_HEIGHT + ROBOT_NECK_LENGTH, ROBOT_BODY_DEPTH * 0.5);
    glRotatef(head_x_rotate, 1.0, 0.0, 0.0);
    glRotatef(head_y_rotate, 0.0, 1.0, 0.0);
    glTranslatef(-ROBOT_BODY_WIDTH * 0.5, 0.0, -ROBOT_BODY_DEPTH * 0.5);
    rectCuboid(ROBOT_BODY_WIDTH * 0.75, ROBOT_BODY_HEIGHT * 0.4, ROBOT_BODY_DEPTH);

    /* FACE */
    glPushMatrix();
    glTranslatef(ROBOT_BODY_WIDTH * 0.375, ROBOT_BODY_HEIGHT * 0.15, ROBOT_BODY_DEPTH);
    gluCylinder(gluNewQuadric(), 0.4, 0.1, 0.4, 10.0, 10.0);
    glColor3f(0.1, 0.1, 0.2);
    glMaterialf(GL_FRONT, GL_SHININESS, 128.0);
    glTranslatef(ROBOT_BODY_WIDTH * 0.1875, ROBOT_BODY_HEIGHT * 0.1, -0.5);
    gluSphere(gluNewQuadric(), 0.8, 30, 30);
    glTranslatef(-ROBOT_BODY_WIDTH * 0.375, 0.0, 0.0);
    gluSphere(gluNewQuadric(), 0.8, 30, 30);
    glPopMatrix();
    glPopMatrix();

    /* WHEELS */
    glColor3f(0.3, 0.3, 0.3); 
    glMaterialf(GL_FRONT, GL_SHININESS, 70.0);
    glPushMatrix();
    glTranslatef(0.2, 0.0, ROBOT_BODY_DEPTH * 0.5);
    glRotatef(90.0, 0.0, 1.0, 0.0);
    gluCylinder(gluNewQuadric(), ROBOT_BODY_DEPTH * 0.5, ROBOT_BODY_DEPTH * 0.5, ROBOT_BODY_WIDTH * 0.25, 10.0, 10.0);
    glTranslatef(0.0, 0.0, (ROBOT_BODY_WIDTH * 0.75) - 0.4);
    gluCylinder(gluNewQuadric(), ROBOT_BODY_DEPTH * 0.5, ROBOT_BODY_DEPTH * 0.5, ROBOT_BODY_WIDTH * 0.25, 10.0, 10.0);
    glPopMatrix();
    glColor3f(0.6, 0.7, 1.0);

    glPushMatrix(); // arm
    glTranslatef(arm_pos[0], arm_pos[1], arm_pos[2]);
    gluSphere(gluNewQuadric(), 0.7, 10, 10);
    glRotatef(shoulder_x_rotate, 1.0, 0.0, 0.0);
    glRotatef(shoulder_y_rotate, 0.0, 1.0, 0.0);
    glPushMatrix(); // upper arm
    gluCylinder(gluNewQuadric(), 0.7, 0.7, arm_length, 10.0, 10.0);
    glTranslatef(0.0, 0.0, arm_length);
    gluSphere(gluNewQuadric(), 0.7, 10, 10);

    glPushMatrix(); // lower arm
    glRotatef(elbow_x_rotate, 1.0, 0.0, 0.0);
    gluCylinder(gluNewQuadric(), 0.7, 0.5, arm_length, 10.0, 10.0);
    glTranslatef(0.0, 0.0, arm_length);
    gluSphere(gluNewQuadric(), 0.5, 10, 10);

    glPushMatrix();
    glRotatef(hand_y_rotate, 0.0, 0.0, 1.0);
    glColor3f(0.1, 0.1, 0.1);
    drawHand();

    glPopMatrix(); // hand
    glPopMatrix(); // lower arm
    glPopMatrix(); // upper arm
    glPopMatrix(); // arm
    glPopMatrix(); // body
}

void displayString(float x, float y, void *font, const char *string)
{
    glDisable(GL_LIGHTING);
    const char *c;
    glRasterPos2f(x, y);
    for (c=string; *c != '\0'; c++) {
        glutBitmapCharacter(font, *c);
    }
    glEnable(GL_LIGHTING);
}

void displayAdjustAmbient()
{
    if(adjust_ambient)
    {    GLfloat box_w = AMBIENT_BOX_W / win_width, box_h = AMBIENT_BOX_H / win_height,
                box_x = 0.5 - box_w * 0.5,
                box_y = 0.5 - box_w * 0.5;

        std::string s(user_input.begin(), user_input.end());

        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        gluOrtho2D(0.0, 1.0, 0.0, 1.0);

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glColor4f(0.0, 0.0, 0.0, 0.7);
        glBegin(GL_QUADS);
        glVertex2f(box_x, box_y);
        glVertex2f(box_x + box_w, box_y);
        glVertex2f(box_x + box_w, box_y + box_h);
        glVertex2f(box_x, box_y + box_h);
        glEnd();
        glDisable(GL_BLEND);

        glColor3f(1.0, 1.0, 1.0);
        displayString(box_x + (30.0 / win_width), box_y + box_h - (30.0 / win_height), GLUT_BITMAP_HELVETICA_18, "Adjust ambient light");
        displayString(box_x + (30.0 / win_width), box_y + box_h - (48.0 / win_height), GLUT_BITMAP_HELVETICA_12, "Enter 3 integers between 0-100 seperated by spaces");
        displayString(box_x + (30.0 / win_width), box_y + box_h - (100.0 / win_height), GLUT_BITMAP_TIMES_ROMAN_24, s.c_str());
        glColor3f(1.0, 0.0, 0.0);
        displayString(box_x + (120.0 / win_width), box_y + (30.0 / win_height), GLUT_BITMAP_HELVETICA_12, "Press [ENTER] to continue");
        glEnable(GL_DEPTH_TEST);

        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();}
}

void toggleAdjustAmbient()
{
    if(!show_help) // show only when user not reading help window
        {
            adjust_ambient = true;
        }
}

void displayHelp()
{
    if(show_help)
    {   
        GLfloat box_w, box_h;
        if(win_width >= START_WIDTH && win_height >= START_HEIGHT) // set to max size
        {
            box_w = HELP_MAX_WIDTH / win_width;
            box_h = HELP_MAX_HEIGHT / win_height;
        }
        else // set to relative size
        {
            box_w = 0.8;
            box_h = ( box_w * win_width * 2.0 ) / (win_height * 3.0);
        }
        GLfloat box_x = 0.5 - box_w * 0.5,
                box_y = 0.5 - box_w * 0.5;

        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        gluOrtho2D(0.0, 1.0, 0.0, 1.0);

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, textures[TEXTURE_HELP]);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
        glColor4f(0.0, 0.0, 0.0, 0.7);
        glBegin(GL_QUADS);
        glTexCoord2f(0.0, 1.0);
        glVertex2f(box_x, box_y);
        glTexCoord2f(1.0, 1.0);
        glVertex2f(box_x + box_w, box_y);
        glTexCoord2f(1.0, 0.0);
        glVertex2f(box_x + box_w, box_y + box_h);
        glTexCoord2f(0.0, 0.0);
        glVertex2f(box_x, box_y + box_h);
        glEnd();
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_LIGHTING);

        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();}
}

void toggleHelp()
{
    show_help = !show_help;
}

void displayMovingMode()
{
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0.0, 1.0, 1.0, 0.0);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glDisable(GL_DEPTH_TEST);
    glColor3f(0.0, 1.0, 0.0);
    switch (mvstate)
    {
    case MOV_ROBOT:
        displayString(5.0 / win_width, 25.0 / win_height, GLUT_BITMAP_TIMES_ROMAN_24, "Robot Mode");
        break;
    
    case MOV_CAM:
        displayString(5.0 / win_width, 25.0 / win_height, GLUT_BITMAP_TIMES_ROMAN_24, "Camera Mode");
        break;

    case MOV_LIGHT:
        displayString(5.0 / win_width, 25.0 / win_height, GLUT_BITMAP_TIMES_ROMAN_24, "Light Mode");
        break;
    }
    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void HandleKeystrokes(unsigned char key)
{
    if(adjust_ambient)
    {
        if((key >= '0' && key <= '9') || key == KEY_SPACE)
            user_input.push_back(key);

        else if(key == KEY_BACKSPACE)
            user_input.pop_back();

        else if(key == KEY_ENTER)
        {
            std::string s(user_input.begin(), user_input.end());
            sscanf(s.c_str(), "%f %f %f", &red, &green, &blue);
            red *= 0.01;
            green *= 0.01;
            blue *= 0.01;
            adjust_ambient = false;
            user_input.clear();
        }

        else if(key == KEY_ESC)
        {
            adjust_ambient = false;
            user_input.clear();
        }
    }
}

void Keyboard(unsigned char key, int x, int y)
{
    HandleKeystrokes(key);
    switch (mvstate)
    {
    case MOV_ROBOT:
        switch (key)
        {
        case 'd': // MOVE HEAD RIGHT
            if(head_y_rotate > -60.0)
                head_y_rotate -= 1;
            break;

        case 'a': // MOVE HEAD LEFT
            if(head_y_rotate < 60.0)
                head_y_rotate += 1;
            break;

        case 'w': // MOVE HEAD UP
            if(head_x_rotate > -10.0)
                head_x_rotate -= 1;
            break;

        case 's': // MOVE HEAD DOWN
            if(head_x_rotate < 17.0)
                head_x_rotate += 1;
            break;
        
        case 'h': // MOVE ARM RIGHT
            if(shoulder_y_rotate > -120.0)
                shoulder_y_rotate -= 1;
            break;

        case 'f': // MOVE ARM LEFT
            if(shoulder_y_rotate < -10.0)
                shoulder_y_rotate += 1;
            break;

        case 't': // MOVE ARM UP
            if(shoulder_x_rotate > 0.0)
                shoulder_x_rotate -= 1;
            break;

        case 'g': // MOVE ARM DOWN
            if(shoulder_x_rotate < 90.0)
                shoulder_x_rotate += 1;
            break;

        case 'j': // CLOSE ELBOW
            if(elbow_x_rotate > -160.0)
                elbow_x_rotate -= 1;
            break;

        case 'u': // OPEN ELBOW
            if(elbow_x_rotate < 0.0)
                elbow_x_rotate += 1;
            break;

        case 'i': // ROTATE HAND INWARDS
            hand_y_rotate += 1;
            break;

        case 'k': // ROTATE HAND OUTWARDS
            hand_y_rotate -= 1;
            break;

        default:
            break;
        }
        break;
    case MOV_LIGHT:
        switch (key)
        {
        case 'w': // MOVE LIGHT DIRECTION UP
            if(light_yref < 0.0)
                light_yref += 0.01;
            break;

        case 's': // MOVE LIGHT DIRECTION DOWN
            if(light_yref > -1.0)
                light_yref -= 0.01;
            break;

        case 'd': // MOVE LIGHT DIRECTION TOWARDS X+
            if(light_xref < 1.0)
                light_xref += 0.01;
            break;

        case 'a': // MOVE LIGHT DIRECTION TOWARDS X-
            if(light_xref > -1.0)
                light_xref -= 0.01;
            break;

        case 'e': // MOVE LIGHT DIRECTION TOWARDS Z+
            if(light_zref < 1.0)
                light_zref += 0.01;
            break;

        case 'q': // MOVE LIGHT DIRECTION TOWARDS Z-
            if(light_zref > -1.0)
                light_zref -= 0.01;
            break;

        case 'r':
            if(red < 1.0 && green < 1.0 && blue < 1.0)
            {
                red += 0.05;
                green += 0.05;
                blue += 0.05;
            }   
            break;

        case 'f':
            if(red > 0.0 && blue > 0.0 && green > 0.0)
            {
                red -= 0.05;
                blue -= 0.05;
                green -= 0.05;
            }
            break;
        }
        break;
    }

    glutPostRedisplay();
}

void SpecialKeyboard(int key, int x, int y)
{
    switch (key)
    {
    case GLUT_KEY_F1: // SHOW/HIDE HELP
        show_help = !show_help;
        break;

    case GLUT_KEY_F2: // MOVE ROBOT STATE
        mvstate = MOV_ROBOT;
        break;

    case GLUT_KEY_F3: // TOGGLE FIRST PERSON VIEW
        first_person = !first_person;
        if(first_person)
        {
            mvstate = MOV_ROBOT;
            glutPostRedisplay();
        }
        break;

    case GLUT_KEY_F4: // MOVE CAMERA
        mvstate = MOV_CAM;
        break;

    case GLUT_KEY_F5: // MOVE LIGHT SOURCE
        mvstate = MOV_LIGHT;
        break;
    
    default:
        switch (mvstate)
        {
        case MOV_ROBOT:
            switch (key)
            {
            case GLUT_KEY_UP: // MOVE ROBOT FORWARD
                if(robotx < 50.0)
                    robotx += 0.5 * sin(robot_y_rotate * PI / 180.0);
                if(robotz < 50.0)
                    robotz += 0.5 * cos(robot_y_rotate * PI / 180.0);
                break;

            case GLUT_KEY_DOWN: // MOVE ROBOT BACKWARDS
                if(robotx > -50.0)
                    robotx += 0.5 * sin((180.0 + robot_y_rotate) * PI / 180.0);
                if(robotz > -50.0)
                    robotz += 0.5 * cos((180.0 + robot_y_rotate) * PI / 180.0);
                break;

            case GLUT_KEY_RIGHT: // TURN RIGHT
                robot_y_rotate -= 2;
                break;

            case GLUT_KEY_LEFT: // TURN LEFT
                robot_y_rotate += 2;
                break;
            
            default:
                break;
            }
            break;
        
        case MOV_CAM:
            switch (key)
            {
            case GLUT_KEY_UP: // MOVE CAMERA CLOSER
                if(cam_dist > 0.0)
                    cam_dist -= 1;
                break;

            case GLUT_KEY_DOWN: // MOVE CAMERA AWAY
                if(cam_dist < 120.0)
                    cam_dist += 1;
                break;

            case GLUT_KEY_RIGHT: // ROTATE CAMERA RIGHT
                world_rot -= 1;
                break;

            case GLUT_KEY_LEFT: // ROTATE CAMERA LEFT
                world_rot += 1;
                break;
            
            default:
                break;
            }

        case(MOV_LIGHT):
            switch (key)
            {
            case GLUT_KEY_UP: // MOVE LIGHT SOURCE UP
                if(light_y < 240.0)
                    light_y += 1;
                break;

            case GLUT_KEY_DOWN: // MOVE LIGHT SOURCE DOWN
                if(light_y > 0.0)
                    light_y -= 1;
                break;

            case GLUT_KEY_RIGHT: // MOVE LIGHT SOURCE TOWARDS X+
                if(light_x < 100.0)
                    light_x += 1;
                break;

            case GLUT_KEY_LEFT: // MOVE LIGHT SOURCE TOWARDS X-
                if(light_x > -100.0)
                    light_x -= 1;
                break;

            case GLUT_KEY_END: // MOVE LIGHT SOURCE TOWARDS Z+
                if(light_z < 100.0)
                    light_z += 1;
                break;

            case GLUT_KEY_HOME: // MOVE LIGHT SOURCE TOWARDS Z-
                if(light_z > -100.0)
                    light_z -= 1;
                break;
            }
        }
        break;
    }
    glutPostRedisplay();
}
