#include <GL/gl.h>
#ifndef SABABOT_H
#define SABABOT_H
#define PI 3.14159265

/**********************************
 **********  CONSTANTS  ***********
 **********************************/
enum txtrs
{
    TEXTURE_MARBLE,
    TEXTURE_WALL,
    TEXTURE_WOOD,
    TEXTURE_HELP
};

enum states
{
    MOV_ROBOT,
    MOV_CAM,
    MOV_LIGHT
};

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

/**********************************
 **********  FUNCTIONS  ***********
 **********************************/

/**
 * @brief 
 * 
 * @param new_aspect 
 */
void updateWindowAspect(float new_aspect);

/**
 * @brief 
 * 
 * @param filename 
 * @param target 
 * @param format 
 * @param nchannels 
 */
void loadTexture(const char *filename, GLenum target, GLenum format, int nchannels);

/**
 * @brief 
 * 
 */
void initTextures();

/**
 * @brief 
 * 
 */
void initCamera();

/**
 * @brief 
 * 
 */
void initLight();

/**
 * @brief 
 * 
 * @param size 
 */
void displayfloor(float size);

/**
 * @brief 
 * 
 * @param size 
 * @param height 
 */
void displayWalls(float size, float height);

/**
 * @brief 
 * 
 * @param x 
 * @param y 
 * @param z 
 * @param subdiv 
 */
void rectCuboidDiv(float x, float y, float z, int subdiv);

/**
 * @brief 
 * 
 * @param x 
 * @param y 
 * @param z 
 */
void rectCuboid(float x, float y, float z);

/**
 * @brief 
 * 
 * @param x 
 * @param y 
 * @param z 
 * @param rot 
 */
void displayTeapot(float x, float y, float z, float rot);

/**
 * @brief 
 * 
 * @param posx 
 * @param posz 
 * @param height 
 * @param sizex 
 * @param sizez 
 * @param thickness 
 */
void displayTable(float posx, float posz, float height, float sizex, float sizez, float thickness);

/**
 * @brief 
 * 
 * @param posx 
 * @param posz 
 * @param height 
 * @param seat_width 
 * @param seat_depth 
 * @param thickness 
 */
void displayChair(float posx, float posz, float height, float seat_width, float seat_depth, float thickness);

/**
 * @brief 
 * 
 * @param posx 
 * @param posz 
 * @param width 
 * @param height 
 * @param depth 
 */
void displayFridge(float posx, float posz, float width, float height, float depth);

/**
 * @brief 
 * 
 * @param posx 
 * @param posz 
 * @param width 
 * @param height 
 */
void displayBin(float posx, float posz, float width, float height);

/**
 * @brief 
 * 
 */
void drawHand();

/**
 * @brief 
 * 
 */
void displayRobot();

/**
 * @brief 
 * 
 * @param x 
 * @param y 
 * @param font 
 * @param string 
 */
void displayString(float x, float y, void *font, const char *string);

/**
 * @brief 
 * 
 */
void displayAdjustAmbient();

/**
 * @brief 
 * 
 */
void toggleAdjustAmbient();

/**
 * @brief 
 * 
 */
void displayHelp();

/**
 * @brief 
 * 
 */
void toggleHelp();

/**
 * @brief 
 * 
 */
void displayMovingMode();

/**
 * @brief 
 * 
 * @param key 
 */
void HandleKeystrokes(unsigned char key);

/**
 * @brief 
 * 
 * @param key 
 * @param x 
 * @param y 
 */
void Keyboard(unsigned char key, int x, int y);

/**
 * @brief 
 * 
 * @param key 
 * @param x 
 * @param y 
 */
void SpecialKeyboard(int key, int x, int y);

#endif