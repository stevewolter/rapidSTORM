    void add_value( Eigen::Array<Number,1,1>& result ) {
        result(0,0) = ( exp( - 0.5 * ( pow(( x - x0)  / ( s0x * sqrt(( zx -
        z0)  / dzx[1] + pow(( zx - z0)  / dzx[2], 2) + pow(( zx - z0)  /
        dzx[3], 3) + pow(( zx - z0)  / dzx[4], 4) + 1)) , 2) + pow(( y - y0)
        / ( s0y * sqrt(( zy - z0)  / dzy[1] + pow(( zy - z0)  / dzy[2], 2)
        + pow(( zy - z0)  / dzy[3], 3) + pow(( zy - z0)  / dzy[4], 4) + 1))
        , 2)) ) * theta * A * pixelarea)  / ( 2 * Pi * s0x * sqrt(( zx - z0)
        / dzx[1] + pow(( zx - z0)  / dzx[2], 2) + pow(( zx - z0)  / dzx[3],
        3) + pow(( zx - z0)  / dzx[4], 4) + 1) * s0y * sqrt(( zy - z0)  /
        dzy[1] + pow(( zy - z0)  / dzy[2], 2) + pow(( zy - z0)  / dzy[3],
        3) + pow(( zy - z0)  / dzy[4], 4) + 1)) ;
    }

    template <typename Target> void derivative( Target result, Amplitude ) {
        result(0,0) = ( exp( - 0.5 * ( pow(( x - x0)  / ( s0x * sqrt(( zx -
        z0)  / dzx[1] + pow(( zx - z0)  / dzx[2], 2) + pow(( zx - z0)  /
        dzx[3], 3) + pow(( zx - z0)  / dzx[4], 4) + 1)) , 2) + pow(( y - y0)
        / ( s0y * sqrt(( zy - z0)  / dzy[1] + pow(( zy - z0)  / dzy[2], 2)
        + pow(( zy - z0)  / dzy[3], 3) + pow(( zy - z0)  / dzy[4], 4) + 1))
        , 2)) ) * theta * pixelarea)  / ( 2 * Pi * s0x * sqrt(( zx - z0)  /
        dzx[1] + pow(( zx - z0)  / dzx[2], 2) + pow(( zx - z0)  / dzx[3],
        3) + pow(( zx - z0)  / dzx[4], 4) + 1) * s0y * sqrt(( zy - z0)  /
        dzy[1] + pow(( zy - z0)  / dzy[2], 2) + pow(( zy - z0)  / dzy[3],
        3) + pow(( zy - z0)  / dzy[4], 4) + 1)) ;
    }

    template <typename Target> void derivative( Target result, Prefactor ) {
        result(0,0) = ( exp( - 0.5 * ( pow(( x - x0)  / ( s0x * sqrt(( zx -
        z0)  / dzx[1] + pow(( zx - z0)  / dzx[2], 2) + pow(( zx - z0)  /
        dzx[3], 3) + pow(( zx - z0)  / dzx[4], 4) + 1)) , 2) + pow(( y -
        y0)  / ( s0y * sqrt(( zy - z0)  / dzy[1] + pow(( zy - z0)  / dzy[2],
        2) + pow(( zy - z0)  / dzy[3], 3) + pow(( zy - z0)  / dzy[4], 4) +
        1)) , 2)) ) * A * pixelarea)  / ( 2 * Pi * s0x * sqrt(( zx - z0)  /
        dzx[1] + pow(( zx - z0)  / dzx[2], 2) + pow(( zx - z0)  / dzx[3],
        3) + pow(( zx - z0)  / dzx[4], 4) + 1) * s0y * sqrt(( zy - z0)  /
        dzy[1] + pow(( zy - z0)  / dzy[2], 2) + pow(( zy - z0)  / dzy[3],
        3) + pow(( zy - z0)  / dzy[4], 4) + 1)) ;
    }

    template <typename Target> void derivative( Target result, MeanZ ) {
        result(0,0) = ( ( -64 * pow(s0x, 2) * pow(( zx - z0)  / dzx[1]
        + pow(( zx - z0)  / dzx[2], 2) + pow(( zx - z0)  / dzx[3], 3) +
        pow(( zx - z0)  / dzx[4], 4) + 1, 2) * pow(s0y, 2) * ( ( zy - z0)
        / dzy[1] + pow(( zy - z0)  / dzy[2], 2) + pow(( zy - z0)  / dzy[3],
        3) + pow(( zy - z0)  / dzy[4], 4) + 1)  * pow(z0, 3) * pow(dzx[2],
        2) * dzx[1] * pow(dzx[3], 3) * pow(dzx[4], 4) * pow(dzy[2], 2) *
        dzy[1] * pow(dzy[3], 3) + 192 * pow(s0x, 2) * pow(( zx - z0)  /
        dzx[1] + pow(( zx - z0)  / dzx[2], 2) + pow(( zx - z0)  / dzx[3],
        3) + pow(( zx - z0)  / dzx[4], 4) + 1, 2) * pow(s0y, 2) * ( ( zy -
        z0)  / dzy[1] + pow(( zy - z0)  / dzy[2], 2) + pow(( zy - z0)  /
        dzy[3], 3) + pow(( zy - z0)  / dzy[4], 4) + 1)  * pow(z0, 2) *
        pow(dzx[2], 2) * dzx[1] * pow(dzx[3], 3) * pow(dzx[4], 4) * zy *
        pow(dzy[2], 2) * dzy[1] * pow(dzy[3], 3) + 48 * pow(s0x, 2) * pow((
        zx - z0)  / dzx[1] + pow(( zx - z0)  / dzx[2], 2) + pow(( zx - z0)
        / dzx[3], 3) + pow(( zx - z0)  / dzx[4], 4) + 1, 2) * pow(s0y, 2) *
        ( ( zy - z0)  / dzy[1] + pow(( zy - z0)  / dzy[2], 2) + pow(( zy -
        z0)  / dzy[3], 3) + pow(( zy - z0)  / dzy[4], 4) + 1)  * pow(z0,
        2) * pow(dzx[2], 2) * dzx[1] * pow(dzx[3], 3) * pow(dzx[4], 4)
        * pow(dzy[2], 2) * dzy[1] * pow(dzy[4], 4) + -192 * pow(s0x, 2) *
        pow(( zx - z0)  / dzx[1] + pow(( zx - z0)  / dzx[2], 2) + pow(( zx -
        z0)  / dzx[3], 3) + pow(( zx - z0)  / dzx[4], 4) + 1, 2) * pow(s0y,
        2) * ( ( zy - z0)  / dzy[1] + pow(( zy - z0)  / dzy[2], 2) + pow((
        zy - z0)  / dzy[3], 3) + pow(( zy - z0)  / dzy[4], 4) + 1)  * z0 *
        pow(dzx[2], 2) * dzx[1] * pow(dzx[3], 3) * pow(dzx[4], 4) * pow(zy,
        2) * pow(dzy[2], 2) * dzy[1] * pow(dzy[3], 3) - 96 * pow(s0x, 2)
        * pow(( zx - z0)  / dzx[1] + pow(( zx - z0)  / dzx[2], 2) + pow((
        zx - z0)  / dzx[3], 3) + pow(( zx - z0)  / dzx[4], 4) + 1, 2) *
        pow(s0y, 2) * ( ( zy - z0)  / dzy[1] + pow(( zy - z0)  / dzy[2], 2)
        + pow(( zy - z0)  / dzy[3], 3) + pow(( zy - z0)  / dzy[4], 4) + 1)
        * z0 * pow(dzx[2], 2) * dzx[1] * pow(dzx[3], 3) * pow(dzx[4], 4) *
        zy * pow(dzy[2], 2) * dzy[1] * pow(dzy[4], 4) - 32 * pow(s0x, 2) *
        pow(( zx - z0)  / dzx[1] + pow(( zx - z0)  / dzx[2], 2) + pow(( zx -
        z0)  / dzx[3], 3) + pow(( zx - z0)  / dzx[4], 4) + 1, 2) * pow(s0y,
        2) * ( ( zy - z0)  / dzy[1] + pow(( zy - z0)  / dzy[2], 2) + pow((
        zy - z0)  / dzy[3], 3) + pow(( zy - z0)  / dzy[4], 4) + 1)  * z0 *
        pow(dzx[2], 2) * dzx[1] * pow(dzx[3], 3) * pow(dzx[4], 4) * dzy[1]
        * pow(dzy[3], 3) * pow(dzy[4], 4) + 64 * pow(s0x, 2) * pow(( zx -
        z0)  / dzx[1] + pow(( zx - z0)  / dzx[2], 2) + pow(( zx - z0)  /
        dzx[3], 3) + pow(( zx - z0)  / dzx[4], 4) + 1, 2) * pow(s0y, 2) * (
        ( zy - z0)  / dzy[1] + pow(( zy - z0)  / dzy[2], 2) + pow(( zy - z0)
        / dzy[3], 3) + pow(( zy - z0)  / dzy[4], 4) + 1)  * pow(dzx[2], 2) *
        dzx[1] * pow(dzx[3], 3) * pow(dzx[4], 4) * pow(zy, 3) * pow(dzy[2],
        2) * dzy[1] * pow(dzy[3], 3) + 48 * pow(s0x, 2) * pow(( zx - z0)  /
        dzx[1] + pow(( zx - z0)  / dzx[2], 2) + pow(( zx - z0)  / dzx[3],
        3) + pow(( zx - z0)  / dzx[4], 4) + 1, 2) * pow(s0y, 2) * ( ( zy -
        z0)  / dzy[1] + pow(( zy - z0)  / dzy[2], 2) + pow(( zy - z0)  /
        dzy[3], 3) + pow(( zy - z0)  / dzy[4], 4) + 1)  * pow(dzx[2], 2) *
        dzx[1] * pow(dzx[3], 3) * pow(dzx[4], 4) * pow(zy, 2) * pow(dzy[2],
        2) * dzy[1] * pow(dzy[4], 4) + 32 * pow(s0x, 2) * pow(( zx - z0)  /
        dzx[1] + pow(( zx - z0)  / dzx[2], 2) + pow(( zx - z0)  / dzx[3], 3)
        + pow(( zx - z0)  / dzx[4], 4) + 1, 2) * pow(s0y, 2) * ( ( zy - z0)
        / dzy[1] + pow(( zy - z0)  / dzy[2], 2) + pow(( zy - z0)  / dzy[3],
        3) + pow(( zy - z0)  / dzy[4], 4) + 1)  * pow(dzx[2], 2) * dzx[1]
        * pow(dzx[3], 3) * pow(dzx[4], 4) * zy * dzy[1] * pow(dzy[3], 3) *
        pow(dzy[4], 4) + 16 * pow(s0x, 2) * pow(( zx - z0)  / dzx[1] + pow((
        zx - z0)  / dzx[2], 2) + pow(( zx - z0)  / dzx[3], 3) + pow(( zx -
        z0)  / dzx[4], 4) + 1, 2) * pow(s0y, 2) * ( ( zy - z0)  / dzy[1] +
        pow(( zy - z0)  / dzy[2], 2) + pow(( zy - z0)  / dzy[3], 3) + pow((
        zy - z0)  / dzy[4], 4) + 1)  * pow(dzx[2], 2) * dzx[1] * pow(dzx[3],
        3) * pow(dzx[4], 4) * pow(dzy[2], 2) * pow(dzy[3], 3) * pow(dzy[4],
        4) + 64. * pow(s0x, 2) * pow(( zx - z0)  / dzx[1] + pow(( zx - z0)  /
        dzx[2], 2) + pow(( zx - z0)  / dzx[3], 3) + pow(( zx - z0)  / dzx[4],
        4) + 1, 2) * pow(z0, 3) * pow(dzx[2], 2) * dzx[1] * pow(dzx[3], 3) *
        pow(dzx[4], 4) * pow(y, 2) * pow(dzy[2], 2) * dzy[1] * pow(dzy[3], 3)
        - 128. * pow(s0x, 2) * pow(( zx - z0)  / dzx[1] + pow(( zx - z0)  /
        dzx[2], 2) + pow(( zx - z0)  / dzx[3], 3) + pow(( zx - z0)  / dzx[4],
        4) + 1, 2) * pow(z0, 3) * pow(dzx[2], 2) * dzx[1] * pow(dzx[3], 3) *
        pow(dzx[4], 4) * y * y0 * pow(dzy[2], 2) * dzy[1] * pow(dzy[3], 3)
        + 64. * pow(s0x, 2) * pow(( zx - z0)  / dzx[1] + pow(( zx - z0)  /
        dzx[2], 2) + pow(( zx - z0)  / dzx[3], 3) + pow(( zx - z0)  / dzx[4],
        4) + 1, 2) * pow(z0, 3) * pow(dzx[2], 2) * dzx[1] * pow(dzx[3], 3) *
        pow(dzx[4], 4) * pow(y0, 2) * pow(dzy[2], 2) * dzy[1] * pow(dzy[3],
        3) + -192. * pow(s0x, 2) * pow(( zx - z0)  / dzx[1] + pow(( zx
        - z0)  / dzx[2], 2) + pow(( zx - z0)  / dzx[3], 3) + pow(( zx -
        z0)  / dzx[4], 4) + 1, 2) * pow(z0, 2) * pow(dzx[2], 2) * dzx[1] *
        pow(dzx[3], 3) * pow(dzx[4], 4) * pow(y, 2) * zy * pow(dzy[2], 2)
        * dzy[1] * pow(dzy[3], 3) - 48. * pow(s0x, 2) * pow(( zx - z0)  /
        dzx[1] + pow(( zx - z0)  / dzx[2], 2) + pow(( zx - z0)  / dzx[3], 3)
        + pow(( zx - z0)  / dzx[4], 4) + 1, 2) * pow(z0, 2) * pow(dzx[2], 2)
        * dzx[1] * pow(dzx[3], 3) * pow(dzx[4], 4) * pow(y, 2) * pow(dzy[2],
        2) * dzy[1] * pow(dzy[4], 4) + 384. * pow(s0x, 2) * pow(( zx - z0)  /
        dzx[1] + pow(( zx - z0)  / dzx[2], 2) + pow(( zx - z0)  / dzx[3], 3) +
        pow(( zx - z0)  / dzx[4], 4) + 1, 2) * pow(z0, 2) * pow(dzx[2], 2) *
        dzx[1] * pow(dzx[3], 3) * pow(dzx[4], 4) * y * y0 * zy * pow(dzy[2],
        2) * dzy[1] * pow(dzy[3], 3) + 96. * pow(s0x, 2) * pow(( zx - z0)
        / dzx[1] + pow(( zx - z0)  / dzx[2], 2) + pow(( zx - z0)  / dzx[3],
        3) + pow(( zx - z0)  / dzx[4], 4) + 1, 2) * pow(z0, 2) * pow(dzx[2],
        2) * dzx[1] * pow(dzx[3], 3) * pow(dzx[4], 4) * y * y0 * pow(dzy[2],
        2) * dzy[1] * pow(dzy[4], 4) + -192. * pow(s0x, 2) * pow(( zx - z0)
        / dzx[1] + pow(( zx - z0)  / dzx[2], 2) + pow(( zx - z0)  / dzx[3],
        3) + pow(( zx - z0)  / dzx[4], 4) + 1, 2) * pow(z0, 2) * pow(dzx[2],
        2) * dzx[1] * pow(dzx[3], 3) * pow(dzx[4], 4) * pow(y0, 2) * zy *
        pow(dzy[2], 2) * dzy[1] * pow(dzy[3], 3) - 48. * pow(s0x, 2) * pow((
        zx - z0)  / dzx[1] + pow(( zx - z0)  / dzx[2], 2) + pow(( zx - z0)
        / dzx[3], 3) + pow(( zx - z0)  / dzx[4], 4) + 1, 2) * pow(z0, 2) *
        pow(dzx[2], 2) * dzx[1] * pow(dzx[3], 3) * pow(dzx[4], 4) * pow(y0,
        2) * pow(dzy[2], 2) * dzy[1] * pow(dzy[4], 4) + 192. * pow(s0x, 2)
        * pow(( zx - z0)  / dzx[1] + pow(( zx - z0)  / dzx[2], 2) + pow((
        zx - z0)  / dzx[3], 3) + pow(( zx - z0)  / dzx[4], 4) + 1, 2) * z0 *
        pow(dzx[2], 2) * dzx[1] * pow(dzx[3], 3) * pow(dzx[4], 4) * pow(y,
        2) * pow(zy, 2) * pow(dzy[2], 2) * dzy[1] * pow(dzy[3], 3) + 96. *
        pow(s0x, 2) * pow(( zx - z0)  / dzx[1] + pow(( zx - z0)  / dzx[2], 2)
        + pow(( zx - z0)  / dzx[3], 3) + pow(( zx - z0)  / dzx[4], 4) + 1,
        2) * z0 * pow(dzx[2], 2) * dzx[1] * pow(dzx[3], 3) * pow(dzx[4], 4)
        * pow(y, 2) * zy * pow(dzy[2], 2) * dzy[1] * pow(dzy[4], 4) + 32. *
        pow(s0x, 2) * pow(( zx - z0)  / dzx[1] + pow(( zx - z0)  / dzx[2],
        2) + pow(( zx - z0)  / dzx[3], 3) + pow(( zx - z0)  / dzx[4], 4) +
        1, 2) * z0 * pow(dzx[2], 2) * dzx[1] * pow(dzx[3], 3) * pow(dzx[4],
        4) * pow(y, 2) * dzy[1] * pow(dzy[3], 3) * pow(dzy[4], 4) + -384. *
        pow(s0x, 2) * pow(( zx - z0)  / dzx[1] + pow(( zx - z0)  / dzx[2], 2)
        + pow(( zx - z0)  / dzx[3], 3) + pow(( zx - z0)  / dzx[4], 4) + 1, 2)
        * z0 * pow(dzx[2], 2) * dzx[1] * pow(dzx[3], 3) * pow(dzx[4], 4) * y *
        y0 * pow(zy, 2) * pow(dzy[2], 2) * dzy[1] * pow(dzy[3], 3) - 192. *
        pow(s0x, 2) * pow(( zx - z0)  / dzx[1] + pow(( zx - z0)  / dzx[2],
        2) + pow(( zx - z0)  / dzx[3], 3) + pow(( zx - z0)  / dzx[4], 4) +
        1, 2) * z0 * pow(dzx[2], 2) * dzx[1] * pow(dzx[3], 3) * pow(dzx[4],
        4) * y * y0 * zy * pow(dzy[2], 2) * dzy[1] * pow(dzy[4], 4) - 64. *
        pow(s0x, 2) * pow(( zx - z0)  / dzx[1] + pow(( zx - z0)  / dzx[2], 2)
        + pow(( zx - z0)  / dzx[3], 3) + pow(( zx - z0)  / dzx[4], 4) + 1,
        2) * z0 * pow(dzx[2], 2) * dzx[1] * pow(dzx[3], 3) * pow(dzx[4], 4)
        * y * y0 * dzy[1] * pow(dzy[3], 3) * pow(dzy[4], 4) + 192. * pow(s0x,
        2) * pow(( zx - z0)  / dzx[1] + pow(( zx - z0)  / dzx[2], 2) + pow((
        zx - z0)  / dzx[3], 3) + pow(( zx - z0)  / dzx[4], 4) + 1, 2) * z0 *
        pow(dzx[2], 2) * dzx[1] * pow(dzx[3], 3) * pow(dzx[4], 4) * pow(y0,
        2) * pow(zy, 2) * pow(dzy[2], 2) * dzy[1] * pow(dzy[3], 3) + 96. *
        pow(s0x, 2) * pow(( zx - z0)  / dzx[1] + pow(( zx - z0)  / dzx[2], 2)
        + pow(( zx - z0)  / dzx[3], 3) + pow(( zx - z0)  / dzx[4], 4) + 1,
        2) * z0 * pow(dzx[2], 2) * dzx[1] * pow(dzx[3], 3) * pow(dzx[4], 4)
        * pow(y0, 2) * zy * pow(dzy[2], 2) * dzy[1] * pow(dzy[4], 4) + 32. *
        pow(s0x, 2) * pow(( zx - z0)  / dzx[1] + pow(( zx - z0)  / dzx[2],
        2) + pow(( zx - z0)  / dzx[3], 3) + pow(( zx - z0)  / dzx[4], 4) +
        1, 2) * z0 * pow(dzx[2], 2) * dzx[1] * pow(dzx[3], 3) * pow(dzx[4],
        4) * pow(y0, 2) * dzy[1] * pow(dzy[3], 3) * pow(dzy[4], 4) + -64. *
        pow(s0x, 2) * pow(( zx - z0)  / dzx[1] + pow(( zx - z0)  / dzx[2], 2)
        + pow(( zx - z0)  / dzx[3], 3) + pow(( zx - z0)  / dzx[4], 4) + 1, 2)
        * pow(dzx[2], 2) * dzx[1] * pow(dzx[3], 3) * pow(dzx[4], 4) * pow(y,
        2) * pow(zy, 3) * pow(dzy[2], 2) * dzy[1] * pow(dzy[3], 3) - 48. *
        pow(s0x, 2) * pow(( zx - z0)  / dzx[1] + pow(( zx - z0)  / dzx[2], 2)
        + pow(( zx - z0)  / dzx[3], 3) + pow(( zx - z0)  / dzx[4], 4) + 1, 2)
        * pow(dzx[2], 2) * dzx[1] * pow(dzx[3], 3) * pow(dzx[4], 4) * pow(y,
        2) * pow(zy, 2) * pow(dzy[2], 2) * dzy[1] * pow(dzy[4], 4) - 32. *
        pow(s0x, 2) * pow(( zx - z0)  / dzx[1] + pow(( zx - z0)  / dzx[2],
        2) + pow(( zx - z0)  / dzx[3], 3) + pow(( zx - z0)  / dzx[4], 4) +
        1, 2) * pow(dzx[2], 2) * dzx[1] * pow(dzx[3], 3) * pow(dzx[4], 4)
        * pow(y, 2) * zy * dzy[1] * pow(dzy[3], 3) * pow(dzy[4], 4) - 16. *
        pow(s0x, 2) * pow(( zx - z0)  / dzx[1] + pow(( zx - z0)  / dzx[2],
        2) + pow(( zx - z0)  / dzx[3], 3) + pow(( zx - z0)  / dzx[4], 4) +
        1, 2) * pow(dzx[2], 2) * dzx[1] * pow(dzx[3], 3) * pow(dzx[4], 4) *
        pow(y, 2) * pow(dzy[2], 2) * pow(dzy[3], 3) * pow(dzy[4], 4) + 128. *
        pow(s0x, 2) * pow(( zx - z0)  / dzx[1] + pow(( zx - z0)  / dzx[2],
        2) + pow(( zx - z0)  / dzx[3], 3) + pow(( zx - z0)  / dzx[4], 4) + 1,
        2) * pow(dzx[2], 2) * dzx[1] * pow(dzx[3], 3) * pow(dzx[4], 4) * y *
        y0 * pow(zy, 3) * pow(dzy[2], 2) * dzy[1] * pow(dzy[3], 3) + 96. *
        pow(s0x, 2) * pow(( zx - z0)  / dzx[1] + pow(( zx - z0)  / dzx[2],
        2) + pow(( zx - z0)  / dzx[3], 3) + pow(( zx - z0)  / dzx[4], 4) + 1,
        2) * pow(dzx[2], 2) * dzx[1] * pow(dzx[3], 3) * pow(dzx[4], 4) * y *
        y0 * pow(zy, 2) * pow(dzy[2], 2) * dzy[1] * pow(dzy[4], 4) + 64. *
        pow(s0x, 2) * pow(( zx - z0)  / dzx[1] + pow(( zx - z0)  / dzx[2],
        2) + pow(( zx - z0)  / dzx[3], 3) + pow(( zx - z0)  / dzx[4], 4) +
        1, 2) * pow(dzx[2], 2) * dzx[1] * pow(dzx[3], 3) * pow(dzx[4], 4)
        * y * y0 * zy * dzy[1] * pow(dzy[3], 3) * pow(dzy[4], 4) + 32. *
        pow(s0x, 2) * pow(( zx - z0)  / dzx[1] + pow(( zx - z0)  / dzx[2],
        2) + pow(( zx - z0)  / dzx[3], 3) + pow(( zx - z0)  / dzx[4], 4) +
        1, 2) * pow(dzx[2], 2) * dzx[1] * pow(dzx[3], 3) * pow(dzx[4], 4) *
        y * y0 * pow(dzy[2], 2) * pow(dzy[3], 3) * pow(dzy[4], 4) + -64. *
        pow(s0x, 2) * pow(( zx - z0)  / dzx[1] + pow(( zx - z0)  / dzx[2],
        2) + pow(( zx - z0)  / dzx[3], 3) + pow(( zx - z0)  / dzx[4], 4) +
        1, 2) * pow(dzx[2], 2) * dzx[1] * pow(dzx[3], 3) * pow(dzx[4], 4) *
        pow(y0, 2) * pow(zy, 3) * pow(dzy[2], 2) * dzy[1] * pow(dzy[3], 3)
        - 48. * pow(s0x, 2) * pow(( zx - z0)  / dzx[1] + pow(( zx - z0)  /
        dzx[2], 2) + pow(( zx - z0)  / dzx[3], 3) + pow(( zx - z0)  / dzx[4],
        4) + 1, 2) * pow(dzx[2], 2) * dzx[1] * pow(dzx[3], 3) * pow(dzx[4],
        4) * pow(y0, 2) * pow(zy, 2) * pow(dzy[2], 2) * dzy[1] * pow(dzy[4],
        4) - 32. * pow(s0x, 2) * pow(( zx - z0)  / dzx[1] + pow(( zx - z0)  /
        dzx[2], 2) + pow(( zx - z0)  / dzx[3], 3) + pow(( zx - z0)  / dzx[4],
        4) + 1, 2) * pow(dzx[2], 2) * dzx[1] * pow(dzx[3], 3) * pow(dzx[4], 4)
        * pow(y0, 2) * zy * dzy[1] * pow(dzy[3], 3) * pow(dzy[4], 4) - 16. *
        pow(s0x, 2) * pow(( zx - z0)  / dzx[1] + pow(( zx - z0)  / dzx[2],
        2) + pow(( zx - z0)  / dzx[3], 3) + pow(( zx - z0)  / dzx[4], 4) +
        1, 2) * pow(dzx[2], 2) * dzx[1] * pow(dzx[3], 3) * pow(dzx[4], 4) *
        pow(y0, 2) * pow(dzy[2], 2) * pow(dzy[3], 3) * pow(dzy[4], 4) + 64 *
        pow(s0x, 2) * ( ( zx - z0)  / dzx[1] + pow(( zx - z0)  / dzx[2], 2) +
        pow(( zx - z0)  / dzx[3], 3) + pow(( zx - z0)  / dzx[4], 4) + 1)  *
        pow(s0y, 2) * pow(( zy - z0)  / dzy[1] + pow(( zy - z0)  / dzy[2],
        2) + pow(( zy - z0)  / dzy[3], 3) + pow(( zy - z0)  / dzy[4], 4)
        + 1, 2) * pow(zx, 3) * pow(dzx[2], 2) * dzx[1] * pow(dzx[3], 3) *
        pow(dzy[2], 2) * dzy[1] * pow(dzy[3], 3) * pow(dzy[4], 4) + -192 *
        pow(s0x, 2) * ( ( zx - z0)  / dzx[1] + pow(( zx - z0)  / dzx[2], 2)
        + pow(( zx - z0)  / dzx[3], 3) + pow(( zx - z0)  / dzx[4], 4) + 1)
        * pow(s0y, 2) * pow(( zy - z0)  / dzy[1] + pow(( zy - z0)  / dzy[2],
        2) + pow(( zy - z0)  / dzy[3], 3) + pow(( zy - z0)  / dzy[4], 4) +
        1, 2) * pow(zx, 2) * z0 * pow(dzx[2], 2) * dzx[1] * pow(dzx[3], 3)
        * pow(dzy[2], 2) * dzy[1] * pow(dzy[3], 3) * pow(dzy[4], 4) + 48 *
        pow(s0x, 2) * ( ( zx - z0)  / dzx[1] + pow(( zx - z0)  / dzx[2], 2)
        + pow(( zx - z0)  / dzx[3], 3) + pow(( zx - z0)  / dzx[4], 4) + 1)  *
        pow(s0y, 2) * pow(( zy - z0)  / dzy[1] + pow(( zy - z0)  / dzy[2], 2)
        + pow(( zy - z0)  / dzy[3], 3) + pow(( zy - z0)  / dzy[4], 4) + 1, 2)
        * pow(zx, 2) * pow(dzx[2], 2) * dzx[1] * pow(dzx[4], 4) * pow(dzy[2],
        2) * dzy[1] * pow(dzy[3], 3) * pow(dzy[4], 4) + 192 * pow(s0x, 2) *
        ( ( zx - z0)  / dzx[1] + pow(( zx - z0)  / dzx[2], 2) + pow(( zx -
        z0)  / dzx[3], 3) + pow(( zx - z0)  / dzx[4], 4) + 1)  * pow(s0y,
        2) * pow(( zy - z0)  / dzy[1] + pow(( zy - z0)  / dzy[2], 2) + pow((
        zy - z0)  / dzy[3], 3) + pow(( zy - z0)  / dzy[4], 4) + 1, 2) * zx *
        pow(z0, 2) * pow(dzx[2], 2) * dzx[1] * pow(dzx[3], 3) * pow(dzy[2],
        2) * dzy[1] * pow(dzy[3], 3) * pow(dzy[4], 4) - 96 * pow(s0x, 2) *
        ( ( zx - z0)  / dzx[1] + pow(( zx - z0)  / dzx[2], 2) + pow(( zx -
        z0)  / dzx[3], 3) + pow(( zx - z0)  / dzx[4], 4) + 1)  * pow(s0y, 2)
        * pow(( zy - z0)  / dzy[1] + pow(( zy - z0)  / dzy[2], 2) + pow((
        zy - z0)  / dzy[3], 3) + pow(( zy - z0)  / dzy[4], 4) + 1, 2) *
        zx * z0 * pow(dzx[2], 2) * dzx[1] * pow(dzx[4], 4) * pow(dzy[2],
        2) * dzy[1] * pow(dzy[3], 3) * pow(dzy[4], 4) + 32 * pow(s0x, 2) *
        ( ( zx - z0)  / dzx[1] + pow(( zx - z0)  / dzx[2], 2) + pow(( zx -
        z0)  / dzx[3], 3) + pow(( zx - z0)  / dzx[4], 4) + 1)  * pow(s0y,
        2) * pow(( zy - z0)  / dzy[1] + pow(( zy - z0)  / dzy[2], 2) +
        pow(( zy - z0)  / dzy[3], 3) + pow(( zy - z0)  / dzy[4], 4) + 1,
        2) * zx * dzx[1] * pow(dzx[3], 3) * pow(dzx[4], 4) * pow(dzy[2],
        2) * dzy[1] * pow(dzy[3], 3) * pow(dzy[4], 4) + -64 * pow(s0x, 2) *
        ( ( zx - z0)  / dzx[1] + pow(( zx - z0)  / dzx[2], 2) + pow(( zx -
        z0)  / dzx[3], 3) + pow(( zx - z0)  / dzx[4], 4) + 1)  * pow(s0y, 2)
        * pow(( zy - z0)  / dzy[1] + pow(( zy - z0)  / dzy[2], 2) + pow((
        zy - z0)  / dzy[3], 3) + pow(( zy - z0)  / dzy[4], 4) + 1, 2) *
        pow(z0, 3) * pow(dzx[2], 2) * dzx[1] * pow(dzx[3], 3) * pow(dzy[2],
        2) * dzy[1] * pow(dzy[3], 3) * pow(dzy[4], 4) + 48 * pow(s0x, 2) *
        ( ( zx - z0)  / dzx[1] + pow(( zx - z0)  / dzx[2], 2) + pow(( zx -
        z0)  / dzx[3], 3) + pow(( zx - z0)  / dzx[4], 4) + 1)  * pow(s0y, 2)
        * pow(( zy - z0)  / dzy[1] + pow(( zy - z0)  / dzy[2], 2) + pow((
        zy - z0)  / dzy[3], 3) + pow(( zy - z0)  / dzy[4], 4) + 1, 2) *
        pow(z0, 2) * pow(dzx[2], 2) * dzx[1] * pow(dzx[4], 4) * pow(dzy[2],
        2) * dzy[1] * pow(dzy[3], 3) * pow(dzy[4], 4) - 32 * pow(s0x, 2) *
        ( ( zx - z0)  / dzx[1] + pow(( zx - z0)  / dzx[2], 2) + pow(( zx -
        z0)  / dzx[3], 3) + pow(( zx - z0)  / dzx[4], 4) + 1)  * pow(s0y,
        2) * pow(( zy - z0)  / dzy[1] + pow(( zy - z0)  / dzy[2], 2) + pow((
        zy - z0)  / dzy[3], 3) + pow(( zy - z0)  / dzy[4], 4) + 1, 2) * z0 *
        dzx[1] * pow(dzx[3], 3) * pow(dzx[4], 4) * pow(dzy[2], 2) * dzy[1]
        * pow(dzy[3], 3) * pow(dzy[4], 4) + 16 * pow(s0x, 2) * ( ( zx - z0)
        / dzx[1] + pow(( zx - z0)  / dzx[2], 2) + pow(( zx - z0)  / dzx[3],
        3) + pow(( zx - z0)  / dzx[4], 4) + 1)  * pow(s0y, 2) * pow(( zy -
        z0)  / dzy[1] + pow(( zy - z0)  / dzy[2], 2) + pow(( zy - z0)  /
        dzy[3], 3) + pow(( zy - z0)  / dzy[4], 4) + 1, 2) * pow(dzx[2],
        2) * pow(dzx[3], 3) * pow(dzx[4], 4) * pow(dzy[2], 2) * dzy[1] *
        pow(dzy[3], 3) * pow(dzy[4], 4) + -64. * pow(s0y, 2) * pow(( zy - z0)
        / dzy[1] + pow(( zy - z0)  / dzy[2], 2) + pow(( zy - z0)  / dzy[3],
        3) + pow(( zy - z0)  / dzy[4], 4) + 1, 2) * pow(x, 2) * pow(zx, 3) *
        pow(dzx[2], 2) * dzx[1] * pow(dzx[3], 3) * pow(dzy[2], 2) * dzy[1] *
        pow(dzy[3], 3) * pow(dzy[4], 4) + 192. * pow(s0y, 2) * pow(( zy - z0)
        / dzy[1] + pow(( zy - z0)  / dzy[2], 2) + pow(( zy - z0)  / dzy[3], 3)
        + pow(( zy - z0)  / dzy[4], 4) + 1, 2) * pow(x, 2) * pow(zx, 2) * z0 *
        pow(dzx[2], 2) * dzx[1] * pow(dzx[3], 3) * pow(dzy[2], 2) * dzy[1] *
        pow(dzy[3], 3) * pow(dzy[4], 4) - 48. * pow(s0y, 2) * pow(( zy - z0)
        / dzy[1] + pow(( zy - z0)  / dzy[2], 2) + pow(( zy - z0)  / dzy[3],
        3) + pow(( zy - z0)  / dzy[4], 4) + 1, 2) * pow(x, 2) * pow(zx, 2) *
        pow(dzx[2], 2) * dzx[1] * pow(dzx[4], 4) * pow(dzy[2], 2) * dzy[1]
        * pow(dzy[3], 3) * pow(dzy[4], 4) + -192. * pow(s0y, 2) * pow(( zy
        - z0)  / dzy[1] + pow(( zy - z0)  / dzy[2], 2) + pow(( zy - z0)  /
        dzy[3], 3) + pow(( zy - z0)  / dzy[4], 4) + 1, 2) * pow(x, 2) * zx *
        pow(z0, 2) * pow(dzx[2], 2) * dzx[1] * pow(dzx[3], 3) * pow(dzy[2],
        2) * dzy[1] * pow(dzy[3], 3) * pow(dzy[4], 4) + 96. * pow(s0y, 2) *
        pow(( zy - z0)  / dzy[1] + pow(( zy - z0)  / dzy[2], 2) + pow(( zy -
        z0)  / dzy[3], 3) + pow(( zy - z0)  / dzy[4], 4) + 1, 2) * pow(x, 2)
        * zx * z0 * pow(dzx[2], 2) * dzx[1] * pow(dzx[4], 4) * pow(dzy[2],
        2) * dzy[1] * pow(dzy[3], 3) * pow(dzy[4], 4) - 32. * pow(s0y, 2) *
        pow(( zy - z0)  / dzy[1] + pow(( zy - z0)  / dzy[2], 2) + pow(( zy -
        z0)  / dzy[3], 3) + pow(( zy - z0)  / dzy[4], 4) + 1, 2) * pow(x, 2)
        * zx * dzx[1] * pow(dzx[3], 3) * pow(dzx[4], 4) * pow(dzy[2], 2) *
        dzy[1] * pow(dzy[3], 3) * pow(dzy[4], 4) + 64. * pow(s0y, 2) * pow((
        zy - z0)  / dzy[1] + pow(( zy - z0)  / dzy[2], 2) + pow(( zy - z0)
        / dzy[3], 3) + pow(( zy - z0)  / dzy[4], 4) + 1, 2) * pow(x, 2) *
        pow(z0, 3) * pow(dzx[2], 2) * dzx[1] * pow(dzx[3], 3) * pow(dzy[2],
        2) * dzy[1] * pow(dzy[3], 3) * pow(dzy[4], 4) - 48. * pow(s0y, 2) *
        pow(( zy - z0)  / dzy[1] + pow(( zy - z0)  / dzy[2], 2) + pow(( zy -
        z0)  / dzy[3], 3) + pow(( zy - z0)  / dzy[4], 4) + 1, 2) * pow(x, 2)
        * pow(z0, 2) * pow(dzx[2], 2) * dzx[1] * pow(dzx[4], 4) * pow(dzy[2],
        2) * dzy[1] * pow(dzy[3], 3) * pow(dzy[4], 4) + 32. * pow(s0y, 2) *
        pow(( zy - z0)  / dzy[1] + pow(( zy - z0)  / dzy[2], 2) + pow(( zy -
        z0)  / dzy[3], 3) + pow(( zy - z0)  / dzy[4], 4) + 1, 2) * pow(x, 2)
        * z0 * dzx[1] * pow(dzx[3], 3) * pow(dzx[4], 4) * pow(dzy[2], 2) *
        dzy[1] * pow(dzy[3], 3) * pow(dzy[4], 4) - 16. * pow(s0y, 2) * pow((
        zy - z0)  / dzy[1] + pow(( zy - z0)  / dzy[2], 2) + pow(( zy - z0)
        / dzy[3], 3) + pow(( zy - z0)  / dzy[4], 4) + 1, 2) * pow(x, 2) *
        pow(dzx[2], 2) * pow(dzx[3], 3) * pow(dzx[4], 4) * pow(dzy[2], 2)
        * dzy[1] * pow(dzy[3], 3) * pow(dzy[4], 4) + 128. * pow(s0y, 2) *
        pow(( zy - z0)  / dzy[1] + pow(( zy - z0)  / dzy[2], 2) + pow(( zy -
        z0)  / dzy[3], 3) + pow(( zy - z0)  / dzy[4], 4) + 1, 2) * x * x0 *
        pow(zx, 3) * pow(dzx[2], 2) * dzx[1] * pow(dzx[3], 3) * pow(dzy[2],
        2) * dzy[1] * pow(dzy[3], 3) * pow(dzy[4], 4) + -384. * pow(s0y, 2)
        * pow(( zy - z0)  / dzy[1] + pow(( zy - z0)  / dzy[2], 2) + pow((
        zy - z0)  / dzy[3], 3) + pow(( zy - z0)  / dzy[4], 4) + 1, 2) * x *
        x0 * pow(zx, 2) * z0 * pow(dzx[2], 2) * dzx[1] * pow(dzx[3], 3) *
        pow(dzy[2], 2) * dzy[1] * pow(dzy[3], 3) * pow(dzy[4], 4) + 96. *
        pow(s0y, 2) * pow(( zy - z0)  / dzy[1] + pow(( zy - z0)  / dzy[2],
        2) + pow(( zy - z0)  / dzy[3], 3) + pow(( zy - z0)  / dzy[4], 4) + 1,
        2) * x * x0 * pow(zx, 2) * pow(dzx[2], 2) * dzx[1] * pow(dzx[4], 4)
        * pow(dzy[2], 2) * dzy[1] * pow(dzy[3], 3) * pow(dzy[4], 4) + 384. *
        pow(s0y, 2) * pow(( zy - z0)  / dzy[1] + pow(( zy - z0)  / dzy[2], 2)
        + pow(( zy - z0)  / dzy[3], 3) + pow(( zy - z0)  / dzy[4], 4) + 1, 2)
        * x * x0 * zx * pow(z0, 2) * pow(dzx[2], 2) * dzx[1] * pow(dzx[3], 3)
        * pow(dzy[2], 2) * dzy[1] * pow(dzy[3], 3) * pow(dzy[4], 4) - 192. *
        pow(s0y, 2) * pow(( zy - z0)  / dzy[1] + pow(( zy - z0)  / dzy[2],
        2) + pow(( zy - z0)  / dzy[3], 3) + pow(( zy - z0)  / dzy[4], 4) +
        1, 2) * x * x0 * zx * z0 * pow(dzx[2], 2) * dzx[1] * pow(dzx[4], 4)
        * pow(dzy[2], 2) * dzy[1] * pow(dzy[3], 3) * pow(dzy[4], 4) + 64. *
        pow(s0y, 2) * pow(( zy - z0)  / dzy[1] + pow(( zy - z0)  / dzy[2],
        2) + pow(( zy - z0)  / dzy[3], 3) + pow(( zy - z0)  / dzy[4], 4)
        + 1, 2) * x * x0 * zx * dzx[1] * pow(dzx[3], 3) * pow(dzx[4], 4) *
        pow(dzy[2], 2) * dzy[1] * pow(dzy[3], 3) * pow(dzy[4], 4) + -128. *
        pow(s0y, 2) * pow(( zy - z0)  / dzy[1] + pow(( zy - z0)  / dzy[2], 2)
        + pow(( zy - z0)  / dzy[3], 3) + pow(( zy - z0)  / dzy[4], 4) + 1,
        2) * x * x0 * pow(z0, 3) * pow(dzx[2], 2) * dzx[1] * pow(dzx[3], 3)
        * pow(dzy[2], 2) * dzy[1] * pow(dzy[3], 3) * pow(dzy[4], 4) + 96. *
        pow(s0y, 2) * pow(( zy - z0)  / dzy[1] + pow(( zy - z0)  / dzy[2], 2)
        + pow(( zy - z0)  / dzy[3], 3) + pow(( zy - z0)  / dzy[4], 4) + 1,
        2) * x * x0 * pow(z0, 2) * pow(dzx[2], 2) * dzx[1] * pow(dzx[4], 4)
        * pow(dzy[2], 2) * dzy[1] * pow(dzy[3], 3) * pow(dzy[4], 4) - 64. *
        pow(s0y, 2) * pow(( zy - z0)  / dzy[1] + pow(( zy - z0)  / dzy[2],
        2) + pow(( zy - z0)  / dzy[3], 3) + pow(( zy - z0)  / dzy[4], 4)
        + 1, 2) * x * x0 * z0 * dzx[1] * pow(dzx[3], 3) * pow(dzx[4], 4) *
        pow(dzy[2], 2) * dzy[1] * pow(dzy[3], 3) * pow(dzy[4], 4) + 32. *
        pow(s0y, 2) * pow(( zy - z0)  / dzy[1] + pow(( zy - z0)  / dzy[2],
        2) + pow(( zy - z0)  / dzy[3], 3) + pow(( zy - z0)  / dzy[4], 4)
        + 1, 2) * x * x0 * pow(dzx[2], 2) * pow(dzx[3], 3) * pow(dzx[4],
        4) * pow(dzy[2], 2) * dzy[1] * pow(dzy[3], 3) * pow(dzy[4], 4)
        + -64. * pow(s0y, 2) * pow(( zy - z0)  / dzy[1] + pow(( zy - z0)
        / dzy[2], 2) + pow(( zy - z0)  / dzy[3], 3) + pow(( zy - z0)  /
        dzy[4], 4) + 1, 2) * pow(x0, 2) * pow(zx, 3) * pow(dzx[2], 2) *
        dzx[1] * pow(dzx[3], 3) * pow(dzy[2], 2) * dzy[1] * pow(dzy[3], 3)
        * pow(dzy[4], 4) + 192. * pow(s0y, 2) * pow(( zy - z0)  / dzy[1]
        + pow(( zy - z0)  / dzy[2], 2) + pow(( zy - z0)  / dzy[3], 3) +
        pow(( zy - z0)  / dzy[4], 4) + 1, 2) * pow(x0, 2) * pow(zx, 2) *
        z0 * pow(dzx[2], 2) * dzx[1] * pow(dzx[3], 3) * pow(dzy[2], 2) *
        dzy[1] * pow(dzy[3], 3) * pow(dzy[4], 4) - 48. * pow(s0y, 2) * pow((
        zy - z0)  / dzy[1] + pow(( zy - z0)  / dzy[2], 2) + pow(( zy - z0)
        / dzy[3], 3) + pow(( zy - z0)  / dzy[4], 4) + 1, 2) * pow(x0, 2) *
        pow(zx, 2) * pow(dzx[2], 2) * dzx[1] * pow(dzx[4], 4) * pow(dzy[2],
        2) * dzy[1] * pow(dzy[3], 3) * pow(dzy[4], 4) + -192. * pow(s0y, 2) *
        pow(( zy - z0)  / dzy[1] + pow(( zy - z0)  / dzy[2], 2) + pow(( zy -
        z0)  / dzy[3], 3) + pow(( zy - z0)  / dzy[4], 4) + 1, 2) * pow(x0,
        2) * zx * pow(z0, 2) * pow(dzx[2], 2) * dzx[1] * pow(dzx[3], 3) *
        pow(dzy[2], 2) * dzy[1] * pow(dzy[3], 3) * pow(dzy[4], 4) + 96. *
        pow(s0y, 2) * pow(( zy - z0)  / dzy[1] + pow(( zy - z0)  / dzy[2],
        2) + pow(( zy - z0)  / dzy[3], 3) + pow(( zy - z0)  / dzy[4], 4) + 1,
        2) * pow(x0, 2) * zx * z0 * pow(dzx[2], 2) * dzx[1] * pow(dzx[4], 4)
        * pow(dzy[2], 2) * dzy[1] * pow(dzy[3], 3) * pow(dzy[4], 4) - 32. *
        pow(s0y, 2) * pow(( zy - z0)  / dzy[1] + pow(( zy - z0)  / dzy[2],
        2) + pow(( zy - z0)  / dzy[3], 3) + pow(( zy - z0)  / dzy[4], 4) +
        1, 2) * pow(x0, 2) * zx * dzx[1] * pow(dzx[3], 3) * pow(dzx[4], 4)
        * pow(dzy[2], 2) * dzy[1] * pow(dzy[3], 3) * pow(dzy[4], 4) + 64. *
        pow(s0y, 2) * pow(( zy - z0)  / dzy[1] + pow(( zy - z0)  / dzy[2], 2)
        + pow(( zy - z0)  / dzy[3], 3) + pow(( zy - z0)  / dzy[4], 4) + 1, 2)
        * pow(x0, 2) * pow(z0, 3) * pow(dzx[2], 2) * dzx[1] * pow(dzx[3], 3)
        * pow(dzy[2], 2) * dzy[1] * pow(dzy[3], 3) * pow(dzy[4], 4) - 48. *
        pow(s0y, 2) * pow(( zy - z0)  / dzy[1] + pow(( zy - z0)  / dzy[2],
        2) + pow(( zy - z0)  / dzy[3], 3) + pow(( zy - z0)  / dzy[4], 4) +
        1, 2) * pow(x0, 2) * pow(z0, 2) * pow(dzx[2], 2) * dzx[1] * pow(dzx[4],
        4) * pow(dzy[2], 2) * dzy[1] * pow(dzy[3], 3) * pow(dzy[4], 4) + 32. *
        pow(s0y, 2) * pow(( zy - z0)  / dzy[1] + pow(( zy - z0)  / dzy[2],
        2) + pow(( zy - z0)  / dzy[3], 3) + pow(( zy - z0)  / dzy[4], 4) +
        1, 2) * pow(x0, 2) * z0 * dzx[1] * pow(dzx[3], 3) * pow(dzx[4], 4)
        * pow(dzy[2], 2) * dzy[1] * pow(dzy[3], 3) * pow(dzy[4], 4) - 16. *
        pow(s0y, 2) * pow(( zy - z0)  / dzy[1] + pow(( zy - z0)  / dzy[2],
        2) + pow(( zy - z0)  / dzy[3], 3) + pow(( zy - z0)  / dzy[4], 4) +
        1, 2) * pow(x0, 2) * pow(dzx[2], 2) * pow(dzx[3], 3) * pow(dzx[4],
        4) * pow(dzy[2], 2) * dzy[1] * pow(dzy[3], 3) * pow(dzy[4], 4))  *
        exp( - 0.5 * ( pow(( x - x0)  / ( s0x * sqrt(( zx - z0)  / dzx[1] +
        pow(( zx - z0)  / dzx[2], 2) + pow(( zx - z0)  / dzx[3], 3) + pow((
        zx - z0)  / dzx[4], 4) + 1)) , 2) + pow(( y - y0)  / ( s0y * sqrt((
        zy - z0)  / dzy[1] + pow(( zy - z0)  / dzy[2], 2) + pow(( zy - z0)
        / dzy[3], 3) + pow(( zy - z0)  / dzy[4], 4) + 1)) , 2)) ) * theta *
        A * pixelarea)  / ( 64 * Pi * pow(s0x, 3) * pow(sqrt(( zx - z0)  /
        dzx[1] + pow(( zx - z0)  / dzx[2], 2) + pow(( zx - z0)  / dzx[3], 3)
        + pow(( zx - z0)  / dzx[4], 4) + 1), 5) * pow(s0y, 3) * pow(sqrt((
        zy - z0)  / dzy[1] + pow(( zy - z0)  / dzy[2], 2) + pow(( zy - z0)
        / dzy[3], 3) + pow(( zy - z0)  / dzy[4], 4) + 1), 5) * pow(dzx[2],
        2) * dzx[1] * pow(dzx[3], 3) * pow(dzx[4], 4) * pow(dzy[2], 2) *
        dzy[1] * pow(dzy[3], 3) * pow(dzy[4], 4)) ;
    }

    template <typename Target> void derivative( Target result, XPosition ) {
        result(0,0) = ( ( x0 - x)  * theta * A * pixelarea * exp( - 0.5 *
        ( pow(( x - x0)  / ( s0x * sqrt(( zx - z0)  / dzx[1] + pow(( zx -
        z0)  / dzx[2], 2) + pow(( zx - z0)  / dzx[3], 3) + pow(( zx - z0)
        / dzx[4], 4) + 1)) , 2) + pow(( y - y0)  / ( s0y * sqrt(( zy - z0)
        / dzy[1] + pow(( zy - z0)  / dzy[2], 2) + pow(( zy - z0)  / dzy[3],
        3) + pow(( zy - z0)  / dzy[4], 4) + 1)) , 2)) ))  / ( 2 * pow(s0x,
        3) * pow(sqrt(( zx - z0)  / dzx[1] + pow(( zx - z0)  / dzx[2], 2) +
        pow(( zx - z0)  / dzx[3], 3) + pow(( zx - z0)  / dzx[4], 4) + 1), 3)
        * Pi * s0y * sqrt(( zy - z0)  / dzy[1] + pow(( zy - z0)  / dzy[2], 2)
        + pow(( zy - z0)  / dzy[3], 3) + pow(( zy - z0)  / dzy[4], 4) + 1)) ;
    }

    template <typename Target> void derivative( Target result, Mean<0> ) {
        result(0,0) = ( ( x - x0)  * theta * A * pixelarea * exp( - 0.5 *
        ( pow(( x - x0)  / ( s0x * sqrt(( zx - z0)  / dzx[1] + pow(( zx -
        z0)  / dzx[2], 2) + pow(( zx - z0)  / dzx[3], 3) + pow(( zx - z0)
        / dzx[4], 4) + 1)) , 2) + pow(( y - y0)  / ( s0y * sqrt(( zy - z0)
        / dzy[1] + pow(( zy - z0)  / dzy[2], 2) + pow(( zy - z0)  / dzy[3],
        3) + pow(( zy - z0)  / dzy[4], 4) + 1)) , 2)) ))  / ( 2 * pow(s0x,
        3) * pow(sqrt(( zx - z0)  / dzx[1] + pow(( zx - z0)  / dzx[2], 2) +
        pow(( zx - z0)  / dzx[3], 3) + pow(( zx - z0)  / dzx[4], 4) + 1), 3)
        * Pi * s0y * sqrt(( zy - z0)  / dzy[1] + pow(( zy - z0)  / dzy[2], 2)
        + pow(( zy - z0)  / dzy[3], 3) + pow(( zy - z0)  / dzy[4], 4) + 1)) ;
    }

    template <typename Target> void derivative( Target result, ZPosition<0> ) {
        result(0,0) = ( ( -16 * pow(s0x, 2) * ( ( zx - z0)  / dzx[1] + pow((
        zx - z0)  / dzx[2], 2) + pow(( zx - z0)  / dzx[3], 3) + pow(( zx -
        z0)  / dzx[4], 4) + 1)  * dzx[1] * pow(zx, 3) * pow(dzx[2], 2) *
        pow(dzx[3], 3) + 48 * pow(s0x, 2) * ( ( zx - z0)  / dzx[1] + pow((
        zx - z0)  / dzx[2], 2) + pow(( zx - z0)  / dzx[3], 3) + pow(( zx -
        z0)  / dzx[4], 4) + 1)  * dzx[1] * pow(zx, 2) * z0 * pow(dzx[2], 2) *
        pow(dzx[3], 3) - 12 * pow(s0x, 2) * ( ( zx - z0)  / dzx[1] + pow(( zx -
        z0)  / dzx[2], 2) + pow(( zx - z0)  / dzx[3], 3) + pow(( zx - z0)  /
        dzx[4], 4) + 1)  * dzx[1] * pow(zx, 2) * pow(dzx[2], 2) * pow(dzx[4],
        4) + -48 * pow(s0x, 2) * ( ( zx - z0)  / dzx[1] + pow(( zx - z0)  /
        dzx[2], 2) + pow(( zx - z0)  / dzx[3], 3) + pow(( zx - z0)  / dzx[4],
        4) + 1)  * dzx[1] * zx * pow(z0, 2) * pow(dzx[2], 2) * pow(dzx[3],
        3) + 24 * pow(s0x, 2) * ( ( zx - z0)  / dzx[1] + pow(( zx - z0)  /
        dzx[2], 2) + pow(( zx - z0)  / dzx[3], 3) + pow(( zx - z0)  / dzx[4],
        4) + 1)  * dzx[1] * zx * z0 * pow(dzx[2], 2) * pow(dzx[4], 4) - 8 *
        pow(s0x, 2) * ( ( zx - z0)  / dzx[1] + pow(( zx - z0)  / dzx[2], 2)
        + pow(( zx - z0)  / dzx[3], 3) + pow(( zx - z0)  / dzx[4], 4) + 1)
        * dzx[1] * zx * pow(dzx[3], 3) * pow(dzx[4], 4) + 16 * pow(s0x, 2) *
        ( ( zx - z0)  / dzx[1] + pow(( zx - z0)  / dzx[2], 2) + pow(( zx -
        z0)  / dzx[3], 3) + pow(( zx - z0)  / dzx[4], 4) + 1)  * dzx[1] *
        pow(z0, 3) * pow(dzx[2], 2) * pow(dzx[3], 3) - 12 * pow(s0x, 2) *
        ( ( zx - z0)  / dzx[1] + pow(( zx - z0)  / dzx[2], 2) + pow(( zx -
        z0)  / dzx[3], 3) + pow(( zx - z0)  / dzx[4], 4) + 1)  * dzx[1] *
        pow(z0, 2) * pow(dzx[2], 2) * pow(dzx[4], 4) + 8 * pow(s0x, 2) * (
        ( zx - z0)  / dzx[1] + pow(( zx - z0)  / dzx[2], 2) + pow(( zx - z0)
        / dzx[3], 3) + pow(( zx - z0)  / dzx[4], 4) + 1)  * dzx[1] * z0 *
        pow(dzx[3], 3) * pow(dzx[4], 4) - 4 * pow(s0x, 2) * ( ( zx - z0)  /
        dzx[1] + pow(( zx - z0)  / dzx[2], 2) + pow(( zx - z0)  / dzx[3], 3) +
        pow(( zx - z0)  / dzx[4], 4) + 1)  * pow(dzx[2], 2) * pow(dzx[3], 3) *
        pow(dzx[4], 4) + 16. * pow(x, 2) * dzx[1] * pow(zx, 3) * pow(dzx[2],
        2) * pow(dzx[3], 3) + -48. * pow(x, 2) * dzx[1] * pow(zx, 2) * z0 *
        pow(dzx[2], 2) * pow(dzx[3], 3) + 12. * pow(x, 2) * dzx[1] * pow(zx,
        2) * pow(dzx[2], 2) * pow(dzx[4], 4) + 48. * pow(x, 2) * dzx[1] *
        zx * pow(z0, 2) * pow(dzx[2], 2) * pow(dzx[3], 3) - 24. * pow(x, 2)
        * dzx[1] * zx * z0 * pow(dzx[2], 2) * pow(dzx[4], 4) + 8. * pow(x,
        2) * dzx[1] * zx * pow(dzx[3], 3) * pow(dzx[4], 4) + -16. * pow(x,
        2) * dzx[1] * pow(z0, 3) * pow(dzx[2], 2) * pow(dzx[3], 3) + 12. *
        pow(x, 2) * dzx[1] * pow(z0, 2) * pow(dzx[2], 2) * pow(dzx[4], 4) -
        8. * pow(x, 2) * dzx[1] * z0 * pow(dzx[3], 3) * pow(dzx[4], 4) + 4. *
        pow(x, 2) * pow(dzx[2], 2) * pow(dzx[3], 3) * pow(dzx[4], 4) + -32. *
        x * x0 * dzx[1] * pow(zx, 3) * pow(dzx[2], 2) * pow(dzx[3], 3) + 96. *
        x * x0 * dzx[1] * pow(zx, 2) * z0 * pow(dzx[2], 2) * pow(dzx[3], 3)
        - 24. * x * x0 * dzx[1] * pow(zx, 2) * pow(dzx[2], 2) * pow(dzx[4],
        4) + -96. * x * x0 * dzx[1] * zx * pow(z0, 2) * pow(dzx[2], 2) *
        pow(dzx[3], 3) + 48. * x * x0 * dzx[1] * zx * z0 * pow(dzx[2], 2)
        * pow(dzx[4], 4) - 16. * x * x0 * dzx[1] * zx * pow(dzx[3], 3) *
        pow(dzx[4], 4) + 32. * x * x0 * dzx[1] * pow(z0, 3) * pow(dzx[2], 2)
        * pow(dzx[3], 3) - 24. * x * x0 * dzx[1] * pow(z0, 2) * pow(dzx[2],
        2) * pow(dzx[4], 4) + 16. * x * x0 * dzx[1] * z0 * pow(dzx[3], 3)
        * pow(dzx[4], 4) - 8. * x * x0 * pow(dzx[2], 2) * pow(dzx[3], 3) *
        pow(dzx[4], 4) + 16. * pow(x0, 2) * dzx[1] * pow(zx, 3) * pow(dzx[2],
        2) * pow(dzx[3], 3) + -48. * pow(x0, 2) * dzx[1] * pow(zx, 2) * z0 *
        pow(dzx[2], 2) * pow(dzx[3], 3) + 12. * pow(x0, 2) * dzx[1] * pow(zx,
        2) * pow(dzx[2], 2) * pow(dzx[4], 4) + 48. * pow(x0, 2) * dzx[1] *
        zx * pow(z0, 2) * pow(dzx[2], 2) * pow(dzx[3], 3) - 24. * pow(x0, 2)
        * dzx[1] * zx * z0 * pow(dzx[2], 2) * pow(dzx[4], 4) + 8. * pow(x0,
        2) * dzx[1] * zx * pow(dzx[3], 3) * pow(dzx[4], 4) + -16. * pow(x0,
        2) * dzx[1] * pow(z0, 3) * pow(dzx[2], 2) * pow(dzx[3], 3) + 12. *
        pow(x0, 2) * dzx[1] * pow(z0, 2) * pow(dzx[2], 2) * pow(dzx[4], 4)
        - 8. * pow(x0, 2) * dzx[1] * z0 * pow(dzx[3], 3) * pow(dzx[4], 4) +
        4. * pow(x0, 2) * pow(dzx[2], 2) * pow(dzx[3], 3) * pow(dzx[4], 4))
        * theta * A * pixelarea * exp( - 0.5 * ( pow(( x - x0)  / ( s0x *
        sqrt(( zx - z0)  / dzx[1] + pow(( zx - z0)  / dzx[2], 2) + pow((
        zx - z0)  / dzx[3], 3) + pow(( zx - z0)  / dzx[4], 4) + 1)) , 2)
        + pow(( y - y0)  / ( s0y * sqrt(( zy - z0)  / dzy[1] + pow(( zy -
        z0)  / dzy[2], 2) + pow(( zy - z0)  / dzy[3], 3) + pow(( zy - z0)
        / dzy[4], 4) + 1)) , 2)) ))  / ( 16 * Pi * pow(s0x, 3) * pow(sqrt((
        zx - z0)  / dzx[1] + pow(( zx - z0)  / dzx[2], 2) + pow(( zx - z0)
        / dzx[3], 3) + pow(( zx - z0)  / dzx[4], 4) + 1), 5) * s0y * sqrt((
        zy - z0)  / dzy[1] + pow(( zy - z0)  / dzy[2], 2) + pow(( zy - z0)  /
        dzy[3], 3) + pow(( zy - z0)  / dzy[4], 4) + 1) * dzx[1] * pow(dzx[2],
        2) * pow(dzx[3], 3) * pow(dzx[4], 4)) ;
    }

    template <typename Target> void derivative( Target result, BestSigma<0> ) {
        result(0,0) = ( ( -2 * pow(s0x, 2) * ( ( zx - z0)  / dzx[1] + pow((
        zx - z0)  / dzx[2], 2) + pow(( zx - z0)  / dzx[3], 3) + pow(( zx - z0)
        / dzx[4], 4) + 1)  + 2. * pow(x, 2) - 4. * x * x0 + 2. * pow(x0, 2))
        * theta * A * pixelarea * exp( - 0.5 * ( pow(( x - x0)  / ( s0x *
        sqrt(( zx - z0)  / dzx[1] + pow(( zx - z0)  / dzx[2], 2) + pow((
        zx - z0)  / dzx[3], 3) + pow(( zx - z0)  / dzx[4], 4) + 1)) , 2)
        + pow(( y - y0)  / ( s0y * sqrt(( zy - z0)  / dzy[1] + pow(( zy -
        z0)  / dzy[2], 2) + pow(( zy - z0)  / dzy[3], 3) + pow(( zy - z0)
        / dzy[4], 4) + 1)) , 2)) ))  / ( 4 * Pi * pow(s0x, 4) * pow(sqrt((
        zx - z0)  / dzx[1] + pow(( zx - z0)  / dzx[2], 2) + pow(( zx - z0)
        / dzx[3], 3) + pow(( zx - z0)  / dzx[4], 4) + 1), 3) * s0y * sqrt((
        zy - z0)  / dzy[1] + pow(( zy - z0)  / dzy[2], 2) + pow(( zy - z0)
        / dzy[3], 3) + pow(( zy - z0)  / dzy[4], 4) + 1)) ;
    }

    template <typename Target> void derivative( Target result, DeltaSigma<0,1>
    ) {
        result(0,0) = ( ( -4 * pow(s0x, 2) * ( ( zx - z0)  / dzx[1] + pow((
        zx - z0)  / dzx[2], 2) + pow(( zx - z0)  / dzx[3], 3) + pow(( zx -
        z0)  / dzx[4], 4) + 1)  * z0 + 4 * pow(s0x, 2) * ( ( zx - z0)  /
        dzx[1] + pow(( zx - z0)  / dzx[2], 2) + pow(( zx - z0)  / dzx[3], 3)
        + pow(( zx - z0)  / dzx[4], 4) + 1)  * zx + 4. * pow(x, 2) * z0 -
        4. * pow(x, 2) * zx + -8. * x * x0 * z0 + 8. * x * x0 * zx + 4. *
        pow(x0, 2) * z0 - 4. * pow(x0, 2) * zx)  * theta * A * pixelarea *
        exp( - 0.5 * ( pow(( x - x0)  / ( s0x * sqrt(( zx - z0)  / dzx[1]
        + pow(( zx - z0)  / dzx[2], 2) + pow(( zx - z0)  / dzx[3], 3) +
        pow(( zx - z0)  / dzx[4], 4) + 1)) , 2) + pow(( y - y0)  / ( s0y *
        sqrt(( zy - z0)  / dzy[1] + pow(( zy - z0)  / dzy[2], 2) + pow(( zy -
        z0)  / dzy[3], 3) + pow(( zy - z0)  / dzy[4], 4) + 1)) , 2)) ))  /
        ( 16 * Pi * pow(s0x, 3) * pow(sqrt(( zx - z0)  / dzx[1] + pow(( zx -
        z0)  / dzx[2], 2) + pow(( zx - z0)  / dzx[3], 3) + pow(( zx - z0)  /
        dzx[4], 4) + 1), 5) * s0y * sqrt(( zy - z0)  / dzy[1] + pow(( zy -
        z0)  / dzy[2], 2) + pow(( zy - z0)  / dzy[3], 3) + pow(( zy - z0)
        / dzy[4], 4) + 1) * pow(dzx[1], 2)) ;
    }

    template <typename Target> void derivative( Target result, DeltaSigma<0,2>
    ) {
        result(0,0) = ( ( 8 * pow(s0x, 2) * ( ( zx - z0)  / dzx[1] + pow((
        zx - z0)  / dzx[2], 2) + pow(( zx - z0)  / dzx[3], 3) + pow(( zx - z0)
        / dzx[4], 4) + 1)  * pow(zx, 2) - 16 * pow(s0x, 2) * ( ( zx - z0)  /
        dzx[1] + pow(( zx - z0)  / dzx[2], 2) + pow(( zx - z0)  / dzx[3], 3)
        + pow(( zx - z0)  / dzx[4], 4) + 1)  * zx * z0 + 8 * pow(s0x, 2) *
        ( ( zx - z0)  / dzx[1] + pow(( zx - z0)  / dzx[2], 2) + pow(( zx -
        z0)  / dzx[3], 3) + pow(( zx - z0)  / dzx[4], 4) + 1)  * pow(z0, 2)
        + -8. * pow(x, 2) * pow(zx, 2) + 16. * pow(x, 2) * zx * z0 - 8. *
        pow(x, 2) * pow(z0, 2) + 16. * x * x0 * pow(zx, 2) - 32. * x * x0 *
        zx * z0 + 16. * x * x0 * pow(z0, 2) + -8. * pow(x0, 2) * pow(zx,
        2) + 16. * pow(x0, 2) * zx * z0 - 8. * pow(x0, 2) * pow(z0, 2))
        * theta * A * pixelarea * exp( - 0.5 * ( pow(( x - x0)  / ( s0x *
        sqrt(( zx - z0)  / dzx[1] + pow(( zx - z0)  / dzx[2], 2) + pow((
        zx - z0)  / dzx[3], 3) + pow(( zx - z0)  / dzx[4], 4) + 1)) , 2)
        + pow(( y - y0)  / ( s0y * sqrt(( zy - z0)  / dzy[1] + pow(( zy -
        z0)  / dzy[2], 2) + pow(( zy - z0)  / dzy[3], 3) + pow(( zy - z0)
        / dzy[4], 4) + 1)) , 2)) ))  / ( 16 * Pi * pow(s0x, 3) * pow(sqrt((
        zx - z0)  / dzx[1] + pow(( zx - z0)  / dzx[2], 2) + pow(( zx - z0)
        / dzx[3], 3) + pow(( zx - z0)  / dzx[4], 4) + 1), 5) * s0y * sqrt((
        zy - z0)  / dzy[1] + pow(( zy - z0)  / dzy[2], 2) + pow(( zy - z0)
        / dzy[3], 3) + pow(( zy - z0)  / dzy[4], 4) + 1) * pow(dzx[2], 3)) ;
    }

    template <typename Target> void derivative( Target result, DeltaSigma<0,3>
    ) {
        result(0,0) = ( ( 12 * pow(s0x, 2) * ( ( zx - z0)  / dzx[1] + pow((
        zx - z0)  / dzx[2], 2) + pow(( zx - z0)  / dzx[3], 3) + pow(( zx -
        z0)  / dzx[4], 4) + 1)  * pow(zx, 3) - 36 * pow(s0x, 2) * ( ( zx -
        z0)  / dzx[1] + pow(( zx - z0)  / dzx[2], 2) + pow(( zx - z0)  /
        dzx[3], 3) + pow(( zx - z0)  / dzx[4], 4) + 1)  * pow(zx, 2) * z0 +
        36 * pow(s0x, 2) * ( ( zx - z0)  / dzx[1] + pow(( zx - z0)  / dzx[2],
        2) + pow(( zx - z0)  / dzx[3], 3) + pow(( zx - z0)  / dzx[4], 4) +
        1)  * zx * pow(z0, 2) - 12 * pow(s0x, 2) * ( ( zx - z0)  / dzx[1]
        + pow(( zx - z0)  / dzx[2], 2) + pow(( zx - z0)  / dzx[3], 3) +
        pow(( zx - z0)  / dzx[4], 4) + 1)  * pow(z0, 3) + -12. * pow(x, 2)
        * pow(zx, 3) + 36. * pow(x, 2) * pow(zx, 2) * z0 - 36. * pow(x,
        2) * zx * pow(z0, 2) + 12. * pow(x, 2) * pow(z0, 3) + 24. * x *
        x0 * pow(zx, 3) - 72. * x * x0 * pow(zx, 2) * z0 + 72. * x * x0 *
        zx * pow(z0, 2) - 24. * x * x0 * pow(z0, 3) + -12. * pow(x0, 2) *
        pow(zx, 3) + 36. * pow(x0, 2) * pow(zx, 2) * z0 - 36. * pow(x0, 2)
        * zx * pow(z0, 2) + 12. * pow(x0, 2) * pow(z0, 3))  * theta * A *
        pixelarea * exp( - 0.5 * ( pow(( x - x0)  / ( s0x * sqrt(( zx - z0)
        / dzx[1] + pow(( zx - z0)  / dzx[2], 2) + pow(( zx - z0)  / dzx[3],
        3) + pow(( zx - z0)  / dzx[4], 4) + 1)) , 2) + pow(( y - y0)  /
        ( s0y * sqrt(( zy - z0)  / dzy[1] + pow(( zy - z0)  / dzy[2], 2) +
        pow(( zy - z0)  / dzy[3], 3) + pow(( zy - z0)  / dzy[4], 4) + 1)) ,
        2)) ))  / ( 16 * Pi * pow(s0x, 3) * pow(sqrt(( zx - z0)  / dzx[1] +
        pow(( zx - z0)  / dzx[2], 2) + pow(( zx - z0)  / dzx[3], 3) + pow((
        zx - z0)  / dzx[4], 4) + 1), 5) * s0y * sqrt(( zy - z0)  / dzy[1] +
        pow(( zy - z0)  / dzy[2], 2) + pow(( zy - z0)  / dzy[3], 3) + pow((
        zy - z0)  / dzy[4], 4) + 1) * pow(dzx[3], 4)) ;
    }

    template <typename Target> void derivative( Target result, DeltaSigma<0,4>
    ) {
        result(0,0) = ( ( 16 * pow(s0x, 2) * ( ( zx - z0)  / dzx[1] + pow((
        zx - z0)  / dzx[2], 2) + pow(( zx - z0)  / dzx[3], 3) + pow(( zx -
        z0)  / dzx[4], 4) + 1)  * pow(zx, 4) - 64 * pow(s0x, 2) * ( ( zx -
        z0)  / dzx[1] + pow(( zx - z0)  / dzx[2], 2) + pow(( zx - z0)  /
        dzx[3], 3) + pow(( zx - z0)  / dzx[4], 4) + 1)  * pow(zx, 3) * z0 +
        96 * pow(s0x, 2) * ( ( zx - z0)  / dzx[1] + pow(( zx - z0)  / dzx[2],
        2) + pow(( zx - z0)  / dzx[3], 3) + pow(( zx - z0)  / dzx[4], 4) + 1)
        * pow(zx, 2) * pow(z0, 2) - 64 * pow(s0x, 2) * ( ( zx - z0)  / dzx[1]
        + pow(( zx - z0)  / dzx[2], 2) + pow(( zx - z0)  / dzx[3], 3) + pow((
        zx - z0)  / dzx[4], 4) + 1)  * zx * pow(z0, 3) + 16 * pow(s0x, 2) *
        ( ( zx - z0)  / dzx[1] + pow(( zx - z0)  / dzx[2], 2) + pow(( zx -
        z0)  / dzx[3], 3) + pow(( zx - z0)  / dzx[4], 4) + 1)  * pow(z0, 4) +
        -16. * pow(x, 2) * pow(zx, 4) + 64. * pow(x, 2) * pow(zx, 3) * z0 -
        96. * pow(x, 2) * pow(zx, 2) * pow(z0, 2) + 64. * pow(x, 2) * zx *
        pow(z0, 3) - 16. * pow(x, 2) * pow(z0, 4) + 32. * x * x0 * pow(zx,
        4) - 128. * x * x0 * pow(zx, 3) * z0 + 192. * x * x0 * pow(zx, 2) *
        pow(z0, 2) - 128. * x * x0 * zx * pow(z0, 3) + 32. * x * x0 * pow(z0,
        4) + -16. * pow(x0, 2) * pow(zx, 4) + 64. * pow(x0, 2) * pow(zx, 3)
        * z0 - 96. * pow(x0, 2) * pow(zx, 2) * pow(z0, 2) + 64. * pow(x0,
        2) * zx * pow(z0, 3) - 16. * pow(x0, 2) * pow(z0, 4))  * theta * A *
        pixelarea * exp( - 0.5 * ( pow(( x - x0)  / ( s0x * sqrt(( zx - z0)
        / dzx[1] + pow(( zx - z0)  / dzx[2], 2) + pow(( zx - z0)  / dzx[3],
        3) + pow(( zx - z0)  / dzx[4], 4) + 1)) , 2) + pow(( y - y0)  /
        ( s0y * sqrt(( zy - z0)  / dzy[1] + pow(( zy - z0)  / dzy[2], 2) +
        pow(( zy - z0)  / dzy[3], 3) + pow(( zy - z0)  / dzy[4], 4) + 1)) ,
        2)) ))  / ( 16 * Pi * pow(s0x, 3) * pow(sqrt(( zx - z0)  / dzx[1] +
        pow(( zx - z0)  / dzx[2], 2) + pow(( zx - z0)  / dzx[3], 3) + pow((
        zx - z0)  / dzx[4], 4) + 1), 5) * s0y * sqrt(( zy - z0)  / dzy[1] +
        pow(( zy - z0)  / dzy[2], 2) + pow(( zy - z0)  / dzy[3], 3) + pow((
        zy - z0)  / dzy[4], 4) + 1) * pow(dzx[4], 5)) ;
    }

    template <typename Target> void derivative( Target result, YPosition ) {
        result(0,0) = ( ( y0 - y)  * theta * A * pixelarea * exp( - 0.5 *
        ( pow(( x - x0)  / ( s0x * sqrt(( zx - z0)  / dzx[1] + pow(( zx -
        z0)  / dzx[2], 2) + pow(( zx - z0)  / dzx[3], 3) + pow(( zx - z0)
        / dzx[4], 4) + 1)) , 2) + pow(( y - y0)  / ( s0y * sqrt(( zy - z0)
        / dzy[1] + pow(( zy - z0)  / dzy[2], 2) + pow(( zy - z0)  / dzy[3],
        3) + pow(( zy - z0)  / dzy[4], 4) + 1)) , 2)) ))  / ( 2 * pow(s0y,
        3) * pow(sqrt(( zy - z0)  / dzy[1] + pow(( zy - z0)  / dzy[2], 2) +
        pow(( zy - z0)  / dzy[3], 3) + pow(( zy - z0)  / dzy[4], 4) + 1), 3)
        * Pi * s0x * sqrt(( zx - z0)  / dzx[1] + pow(( zx - z0)  / dzx[2], 2)
        + pow(( zx - z0)  / dzx[3], 3) + pow(( zx - z0)  / dzx[4], 4) + 1)) ;
    }

    template <typename Target> void derivative( Target result, Mean<1> ) {
        result(0,0) = ( ( y - y0)  * theta * A * pixelarea * exp( - 0.5 *
        ( pow(( x - x0)  / ( s0x * sqrt(( zx - z0)  / dzx[1] + pow(( zx -
        z0)  / dzx[2], 2) + pow(( zx - z0)  / dzx[3], 3) + pow(( zx - z0)
        / dzx[4], 4) + 1)) , 2) + pow(( y - y0)  / ( s0y * sqrt(( zy - z0)
        / dzy[1] + pow(( zy - z0)  / dzy[2], 2) + pow(( zy - z0)  / dzy[3],
        3) + pow(( zy - z0)  / dzy[4], 4) + 1)) , 2)) ))  / ( 2 * pow(s0y,
        3) * pow(sqrt(( zy - z0)  / dzy[1] + pow(( zy - z0)  / dzy[2], 2) +
        pow(( zy - z0)  / dzy[3], 3) + pow(( zy - z0)  / dzy[4], 4) + 1), 3)
        * Pi * s0x * sqrt(( zx - z0)  / dzx[1] + pow(( zx - z0)  / dzx[2], 2)
        + pow(( zx - z0)  / dzx[3], 3) + pow(( zx - z0)  / dzx[4], 4) + 1)) ;
    }

    template <typename Target> void derivative( Target result, ZPosition<1> ) {
        result(0,0) = ( ( -16 * pow(s0y, 2) * ( ( zy - z0)  / dzy[1] + pow((
        zy - z0)  / dzy[2], 2) + pow(( zy - z0)  / dzy[3], 3) + pow(( zy -
        z0)  / dzy[4], 4) + 1)  * dzy[1] * pow(zy, 3) * pow(dzy[2], 2) *
        pow(dzy[3], 3) + 48 * pow(s0y, 2) * ( ( zy - z0)  / dzy[1] + pow((
        zy - z0)  / dzy[2], 2) + pow(( zy - z0)  / dzy[3], 3) + pow(( zy -
        z0)  / dzy[4], 4) + 1)  * dzy[1] * pow(zy, 2) * z0 * pow(dzy[2], 2) *
        pow(dzy[3], 3) - 12 * pow(s0y, 2) * ( ( zy - z0)  / dzy[1] + pow(( zy -
        z0)  / dzy[2], 2) + pow(( zy - z0)  / dzy[3], 3) + pow(( zy - z0)  /
        dzy[4], 4) + 1)  * dzy[1] * pow(zy, 2) * pow(dzy[2], 2) * pow(dzy[4],
        4) + -48 * pow(s0y, 2) * ( ( zy - z0)  / dzy[1] + pow(( zy - z0)  /
        dzy[2], 2) + pow(( zy - z0)  / dzy[3], 3) + pow(( zy - z0)  / dzy[4],
        4) + 1)  * dzy[1] * zy * pow(z0, 2) * pow(dzy[2], 2) * pow(dzy[3],
        3) + 24 * pow(s0y, 2) * ( ( zy - z0)  / dzy[1] + pow(( zy - z0)  /
        dzy[2], 2) + pow(( zy - z0)  / dzy[3], 3) + pow(( zy - z0)  / dzy[4],
        4) + 1)  * dzy[1] * zy * z0 * pow(dzy[2], 2) * pow(dzy[4], 4) - 8 *
        pow(s0y, 2) * ( ( zy - z0)  / dzy[1] + pow(( zy - z0)  / dzy[2], 2)
        + pow(( zy - z0)  / dzy[3], 3) + pow(( zy - z0)  / dzy[4], 4) + 1)
        * dzy[1] * zy * pow(dzy[3], 3) * pow(dzy[4], 4) + 16 * pow(s0y, 2) *
        ( ( zy - z0)  / dzy[1] + pow(( zy - z0)  / dzy[2], 2) + pow(( zy -
        z0)  / dzy[3], 3) + pow(( zy - z0)  / dzy[4], 4) + 1)  * dzy[1] *
        pow(z0, 3) * pow(dzy[2], 2) * pow(dzy[3], 3) - 12 * pow(s0y, 2) *
        ( ( zy - z0)  / dzy[1] + pow(( zy - z0)  / dzy[2], 2) + pow(( zy -
        z0)  / dzy[3], 3) + pow(( zy - z0)  / dzy[4], 4) + 1)  * dzy[1] *
        pow(z0, 2) * pow(dzy[2], 2) * pow(dzy[4], 4) + 8 * pow(s0y, 2) * (
        ( zy - z0)  / dzy[1] + pow(( zy - z0)  / dzy[2], 2) + pow(( zy - z0)
        / dzy[3], 3) + pow(( zy - z0)  / dzy[4], 4) + 1)  * dzy[1] * z0 *
        pow(dzy[3], 3) * pow(dzy[4], 4) - 4 * pow(s0y, 2) * ( ( zy - z0)  /
        dzy[1] + pow(( zy - z0)  / dzy[2], 2) + pow(( zy - z0)  / dzy[3], 3) +
        pow(( zy - z0)  / dzy[4], 4) + 1)  * pow(dzy[2], 2) * pow(dzy[3], 3) *
        pow(dzy[4], 4) + 16. * pow(y, 2) * dzy[1] * pow(zy, 3) * pow(dzy[2],
        2) * pow(dzy[3], 3) + -48. * pow(y, 2) * dzy[1] * pow(zy, 2) * z0 *
        pow(dzy[2], 2) * pow(dzy[3], 3) + 12. * pow(y, 2) * dzy[1] * pow(zy,
        2) * pow(dzy[2], 2) * pow(dzy[4], 4) + 48. * pow(y, 2) * dzy[1] *
        zy * pow(z0, 2) * pow(dzy[2], 2) * pow(dzy[3], 3) - 24. * pow(y, 2)
        * dzy[1] * zy * z0 * pow(dzy[2], 2) * pow(dzy[4], 4) + 8. * pow(y,
        2) * dzy[1] * zy * pow(dzy[3], 3) * pow(dzy[4], 4) + -16. * pow(y,
        2) * dzy[1] * pow(z0, 3) * pow(dzy[2], 2) * pow(dzy[3], 3) + 12. *
        pow(y, 2) * dzy[1] * pow(z0, 2) * pow(dzy[2], 2) * pow(dzy[4], 4) -
        8. * pow(y, 2) * dzy[1] * z0 * pow(dzy[3], 3) * pow(dzy[4], 4) + 4. *
        pow(y, 2) * pow(dzy[2], 2) * pow(dzy[3], 3) * pow(dzy[4], 4) + -32. *
        y * y0 * dzy[1] * pow(zy, 3) * pow(dzy[2], 2) * pow(dzy[3], 3) + 96. *
        y * y0 * dzy[1] * pow(zy, 2) * z0 * pow(dzy[2], 2) * pow(dzy[3], 3)
        - 24. * y * y0 * dzy[1] * pow(zy, 2) * pow(dzy[2], 2) * pow(dzy[4],
        4) + -96. * y * y0 * dzy[1] * zy * pow(z0, 2) * pow(dzy[2], 2) *
        pow(dzy[3], 3) + 48. * y * y0 * dzy[1] * zy * z0 * pow(dzy[2], 2)
        * pow(dzy[4], 4) - 16. * y * y0 * dzy[1] * zy * pow(dzy[3], 3) *
        pow(dzy[4], 4) + 32. * y * y0 * dzy[1] * pow(z0, 3) * pow(dzy[2], 2)
        * pow(dzy[3], 3) - 24. * y * y0 * dzy[1] * pow(z0, 2) * pow(dzy[2],
        2) * pow(dzy[4], 4) + 16. * y * y0 * dzy[1] * z0 * pow(dzy[3], 3)
        * pow(dzy[4], 4) - 8. * y * y0 * pow(dzy[2], 2) * pow(dzy[3], 3) *
        pow(dzy[4], 4) + 16. * pow(y0, 2) * dzy[1] * pow(zy, 3) * pow(dzy[2],
        2) * pow(dzy[3], 3) + -48. * pow(y0, 2) * dzy[1] * pow(zy, 2) * z0 *
        pow(dzy[2], 2) * pow(dzy[3], 3) + 12. * pow(y0, 2) * dzy[1] * pow(zy,
        2) * pow(dzy[2], 2) * pow(dzy[4], 4) + 48. * pow(y0, 2) * dzy[1] *
        zy * pow(z0, 2) * pow(dzy[2], 2) * pow(dzy[3], 3) - 24. * pow(y0, 2)
        * dzy[1] * zy * z0 * pow(dzy[2], 2) * pow(dzy[4], 4) + 8. * pow(y0,
        2) * dzy[1] * zy * pow(dzy[3], 3) * pow(dzy[4], 4) + -16. * pow(y0,
        2) * dzy[1] * pow(z0, 3) * pow(dzy[2], 2) * pow(dzy[3], 3) + 12. *
        pow(y0, 2) * dzy[1] * pow(z0, 2) * pow(dzy[2], 2) * pow(dzy[4], 4)
        - 8. * pow(y0, 2) * dzy[1] * z0 * pow(dzy[3], 3) * pow(dzy[4], 4) +
        4. * pow(y0, 2) * pow(dzy[2], 2) * pow(dzy[3], 3) * pow(dzy[4], 4))
        * theta * A * pixelarea * exp( - 0.5 * ( pow(( x - x0)  / ( s0x *
        sqrt(( zx - z0)  / dzx[1] + pow(( zx - z0)  / dzx[2], 2) + pow((
        zx - z0)  / dzx[3], 3) + pow(( zx - z0)  / dzx[4], 4) + 1)) , 2)
        + pow(( y - y0)  / ( s0y * sqrt(( zy - z0)  / dzy[1] + pow(( zy -
        z0)  / dzy[2], 2) + pow(( zy - z0)  / dzy[3], 3) + pow(( zy - z0)
        / dzy[4], 4) + 1)) , 2)) ))  / ( 16 * Pi * s0x * sqrt(( zx - z0)  /
        dzx[1] + pow(( zx - z0)  / dzx[2], 2) + pow(( zx - z0)  / dzx[3],
        3) + pow(( zx - z0)  / dzx[4], 4) + 1) * pow(s0y, 3) * pow(sqrt((
        zy - z0)  / dzy[1] + pow(( zy - z0)  / dzy[2], 2) + pow(( zy - z0)
        / dzy[3], 3) + pow(( zy - z0)  / dzy[4], 4) + 1), 5) * dzy[1] *
        pow(dzy[2], 2) * pow(dzy[3], 3) * pow(dzy[4], 4)) ;
    }

    template <typename Target> void derivative( Target result, BestSigma<1> ) {
        result(0,0) = ( ( -2 * pow(s0y, 2) * ( ( zy - z0)  / dzy[1] + pow((
        zy - z0)  / dzy[2], 2) + pow(( zy - z0)  / dzy[3], 3) + pow(( zy - z0)
        / dzy[4], 4) + 1)  + 2. * pow(y, 2) - 4. * y * y0 + 2. * pow(y0, 2))
        * theta * A * pixelarea * exp( - 0.5 * ( pow(( x - x0)  / ( s0x *
        sqrt(( zx - z0)  / dzx[1] + pow(( zx - z0)  / dzx[2], 2) + pow((
        zx - z0)  / dzx[3], 3) + pow(( zx - z0)  / dzx[4], 4) + 1)) , 2)
        + pow(( y - y0)  / ( s0y * sqrt(( zy - z0)  / dzy[1] + pow(( zy -
        z0)  / dzy[2], 2) + pow(( zy - z0)  / dzy[3], 3) + pow(( zy - z0)
        / dzy[4], 4) + 1)) , 2)) ))  / ( 4 * Pi * s0x * sqrt(( zx - z0)  /
        dzx[1] + pow(( zx - z0)  / dzx[2], 2) + pow(( zx - z0)  / dzx[3],
        3) + pow(( zx - z0)  / dzx[4], 4) + 1) * pow(s0y, 4) * pow(sqrt((
        zy - z0)  / dzy[1] + pow(( zy - z0)  / dzy[2], 2) + pow(( zy - z0)
        / dzy[3], 3) + pow(( zy - z0)  / dzy[4], 4) + 1), 3)) ;
    }

    template <typename Target> void derivative( Target result, DeltaSigma<1,1>
    ) {
        result(0,0) = ( ( -4 * pow(s0y, 2) * ( ( zy - z0)  / dzy[1] + pow((
        zy - z0)  / dzy[2], 2) + pow(( zy - z0)  / dzy[3], 3) + pow(( zy -
        z0)  / dzy[4], 4) + 1)  * z0 + 4 * pow(s0y, 2) * ( ( zy - z0)  /
        dzy[1] + pow(( zy - z0)  / dzy[2], 2) + pow(( zy - z0)  / dzy[3], 3)
        + pow(( zy - z0)  / dzy[4], 4) + 1)  * zy + 4. * pow(y, 2) * z0 -
        4. * pow(y, 2) * zy + -8. * y * y0 * z0 + 8. * y * y0 * zy + 4. *
        pow(y0, 2) * z0 - 4. * pow(y0, 2) * zy)  * theta * A * pixelarea *
        exp( - 0.5 * ( pow(( x - x0)  / ( s0x * sqrt(( zx - z0)  / dzx[1] +
        pow(( zx - z0)  / dzx[2], 2) + pow(( zx - z0)  / dzx[3], 3) + pow((
        zx - z0)  / dzx[4], 4) + 1)) , 2) + pow(( y - y0)  / ( s0y * sqrt((
        zy - z0)  / dzy[1] + pow(( zy - z0)  / dzy[2], 2) + pow(( zy - z0)
        / dzy[3], 3) + pow(( zy - z0)  / dzy[4], 4) + 1)) , 2)) ))  / ( 16
        * Pi * s0x * sqrt(( zx - z0)  / dzx[1] + pow(( zx - z0)  / dzx[2],
        2) + pow(( zx - z0)  / dzx[3], 3) + pow(( zx - z0)  / dzx[4], 4) +
        1) * pow(s0y, 3) * pow(sqrt(( zy - z0)  / dzy[1] + pow(( zy - z0)
        / dzy[2], 2) + pow(( zy - z0)  / dzy[3], 3) + pow(( zy - z0)  /
        dzy[4], 4) + 1), 5) * pow(dzy[1], 2)) ;
    }

    template <typename Target> void derivative( Target result, DeltaSigma<1,2>
    ) {
        result(0,0) = ( ( 8 * pow(s0y, 2) * ( ( zy - z0)  / dzy[1] + pow((
        zy - z0)  / dzy[2], 2) + pow(( zy - z0)  / dzy[3], 3) + pow(( zy - z0)
        / dzy[4], 4) + 1)  * pow(zy, 2) - 16 * pow(s0y, 2) * ( ( zy - z0)  /
        dzy[1] + pow(( zy - z0)  / dzy[2], 2) + pow(( zy - z0)  / dzy[3], 3)
        + pow(( zy - z0)  / dzy[4], 4) + 1)  * zy * z0 + 8 * pow(s0y, 2) * (
        ( zy - z0)  / dzy[1] + pow(( zy - z0)  / dzy[2], 2) + pow(( zy - z0)
        / dzy[3], 3) + pow(( zy - z0)  / dzy[4], 4) + 1)  * pow(z0, 2) + -8. *
        pow(y, 2) * pow(zy, 2) + 16. * pow(y, 2) * zy * z0 - 8. * pow(y, 2)
        * pow(z0, 2) + 16. * y * y0 * pow(zy, 2) - 32. * y * y0 * zy * z0 +
        16. * y * y0 * pow(z0, 2) + -8. * pow(y0, 2) * pow(zy, 2) + 16. *
        pow(y0, 2) * zy * z0 - 8. * pow(y0, 2) * pow(z0, 2))  * theta * A *
        pixelarea * exp( - 0.5 * ( pow(( x - x0)  / ( s0x * sqrt(( zx - z0)
        / dzx[1] + pow(( zx - z0)  / dzx[2], 2) + pow(( zx - z0)  / dzx[3],
        3) + pow(( zx - z0)  / dzx[4], 4) + 1)) , 2) + pow(( y - y0)  /
        ( s0y * sqrt(( zy - z0)  / dzy[1] + pow(( zy - z0)  / dzy[2], 2) +
        pow(( zy - z0)  / dzy[3], 3) + pow(( zy - z0)  / dzy[4], 4) + 1)) ,
        2)) ))  / ( 16 * Pi * s0x * sqrt(( zx - z0)  / dzx[1] + pow(( zx -
        z0)  / dzx[2], 2) + pow(( zx - z0)  / dzx[3], 3) + pow(( zx - z0)
        / dzx[4], 4) + 1) * pow(s0y, 3) * pow(sqrt(( zy - z0)  / dzy[1] +
        pow(( zy - z0)  / dzy[2], 2) + pow(( zy - z0)  / dzy[3], 3) + pow((
        zy - z0)  / dzy[4], 4) + 1), 5) * pow(dzy[2], 3)) ;
    }

    template <typename Target> void derivative( Target result, DeltaSigma<1,3>
    ) {
        result(0,0) = ( ( 12 * pow(s0y, 2) * ( ( zy - z0)  / dzy[1] + pow((
        zy - z0)  / dzy[2], 2) + pow(( zy - z0)  / dzy[3], 3) + pow(( zy -
        z0)  / dzy[4], 4) + 1)  * pow(zy, 3) - 36 * pow(s0y, 2) * ( ( zy -
        z0)  / dzy[1] + pow(( zy - z0)  / dzy[2], 2) + pow(( zy - z0)  /
        dzy[3], 3) + pow(( zy - z0)  / dzy[4], 4) + 1)  * pow(zy, 2) * z0 +
        36 * pow(s0y, 2) * ( ( zy - z0)  / dzy[1] + pow(( zy - z0)  / dzy[2],
        2) + pow(( zy - z0)  / dzy[3], 3) + pow(( zy - z0)  / dzy[4], 4) +
        1)  * zy * pow(z0, 2) - 12 * pow(s0y, 2) * ( ( zy - z0)  / dzy[1]
        + pow(( zy - z0)  / dzy[2], 2) + pow(( zy - z0)  / dzy[3], 3) +
        pow(( zy - z0)  / dzy[4], 4) + 1)  * pow(z0, 3) + -12. * pow(y, 2)
        * pow(zy, 3) + 36. * pow(y, 2) * pow(zy, 2) * z0 - 36. * pow(y,
        2) * zy * pow(z0, 2) + 12. * pow(y, 2) * pow(z0, 3) + 24. * y *
        y0 * pow(zy, 3) - 72. * y * y0 * pow(zy, 2) * z0 + 72. * y * y0 *
        zy * pow(z0, 2) - 24. * y * y0 * pow(z0, 3) + -12. * pow(y0, 2) *
        pow(zy, 3) + 36. * pow(y0, 2) * pow(zy, 2) * z0 - 36. * pow(y0, 2)
        * zy * pow(z0, 2) + 12. * pow(y0, 2) * pow(z0, 3))  * theta * A *
        pixelarea * exp( - 0.5 * ( pow(( x - x0)  / ( s0x * sqrt(( zx - z0)
        / dzx[1] + pow(( zx - z0)  / dzx[2], 2) + pow(( zx - z0)  / dzx[3],
        3) + pow(( zx - z0)  / dzx[4], 4) + 1)) , 2) + pow(( y - y0)  /
        ( s0y * sqrt(( zy - z0)  / dzy[1] + pow(( zy - z0)  / dzy[2], 2) +
        pow(( zy - z0)  / dzy[3], 3) + pow(( zy - z0)  / dzy[4], 4) + 1)) ,
        2)) ))  / ( 16 * Pi * s0x * sqrt(( zx - z0)  / dzx[1] + pow(( zx -
        z0)  / dzx[2], 2) + pow(( zx - z0)  / dzx[3], 3) + pow(( zx - z0)
        / dzx[4], 4) + 1) * pow(s0y, 3) * pow(sqrt(( zy - z0)  / dzy[1] +
        pow(( zy - z0)  / dzy[2], 2) + pow(( zy - z0)  / dzy[3], 3) + pow((
        zy - z0)  / dzy[4], 4) + 1), 5) * pow(dzy[3], 4)) ;
    }

    template <typename Target> void derivative( Target result, DeltaSigma<1,4>
    ) {
        result(0,0) = ( ( 16 * pow(s0y, 2) * ( ( zy - z0)  / dzy[1] + pow((
        zy - z0)  / dzy[2], 2) + pow(( zy - z0)  / dzy[3], 3) + pow(( zy -
        z0)  / dzy[4], 4) + 1)  * pow(zy, 4) - 64 * pow(s0y, 2) * ( ( zy -
        z0)  / dzy[1] + pow(( zy - z0)  / dzy[2], 2) + pow(( zy - z0)  /
        dzy[3], 3) + pow(( zy - z0)  / dzy[4], 4) + 1)  * pow(zy, 3) * z0 +
        96 * pow(s0y, 2) * ( ( zy - z0)  / dzy[1] + pow(( zy - z0)  / dzy[2],
        2) + pow(( zy - z0)  / dzy[3], 3) + pow(( zy - z0)  / dzy[4], 4) + 1)
        * pow(zy, 2) * pow(z0, 2) - 64 * pow(s0y, 2) * ( ( zy - z0)  / dzy[1]
        + pow(( zy - z0)  / dzy[2], 2) + pow(( zy - z0)  / dzy[3], 3) + pow((
        zy - z0)  / dzy[4], 4) + 1)  * zy * pow(z0, 3) + 16 * pow(s0y, 2) *
        ( ( zy - z0)  / dzy[1] + pow(( zy - z0)  / dzy[2], 2) + pow(( zy -
        z0)  / dzy[3], 3) + pow(( zy - z0)  / dzy[4], 4) + 1)  * pow(z0, 4) +
        -16. * pow(y, 2) * pow(zy, 4) + 64. * pow(y, 2) * pow(zy, 3) * z0 -
        96. * pow(y, 2) * pow(zy, 2) * pow(z0, 2) + 64. * pow(y, 2) * zy *
        pow(z0, 3) - 16. * pow(y, 2) * pow(z0, 4) + 32. * y * y0 * pow(zy,
        4) - 128. * y * y0 * pow(zy, 3) * z0 + 192. * y * y0 * pow(zy, 2) *
        pow(z0, 2) - 128. * y * y0 * zy * pow(z0, 3) + 32. * y * y0 * pow(z0,
        4) + -16. * pow(y0, 2) * pow(zy, 4) + 64. * pow(y0, 2) * pow(zy, 3)
        * z0 - 96. * pow(y0, 2) * pow(zy, 2) * pow(z0, 2) + 64. * pow(y0,
        2) * zy * pow(z0, 3) - 16. * pow(y0, 2) * pow(z0, 4))  * theta * A *
        pixelarea * exp( - 0.5 * ( pow(( x - x0)  / ( s0x * sqrt(( zx - z0)
        / dzx[1] + pow(( zx - z0)  / dzx[2], 2) + pow(( zx - z0)  / dzx[3],
        3) + pow(( zx - z0)  / dzx[4], 4) + 1)) , 2) + pow(( y - y0)  /
        ( s0y * sqrt(( zy - z0)  / dzy[1] + pow(( zy - z0)  / dzy[2], 2) +
        pow(( zy - z0)  / dzy[3], 3) + pow(( zy - z0)  / dzy[4], 4) + 1)) ,
        2)) ))  / ( 16 * Pi * s0x * sqrt(( zx - z0)  / dzx[1] + pow(( zx -
        z0)  / dzx[2], 2) + pow(( zx - z0)  / dzx[3], 3) + pow(( zx - z0)
        / dzx[4], 4) + 1) * pow(s0y, 3) * pow(sqrt(( zy - z0)  / dzy[1] +
        pow(( zy - z0)  / dzy[2], 2) + pow(( zy - z0)  / dzy[3], 3) + pow((
        zy - z0)  / dzy[4], 4) + 1), 5) * pow(dzy[4], 5)) ;
    }

