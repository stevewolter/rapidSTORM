#include <locprec/NoiseSource.h>
#include <CImgBuffer/Buffer.h>
#include <CImgBuffer/Image.h>
#include <CImgBuffer/Slot.h>

using namespace cimg_library;
using namespace CImgBuffer;

typedef CImgBuffer::Buffer< Image<unsigned short> > BufType;

int main(int argc, char *argv[]) throw() {
    locprec::NoiseMethod<unsigned short>::registerMethod();
    CImgBuffer::Config<Image<unsigned short> > cimgconf;
    cimgconf.registerEntries(cimgconf);
    *((EntryDouble*)cimgconf["FluorophoreNumber"]) = 0;
    *((EntryUnsignedLong*)cimgconf["ImageNumber"]) = 1000;
    cimgconf.readConfig(argc,argv);

    BufType buffer(cimgconf);

    double mean = 0, M2 = 0;
    unsigned long n = 0;

    for (BufType::iterator i = buffer.begin(); i != buffer.end();
                                i++)
    {
        CImgBuffer::Claim< Image<unsigned short> > claim = i->claim();
        Image<unsigned short> &image = *claim;

        unsigned int sz = image.size();
        for (unsigned int j = 0; j < sz; j++) {
            n++;
            int x = image.ptr()[j];
            double delta = x - mean;
            mean += delta / n;
            M2 += delta * (x - mean);
        }
    }

    cout << "Mean " << mean << " and variance " << (M2/(n-1)) << endl;
    return 0;
}
