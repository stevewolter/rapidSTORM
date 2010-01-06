namespace dStorm {
namespace LocalizationFile {
namespace field {

template <typename Type> class type_string;
template <> class type_string<int>;
template <> class type_string<float>;
template <> class type_string<double>;

class Interface; 
inline Interface *new_clone( const Interface& i );
template <typename ValueType> class Unknown; 
template <typename Properties> class Known;
template <typename Properties> class KnownWithResolution;

}
}
}
