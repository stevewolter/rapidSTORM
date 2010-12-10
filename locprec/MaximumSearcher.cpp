#include "MaximumSearcher.h"
#include <dStorm/CandidateTree.h>
#include <fit++/Exponential2D.hh>
#include <limits>
#include <map>
#include <list>
#include <algorithm>
#include "foreach.h"

#define SQ(x) ((x)*(x))

using namespace std;
using namespace dStorm;

namespace locprec {
    template<class TYPE> 
    struct MoleculeLocalizator : public unary_function<void, const TYPE> {
        std::list<MoleculeLocalization>& localizations;

        MoleculeLocalizator(std::list<MoleculeLocalization>& l) throw()
        : localizations(l) {}

#if 0
        void operator()(const TYPE& x) {
            cout << x.first->x()/10.0 << " " << x.first->y()/10.0 << "\n";
            for (list<const dStorm::Localization*>::const_iterator it 
                    = x.second.begin(); it != x.second.end(); it++)
            {
                cout << (*it)->x() << " " << (*it)->y() << "\n";
            }
            cout << "\n\n";
        }
#endif

        void operator()(const TYPE& x) throw() {
            if ( x.second.size() > 30 ) {
                MoleculeLocalization fresh;
                for (list<const dStorm::Localization*>::const_iterator it 
                        = x.second.begin(); it != x.second.end(); it++)
                    fresh.add(**it);
                localizations.push_back(fresh);
            }
        }
    };

    template <typename From, typename To>
    class LocalizationMapper 
        : public unary_function<void, From>
    {
      public:
        typedef map<const To*, list<From*> > Map;
        typedef pair<const To*, list<From*> > Pair;

        LocalizationMapper() throw() {}

        LocalizationMapper(const list<To>& spots) throw()
        {
            typename list<To>::const_iterator spot;
            for (spot = spots.begin();
                 spot != spots.end(); spot++)
            {
                size_t cx = spot->x()/10, cy = spot->y()/10;
                for (int dx = -1; dx <= 1; dx++)
                  for (int dy = -1; dy <= 1; dy++) {
                    if (cache.size() <= cy+dy)
                        cache.resize(cy+dy+10);
                    if (cache[cy+dy].size() <= cx+dx)
                        cache[cy+dy].resize(cx+dx+10);

                    cache[cy+dy][cx+dx].push_back( &*spot );
                  }
            }
        }

        void operator()(const From& loc) throw()
        {
            double minDist = numeric_limits<double>::max();
            const To* minCand = NULL;
            int cx = (int)loc.x(), cy = (int)loc.y();
            if (int(cache.size()) <= cy || int(cache[cy].size()) <= cx) {
                cout << "Too small cache for " << cy << " " << cx << endl;
                return;
            }
            typename list<const To*>::const_iterator spot;
            for ( spot = cache[cy][cx].begin(); 
                  spot != cache[cy][cx].end(); spot++ )
            {
                double dist = SQ((*spot)->x()/10.0 - loc.x()) +
                              SQ((*spot)->y()/10.0 - loc.y());
                if (dist < minDist) {
                    minDist = dist;
                    minCand = *spot;
                }
            }
            if (minCand) {
                mapping.insert(
                    make_pair(minCand, 
                        list<From*>())).first->second.push_back(&loc);
            }
        }

        const map<const To*, list<From*> >& getMapping() const throw()
            { return mapping; }

      private:
        map<const To*, list<From*> > mapping;
        vector<vector<list<const To*> > > cache;
    };

    void MaximumSearcher::finish() throw()
    {
        dStorm::Crankshaft::finish();
        ost::MutexLock lock(mutex);

        dStorm::CandidateTree<float> candidates(3,3,0,0);
        candidates.setLimit(100000);
        const cimg_library::CImg<float>& image = binnedImage();
        candidates.fillMax(image);
        list<Spot> foundSpots;
        for (dStorm::CandidateTree<float>::iterator
             it = candidates.begin(); it.hasMore(); it++)
            foundSpots.push_back(it->second);

        dStorm::Localizations locs = localizations.getResults();
        LocalizationMapper<const Localization, Spot> mapper(foundSpots),
            result;
        result = for_each( locs.begin(), locs.end(), mapper );

        MoleculeLocalizator<
            pair< const Spot*, list<const Localization*> > >
            localizator(molLocs);
        for_each( result.getMapping().begin(), result.getMapping().end(),
                  localizator);
    }
}
