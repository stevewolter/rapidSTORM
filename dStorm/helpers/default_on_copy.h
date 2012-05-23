#ifndef DSTORM_HELPERS_DEFAULT_ON_COPY_H
#define DSTORM_HELPERS_DEFAULT_ON_COPY_H

namespace dStorm {

template <typename Base>
struct default_on_copy : public Base {
    default_on_copy( const default_on_copy& ) {}
    default_on_copy() {}
    template <typename T1> default_on_copy( T1 t ) : Base(t) {}
    template <typename T1> default_on_copy operator=( T1 t ) {
        static_cast< Base& >(*this) = t;
        return *this;
    }
};

}

#endif
