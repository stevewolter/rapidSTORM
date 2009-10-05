/** \file Config.h
 *  This file declares the Config class. */
#ifndef CIMGBUFFER_CONFIG_H
#define CIMGBUFFER_CONFIG_H

#include <simparm/Set.hh>
#include <simparm/Entry.hh>
#include <simparm/ChoiceEntry.hh>
#include <simparm/FileEntry.hh>
#include <simparm/NumericEntry.hh>
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
    class BaseSource;
    class BaseInputConfig;
    class Config;
    class BasenameWatcher;

    class InputConfigChoice 
    : public simparm::NodeChoiceEntry< BaseInputConfig > 
    {
      public:
        InputConfigChoice( std::string name, std::string desc );
        InputConfigChoice( const InputConfigChoice& o );

        InputConfigChoice* clone() const 
            { return new InputConfigChoice(*this); }

        void copyChoices(const InputConfigChoice& o, Config& master)
;
    };

    /** The Config class provides a configuration interface for the
     *  Buffer class. It's \c inputMethod field allows selection of
     *  an input method which is retrievable by the makeImageSource()
     *  method. In collaboration with the InputConfig class this field
     *  can be supplied with choices. */
    class _Config : public simparm::Set {
      public:
        /** The constructor fills the inputMethod field with one
         *  InputConfig per InputMethod class. */
        _Config ();
        ~_Config ();

        /** The inputMethod choice field selects one of the InputConfig
         *  objects that were produced in the constructor. The input method
         *  selected here will be used by the makeImageSource() 
         *  function. */
        InputConfigChoice inputMethod;

        /** Since many input methods need a kind of input file, this
         *  field is provided centrally here. Include it in the InputConfig
         *  of an InputMethod if you need an input file. */
        simparm::FileEntry inputFile;
        /** \var firstImage
         *       Gives the first image in the sequence
         *       (numbered from 0) that should be returned by the
         *       Buffer. InputConfig's that are able to support
         *       this feature should include \c firstImage in their
         *       Set. */
        /*  \var lastImage gives the last image in the sequence
         *       (numbered from 0) that should be returned by the
         *       Buffer. InputConfig's that are able to support
         *       this feature should include \c lastImage in their
         *       Set. */
        simparm::UnsignedLongEntry firstImage, lastImage;

        /** Make an Source object according to the current settings. */
        std::auto_ptr< BaseSource > makeImageSource() const 
;
        /** Make an Source object for the specified method. */
        std::auto_ptr< BaseSource > makeImageSource(int method) const 
;

      protected:
        /** Register all named Entry objects in this Config. **/
        void registerNamedEntries();
    };

    class Config : public _Config {
      private:
        std::auto_ptr<BasenameWatcher> watcher;
      public:
        Config();
        Config(const Config& c);
        ~Config();

        Config* clone() const { return new Config(*this); }

        void output(std::ostream &) const;

        simparm::Attribute<std::string> basename;
    };
}
#endif
