#ifndef LOCPREC_RELATE_H
#define LOCPREC_RELATE_H

#include <data-c++/Vector.h>
#include <limits>
#include <queue>

namespace locprec {

template <typename ObjectA, typename ObjectB>
void relate(const data_cpp::Vector<ObjectB> &objects,
                       const data_cpp::Vector<ObjectA*>& fluorophores,
                       data_cpp::Vector<ObjectA*>& relations,
                       int mdx, int mdy)
{
    data_cpp::Vector<double> 
        distances(fluorophores.size(), std::numeric_limits<double>::max());
    data_cpp::Vector<int> fluorRel(fluorophores.size(), -1);
    std::queue<int> process;
    double delta = 1E-10;

    for (int i = 0; i < objects.size(); i++)
        process.push( i );

    while (process.size() > 0) {
        int object_index = process.front();
        process.pop();
        const ObjectB* object = &objects[object_index];

        double best_distance = std::numeric_limits<double>::max();
        ObjectA *best_fluorophore = NULL;

        int dist_index = 0, take_fluorophore = -1;
        for (typename data_cpp::Vector<ObjectA*>::const_iterator fluo = fluorophores.begin(); fluo != fluorophores.end(); fluo++)
        {
            if ( fabs((*fluo)->x() - object->x()) <= mdx &&
                 fabs((*fluo)->y() - object->y()) <= mdy )
            {
                double dx = ((*fluo)->x() - object->x());
                double dy = ((*fluo)->y() - object->y());
                double distance = dx*dx + dy*dy;

                if (distance+delta < best_distance && 
                    distance+delta < distances[dist_index]) 
                {
                    take_fluorophore = dist_index;
                    best_distance = distance;
                    best_fluorophore = *fluo;
                }
            }
            dist_index++;
        }

        if (take_fluorophore != -1) {
            distances[take_fluorophore] = best_distance;
            int to_oust = fluorRel[take_fluorophore];
            fluorRel[take_fluorophore] = object_index;
            relations[object_index] = best_fluorophore;

            if (to_oust != -1) {
                relations[to_oust] = NULL;
                process.push(to_oust);
            }
        }
    }
}

}

#endif
