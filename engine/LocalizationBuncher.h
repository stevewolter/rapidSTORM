#ifndef DSTORM_ENGINE_LOCALIZATIONBUNCHER_H
#define DSTORM_ENGINE_LOCALIZATIONBUNCHER_H

#include <dStorm/input/Drain.h>
#include <dStorm/input/Traits.h>
#include <dStorm/Localization.h>
#include <map>
#include <memory>
#include <dStorm/output/Output.h>
#include <dStorm/data-c++/Vector.h>

namespace dStorm {
namespace engine {

class LocalizationBuncher : public input::Drain<Localization> {
    class Can : public data_cpp::Vector<Localization> {
        int number_of_traces( const Localization& );
        void deep_copy(const Localization& from, 
                            data_cpp::Vector<Localization>& to);
      public:
        void push_back( const Localization& l );
        data_cpp::Vector< output::Trace > traces;
    };

    typedef std::map<frame_index,Can* > Canned;
    std::auto_ptr< Can > buffer;
    Canned canned;
    frame_index first, last, lastInFile;
    frame_index currentImage, outputImage;
    output::Output::EngineResult engine_result;
    output::Output& target;

    void output( Can* locs ) throw(output::Output*);
    void print_canned_results_where_possible() throw(output::Output*);
    void can_results_or_publish( frame_index lookahead )
        throw(output::Output*);

    void put_deep_copy_into_can( const Localization &loc, Can& can );

    LocalizationBuncher(const LocalizationBuncher&);
    LocalizationBuncher& operator=(const LocalizationBuncher&);

    int last_index;
    void reset();

  public:
    LocalizationBuncher(output::Output& output);
    ~LocalizationBuncher();
    
    void ensure_finished() ;
    void noteTraits(const input::Traits<Localization>&,
                    frame_index firstImage, 
                    frame_index lastImage) ;

    input::Management
        accept(int index, int number, Localization *l);
};

}
}

#endif