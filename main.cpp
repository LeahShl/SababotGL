/* 
 * This progran uses sababot.cpp functions in order to
 * display and run SababotGL.
 */
#include "sababot.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

/*
 * Initializes window, sets some defaults and loads textures to buffer.
 * To be called from the main function.
 */
void Init(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(START_WIDTH, START_HEIGHT);
    glutCreateWindow("SababotGL");

    glClearColor(0.0f, 0.0f, 0.2f, 0.0f);
    glShadeModel(GL_SMOOTH);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    initTextures();
}

/*
 * Display callback function
 */
void Display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Sets camera and frustum
    initCamera();

    // Sets light source
    initLight();

    // The position (and sometimes size) of the elements is relative to the floor size
    displayfloor(FLOOR_SIZE);
    displayWalls(FLOOR_SIZE, FLOOR_SIZE * 0.3f);

    // The table is located 20 units from the +Z wall, where x=0
    displayTable(-TABLE_SIZEX * 0.5f, FLOOR_SIZE * 0.5f - 20.0f, TABLE_HEIGHT, TABLE_SIZEX, TABLE_SIZEZ, TABLE_THICKNESS);

    // The teapot is located on the table, right in the middle
    displayTeapot(0.0f, TABLE_HEIGHT + 2.0f, FLOOR_SIZE * 0.5f - 15.0f, 45.0f);

    // The chair is located twice-its-depth units from the table, where x=0
    displayChair(-CHAIR_SEAT_WIDTH * 0.5f, FLOOR_SIZE * 0.5f - 20.0f - 2.0f * CHAIR_SEAT_DEPTH, TABLE_HEIGHT - 4.0f, CHAIR_SEAT_WIDTH, CHAIR_SEAT_DEPTH, TABLE_THICKNESS);

    // The fridge is located 0.2*FLOOR_SIZE units towards the -X wall
    displayFridge(-FLOOR_SIZE * 0.2f, -FLOOR_SIZE * 0.5f, FRIDGE_WIDTH, FRIDGE_HEIGHT, FRIDGE_DEPTH);

    // The garbage bin is located 15 units from the fridge towards the +X wall
    displayBin(-FLOOR_SIZE * 0.2f + 15.0, -FLOOR_SIZE * 0.5f + 5.0f, 5.0f, 5.0f);

    displayRobot();
    displayMovingMode();
    displayAdjustAmbient(); // displays only if needed
    displayHelp(); // displays only if needed
    glutSwapBuffers();
}

/*
 * Reshape callback function
 */
void Reshape(GLsizei w, GLsizei h)
{
    updateWindowAspect(1.0 * w / h);
    glViewport(0, 0, w, h);
}

/*
 * Right-click menu
 */
void Menu(int value)
{
    switch (value)
    {
    case 0:
        toggleAdjustAmbient();
        glutPostRedisplay();
        break;

    case 1:
        toggleHelp();
        glutPostRedisplay();
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
    Init(argc, argv);
    RegisterCallbacks();

    glutCreateMenu(Menu);
    glutAddMenuEntry("Adjust ambient light", 0);
    glutAddMenuEntry("help", 1);
    glutAddMenuEntry("quit", 2);
    glutAttachMenu(GLUT_RIGHT_BUTTON);

    glutMainLoop();
}