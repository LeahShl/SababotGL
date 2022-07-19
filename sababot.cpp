#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define TEXTURE_MARBLE 0
#define TEXTURE_WOOD 1
GLuint textures[2];

GLint winWidth = 1200, winHeight = 800;
GLfloat cam_dist = 50.0, world_rot = 0.0;
GLfloat xref = 0.0, yref = 0.0, zref = 0.0;
GLfloat Vx = 0.0, Vy = 1.0, Vz = 0.0;
GLfloat fov = 30.0, aspect = winWidth / winHeight, zNear = 25.0, zFar = 1000.0;

enum states
{
    MOV_ROBOT,
    MOV_CAM,
    MOV_LIGHT
};

int mvstate = MOV_ROBOT;
int first_person = false;

void InitGlut(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(winWidth, winHeight);
    glutCreateWindow("SababotGL");
}

void Init()
{
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glShadeModel(GL_SMOOTH);

    // Init camera
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(fov, aspect, zNear, zFar);
    glMatrixMode(GL_MODELVIEW);

    // Init illumination
    GLfloat light_position[] = {60.0, 60.0, 60.0, 0.0};
    GLfloat white_light[] = {1.0, 1.0, 1.0, 1.0};
    GLfloat lmodel_ambient[] = {0.5, 0.5, 0.5, 1.0};
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, white_light);
    glLightfv(GL_LIGHT0, GL_SPECULAR, white_light);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_DEPTH_TEST);

    // Init textures
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glGenTextures(2, textures);
    glBindTexture(GL_TEXTURE_2D, textures[TEXTURE_MARBLE]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_MIPMAP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    unsigned char *image = stbi_load("textures/marble.png", &width, &height, &nrChannels, 3);
    if (image)
    {
        glTexImage2D(GL_TEXTURE_2D, 2, GL_RGB, width, height, 1, GL_RGB, GL_UNSIGNED_BYTE, image);
        gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, width, height, GL_RGB, GL_UNSIGNED_BYTE, image);
    }

    glBindTexture(GL_TEXTURE_2D, textures[TEXTURE_WOOD]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_MIPMAP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    image = stbi_load("textures/wood.png", &width, &height, &nrChannels, 3);
    if (image)
    {
        glTexImage2D(GL_TEXTURE_2D, 8, GL_RGB, width, height, 1, GL_RGB, GL_UNSIGNED_BYTE, image);
        gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, width, height, GL_RGB, GL_UNSIGNED_BYTE, image);
    }

    stbi_image_free(image);
}

/* Helper function so I could see what I'm doing.
 * Draws x,y,z axes.
 * X is red, Y is green, Z is blue.
 */
void displayDebug()
{
    glPushMatrix();

    glBegin(GL_LINES);
    glColor3f(1.0, 0.0, 0.0);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(1000.0, 0.0, 0.0);

    glColor3f(0.0, 1.0, 0.0);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(0.0, 1000.0, 0.0);

    glColor3f(0.0, 0.0, 1.0);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(0.0, 0.0, 1000.0);
    glEnd();
    glPopMatrix();
}

void displayFloor(float size)
{
    GLfloat specref[] = {1.0, 1.0, 1.0, 1.0};
    glMaterialfv(GL_FRONT, GL_SPECULAR, specref);
    glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);
    glMateriali(GL_FRONT, GL_SHININESS, 128);

    glPushMatrix();
    glTranslatef(-size / 2, 0.0, -size / 2);
    glColor3f(1.0, 1.0, 1.0);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textures[TEXTURE_MARBLE]);
    glBegin(GL_QUADS);
    glNormal3f(0.0, 1.0, 0.0);
    glTexCoord2f(0.0, 0.0);
    glVertex3f(0.0, 0.0, 0.0);
    glTexCoord2f(0.0, 2.0);
    glVertex3f(0.0, 0.0, size);
    glTexCoord2f(2.0, 2.0);
    glVertex3f(size, 0.0, size);
    glTexCoord2f(2.0, 0.0);
    glVertex3f(size, 0.0, 0.0);
    glEnd();
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();
    glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SINGLE_COLOR);
}

void rectCuboid(float x, float y, float z)
{
    // Top face
    glBegin(GL_QUADS);
    glNormal3f(0.0, 1.0, 0.0);
    glTexCoord2f(0.0, 0.0);
    glVertex3f(0.0, y, 0.0);
    glTexCoord2f(1.0, 0.0);
    glVertex3f(x, y, 0.0);
    glTexCoord2f(1.0, 1.0);
    glVertex3f(x, y, z);
    glTexCoord2f(0.0, 1.0);
    glVertex3f(0.0, y, z);
    glEnd();

    // Side face 1
    glBegin(GL_QUADS);
    glNormal3f(-1.0, 0.0, 0.0);
    glTexCoord2f(0.0, 0.0);
    glVertex3f(0.0, 0.0, 0.0);
    glTexCoord2f(1.0, 0.0);
    glVertex3f(0.0, y, 0.0);
    glTexCoord2f(1.0, 1.0);
    glVertex3f(0.0, y, z);
    glTexCoord2f(0.0, 1.0);
    glVertex3f(0.0, 0.0, z);
    glEnd();

    // Side face 2
    glBegin(GL_QUADS);
    glNormal3f(0.0, 0.0, -1.0);
    glTexCoord2f(0.0, 0.0);
    glVertex3f(0.0, 0.0, 0.0);
    glTexCoord2f(0.0, 1.0);
    glVertex3f(0.0, y, 0.0);
    glTexCoord2f(1.0, 1.0);
    glVertex3f(x, y, 0.0);
    glTexCoord2f(1.0, 0.0);
    glVertex3f(x, 0.0, 0.0);
    glEnd();

    // Side face 3
    glBegin(GL_QUADS);
    glNormal3f(1.0, 0.0, 0.0);
    glTexCoord2f(0.0, 0.0);
    glVertex3f(0.0, 0.0, z);
    glTexCoord2f(0.0, 1.0);
    glVertex3f(0.0, y, z);
    glTexCoord2f(1.0, 1.0);
    glVertex3f(x, y, z);
    glTexCoord2f(1.0, 0.0);
    glVertex3f(x, 0.0, z);
    glEnd();

    // Side face 4
    glBegin(GL_QUADS);
    glNormal3f(0.0, 0.0, 1.0);
    glTexCoord2f(0.0, 0.0);
    glVertex3f(x, 0.0, 0.0);
    glTexCoord2f(1.0, 0.0);
    glVertex3f(x, y, 0.0);
    glTexCoord2f(1.0, 1.0);
    glVertex3f(x, y, z);
    glTexCoord2f(0.0, 1.0);
    glVertex3f(x, 0.0, z);
    glEnd();

    // Bottom face
    glBegin(GL_QUADS);
    glNormal3f(0.0, -1.0, 0.0);
    glTexCoord2f(0.0, 0.0);
    glVertex3f(0.0, 0.0, 0.0);
    glTexCoord2f(0.0, 1.0);
    glVertex3f(0.0, 0.0, z);
    glTexCoord2f(1.0, 1.0);
    glVertex3f(x, 0.0, z);
    glTexCoord2f(1.0, 0.0);
    glVertex3f(x, 0.0, 0.0);
    glEnd();
}

void simpleNURBS(float width, float height, float curvature)
{
    GLfloat ctlpoints[4][4][3];
    GLUnurbsObj *theNurb;
    GLfloat knots[8] = {0.0, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 1.0};

    // init surface
    int u, v;
    for (u = 0; u < 4; u++)
    {
        for (v = 0; v < 4; v++)
        {
            ctlpoints[u][v][0] = width * ((GLfloat)u / 3.0);
            ctlpoints[u][v][1] = height * ((GLfloat)v / 3.0);

            if ((u == 1 || u == 2) && (v == 1 || v == 2))
                ctlpoints[u][v][2] = curvature;
            else
                ctlpoints[u][v][2] = 0.0;
        }
    }

    theNurb = gluNewNurbsRenderer();
    gluNurbsProperty(theNurb, GLU_SAMPLING_TOLERANCE, 25.0);
    gluNurbsProperty(theNurb, GLU_DISPLAY_MODE, GLU_FILL);

    glEnable(GL_AUTO_NORMAL);
    gluBeginSurface(theNurb);
    gluNurbsSurface(theNurb,
                    8, knots, 8, knots,
                    4 * 3, 3, &ctlpoints[0][0][0],
                    4, 4, GL_MAP2_VERTEX_3);
    gluEndSurface(theNurb);
    glDisable(GL_AUTO_NORMAL);
}

void displayTable(float posx, float posz, float height, float sizex, float sizez, float thickness)
{
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textures[TEXTURE_WOOD]);
    glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);
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
    glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);
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
    glColor3f(0.7, 0.7, 0.7);
    glPushMatrix();
    glTranslatef(posx, 0.0, posz);
    rectCuboid(width, height, depth);

    glTranslatef(0.3, 0.0, depth);
    rectCuboid(width - 0.6, height, 0.2);
    glTranslatef(-0.2, 0.0, 0.2);
    rectCuboid(width, height, 1.0);

    glTranslatef(0.0, 0.0, 1.2);
    simpleNURBS(width, door_split, 1.0);
    glTranslatef(0.0, door_split, 0.0);
    simpleNURBS(width, height - door_split, 1.0);
    glPopMatrix();
}

void drawHand()
{
    glEnable(GL_NORMALIZE);
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
    glDisable(GL_NORMALIZE);
}

GLfloat robotx = 0.0, robotz = 0.0, robot_y_rotate = 0.0;
GLfloat head_x_rotate = 0.0, head_y_rotate = 0.0;
GLfloat shoulder_x_rotate = 90.0, shoulder_y_rotate = -10.0;
GLfloat elbow_x_rotate = -20.0, hand_y_rotate = 0.0;

void displayRobot()
{
    GLfloat body_width = 6.0, body_height = 10.0, body_depth = 3.0;
    GLfloat neck_length = 1.0, arm_length = 3.0;
    GLfloat arm_pos[3] = {0.0, body_height * (float)0.8, body_depth * (float)0.5};

    glColor3f(0.6, 0.7, 1.0);

    glPushMatrix();
    glTranslatef(body_width * 0.5, 0.0, body_depth * 0.5);
    glRotatef(robot_y_rotate, 0.0, 1.0, 0.0);
    glTranslatef(-body_width * 0.5, 0.0, -body_depth * 0.5);
    glTranslatef(0.0, body_depth * 0.5, 0.0);

    /* BODY */
    glPushMatrix();
    // TODO: Shear
    rectCuboid(body_width, body_height, body_depth);
    glPopMatrix();

    /* NECK */
    glPushMatrix();
    glTranslatef(body_width / 2.0, body_height, body_depth / 2.0);
    glRotatef(-90.0, 1.0, 0.0, 0.0);
    gluCylinder(gluNewQuadric(), 0.9, 0.9, neck_length * 1.5, 10.0, 10.0);
    glPopMatrix();

    /* HEAD */
    glPushMatrix();
    glTranslatef((body_width * 0.25) / 2.0, body_height + neck_length, 0.0);
    glTranslatef(body_width * 0.5, 0.0, body_depth * 0.5);
    glRotatef(head_x_rotate, 1.0, 0.0, 0.0);
    glRotatef(head_y_rotate, 0.0, 1.0, 0.0);
    glTranslatef(-body_width * 0.5, 0.0, -body_depth * 0.5);
    rectCuboid(body_width * 0.75, body_height * 0.5, body_depth);
    glPopMatrix();

    /* WHEELS */
    glColor3f(0.3, 0.3, 0.3);
    glPushMatrix();
    glTranslatef(0.2, 0.0, body_depth * 0.5);
    glRotatef(90.0, 0.0, 1.0, 0.0);
    gluCylinder(gluNewQuadric(), body_depth * 0.5, body_depth * 0.5, body_width * 0.25, 10.0, 10.0);
    glTranslatef(0.0, 0.0, (body_width * 0.75) - 0.4);
    gluCylinder(gluNewQuadric(), body_depth * 0.5, body_depth * 0.5, body_width * 0.25, 10.0, 10.0);
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
    drawHand();

    glPopMatrix(); // hand
    glPopMatrix(); // lower arm
    glPopMatrix(); // upper arm
    glPopMatrix(); // arm
    glPopMatrix(); // body
}

void Display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(cam_dist, cam_dist, cam_dist, xref, yref, zref, Vx, Vy, Vz);
    glRotatef(world_rot, 0.0, 1.0, 0.0);
    displayFloor(100.0);
    displayTable(-10.0, 30.0, 10.0, 20.0, 10.0, 1.0);
    displayChair(-2.0, 20.0, 6.0, 6.0, 5.0, 1.0);
    displayFridge(-20.0, -50.0, 8.0, 24.0, 6.0);
    displayRobot();
    displayDebug();
    glutSwapBuffers();
}

void Reshape(GLsizei w, GLsizei h)
{
    winWidth = w;
    winHeight = h;
    aspect = w / h;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(fov, aspect, zNear, zFar);
    glViewport(0, 0, winWidth, winHeight);
    glMatrixMode(GL_MODELVIEW);
    glutSwapBuffers();
    glutPostRedisplay();
}

void Keyboard(unsigned char key, int x, int y)
{
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
    case MOV_CAM:
        switch (key)
        {
        case 'R':
            /* code */
            break;
        }
        break;
    case MOV_LIGHT:
        break;
    }

    glutPostRedisplay();
}

void SpecialKeyboard(int key, int x, int y)
{
    switch (key)
    {
    case GLUT_KEY_F1: // SHOW/HIDE HELP
        /* code */
        break;

    case GLUT_KEY_F2: // MOVE ROBOT STATE
        mvstate = MOV_ROBOT;
        break;

    case GLUT_KEY_F3: // TOGGLE FIRST PERSON VIEW
        first_person = !first_person;
        if(first_person)
            mvstate = MOV_ROBOT;
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
                robotz += 1;
                break;

            case GLUT_KEY_DOWN: // MOVE ROBOT BACKWARD
                robotz -= 1;
                break;

            case GLUT_KEY_RIGHT: // TURN RIGHT
                robot_y_rotate -= 1;
                break;

            case GLUT_KEY_LEFT: // TURN LEFT
                robot_y_rotate += 1;
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

        default:
            break;
        }
        break;
    }
    glutPostRedisplay();
}

void Menu(int value)
{
    switch (value)
    {
    case 0:
        /* code */
        break;

    case 1:
        /* code */
        break;

    case 2:
        glutDestroyWindow(glutGetWindow());
        break;
    }
}

void RegisterCallbacks()
{
    glutDisplayFunc(Display);
    glutReshapeFunc(Reshape);
    glutKeyboardFunc(Keyboard);
    glutSpecialFunc(SpecialKeyboard);
}

int main(int argc, char **argv)
{
    InitGlut(argc, argv);
    Init();
    RegisterCallbacks();

    glutCreateMenu(Menu);
    glutAddMenuEntry("Adjust ambient light", 0);
    glutAddMenuEntry("help", 1);
    glutAddMenuEntry("quit", 2);
    glutAttachMenu(GLUT_RIGHT_BUTTON);

    glutMainLoop();
}