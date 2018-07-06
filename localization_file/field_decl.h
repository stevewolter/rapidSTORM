namespace dStorm {
namespace localization_file {

template <typename Type> struct type_string;
template <> struct type_string<int>;
template <> struct type_string<float>;
template <> struct type_string<double>;

class Field; 
inline Field *new_clone( const Field& i );
template <typename ValueType> class Unknown; 
template <typename Tag> class LocalizationField; 

}
}
