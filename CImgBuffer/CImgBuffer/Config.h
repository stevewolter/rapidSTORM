/** \file Config.h
 *  This file declares the Config class. */
#ifndef CIMGBUFFER_CONFIG_H
#define CIMGBUFFER_CONFIG_H

#include <ConfigSet.hh>
#include <ConfigEntry.hh>
#include <stdexcept>
#include <memory>

/** The CImgBuffer namespace provides the functionality to read
 *  sequences of images. The Buffer class provides a simple, vectoresque
 *  view on the input data while storing as little data as possible.
 *  The Config class allows configuration of the input; several methods
 *  are available and selectable there, with a well-defined interface
 *  to add more.
 **/
namespace CImgBuffer {
    template <class T> class Source;
    template <class T> class InputConfig;
    template <class T> class Config;

    using namespace simparm;

    template <typename T>
    class InputConfigChoice 
    : public ConfigEntryTypedChoice< InputConfig<T> > 
    {
      public:
        InputConfigChoice(string name, string desc ) throw();
        InputConfigChoice( const InputConfigChoice& o ) throw();

        InputConfigChoice* clone() const 
            { return new InputConfigChoice(this->name(), this->desc()); }

        void initChoices( Config<T>& master ) throw();
        void copyChoices(const InputConfigChoice& o, Config<T>& master) throw();
    };

    /** The Config class provides a configuration interface for the
     *  Buffer class. It's \c inputMethod field allows selection of
     *  an input method which is retrievable by the makeImageSource()
     *  method. In collaboration with the InputConfig class this field
     *  can be supplied with choices. */
    template <class T>
    class _Config : public ConfigSet {
      public:
        /** The constructor fills the inputMethod field with one
         *  InputConfig per InputMethod class. */
        _Config () throw ();
        ~_Config ();

        /** The inputMethod choice field selects one of the InputConfig
         *  objects that were produced in the constructor. The input method
         *  selected here will be used by the makeImageSource() 
         *  function. */
        InputConfigChoice<T> inputMethod;

        /** Since many input methods need a kind of input file, this
         *  field is provided centrally here. Include it in the InputConfig
         *  of an InputMethod if you need an input file. */
        ConfigEntryFile inputFile;
        /** \var firstImage
         *       Gives the first image in the sequence
         *       (numbered from 0) that should be returned by the
         *       Buffer. InputConfig's that are able to support
         *       this feature should include \c firstImage in their
         *       ConfigSet. */
        /*  \var lastImage gives the last image in the sequence
         *       (numbered from 0) that should be returned by the
         *       Buffer. InputConfig's that are able to support
         *       this feature should include \c lastImage in their
         *       ConfigSet. */
        ConfigEntryUnsignedLong firstImage, lastImage;

        /** Make an Source object according to the current settings. */
        std::auto_ptr< Source<T> > makeImageSource() const 
            throw(std::exception);
        /** Make an Source object for the specified method. */
        std::auto_ptr< Source<T> > makeImageSource(int method) const 
            throw(std::exception);

      protected:
        /** Register all named ConfigEntry objects in this Config. **/
        void registerNamedEntries() throw();
    };

    template <class T>
    class Config : public _Config<T> {
      public:
        Config() throw();
        Config(const Config<T>& c) throw();
        ~Config() {}

        void output(std::ostream &) const throw();
    };

    template <class T>
    std::ostream& operator<<(std::ostream& o, 
        const Config<T>& c) throw() { c.output(o); return o; }

}
#endif
