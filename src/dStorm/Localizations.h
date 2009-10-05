#ifndef DSTORM_LOCALIZATIONS_H
#define DSTORM_LOCALIZATIONS_H

#include <memory>
#include <stdexcept>
#include <iostream>
#include <data-c++/VectorList.h>
#include <dStorm/Localization.h>

namespace dStorm {
   /** The Localizations class represents a number of localizations
    *  with common width and height. */
   class Localizations : public data_cpp::VectorList<Localization> {
      private:
         int w, h, n;

      public:
        Localizations(int w = 0, int h = 0, int n = 0);
        Localizations(const Localizations&);
        virtual ~Localizations();

        int dimx() const { return w; }
        int dimy() const { return h; }
        int numImages() const { return n; }
        void setDim(int w, int h, int n) 
            { this->w = w; this->h = h; this->n = n; }
   };
}

#endif
