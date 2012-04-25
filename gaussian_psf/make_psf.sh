PARAMETERS="A theta z0"
for j in x y; do
    PARAMETERS="$PARAMETERS $j ${j}0 z$j s0$j"
    for i in `seq 4`; do 
        PARAMETERS="$PARAMETERS dz${j}$i"
    done
done

YACAS_FILE=`mktemp`
cat >$YACAS_FILE <<EOF
width(zo,w0,dz1,dz2,dz3,dz4) := w0 * Sqrt( 1 + ( zo / dz1 ) + (zo/dz2)^2 + (zo/dz3)^3 + (zo/dz4)^4 );
widthx := width(zx-z0,s0x,dzx1,dzx2,dzx3,dzx4);
widthy := width(zy-z0,s0y,dzy1,dzy2,dzy3,dzy4);
psf :=
    (theta * A * pixelarea) / ( 2 * Pi * widthx * widthy ) *
    Exp( -0.5 * ( ( ( x - x0 ) / widthx )^2 + ( (y - y0) / widthy)^2 ) );
Echo( "value = ", CForm( psf ) );
EOF

for param in $PARAMETERS; do 
    echo "Echo( \"$param = \", CForm( Simplify( D($param) psf ) ) );"
done >> $YACAS_FILE

yacas -c $YACAS_FILE \
    | sed -e 's/dz\([xy]\)\([0-9]\)/dz\1[\2]/g' \
    | sed -e 's/value = \(.*\)/    void add_value( Eigen::Array<Number,1,1>\& result ) {\n        result(0,0) = \1;\n    }\n/' \
    | sed -e 's/^A = /Amplitude = /' \
    | sed -e 's/^theta = /Prefactor = /' \
    | sed -e 's/^z0 = /MeanZ = /' \
    | sed -e 's/^x0 = /Mean<0> = /' \
    | sed -e 's/^y0 = /Mean<1> = /' \
    | sed -e 's/^x = /XPosition = /' \
    | sed -e 's/^y = /YPosition = /' \
    | sed -e 's/^zx = /ZPosition<0> = /' \
    | sed -e 's/^zy = /ZPosition<1> = /' \
    | sed -e 's/^s0x = /BestSigma<0> = /' \
    | sed -e 's/^s0y = /BestSigma<1> = /' \
    | sed -e 's/^dzx\[\([0-9]\)\] = /DeltaSigma<0,\1> = /' \
    | sed -e 's/^dzy\[\([0-9]\)\] = /DeltaSigma<1,\1> = /' \
    | sed -e 's/^\([][A-Za-z0-9<>,]*\) = \(.*\)/    template <typename Target>\n    void derivative( Target result, \1 ) {\n        result(0,0) = \2;\n    }\n/' \
    | fmt -w 80

