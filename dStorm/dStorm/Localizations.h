#ifndef DSTORM_LOCALIZATIONS_H
#define DSTORM_LOCALIZATIONS_H

#include <dStorm/Transmission.h>
#include <memory>
#include <stdexcept>
#include <iostream>
#include <data-c++/VectorList.h>

namespace dStorm {
   class Crankshaft;

   class Localizations : public data_cpp::VectorList<Localization> {
      private:
         static void read_header(std::istream &file,
            int &w, int &h, int &n, int &version)
            throw();
         int w, h, n;

      public:
        Localizations(int w = 0, int h = 0, int n = 0) throw();
        Localizations(std::istream &file) throw();
        virtual ~Localizations() throw();

        int dimx() const throw() { return w; }
        int dimy() const throw() { return h; }
        int numImages() const throw() { return n; }
        void setDim(int w, int h, int n) throw() 
            { this->w = w; this->h = h; this->n = n; }

        static void read(std::istream &file, 
                         Transmission &output,
                         unsigned int first = 0,
                         unsigned int last = ((unsigned int)-1),
                         double amp_thres = 0) throw(std::exception);
   };
}

#endif
