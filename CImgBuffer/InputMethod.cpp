#define CIMGBUFFER_INPUTMETHOD_CPP
#include "InputMethod.h"
#include "AutoInputMethod.h"
#include "STM_InputMethod.h"
#include "AndorSIF.h"
#ifdef HAVE_ATMCD32D_H
#include "AndorDirect.h"
#endif

#include "Image.h"
#include "Pixel.h"

using namespace std;

namespace CImgBuffer {

typedef std::vector<InputMethod< Image<Pixel> >*> IMV;
template <> IMV* InputMethod< Image<Pixel> >::inputMethods = NULL;

template <>
void InputMethod< Image<Pixel> >::initInputMethods() throw () {
    if (inputMethods == NULL) {
        inputMethods = new std::vector<InputMethod< Image<Pixel> >*>();
        registerInputMethod( new AutoInputMethod< Image<Pixel> >() );
        registerInputMethod( new AndorSIF<Pixel>() );
#ifdef HAVE_ATMCD32D_H
        registerInputMethod( new AndorDirect::Method() );
#endif
        registerInputMethod( new STMInputMethod<Image<Pixel> >() );
    }
}

}
