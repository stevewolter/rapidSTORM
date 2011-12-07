#ifndef LOCPREC_RELATE_H
#define LOCPREC_RELATE_H

#include <dStorm/data-c++/Vector.h>
#include <limits>
#include <queue>
#include <boost/units/cmath.hpp>
#include <Eigen/Core>
#include <dStorm/unit_matrix_operators.h>

namespace Eigen {

template <typename Unit, typename Value>
inline Value
ei_abs( const boost::units::quantity<Unit,Value>& v )
{
    return ei_abs(v.value());
}

inline float
ei_abs( const boost::units::quantity<boost::units::camera::length,float>& v )
{
    return ei_abs(v.value());
}

}

namespace locprec {

template <typename ObjectA, typename ObjectB>
void relate(const data_cpp::Vector<ObjectB> &objects,
            const data_cpp::Vector<const ObjectA*>& fluorophores,
            data_cpp::Vector<const ObjectA*>& relations,
            quantity<camera::length,float> max_dist_x,
            quantity<camera::length,float> max_dist_y
)
{
    Eigen::Matrix< quantity<camera::length,float>, 2, 1 > max_dist;
    max_dist.x() = max_dist_x;
    max_dist.y() = max_dist_y;

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
        const ObjectA *best_fluorophore = NULL;

        int dist_index = 0, take_fluorophore = -1;
        for (typename data_cpp::Vector<const ObjectA*>::const_iterator fluo = fluorophores.begin(); fluo != fluorophores.end(); fluo++)
        {
            if ( (unitless_value( (*fluo)->position() - *object).cwise().abs().cwise() <= unitless_value(max_dist)).all() )
            {
                double dx = ((*fluo)->x() - object->x())
                            / camera::pixel;
                double dy = ((*fluo)->y() - object->y())
                            / camera::pixel;
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
