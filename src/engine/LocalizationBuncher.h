#ifndef DSTORM_ENGINE_LOCALIZATIONBUNCHER_H
#define DSTORM_ENGINE_LOCALIZATIONBUNCHER_H

#include <CImgBuffer/Drain.h>
#include <CImgBuffer/Traits.h>
#include <dStorm/Localization.h>
#include <map>
#include <memory>
#include <dStorm/Output.h>
#include <data-c++/Vector.h>

namespace dStorm {
class LocalizationBuncher : public CImgBuffer::Drain<Localization> {
    std::auto_ptr< data_cpp::Vector<Localization> > buffer;
    std::map<int,data_cpp::Vector<Localization>* > canned;
    unsigned int first, last;
    unsigned int currentImage, outputImage;
    Output::EngineResult engine_result;
    Output& target;

    void output( data_cpp::Vector<Localization>* locs ) throw(Output*);
    void print_canned_results_where_possible() throw(Output*);
    void can_results_or_publish( int lookahead ) throw(Output*);

    LocalizationBuncher(const LocalizationBuncher&);
    LocalizationBuncher& operator=(const LocalizationBuncher&);

    int last_index;
    void reset();

  public:
    LocalizationBuncher(Output& output);
    ~LocalizationBuncher();
    
    void ensure_finished() ;
    void noteTraits(const CImgBuffer::Traits<Localization>&,
                    unsigned int firstImage, 
                    unsigned int lastImage) ;

    CImgBuffer::Management
        accept(int index, int number, Localization *l);
};
}

#endif
