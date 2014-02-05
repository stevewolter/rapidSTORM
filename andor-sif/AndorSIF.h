#include "config.h"

#ifndef CImgBuffer_ANDORSIF_H
#define CImgBuffer_ANDORSIF_H

#include "engine/Input_decl.h"
#include "input/FileInput.h"
#include "input/Source.h"
#include <memory>
#include <string>
#include <stdexcept>
#include <stdio.h>
#include <simparm/FileEntry.h>
#include "helpers/thread.h"
#include "image/Image.h"
#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/signals2/connection.hpp>

#include "andor-sif/AndorSIF_OpenFile.h"

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
    class Source : public input::Source< engine::ImageStack >
    {
      public:
        typedef dStorm::engine::ImageStack Image;
        typedef input::Source<Image> BaseSource;
        typedef typename BaseSource::TraitsPtr TraitsPtr;

        Source(std::auto_ptr<OpenFile> file);
        virtual ~Source();

        TraitsPtr get_traits( typename BaseSource::Wishes );

        void dispatch(typename BaseSource::Messages m) { assert( ! m.any() ); }
        typename BaseSource::Capabilities capabilities() const
            { return typename BaseSource::Capabilities(); }
        void set_thread_count(int num_threads) OVERRIDE {
            assert(num_threads == 1);
        }

        bool GetNext(int thread, engine::ImageStack* output) OVERRIDE;


      private:
        simparm::NodeHandle current_ui;
        void attach_ui_( simparm::NodeHandle n ) { current_ui = n; }
        std::auto_ptr<OpenFile> file;
        bool has_been_iterated;
        int count;
    };

    /** Config class for Source. Simple config that adds
     *  the sif extension to the input file element. */
    class Config 
    : public input::FileInput< Config, OpenFile >
    {
        simparm::Object name_object;
        friend class FileInput< Config, OpenFile >;
        OpenFile* make_file( const std::string& ) const;
        void modify_meta_info( dStorm::input::MetaInfo& );
        void attach_ui( simparm::NodeHandle n ) { name_object.attach_ui(n); }
        static std::string getName() { return "AndorSIF"; }
      public:
        Config();

        Source* makeSource();

        Config* clone() const { return new Config(*this); }
    };
}
}

#endif  /* double inclusion prevention */
