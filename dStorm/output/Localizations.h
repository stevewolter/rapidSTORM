#ifndef DSTORM_LOCALIZATIONS_H
#define DSTORM_LOCALIZATIONS_H

#include "../Localization.h"
#include "../output/LocalizedImage.h"
#include <vector>
#include <list>
#include "../input/LocalizationTraits.h"

namespace dStorm {
namespace output {
   /** The Localizations class represents a number of localizations
    *  with common width and height. */
   class Localizations {
        typedef LocalizedImage OneImage;
        typedef std::list< OneImage > ImageSequence;
        ImageSequence localizations;
        input::Traits<Localization> t;

        struct _iterator;

      public:
        Localizations() {}
        Localizations(const input::Traits<Localization>& t) : t(t) {}

        const input::Traits<Localization>& getTraits() { return t; }
        void setDim(const input::Traits<Localization>& t) 
            { this->t = t; }

        typedef _iterator const_iterator;
        inline const_iterator begin() const;
        inline const_iterator end() const;
        typedef const_iterator iterator;
        inline iterator begin();
        inline iterator end();

        typedef std::list<OneImage>::iterator image_wise_iterator;
        inline image_wise_iterator begin_imagewise() { return localizations.begin(); }
        inline image_wise_iterator end_imagewise() { return localizations.end(); }
        typedef std::list<OneImage>::const_iterator image_wise_const_iterator;
        inline image_wise_const_iterator begin_imagewise() const { return localizations.begin(); }
        inline image_wise_const_iterator end_imagewise() const { return localizations.end(); }

        const LocalizedImage& operator[]( frame_index i ) const;
        LocalizedImage& operator[]( frame_index i ) ;

        image_wise_iterator insert( const LocalizedImage& );
        void clear() { localizations.clear(); }
   };
}
}

#endif
