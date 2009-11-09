/** \file Config.h
 *  This file declares the Config class. */
#ifndef DSTORM_INPUT_CONFIG_H
#define DSTORM_INPUT_CONFIG_H

#include <simparm/Set.hh>
#include <simparm/Entry.hh>
#include <simparm/ChoiceEntry.hh>
#include <simparm/FileEntry.hh>
#include <simparm/NumericEntry.hh>
#include <stdexcept>
#include <memory>

#include "Source_decl.h"
#include "BasenameWatcher_decl.h"
#include "Method_decl.h"

/** The dStorm::input namespace provides the functionality to read
 *  sequences of images. The Buffer class provides a simple, vectoresque
 *  view on the input data while storing as little data as possible.
 *  The Config class allows configuration of the input; several methods
 *  are available and selectable there, with a well-defined interface
 *  to add more.
 **/
namespace dStorm {
namespace input {
    class Config;

    class MethodChoice 
    : public simparm::NodeChoiceEntry< BaseMethod > 
    {
      public:
        MethodChoice( std::string name, std::string desc );
        MethodChoice( const MethodChoice& o );

        MethodChoice* clone() const 
            { return new MethodChoice(*this); }

        void copyChoices(const MethodChoice& o, Config& master);
    };

    /** The Config class provides a configuration interface for the
     *  Buffer class. It's \c inputMethod field allows selection of
     *  an input method which is retrievable by the makeImageSource()
     *  method. In collaboration with the Method class this field
     *  can be supplied with choices. */
    class _Config : public simparm::Set {
      public:
        /** The constructor inserts the auto-selection method into the
         *  inputMethod field. */
        _Config ();
        ~_Config ();

        /** The inputMethod choice field selects one of the Method
         *  objects that were produced in the constructor. The input method
         *  selected here will be used by the makeImageSource() 
         *  function. */
        MethodChoice inputMethod;

        /** Since many input methods need a kind of input file, this
         *  field is provided centrally here. Include it in the Method
         *  config node if you need an input file. */
        simparm::FileEntry inputFile;
        /** \var firstImage
         *       Gives the first image in the sequence
         *       (numbered from 0) that should be returned by the
         *       Buffer. Method's that are able to support
         *       this feature should include \c firstImage in their
         *       Set. */
        /*  \var lastImage gives the last image in the sequence
         *       (numbered from 0) that should be returned by the
         *       Buffer. Methods that are able to support
         *       this feature should include \c lastImage in their
         *       Set. */
        simparm::UnsignedLongEntry firstImage, lastImage;
        /** General configuration element to give the size of a pixel
         *  in nm. */
        simparm::DoubleEntry pixel_size_in_nm;

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

        /** Make an Source object according to the current settings. */
        std::auto_ptr< BaseSource > makeImageSource() const;
        /** Make an Source object for the specified method. */
        std::auto_ptr< BaseSource > makeImageSource(int method) const;

    };
}
}
#endif
