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
        glRotatef(head_x_rotate, -1.0f, 0.0f, 0.0f);
        glRotatef(robot_y_rotate + head_y_rotate, 0.0f, -1.0f, 0.0f);
        glTranslatef(-robotx, 0.0f, -robotz);
    }
    else
    {
        gluLookAt(cam_dist, cam_dist, cam_dist, xref, yref, zref, Vx, Vy, Vz);
        glRotatef(world_rot, 0.0f, 1.0f, 0.0f);
    }
}

// Initializes a single positional light source.
// All objects have a baseline of ambient and diffuse lighting, but some
// objects have special material properties that are declared seperately.
// The light source has a spotlight cutoff in order to see the effect
// of changing the light's direction. 
void initLight()
{
    GLfloat white_light[] = {1.0f, 1.0f, 1.0f, 0.0f};
    GLfloat light_position[] = {light_x, light_y, light_z, 1.0f};
    GLfloat lmodel_ambient[] = {red, green, blue, 1.0f};
    GLfloat light_direction[] = {light_xref, light_yref, light_zref};
    glLightfv(GL_LIGHT0, GL_DIFFUSE, white_light);
    glLightfv(GL_LIGHT0, GL_SPECULAR, white_light);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 90.0f);
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
    glTranslatef(-size / 2.0f, 0.0f, -size / 2.0f);
    glColor3f(1.0f, 1.0f, 1.0f);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textures[TEXTURE_MARBLE]);
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f);
    // Dividing the floor into smaller quads lets the floor REALLY shine.
    int subdiv = 42;
    float texrepeat = subdiv / 4.0f;
    for(int i=0; i<subdiv; i++)
    {
        for(int j=0; j<subdiv; j++)
        {
            glTexCoord2f(i / texrepeat, j / texrepeat);
            glVertex3f(size * i / subdiv, 0.0f, size * j / subdiv);
            glTexCoord2f(i / texrepeat, (j + 1.0f) / texrepeat);
            glVertex3f(size * i / subdiv, 0.0f, size * (j + 1.0f) / subdiv);
            glTexCoord2f((i + 1.0f) / texrepeat, (j + 1.0f) / texrepeat);
            glVertex3f(size * (i + 1.0f) / subdiv, 0.0f, size * (j + 1.0f) / subdiv);
            glTexCoord2f((i + 1.0f) / texrepeat, j / texrepeat);
            glVertex3f(size * (i + 1.0f) / subdiv, 0.0f, size * j / subdiv);
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
    glColor3f(1.0f, 1.0f, 1.0f);
    glEnable(GL_TEXTURE_1D);
    glBindTexture(GL_TEXTURE_1D, textures[TEXTURE_WALL]);

    // This is the actual position of the camera, given that the initial
    // camera position is at equal spaces from each axis.
    float camera_pos = world_rot - 45.0f;

    if(first_person || cos(camera_pos * PI / 180.0) >= 0.0)
    {
        // draw -Z wall (fridge side)
        glPushMatrix();
        glBegin(GL_QUADS);
        glTexCoord1f(0.0f);
        glVertex3f(-size * 0.5f, height, -size * 0.5f);
        glTexCoord1f(1.0f);
        glVertex3f(size * 0.5f, height, -size * 0.5f);
        glTexCoord1f(1.0f);
        glVertex3f(size * 0.5f, 0.0f, -size * 0.5f);
        glTexCoord1f(0.0f);
        glVertex3f(-size * 0.5f, 0.0f, -size * 0.5f);
        glEnd();
        glPopMatrix();
    }
    if(first_person || cos(camera_pos * PI / 180.0) <= 0.0)
    {
        // draw +Z wall (table side)
        glPushMatrix();
        glBegin(GL_QUADS);
        glTexCoord1f(0.0f);
        glVertex3f(-size * 0.5f, height, size * 0.5f);
        glTexCoord1f(1.0f);
        glVertex3f(size * 0.5f, height, size * 0.5f);
        glTexCoord1f(1.0f);
        glVertex3f(size * 0.5f, 0.0f, size * 0.5f);
        glTexCoord1f(0.0f);
        glVertex3f(-size * 0.5f, 0.0f, size * 0.5f);
        glEnd();
        glPopMatrix();
    }
    if(first_person || sin(camera_pos * PI / 180.0) >= 0.0)
    {
        // draw +X wall (no arm side)
        glPushMatrix();
        glBegin(GL_QUADS);
        glTexCoord1f(0.0f);
        glVertex3f(size * 0.5f, height, size * 0.5f);
        glTexCoord1f(1.0f);
        glVertex3f(size * 0.5f, height, -size * 0.5f);
        glTexCoord1f(1.0f);
        glVertex3f(size * 0.5f, 0.0f, -size * 0.5f);
        glTexCoord1f(0.0f);
        glVertex3f(size * 0.5f, 0.0f, size * 0.5f);
        glEnd();
        glPopMatrix();
    }
    if(first_person || sin(camera_pos * PI / 180.0) <= 0.0)
    {
        // draw -X wall (arm side)
        glPushMatrix();
        glBegin(GL_QUADS);
        glTexCoord1f(0.0f);
        glVertex3f(-size * 0.5f, height, size * 0.5f);
        glTexCoord1f(1.0f);
        glVertex3f(-size * 0.5f, height, -size * 0.5f);
        glTexCoord1f(1.0f);
        glVertex3f(-size * 0.5f, 0.0f, -size * 0.5f);
        glTexCoord1f(0.0f);
        glVertex3f(-size * 0.5f, 0.0f, size * 0.5f);
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
    glNormal3f(0.0f, 1.0f, 0.0f);
    for(int i=0; i<subdiv; i++)
    {
        for(int j=0; j<subdiv; j++)
        {
            glTexCoord2f(i, j);
            glVertex3f(x * i / subdiv, y, z * j / subdiv);
            glTexCoord2f(i + 1.0f, j);
            glVertex3f(x * (i + 1.0f) / subdiv, y, z * j / subdiv);
            glTexCoord2f(i + 1.0f, j + 1.0f);
            glVertex3f(x * (i + 1.0f) / subdiv, y, z * (j + 1.0f) / subdiv);
            glTexCoord2f(i, j + 1.0f);
            glVertex3f(x * i / subdiv, y, z * (j + 1.0f) / subdiv);
        }
    }
    glEnd();

    // Side face 1
    glBegin(GL_QUADS);
    glNormal3f(-1.0f, 0.0f, 0.0f);
    for(int i=0; i<subdiv; i++)
    {
        for(int j=0; j<subdiv; j++)
        {
            glTexCoord2f(i, j);
            glVertex3f(0.0f, y * i / subdiv, z * j / subdiv);
            glTexCoord2f(i + 1.0f, j);
            glVertex3f(0.0f, y * (i + 1.0f) / subdiv, z * j / subdiv);
            glTexCoord2f(i + 1.0f, j + 1.0f);
            glVertex3f(0.0f, y * (i + 1.0f) / subdiv, z * (j + 1.0f) / subdiv);
            glTexCoord2f(i, j + 1.0f);
            glVertex3f(0.0f, y * i / subdiv, z * (j + 1.0f) / subdiv);
        }
    }
    glEnd();

    // Side face 2
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 0.0f, -1.0f);
    for(int i=0; i<subdiv; i++)
    {
        for(int j=0; j<subdiv; j++)
        {
            glTexCoord2f(i, j);
            glVertex3f(x * i / subdiv, y * j / subdiv, 0.0f);
            glTexCoord2f(i, j + 1.0f);
            glVertex3f(x * i / subdiv, y * (j + 1.0f) / subdiv, 0.0f);
            glTexCoord2f(i + 1.0f, j + 1.0f);
            glVertex3f(x * (i + 1.0f) / subdiv, y * (j + 1.0f) / subdiv, 0.0f);
            glTexCoord2f(i + 1.0f, j);
            glVertex3f(x * (i + 1.0f) / subdiv, y * j / subdiv, 0.0f);
        }
    }
    glEnd();

    // Side face 3
    glBegin(GL_QUADS);
    glNormal3f(1.0f, 0.0f, 0.0f);
    for(int i=0; i<subdiv; i++)
    {
        for(int j=0; j<subdiv; j++)
        {
            glTexCoord2f(i, j);
            glVertex3f(x * i / subdiv, y * j / subdiv, z);
            glTexCoord2f(i, j + 1.0f);
            glVertex3f(x * i / subdiv, y * (j + 1.0f) / subdiv, z);
            glTexCoord2f(i + 1.0f, j + 1.0f);
            glVertex3f(x * (i + 1.0f) / subdiv, y * (j + 1.0f) / subdiv, z);
            glTexCoord2f(i + 1.0f, j);
            glVertex3f(x * (i + 1.0f) / subdiv, y * j / subdiv, z);
        }
    }
    glEnd();

    // Side face 4
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 0.0f, 1.0f);
    for(int i=0; i<subdiv; i++)
    {
        for(int j=0; j<subdiv; j++)
        {
            glTexCoord2f(i, j);
            glVertex3f(x, y * i / subdiv, z * j / subdiv);
            glTexCoord2f(i + 1.0f, j);
            glVertex3f(x, y * (i + 1.0f) / subdiv, z * j / subdiv);
            glTexCoord2f(i + 1.0f, j + 1.0f);
            glVertex3f(x, y * (i + 1.0f) / subdiv, z * (j + 1.0f) / subdiv);
            glTexCoord2f(i, j + 1.0f);
            glVertex3f(x, y * i / subdiv, z * (j + 1.0f) / subdiv);
        }
    }
    glEnd();

    // Bottom face
    glBegin(GL_QUADS);
    glNormal3f(0.0f, -1.0f, 0.0f);
    for(int i=0; i<subdiv; i++)
    {
        for(int j=0; j<subdiv; j++)
        {
            glTexCoord2f(i, j);
            glVertex3f(x * i / subdiv, 0.0f, z * j / subdiv);
            glTexCoord2f(i, j + 1.0f);
            glVertex3f(x * i / subdiv, 0.0f, z * (j + 1.0f) / subdiv);
            glTexCoord2f(i + 1.0f, j + 1.0f);
            glVertex3f(x * (i + 1.0f) / subdiv, 0.0f, z * (j + 1.0f) / subdiv);
            glTexCoord2f(i + 1.0f, j);
            glVertex3f(x * (i + 1.0f) / subdiv, 0.0f, z * j / subdiv);
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
    glRotatef(rot, 0.0f, 0.1f, 0.0f);
    glColor3f(0.97f, 0.97f, 0.85f);
    glutSolidTeapot(2.0);
    glPopMatrix();
}

void displayTable(float posx, float posz, float height, float sizex, float sizez, float thickness)
{
    // Set material properties
    glColor3f(0.96f, 0.91f, 0.82f);
    glMateriali(GL_FRONT, GL_SHININESS, 5);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textures[TEXTURE_WOOD]);

    // Start drawing table
    glPushMatrix();
    glTranslatef(posx, height, posz);
    rectCuboid(sizex, thickness, sizez);

    // Leg 1
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    glPushMatrix();
    glTranslatef(1.0f, 1.0f, 0.0f);
    gluCylinder(gluNewQuadric(), 0.5, 0.2, height, 10.0, 10.0);
    glPopMatrix();

    // Leg 2
    glPushMatrix();
    glTranslatef(1.0f, sizez - 1.0f, 0.0f);
    gluCylinder(gluNewQuadric(), 0.5, 0.2, height, 10.0, 10.0);
    glPopMatrix();

    // Leg 3
    glPushMatrix();
    glTranslatef(sizex - 1.0f, sizez - 1.0f, 0.0f);
    gluCylinder(gluNewQuadric(), 0.5, 0.2, height, 10.0, 10.0);
    glPopMatrix();

    // Leg 4
    glPushMatrix();
    glTranslatef(sizex - 1.0f, 1.0f, 0.0f);
    gluCylinder(gluNewQuadric(), 0.5, 0.2, height, 10.0, 10.0);
    glPopMatrix();

    glPopMatrix();

    glDisable(GL_TEXTURE_2D);
    glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SINGLE_COLOR);
}

// Draws the chair using displayTable function to create the seat and legs.
void displayChair(float posx, float posz, float height, float seat_width, float seat_depth, float thickness)
{
    float chair_back_total_height = 7.0;
    float chair_back_height = seat_width / 2.0f;
    float chair_back_width = seat_width - 1.0f;
    glPushMatrix();
    displayTable(posx, posz, height, seat_width, seat_depth, thickness);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textures[TEXTURE_WOOD]);
    glTranslatef(posx, height, posz);
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    glPushMatrix();
    glTranslatef(0.5f, -thickness / 2.0f, 0.5f * chair_back_total_height);
    rectCuboid(chair_back_width, thickness, chair_back_height);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.5f, 0.0f, 0.0f);
    gluCylinder(gluNewQuadric(), 0.3, 0.3, chair_back_total_height, 10.0, 10.0);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(seat_width - 0.5f, 0.0f, 0.0f);
    gluCylinder(gluNewQuadric(), 0.3, 0.3, chair_back_total_height, 10.0, 10.0);
    glPopMatrix();
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
    glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SINGLE_COLOR);
}

// Uses a divided rectangular cuboid in order to get that stainless steel
// shine. I took the material properties from the OpenGL red book example
// for a chrome teapot and tweaked the numbers a bit.
void displayFridge(float posx, float posz, float width, float height, float depth)
{
    float door_split = height * 0.7f;
    int subdiv = 80;

    // Set material properties (chrome)
    GLfloat chrome_amb[] = {0.25f, 0.25f, 0.25f, 1.0f};
    GLfloat chrome_diff[] = {0.4f, 0.4f, 0.4f};
    GLfloat chrome_spec[] = {0.774597f, 0.774597f, 0.774597f};
    glMaterialfv(GL_FRONT, GL_AMBIENT, chrome_amb);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, chrome_diff);
    glMaterialfv(GL_FRONT, GL_SPECULAR, chrome_spec);
    glMaterialf(GL_FRONT, GL_SHININESS, 100.0);
    glColor3f(0.7f, 0.7f, 0.7f);

    glPushMatrix();
    glTranslatef(posx, 0.0f, posz);
    rectCuboidDiv(width, height, depth, subdiv); // fridge body

    glTranslatef(0.4f, 0.0f, depth);
    rectCuboid(width - 0.8f, height, 0.2f); // fridge rubber
    glTranslatef(-0.4f, 0.0f, 0.2f);
    rectCuboidDiv(width, door_split - 0.1f, 1.0f, subdiv); // lower door
    glTranslatef(0.0f, door_split + 0.1f, 0.0f);
    rectCuboidDiv(width, height - door_split - 0.1f, 1.0f, subdiv); // upper door
    glPopMatrix();
}

// This function only displays the garbag bin, without defining a material.
void displayBin(float posx, float posz, float width, float height)
{
    glPushMatrix();
    glTranslatef(posx, 0.0f, posz);
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    gluCylinder(gluNewQuadric(), width * 0.4, width * 0.5, height, 30.0, 30.0);
    glPopMatrix();
}

// Draws a Lego man style hand.
// I wanted a Lego man style hand for the robot, but didn't have enough
// time to do it properly. This is an .obj file converted to GL functions
// by a Python script found on the internet. It doesn't parse texture 
// coordinates or normals, unfortunately.
void drawHand()
{
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
}

void displayRobot()
{
    // Sets material properties
    glMaterialf(GL_FRONT, GL_SHININESS, 70.0f);
    glColor3f(0.6f, 0.7f, 1.0f);

    //////////// BODY ////////////
    glPushMatrix();
    // Positions robot
    glTranslatef(robotx, ROBOT_BODY_DEPTH * 0.5f, robotz);

    // Rotates robot around itself
    glTranslatef(ROBOT_BODY_WIDTH * 0.5f, 0.0f, ROBOT_BODY_DEPTH * 0.5f);
    glRotatef(robot_y_rotate, 0.0f, 1.0f, 0.0f);
    glTranslatef(-ROBOT_BODY_WIDTH * 0.5f, 0.0f, -ROBOT_BODY_DEPTH * 0.5f); 
    
    rectCuboid(ROBOT_BODY_WIDTH, ROBOT_BODY_HEIGHT, ROBOT_BODY_DEPTH);

    //////////// NECK //////////// 
    glPushMatrix();
    // Positions neck
    glTranslatef(ROBOT_BODY_WIDTH / 2.0f, ROBOT_BODY_HEIGHT, ROBOT_BODY_DEPTH / 2.0f);

    // Rotates neck because gluCylinder() draws them around z axis
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);

    gluCylinder(gluNewQuadric(), 0.9f, 0.9f, ROBOT_NECK_LENGTH * 1.5f, 10.0f, 10.0f);
    glPopMatrix(); // neck

    ///////////// HEAD ////////////
    glPushMatrix();
    // Positions head
    glTranslatef(ROBOT_BODY_WIDTH * 0.125f, ROBOT_BODY_HEIGHT + ROBOT_NECK_LENGTH, 0.0f);

    // Pitch and yaw
    glTranslatef(ROBOT_BODY_WIDTH * 0.5f, 0.0f, ROBOT_BODY_DEPTH * 0.5f);
    glRotatef(head_x_rotate, 1.0f, 0.0f, 0.0f);
    glRotatef(head_y_rotate, 0.0f, 1.0f, 0.0f);
    glTranslatef(-ROBOT_BODY_WIDTH * 0.5f, 0.0f, -ROBOT_BODY_DEPTH * 0.5f);

    rectCuboid(ROBOT_BODY_WIDTH * 0.75f, ROBOT_BODY_HEIGHT * 0.4f, ROBOT_BODY_DEPTH);

    //////////// FACE ////////////
    glPushMatrix();
    // Nose
    glTranslatef(ROBOT_BODY_WIDTH * 0.375f, ROBOT_BODY_HEIGHT * 0.15f, ROBOT_BODY_DEPTH);
    gluCylinder(gluNewQuadric(), 0.4f, 0.1f, 0.4f, 10.0f, 10.0f);

    // eyes
    glColor3f(0.1f, 0.1f, 0.2f);
    glMaterialf(GL_FRONT, GL_SHININESS, 128.0f);
    glTranslatef(ROBOT_BODY_WIDTH * 0.1875f, ROBOT_BODY_HEIGHT * 0.1f, -0.5f);
    gluSphere(gluNewQuadric(), 0.8, 30, 30);
    glTranslatef(-ROBOT_BODY_WIDTH * 0.375f, 0.0, 0.0);
    gluSphere(gluNewQuadric(), 0.8, 30, 30);

    glPopMatrix(); // face
    glPopMatrix(); // head

    //////////// WHEELS ////////////
    glPushMatrix();
    glColor3f(0.3f, 0.3f, 0.3f); 
    glMaterialf(GL_FRONT, GL_SHININESS, 70.0f);
    glTranslatef(0.2f, 0.0f, ROBOT_BODY_DEPTH * 0.5f);
    glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
    gluCylinder(gluNewQuadric(), ROBOT_BODY_DEPTH * 0.5, ROBOT_BODY_DEPTH * 0.5, ROBOT_BODY_WIDTH * 0.25, 30, 30);
    glTranslatef(0.0f, 0.0f, (ROBOT_BODY_WIDTH * 0.75f) - 0.4f);
    gluCylinder(gluNewQuadric(), ROBOT_BODY_DEPTH * 0.5, ROBOT_BODY_DEPTH * 0.5, ROBOT_BODY_WIDTH * 0.25, 30, 30);
    glPopMatrix();
    glColor3f(0.6f, 0.7f, 1.0f);

    //////////// SHOULDER ////////////
    glPushMatrix(); 
    // Positions shoulder
    glTranslatef(0.0f, ROBOT_BODY_HEIGHT * 0.8f, ROBOT_BODY_DEPTH * 0.5f);
    // Rotates shoulder
    glRotatef(shoulder_x_rotate, 1.0f, 0.0f, 0.0f);
    glRotatef(shoulder_y_rotate, 0.0f, 1.0f, 0.0f);

    gluSphere(gluNewQuadric(), 0.7, 10, 10);

    //////////// UPPER ARM ////////////
    glPushMatrix(); 
    gluCylinder(gluNewQuadric(), 0.7, 0.7, ROBOT_ARM_LENGTH, 10, 10);
    // Elbow
    glTranslatef(0.0f, 0.0f, ROBOT_ARM_LENGTH);
    gluSphere(gluNewQuadric(), 0.7, 10, 10);

    //////////// LOWER ARM ////////////
    glPushMatrix(); 
    glRotatef(elbow_x_rotate, 1.0f, 0.0f, 0.0f);
    gluCylinder(gluNewQuadric(), 0.7, 0.5, ROBOT_ARM_LENGTH, 10, 10);
    glTranslatef(0.0f, 0.0f, ROBOT_ARM_LENGTH);
    gluSphere(gluNewQuadric(), 0.5, 10, 10);

    //////////// HAND ////////////
    glPushMatrix();
    glRotatef(hand_y_rotate, 0.0f, 0.0f, 1.0f);
    glColor3f(0.1f, 0.1f, 0.1f);
    drawHand();

    glPopMatrix(); // hand
    glPopMatrix(); // lower arm
    glPopMatrix(); // upper arm
    glPopMatrix(); // shoulder
    glPopMatrix(); // body
}

void displayString(float x, float y, void *font, const char *string)
{
    glDisable(GL_LIGHTING); // To show color
    const char *c;
    glRasterPos2f(x, y);
    for (c=string; *c != '\0'; c++) {
        glutBitmapCharacter(font, *c);
    }
    glEnable(GL_LIGHTING);
}

// Displays adjust ambient box, reads user input and shows it.
// I chose to let the user pick 3 integers for ease of use.
// User input is saved as a C++ vector, and parsed in HandleKeystrokes().
//
// The position and size of elements are all floats between 0.0 and 1.0 and
// are relative to the the window size, where (0.0, 0.0) is bottom left
// and (1.0, 1.0) is top right.
void displayAdjustAmbient()
{
    if(adjust_ambient)
    {   
        // Box has a fixed size and placed in the middle
        GLfloat box_w = AMBIENT_BOX_W / win_width, box_h = AMBIENT_BOX_H / win_height,
                box_x = 0.5f - box_w * 0.5f,
                box_y = 0.5f - box_w * 0.5f;

        std::string s(user_input.begin(), user_input.end());

        // Loads 2D projection
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        gluOrtho2D(0.0, 1.0, 0.0, 1.0);
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        glDisable(GL_DEPTH_TEST);   // Makes it float
        glEnable(GL_BLEND);         // Enables transparency

        glColor4f(0.0f, 0.0f, 0.0f, 0.7f);
        glBegin(GL_QUADS);
        glVertex2f(box_x, box_y);
        glVertex2f(box_x + box_w, box_y);
        glVertex2f(box_x + box_w, box_y + box_h);
        glVertex2f(box_x, box_y + box_h);
        glEnd();

        glDisable(GL_BLEND); // Disables transparency

        glColor3f(1.0f, 1.0f, 1.0f);
        // Heading
        displayString(box_x + (30.0f / win_width), box_y + box_h - (30.0f / win_height), GLUT_BITMAP_HELVETICA_18, "Adjust ambient light");
        
        // Subheading
        displayString(box_x + (30.0f / win_width), box_y + box_h - (48.0f / win_height), GLUT_BITMAP_HELVETICA_12, "Enter 3 integers between 0-100 seperated by spaces");
        
        // User input
        displayString(box_x + (30.0f / win_width), box_y + box_h - (100.0f / win_height), GLUT_BITMAP_TIMES_ROMAN_24, s.c_str());
        
        // Further instructions
        glColor3f(1.0f, 0.0f, 0.0f);
        displayString(box_x + (85.0f / win_width), box_y + (30.0f / win_height), GLUT_BITMAP_HELVETICA_12, "Press [ENTER] to continue, [ESC] to exit");
        
        // Back to defaults
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

// Displays a help menu as a slightly transparent image decal over a quad.
// I chose to implement this as a preloaded texture for better performance.
// The help menu is transparent enough so the user can try the keys listed
// and see their effect while still looking at the menu.
//
// The position and size of elements are all floats between 0.0 and 1.0 and
// are relative to the the window size, where (0.0, 0.0) is bottom left
// and (1.0, 1.0) is top right.
void displayHelp()
{
    if(show_help)
    {   
        GLfloat box_w, box_h;
        // sets to max size when possible
        if(win_width >= START_WIDTH && win_height >= START_HEIGHT) 
        {
            box_w = HELP_MAX_WIDTH / win_width;
            box_h = HELP_MAX_HEIGHT / win_height;
        }
        // sets to relative size otherwise
        else 
        {
            box_w = 0.8f;
            box_h = ( box_w * win_width * 2.0f ) / (win_height * 3.0f);
        }

        // positions box in the middle
        GLfloat box_x = 0.5f - box_w * 0.5f,
                box_y = 0.5f - box_w * 0.5f;

        // loads 2D projection
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        gluOrtho2D(0.0, 1.0, 0.0, 1.0); 
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        glDisable(GL_LIGHTING);     // shows color
        glDisable(GL_DEPTH_TEST);   // makes it float
        glEnable(GL_BLEND);         // enables transparency

        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, textures[TEXTURE_HELP]);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
        glColor4f(0.0f, 0.0f, 0.0f, 0.7f);

        glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 1.0f);
        glVertex2f(box_x, box_y);
        glTexCoord2f(1.0f, 1.0f);
        glVertex2f(box_x + box_w, box_y);
        glTexCoord2f(1.0f, 0.0f);
        glVertex2f(box_x + box_w, box_y + box_h);
        glTexCoord2f(0.0f, 0.0f);
        glVertex2f(box_x, box_y + box_h);
        glEnd();

        // Back to default
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_LIGHTING);
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
    }
}

void toggleHelp()
{
    show_help = !show_help;
}

void displayMovingMode()
{
    // Loads 2D projection
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0.0, 1.0, 1.0, 0.0);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);   // Makes it float

    glColor3f(0.0f, 1.0f, 0.0f);
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
    
    // Back to defaults
    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

// Saves user's input when in adjust ambient menu.
// Keep in mind that the user inputs 3 integers between 0-100.
// When given more than 3 numbers in input, it reads only the first 3.
// When given numbers larger than 100, OpenGL will regard them as 1.0.
void HandleKeystrokes(unsigned char key)
{
    if(adjust_ambient)
    {
        // Adds character to input vector
        if((key >= '0' && key <= '9') || key == KEY_SPACE)
            user_input.push_back(key);

        // Deletes character
        else if(key == KEY_BACKSPACE)
            user_input.pop_back();
        
        // Parses input
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

        // Exits
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
            if(head_y_rotate > -60.0f)
                head_y_rotate -= 1.0f;
            break;

        case 'a': // MOVE HEAD LEFT
            if(head_y_rotate < 60.0f)
                head_y_rotate += 1.0f;
            break;

        case 'w': // MOVE HEAD UP
            if(head_x_rotate > -10.0f)
                head_x_rotate -= 1.0f;
            break;

        case 's': // MOVE HEAD DOWN
            if(head_x_rotate < 17.0f)
                head_x_rotate += 1.0f;
            break;
        
        case 'h': // MOVE ARM RIGHT
            if(shoulder_y_rotate > -120.0f)
                shoulder_y_rotate -= 1.0f;
            break;

        case 'f': // MOVE ARM LEFT
            if(shoulder_y_rotate < -10.0f)
                shoulder_y_rotate += 1.0f;
            break;

        case 't': // MOVE ARM UP
            if(shoulder_x_rotate > 0.0f)
                shoulder_x_rotate -= 1.0f;
            break;

        case 'g': // MOVE ARM DOWN
            if(shoulder_x_rotate < 90.0f)
                shoulder_x_rotate += 1.0f;
            break;

        case 'j': // CLOSE ELBOW
            if(elbow_x_rotate > -160.0f)
                elbow_x_rotate -= 1.0f;
            break;

        case 'u': // OPEN ELBOW
            if(elbow_x_rotate < 0.0f)
                elbow_x_rotate += 1.0f;
            break;

        case 'i': // ROTATE HAND INWARDS
            hand_y_rotate += 1.0f;
            break;

        case 'k': // ROTATE HAND OUTWARDS
            hand_y_rotate -= 1.0f;
            break;
        }
        break;
    case MOV_LIGHT:
        switch (key)
        {
        case 'w': // MOVE LIGHT DIRECTION UP
            if(light_yref < 0.0f)
                light_yref += 0.01f;
            break;

        case 's': // MOVE LIGHT DIRECTION DOWN
            if(light_yref > -1.0f)
                light_yref -= 0.01f;
            break;

        case 'd': // MOVE LIGHT DIRECTION TOWARDS X+
            if(light_xref < 1.0f)
                light_xref += 0.01f;
            break;

        case 'a': // MOVE LIGHT DIRECTION TOWARDS X-
            if(light_xref > -1.0f)
                light_xref -= 0.01f;
            break;

        case 'e': // MOVE LIGHT DIRECTION TOWARDS Z+
            if(light_zref < 1.0f)
                light_zref += 0.01f;
            break;

        case 'q': // MOVE LIGHT DIRECTION TOWARDS Z-
            if(light_zref > -1.0f)
                light_zref -= 0.01f;
            break;

        case 'r': // INCREASE AMBIENT ILLUMINATION
            if(red < 1.0f && green < 1.0f && blue < 1.0f)
            {
                red += 0.05f;
                green += 0.05f;
                blue += 0.05f;
            }   
            break;

        case 'f': // DECREASE AMBIENT ILLUMINATION
            if(red > 0.0f && blue > 0.0f && green > 0.0f)
            {
                red -= 0.05f;
                blue -= 0.05f;
                green -= 0.05f;
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
                if(robotx < 50.0f)
                    robotx += 0.5f * sin(robot_y_rotate * PI / 180.0);
                if(robotz < 50.0f)
                    robotz += 0.5f * cos(robot_y_rotate * PI / 180.0);
                break;

            case GLUT_KEY_DOWN: // MOVE ROBOT BACKWARDS
                if(robotx > -50.0f)
                    robotx += 0.5f * sin((180.0 + robot_y_rotate) * PI / 180.0);
                if(robotz > -50.0f)
                    robotz += 0.5f * cos((180.0 + robot_y_rotate) * PI / 180.0);
                break;

            case GLUT_KEY_RIGHT: // TURN RIGHT
                robot_y_rotate -= 2.0f;
                break;

            case GLUT_KEY_LEFT: // TURN LEFT
                robot_y_rotate += 2.0f;
                break;
            
            default:
                break;
            }
            break;
        
        case MOV_CAM:
            switch (key)
            {
            case GLUT_KEY_UP: // MOVE CAMERA CLOSER
                if(cam_dist > 0.0f)
                    cam_dist -= 1.0f;
                break;

            case GLUT_KEY_DOWN: // MOVE CAMERA AWAY
                if(cam_dist < 120.0f)
                    cam_dist += 1.0f;
                break;

            case GLUT_KEY_RIGHT: // ROTATE CAMERA RIGHT
                world_rot -= 1.0f;
                break;

            case GLUT_KEY_LEFT: // ROTATE CAMERA LEFT
                world_rot += 1.0f;
                break;
            
            default:
                break;
            }

        case(MOV_LIGHT):
            switch (key)
            {
            case GLUT_KEY_UP: // MOVE LIGHT SOURCE UP
                if(light_y < 240.0f)
                    light_y += 1.0f;
                break;

            case GLUT_KEY_DOWN: // MOVE LIGHT SOURCE DOWN
                if(light_y > 0.0f)
                    light_y -= 1.0f;
                break;

            case GLUT_KEY_RIGHT: // MOVE LIGHT SOURCE TOWARDS X+
                if(light_x < 100.0f)
                    light_x += 1.0f;
                break;

            case GLUT_KEY_LEFT: // MOVE LIGHT SOURCE TOWARDS X-
                if(light_x > -100.0f)
                    light_x -= 1.0f;
                break;

            case GLUT_KEY_END: // MOVE LIGHT SOURCE TOWARDS Z+
                if(light_z < 100.0f)
                    light_z += 1.0f;
                break;

            case GLUT_KEY_HOME: // MOVE LIGHT SOURCE TOWARDS Z-
                if(light_z > -100.0f)
                    light_z -= 1.0f;
                break;
            }
        }
        break;
    }
    glutPostRedisplay();
}
