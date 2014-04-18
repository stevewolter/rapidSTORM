namespace dStorm {
namespace localization_file {

template <typename Type> class type_string;
template <> class type_string<int>;
template <> class type_string<float>;
template <> class type_string<double>;

class Field; 
inline Field *new_clone( const Field& i );
template <typename ValueType> class Unknown; 
template <typename Tag> class LocalizationField; 

}
}
