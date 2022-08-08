#include "sababot.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

void InitGlut(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(START_WIDTH, START_HEIGHT);
    glutCreateWindow("SababotGL");
}

void Init()
{
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glShadeModel(GL_SMOOTH);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    initTextures();
}

void Display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    initCamera();
    initLight();
    displayfloor(FLOOR_SIZE);
    displayWalls(FLOOR_SIZE, FLOOR_SIZE * 0.3);
    displayTable(-TABLE_SIZEX * 0.5, FLOOR_SIZE * 0.5 - 20.0, TABLE_HEIGHT, TABLE_SIZEX, TABLE_SIZEZ, TABLE_THICKNESS);
    displayTeapot(0.0, TABLE_HEIGHT + 2.0, FLOOR_SIZE * 0.5 - 15.0, 45.0);
    displayChair(-CHAIR_SEAT_WIDTH * 0.5, FLOOR_SIZE * 0.5 - 20.0 - 2 * CHAIR_SEAT_DEPTH, TABLE_HEIGHT - 4.0, CHAIR_SEAT_WIDTH, CHAIR_SEAT_DEPTH, TABLE_THICKNESS);
    displayFridge(-FLOOR_SIZE * 0.2, -FLOOR_SIZE * 0.5, FRIDGE_WIDTH, FRIDGE_HEIGHT, FRIDGE_DEPTH);
    displayBin(-5.0, -FLOOR_SIZE * 0.5 + 5.0, 5.0, 5.0);
    displayRobot();
    displayMovingMode();
    displayAdjustAmbient();
    displayHelp();
    glutSwapBuffers();
}

void Reshape(GLsizei w, GLsizei h)
{
    updateWindowAspect(1.0 * w / h);
    glViewport(0, 0, w, h);
}

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