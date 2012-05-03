#include "config.h"

#ifndef CImgBuffer_ANDORSIF_H
#define CImgBuffer_ANDORSIF_H

#include <dStorm/engine/Input_decl.h>
#include <dStorm/input/FileInput.h>
#include <dStorm/input/Source.h>
#include <memory>
#include <string>
#include <stdexcept>
#include <stdio.h>
#include <simparm/FileEntry.hh>
#include <simparm/TriggerEntry.hh>
#include <dStorm/helpers/thread.h>
#include <dStorm/Image.h>
#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>
#include <simparm/Set.hh>
#include <boost/signals2/connection.hpp>

#include "AndorSIF_OpenFile.h"

#ifndef CImgBuffer_SIFLOADER_CPP
typedef void readsif_File;
typedef void readsif_DataSet;
#endif

namespace dStorm {
class BasenameWatcher;
namespace andor_sif {
    /** The Source provides images from an Andor SIF file, which
     *  is the file format used by the Andor programs.
     *
     *  The Andor SIF source is parameterized by the output pixel
     *  type, which can be one of unsigned short, unsigned int and
     *  float. This is not necessarily the internal type used by
     *  the SIF file. */
    class Source : public simparm::Object,
                   public input::Source< engine::ImageStack >
    {
      public:
        typedef dStorm::engine::ImageStack Image;
        typedef input::Source<Image> BaseSource;
        typedef typename BaseSource::iterator base_iterator;
        typedef typename BaseSource::TraitsPtr TraitsPtr;

        Source(boost::shared_ptr<OpenFile> file);
        virtual ~Source();

        base_iterator begin();
        base_iterator end();
        TraitsPtr get_traits( typename BaseSource::Wishes );

        void dispatch(typename BaseSource::Messages m) { assert( ! m.any() ); }
        typename BaseSource::Capabilities capabilities() const
            { return typename BaseSource::Capabilities(); }

      private:
         simparm::Node& node() { return *this; }
         boost::shared_ptr<OpenFile> file;
         bool has_been_iterated;

         class iterator;
    };

    /** Config class for Source. Simple config that adds
     *  the sif extension to the input file element. */
    class Config 
    : public input::FileInput< Config, OpenFile >,
      public simparm::Object
    {
        friend class FileInput< Config, OpenFile >;
        OpenFile* make_file( const std::string& ) const;
        void modify_meta_info( dStorm::input::MetaInfo& );
      public:
        Config();

        Source* makeSource();
        simparm::Object& getNode() { return *this; }

        Config* clone() const { return new Config(*this); }
    };
}
}

#endif  /* double inclusion prevention */