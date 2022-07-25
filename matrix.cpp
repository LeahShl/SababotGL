#include <math.h>

#define X_AXIS 0
#define Y_AXIS 1
#define Z_AXIS 3

typedef float mat4X4[16];

mat4X4 * loadTranslation(float x, float y, float z)
{
    mat4X4 translate = {1.0, 0.0, 0.0, x,
                        0.0, 1.0, 0.0, y,
                        0.0, 0.0, 1.0, z,
                        0.0, 0.0, 0.0, 1.0};
    return &translate;
}

mat4X4 * loadRotation(float a, int axis)
{
    mat4X4 rotate = {1.0, 0.0, 0.0, 0.0,
                     0.0, 1.0, 0.0, 0.0,
                     0.0, 0.0, 1.0, 0.0,
                     0.0, 0.0, 0.0, 1.0};
    switch (axis)
    {
    case X_AXIS:
        rotate[5] = cos(a);
        rotate[6] = -sin(a);
        rotate[9] = -rotate[6];
        rotate[10] = rotate[5];
        break;
    
    case Y_AXIS:
        rotate[0] = cos(a);
        rotate[2] = sin(a);
        rotate[8] = -rotate[2];
        rotate[10] = rotate[0];
        break;

    case Z_AXIS:
        rotate[0] = cos(a);
        rotate[1] = -sin(a);
        rotate[4] = -rotate[1];
        rotate[5] = rotate[0];
        break;
    }
    return &rotate;
}