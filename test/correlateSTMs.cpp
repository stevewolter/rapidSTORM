#include <dStorm/Fits.h>
#include <fstream>
#include <iostream>
#include <list>
#include <data-c++/Vector.h>
#include "foreach.h"
#include <locprec/relate.h>
#include "../dStorm/Variance.h"
#include <cassert>

using namespace std;
using namespace dStorm;
using namespace data_cpp;

#define SQ(x) ((x)*(x))

int main(int argc, char *argv[]) throw() {
    if (argc < 3) return 1;
    ifstream a(argv[1]), b(argv[2]);
    Fits fa(a), fb(b);
    cerr << fa.numImages() << " " << fb.numImages() << endl;

    int minsize = min(fa.numImages(), fb.numImages());

    Vector< pair<Vector<Localization>, Vector<const Localization*> > >
        data(minsize);

    foreach_const( f, Fits, fa )
        if (f->getImageNumber() < minsize)
            data[f->getImageNumber()].first.push_back(*f);
    foreach_const( f, Fits, fb )
        if (f->getImageNumber() < minsize)
            data[f->getImageNumber()].second.push_back(&(*f));

    Variance distance;
    int missingInSecond = 0, missingInFirst = 0;
    for (int i = 0; i < minsize; i++) {
        Vector<const Localization*> relations(data[i].first.size(), NULL);
        locprec::relate( data[i].first, data[i].second, relations, 1, 1);

        int notInSecond = 0, matches = 0;
        for (int j = 0; j < relations.size(); j++) {
            if (relations[j] == NULL)
                notInSecond++;
            else {
                matches++;
                distance.addValue(sqrt( 
                    SQ(data[i].first[j].x() - relations[j]->x())+
                    SQ(data[i].first[j].y() - relations[j]->y())
                ));
            }
        }
        assert(notInSecond + matches == relations.size());
        missingInSecond += notInSecond;
        missingInFirst += data[i].second.size() - matches;
    }

    cout << distance.N() << " matches with mean distance " << distance.mean() << endl;
    cout << missingInSecond << " unmatched fits in data set 1 and "
         << missingInFirst << " in data set 2." << endl;
}
