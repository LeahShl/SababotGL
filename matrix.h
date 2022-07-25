#include <math.h>
#include <array>
using namespace std;

#define X_AXIS 0
#define Y_AXIS 1
#define Z_AXIS 3

typedef std::array<float, 16> mat4X4;


static mat4X4 identity_mat = {1.0, 0.0, 0.0, 0.0,
                              0.0, 1.0, 0.0, 0.0,
                              0.0, 0.0, 1.0, 0.0,
                              0.0, 0.0, 0.0, 1.0};

mat4X4 loadTranslation(float x, float y, float z)
{
    mat4X4 translate = {1.0, 0.0, 0.0, x,
                        0.0, 1.0, 0.0, y,
                        0.0, 0.0, 1.0, z,
                        0.0, 0.0, 0.0, 1.0};
    return translate;
}

mat4X4 loadRotation(float a, int axis)
{
    mat4X4 rotate = identity_mat;
    
    float cosa = cos(a), sina = sin(a);
    switch (axis)
    {
    case X_AXIS:
        rotate[5] = cosa;
        rotate[6] = -sina;
        rotate[9] = sina;
        rotate[10] = cosa;
        break;
    
    case Y_AXIS:
        rotate[0] = cosa;
        rotate[2] = sina;
        rotate[8] = -sina;
        rotate[10] = cosa;
        break;

    case Z_AXIS:
        rotate[0] = cosa;
        rotate[1] = -sina;
        rotate[4] = sina;
        rotate[5] = cosa;
        break;
    }
    return rotate;
}

mat4X4 matMult(mat4X4 matleft, mat4X4 matright)
{
    float temp;
    mat4X4 result = identity_mat;
    for(int i=0; i<4; i++)
    {
        for(int j=0; j<4; j++)
        {
            temp = 0.0;
            for(int k=0; k<4; k++)
            {
                temp += matleft[4*i+k] * matright[j+4*k];
            }
            result[4*i+j] = temp;
        }
    }
    return result;
}