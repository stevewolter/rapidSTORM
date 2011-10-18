#ifndef DATA_CPP_TRAITS_H
#define DATA_CPP_TRAITS_H

namespace data_cpp {

template <typename Type> 
struct Traits {
    /** Indicates whether a copy operation can be
     *  performed by copying the type's memory or
     *  if the copy constructor must be called. */
    static const bool NontrivialCopy = true;
    /** Indicates whether the type may be moved
     *  from one memory location to another. If
     *  not, it will be copied via the copy 
     *  constructor to the new location and then
     *  deleted. */
    static const bool MemMoveNecessitatesCopy = true;
    /** Indicates whether the destructor must be
     *  called for deleting objects of this Type. */
    static const bool NontrivialDestructor = true;
    /** Ensure n-byte alignment of this type. Zero
     *  indicates no alignment. */
    static const int Alignment = 0;
};

template <>
struct Traits<unsigned char> {
    static const bool NontrivialCopy = false;
    static const bool MemMoveNecessitatesCopy = false;
    static const bool NontrivialDestructor = false;
    static const int Alignment = 0;
};

template <>
class Traits<char> : public Traits<unsigned char> {};
template <>
class Traits<short> : public Traits<unsigned char> {};
template <>
class Traits<unsigned short> : public Traits<unsigned char> {};
template <>
class Traits<int> : public Traits<unsigned char> {};
template <>
class Traits<unsigned int> : public Traits<unsigned char> {};
template <>
class Traits<long> : public Traits<unsigned char> {};
template <>
class Traits<unsigned long> : public Traits<unsigned char> {};
template <>
class Traits<long long> : public Traits<unsigned char> {};
template <>
class Traits<unsigned long long> : public Traits<unsigned char> {};
template <>
class Traits<float> : public Traits<unsigned char> {};
template <>
class Traits<double> : public Traits<unsigned char> {};
template <>
class Traits<long double> : public Traits<unsigned char> {};

template <typename Base, int ArraySize>
class Traits<Base[ArraySize]> : public Traits<Base> {};

template <typename Base>
class Traits<const Base> : public Traits<Base> {};

}

#endif
