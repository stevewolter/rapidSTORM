#include "Localizations.h"
#include <cassert>
#include <string.h>
#include <fstream>
#include <foreach.h>

#include "Crankshaft.h"
#include "Image.h"
#include <CImgBuffer/Image.h>
#include <data-c++/Vector.h>
#include <data-c++/VectorList.h>
#include <cc++/thread.h>

using namespace std;

namespace dStorm {

Localizations::Localizations(int w, int h, int n) throw() : w(w), h(h), n(n) {
}

Localizations::Localizations(std::istream &file) throw() : w(0), h(0), n(0) {
   int version;
   read_header(file, w, h, n, version);
   while (file) {
      double x, y;
      int imNum = 0;
      double str = 0;
      if ( (file >> x >> y) ) {
         if (version >= 2) file >> imNum;
         if (version >= 3) file >> str;
         /* If a new header occurs, the engine resetted in mid-output.
          * This is indicated by a new definition line. If the localizations
          * are written to a file, the old localizations are overwritten,
          * but this cannot be done on pipes. */
         if ( x == w && y == h ) 
            clear();
         else
            push_back(Localization(x,y));
         if (version >= 2) back().setImageNumber(imNum);
         if (version >= 3) back().setStrength(str);
      }
   }
}

Localizations::~Localizations() throw() {
    STATUS("Destructed fit list");
}

void Localizations::read_header(
    istream &file,
    int &w, int &h, int &n,
    int &version) 
throw() 
{
   int nw(0), nh(0), nn(0);
   if (file.peek() == '#') {
        char comment;
        file.get(comment);
   }
   version = 1;
   file >> nw;
   if (w == -2) {
      version = 2;
      file >> nw >> nh;
   } else {
      file >> nh;
      if ( file.peek() == ' ') {
        version = 2;
        file >> nn;
        if ( file.peek() == ' ') {
            version = 3;
            double minStrength;
            file >> minStrength;
        }
      }
   }
   w = nw;
   h = nh;
   n = nn;
}

void Localizations::read(std::istream &file, 
                Transmission &crankshaft,
                unsigned int first, unsigned int last,
                double amp_thres) throw(std::exception)
{
    data_cpp::Vector<Localization> buffer;
    int w(0), h(0), n(0), v(0);
    read_header(file, w, h, n, v);
    if (!file) { cerr << "Premature end of localization data file" << endl; return;}
    unsigned int currentImage = 0;
    Transmission::Result result;
    
    Transmission::AdditionalData data = crankshaft.announceStormSize(w,h,n);

    if (data != Transmission::NoData) {
        throw std::runtime_error("A selected data processing function requires data "
            "about the input data that are not present in a localization file.");
    }

    EngineResult engine_result;

    while (file) {
        double x, y;
        unsigned int imNum = 0;
        double str = 0;
        if ( (file >> x >> y) ) {
            if (v >= 2) file >> imNum;
            if (v >= 3) file >> str;
            /* If a new header occurs, the engine resetted in mid-output. */
            if ( x == w && y == h ) {
                buffer.clear();
                crankshaft.deleteAllResults();
            } else {
                while (currentImage != imNum) {
                    if (currentImage >= first && currentImage <= last) {
                        engine_result.forImage = currentImage;
                        engine_result.first = buffer.ptr();
                        engine_result.number = buffer.size();
                        result = 
                            crankshaft.receiveLocalizations(engine_result);
                        if (result == Transmission::StopEngine) {
                            crankshaft.deleteAllResults();
                            return;
                        }
                    }

                    buffer.clear();
                    currentImage++;
                }
                if (amp_thres <= str)
                    buffer.push_back(Localization(x,y,imNum,str));
            }
        }
    }
    crankshaft.finish();
}


std::ostream&
operator<<(std::ostream &o, const Localization& loc) throw()
{
    return
        o << loc.x() << " " << loc.y() << " " << loc.getImageNumber() 
            << " " << loc.getStrength() << "\n";
}

}
