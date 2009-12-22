#include "LocalizationTraits.h"

namespace dStorm {
namespace input {

Traits::Traits( const engine::InputTraits& imageTraits, int imageNumber )
: size( imageTraits.size.start<Localization::Dim>() ),
  resolution( imageTraits.resolution.start<Localization::Dim>() ),
  imageNumber(imageNumber)
{
    
}

