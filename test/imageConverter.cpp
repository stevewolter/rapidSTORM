#include <CImgBuffer/Config.h>
#include <CImgBuffer/Buffer.h>
#include <CImgBuffer/Image.h>
#include <CImgBuffer/Slot.h>
#include <limits>

using namespace CImgBuffer;
using namespace std;
using namespace cimg_library;

class MyConfig : public CImgBuffer::Config< Image<unsigned short> > {};

int main(int argc, char *argv[]) {
    EntryFile outputFile;
    outputFile.setName("OutputFile");
    outputFile.setDesc("File to write output to");
    outputFile.setHelp("The output data type will be guessed by the file extension.");
    EntryLong imageNumber;
    imageNumber.setName("ImageNumber");
    imageNumber.setDesc("Image to extract");
    imageNumber = -1;
    EntryUnsignedLong x, y, m;
    x.setName("X");
    y.setName("Y");
    m.setName("MaskSize");
    x = 0;
    y = 0;
    m = 0;
    EntryUnsignedLong glaetten;
    glaetten.setName("Smooth");
    glaetten = 0;
    MyConfig config;
    
    config.register_entry(&outputFile);
    config.register_entry(&imageNumber);
    config.register_entry(&x);
    config.register_entry(&y);
    config.register_entry(&m);
    config.register_entry(&glaetten);
    config.inputMethod = "SIF";
    config.readConfig(argc, argv);

    Buffer< Image<unsigned short> > buffer( config );
    Buffer< Image<unsigned short> >::iterator i;

    CImgList<unsigned short> imgList;

    double max = 0, min = numeric_limits<unsigned short>::max();
    for ( i = buffer.begin() ; i != buffer.end(); i++ ) {
        Claim< Image<unsigned short> > claim = i->claim();
        if (claim.isGood()) {
            CImgBuffer::Image<unsigned short>& image = *claim;
            CImg<unsigned short> ausschnitt;
            if (glaetten() > 0) {
                auto_ptr<Image<unsigned short> > sm = 
                    image.averageMask( glaetten() );
                for (unsigned int x = glaetten(); x<image.width-glaetten();x++)
                    for (unsigned int y = glaetten(); y<image.height-glaetten();
                            y++)
                    image(x,y) = sm->at(x-glaetten(), y-glaetten());
            }
            if ( m() > 0)
                image.normalize(0, 255);
            if (m() > 0) {
                ausschnitt = image.get_crop(x()-m(), y()-m(), 0, 0, x()+m(), y()+m(), 0, 0);
            } else
                ausschnitt = image;

            if (imageNumber() == -1)
                imgList << ausschnitt;
            else if (imageNumber() == int(image.getImageNumber())) {
                ausschnitt.save( outputFile().c_str() );
                return 0;
            }
        }
        unsigned short pmin, pmax;
        pmin = (*claim).minmax(pmax);
        if (pmin < min) min = pmin;
        if (pmax > max) max = pmax;
    }

    imgList -= min;
    imgList *= 255 / (max-min);

    imgList.save( outputFile().c_str() );

    return 0;
}
