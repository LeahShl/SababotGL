#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define TEXTURE_MARBLE 0
#define TEXTURE_WOOD 1
GLuint textures[2];

GLint winWidth = 1200, winHeight = 800;
GLfloat xcam = 100.0, ycam = 50.0, zcam = 100.0;
GLfloat xref = 0.0, yref = 0.0, zref = 0.0;
GLfloat Vx = 0.0, Vy = 1.0, Vz = 0.0;
GLfloat fov = 30.0, aspect = winWidth / winHeight, zNear = 25.0, zFar = 1000.0;

enum states
{
    MOV_ROBOT,
    MOV_CAM,
    MOV_LIGHT
};

int mvstate = MOV_CAM;
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
    glMatrixMode(GL_MODELVIEW);
    gluLookAt(xcam, ycam, zcam, xref, yref, zref, Vx, Vy, Vz);
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
    glEnable(GL_AUTO_NORMAL);
    glEnable(GL_NORMALIZE);

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
            ctlpoints[u][v][1] = height * ((GLfloat)v /3.0);

            if ((u == 1 || u == 2) && (v == 1 || v == 2))
                ctlpoints[u][v][2] = curvature;
            else
                ctlpoints[u][v][2] = 0.0;
        }
    }

    theNurb = gluNewNurbsRenderer();
    gluNurbsProperty(theNurb, GLU_SAMPLING_TOLERANCE, 25.0);
    gluNurbsProperty(theNurb, GLU_DISPLAY_MODE, GLU_FILL);

    gluBeginSurface(theNurb);
    gluNurbsSurface(theNurb,
                    8, knots, 8, knots,
                    4 * 3, 3, &ctlpoints[0][0][0],
                    4, 4, GL_MAP2_VERTEX_3);
    gluEndSurface(theNurb);
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

GLfloat robotx = 0.0, robotz = 0.0, robot_y_rotate = 0.0;
GLfloat head_x_rotate = 0.0, head_y_rotate = 0.0;
GLfloat shoulder_x_rotate = 0.0, shoulder_z_rotate = 0.0;
GLfloat elbow_x_rotate = 0.0, hand_y_rotate = 0.0;

void displayRobot()
{
    GLfloat body_width = 6.0, body_height = 10.0, body_depth = 3.0;
    GLfloat neck_length = 1.0;

    glColor3f(0.5, 0.0, 0.0);
    
    glPushMatrix();
    glTranslatef(robotx, 1.0, robotz);
    glRotatef(robot_y_rotate, 0.0, 1.0, 0.0);
    
    /* BODY */
    glPushMatrix();
    // TODO: Shear
    rectCuboid(body_width, body_height, body_depth);
    glPopMatrix();

    /* NECK */
    glPushMatrix();
    glTranslatef(body_width / 2.0, body_height, body_depth / 2.0);
    glRotatef(-90.0, 1.0, 0.0, 0.0);
    gluCylinder(gluNewQuadric(), 1.0, 1.0, neck_length, 10.0, 10.0);
    glPopMatrix();

    /* HEAD */
    glPushMatrix();
    glTranslatef((body_width * 0.25) / 2.0, body_height + neck_length, 0.0);
    glRotatef(head_x_rotate, 1.0, 0.0, 0.0);
    glRotatef(head_y_rotate, 0.0, 1.0, 0.0);
    rectCuboid(body_width * 0.75, body_height * 0.5, body_depth);
    glPopMatrix();

    glPushMatrix();
    // TODO: translate
    // TODO: Draw left wheel
    glPopMatrix();

    glPushMatrix();
    // TODO: translate
    // TODO: Draw right wheel
    glPopMatrix();

    glPushMatrix();
    glRotatef(shoulder_x_rotate, 1.0, 0.0, 0.0);
    glRotatef(shoulder_z_rotate, 0.0, 0.0, 1.0);
    // TODO: Draw shoulder
    // TODO: Draw upper arm
    // TODO: Draw elbow

    glPushMatrix();
    glRotatef(elbow_x_rotate, 1.0, 0.0, 0.0);
    // TODO: Draw lower arm

    glPushMatrix();
    glRotatef(hand_y_rotate, 0.0, 1.0, 0.0);
    // TODO: Draw hand

    glPopMatrix(); // hand
    glPopMatrix(); // lower arm
    glPopMatrix(); // upper arm
    glPopMatrix(); // body
}

void Display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
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
    if (key == GLUT_KEY_F3)
    {
        first_person = !first_person;
        if (first_person)
            mvstate = MOV_ROBOT;
        // TODO: change view
    }
    else
    {
        switch (mvstate)
        {
        case MOV_ROBOT:
            break;
        case MOV_CAM:
            switch (key)
            {
            case GLUT_KEY_UP: // get closer
                ycam -= 1;
                glMatrixMode(GL_MODELVIEW);
                glLoadIdentity();
                gluLookAt(xcam, ycam, zcam, xref, yref, zref, Vx, Vy, Vz);
                break;

            case GLUT_KEY_DOWN: // move away
                ycam += 1;
                glMatrixMode(GL_MODELVIEW);
                glLoadIdentity();
                gluLookAt(xcam, ycam, zcam, xref, yref, zref, Vx, Vy, Vz);
                break;

            case GLUT_KEY_RIGHT: // rotate right
                xcam -= 1;
                zcam += 1;
                glMatrixMode(GL_MODELVIEW);
                glLoadIdentity();
                gluLookAt(xcam, ycam, zcam, xref, yref, zref, Vx, Vy, Vz);
                break;

            case GLUT_KEY_LEFT: // rotate left
                xcam += 1;
                zcam -= 1;
                glMatrixMode(GL_MODELVIEW);
                glLoadIdentity();
                gluLookAt(xcam, ycam, zcam, xref, yref, zref, Vx, Vy, Vz);
                break;
            }
            break;
        case MOV_LIGHT:
            break;
        }
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