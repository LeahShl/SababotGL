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

// GUI
const int START_WIDTH = 1200;
const int START_HEIGHT = 800;
const float HELP_MAX_WIDTH = 960.0;
const float HELP_MAX_HEIGHT = 640.0;
const float AMBIENT_BOX_W = 400.0;
const float AMBIENT_BOX_H = 200.0;

// KEYBOARD
const unsigned char KEY_SPACE = 32;
const unsigned char KEY_BACKSPACE = 8;
const unsigned char KEY_ENTER = 13;
const unsigned char KEY_ESC = 27;

// OBJECT PROPERTIES
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
const float ROBOT_BODY_WIDTH = 6.0;
const float ROBOT_BODY_HEIGHT = 10.0;
const float ROBOT_BODY_DEPTH = 3.0;
const float ROBOT_NECK_LENGTH = 1.0;
const float ROBOT_ARM_LENGTH = 3.0;

// MOVEMENT LIMITS
const float ROBOT_HEAD_RIGHT_LIMIT = -60.0;
const float ROBOT_HEAD_LEFT_LIMIT = 60.0;
const float ROBOT_HEAD_UP_LIMIT = -10.0;
const float ROBOT_HEAD_DOWN_LIMIT = 17.0;
const float ROBOT_ARM_RIGHT_LIMIT = -120.0;
const float ROBOT_ARM_LEFT_LIMIT = -10.0;
const float ROBOT_ARM_UP_LIMIT = -60.0;
const float ROBOT_ARM_DOWN_LIMIT = 90.0;
const float ROBOT_ELBOW_CLOSED = -160.0;
const float ROBOT_ELBOW_OPEN = 0.0;
const float CAM_CLOSEST = 1.0;
const float CAM_FARTHEST = 120.0;
const float CAM_MAX_HEIGHT_FACTOR = 2.0;
const float CAM_MIN_HEIGHT_FACTOR = 0.1;
const float LIGHT_MIN_Y = 1.0;
const float LIGHT_MAX_Y = 240.0;

/**********************************
 **********  FUNCTIONS  ***********
 **********************************/

/**
 * @brief Updates window dimensions and aspect ratio
 * 
 * @param w New window width 
 * @param h New window height
 */
void updateWindowDims(GLsizei w, GLsizei h);

/**
 * @brief Loads a 1D/2D image texture to a target
 * 
 * @param filename Path to file
 * @param target Either GL_TEXTURE_1D or GL_TEXTURE_2D
 * @param format Either GL_RGB or GL_RGBA
 * @param nchannels Should be 3 or 4 according to format
 */
void loadTexture(const char *filename, GLenum target, GLenum format, int nchannels);

/**
 * @brief Initializes textures and loads them to buffer.
 *        Should be called before a display function.
 */
void initTextures();

/**
 * @brief Initializes camera 
 * 
 */
void initCamera();

/**
 * @brief Initializes light source
 * 
 */
void initLight();

/**
 * @brief Displays a marble floor on the XZ plane
 * 
 * @param size The floor size
 */
void displayfloor(float size);

/**
 * @brief Displays textured walls with respect to camera position
 * 
 * @param size The floor size
 * @param height Wall's height
 */
void displayWalls(float size, float height);

/**
 * @brief Displays a rectangular cuboid divided into smaller quads
 * 
 * @param x Size along the X axis
 * @param y Size along the Y axis
 * @param z Size along the Z axis
 * @param subdiv Degree of subdivision
 */
void rectCuboidDiv(float x, float y, float z, int subdiv);

/**
 * @brief Displays a basic rectangular cuboid of 6 quads
 * 
 * @param x Size along the X axis
 * @param y Size along the Y axis
 * @param z Size along the Z axis
 */
void rectCuboid(float x, float y, float z);

/**
 * @brief Displays a ceramic GLUT teapot of a fixed size
 * 
 * @param x X axis position
 * @param y Y axis position
 * @param z Z axis position
 * @param rot Degree of rotation along the Y axis
 */
void displayTeapot(float x, float y, float z, float rot);

/**
 * @brief Displays a wooden table
 * 
 * @param posx X axis position
 * @param posz Y axis position
 * @param height Table height
 * @param sizex Size along the X axis
 * @param sizez Size along the Z axis
 * @param thickness Tabletop thickness
 */
void displayTable(float posx, float posz, float height, float sizex, float sizez, float thickness);

/**
 * @brief Displays a wooden chair
 * 
 * @param posx X axis position
 * @param posz Z axis position
 * @param height Seat height 
 * @param seat_width Seat width
 * @param seat_depth Seat height
 * @param thickness Seat thickness
 */
void displayChair(float posx, float posz, float height, float seat_width, float seat_depth, float thickness);

/**
 * @brief Displays a stainless steel fridge
 * 
 * @param posx X axis position
 * @param posz Z axis position
 * @param width Fridge width
 * @param height Fridge height
 * @param depth Fridge depth
 */
void displayFridge(float posx, float posz, float width, float height, float depth);

/**
 * @brief Displays a stainless steel garbage bin
 * 
 * @param posx X axis position
 * @param posz Z axis position
 * @param width Bin width
 * @param height Bin height
 */
void displayBin(float posx, float posz, float width, float height);

/**
 * @brief Displays a Lego man style hand
 * 
 */
void drawHand();

/**
 * @brief Displays Sababot
 * 
 */
void displayRobot();

/**
 * @brief Displays a floating string on the screen using 2D projection. 
 * 
 * @param x X axis position
 * @param y Y axis position
 * @param font GLUT bitmap font pointer
 * @param string The string to display
 */
void displayString(float x, float y, void *font, const char *string);

/**
 * @brief Displays adjust ambient box when adjust_ambient=true
 * 
 */
void displayAdjustAmbient();

/**
 * @brief Sets on adjust_ambient flag
 * 
 */
void toggleAdjustAmbient();

/**
 * @brief Displays help menu when show_help=true
 * 
 */
void displayHelp();

/**
 * @brief Toggles show_help flag on/off
 * 
 */
void toggleHelp();

/**
 * @brief Displays the type of moving mode (robot, camera, light) on the top left corner
 * 
 */
void displayMovingMode();

/**
 * @brief Saves user's keyboard input for ambient light adjusment
 * 
 * @param key The key that was typed
 */
void HandleKeystrokes(unsigned char key);

/**
 * @brief Keyboard callback function (ASCII characters)
 * 
 * @param key The key that was typed
 * @param x Mouse x
 * @param y Mouse y
 */
void Keyboard(unsigned char key, int x, int y);

/**
 * @brief Special keyboard callback function (non-ASCII characters)
 * 
 * @param key The key that was typed
 * @param x Mouse x
 * @param y Mouse y
 */
void SpecialKeyboard(int key, int x, int y);

#endif