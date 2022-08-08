#include <GL/gl.h>
#ifndef SABABOT_H
#define SABABOT_H
enum txtrs
{
    TEXTURE_MARBLE,
    TEXTURE_WALL,
    TEXTURE_WOOD,
    TEXTURE_HELP
};
#define PI 3.14159265

const int START_WIDTH = 1200;
const int START_HEIGHT = 800;
const float HELP_MAX_WIDTH = 960.0;
const float HELP_MAX_HEIGHT = 640.0;
const float AMBIENT_BOX_W = 400.0;
const float AMBIENT_BOX_H = 200.0;

const unsigned char KEY_SPACE = 32;
const unsigned char KEY_BACKSPACE = 8;
const unsigned char KEY_ENTER = 13;
const unsigned char KEY_ESC = 27;

const float FLOOR_SIZE = 100.0;
const float TABLE_HEIGHT = 10.0;
const float TABLE_SIZEX = 20.0;
const float TABLE_SIZEZ = 10.0;
const float TABLE_THICKNESS = 1.0;
const float CHAIR_SEAT_WIDTH = 6.0;
const float CHAIR_SEAT_DEPTH = 5.0;
const float FRIDGE_WIDTH = 8.0;
const float FRIDGE_HEIGHT = 24.0;
const float FRIDGE_DEPTH = 6.0;
const float BIN_WIDTH = 5.0;
const float BIN_HEIGHT = 5.0;

enum states
{
    MOV_ROBOT,
    MOV_CAM,
    MOV_LIGHT
};

void updateWindowAspect(float new_aspect);

void loadTexture(const char *filename, GLenum target, GLenum format, int nchannels);

void initTextures();

void initCamera();

void initLight();

void displayfloor(float size);

void displayWalls(float size, float height);

void rectCuboidDiv(float x, float y, float z, int subdiv);

void rectCuboid(float x, float y, float z);

void displayTeapot(float x, float y, float z, float rot);

void displayTable(float posx, float posz, float height, float sizex, float sizez, float thickness);

void displayChair(float posx, float posz, float height, float seat_width, float seat_depth, float thickness);

void displayFridge(float posx, float posz, float width, float height, float depth);

void displayBin(float posx, float posz, float width, float height);

void drawHand();

void displayRobot();

void displayString(float x, float y, void *font, const char *string);

void displayAdjustAmbient();

void toggleAdjustAmbient();

void displayHelp();

void toggleHelp();

void displayMovingMode();

void HandleKeystrokes(unsigned char key);

void Keyboard(unsigned char key, int x, int y);

void SpecialKeyboard(int key, int x, int y);

#endif