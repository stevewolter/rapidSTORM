#ifndef DSTORM_DIRECTION_H
#define DSTORM_DIRECTION_H

namespace dStorm {

enum Direction {
    Direction_X = 0,
    Direction_Y = 1,
    Direction_Z = 2,
    Direction_First = Direction_X,
    Direction_2D = Direction_Y+1,
    Direction_3D = Direction_Z+1
};

inline Direction& operator++( Direction& target )
{
    target = static_cast< Direction >( target + 1 ) ;
    return target ;
} 

}

#endif
