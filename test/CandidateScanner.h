#ifndef CANDIDATESCANNER_H
#define CANDIDATESCANNER_H

#include <dStorm/Image.h>
#include <dStorm/Transmission.h>
#include <dStorm/CandidateTree.h>

template <typename Pix>
class CandidateScanner : public dStorm::EngineView {
  protected:
    typedef dStorm::Candidate<Pix> Candidate;
    virtual void good(const Candidate& max) throw() = 0;
    virtual void bad(const Candidate& max) throw() = 0;
    virtual void notry(const Candidate& max) throw() = 0;
    /** @return true If this image should be processed. */
    virtual bool prepareImage(const dStorm::Image &) throw() { return true; }
    virtual void finishImage() throw() {}
  public:
    CandidateScanner(dStorm::CarConfig &config) : config(config) {}

  private:
    const dStorm::CarConfig &config;

    bool isNear(const dStorm::Localization& fit, const dStorm::Spot& s) throw() {
        bool res = (abs(s.x() - fit.x()) <= config.fitWidth() &&
                abs(s.y() - fit.y()) <= config.fitHeight());
        return res;
    }

    void engineStarted(int) throw() {}
    void engineRestarted(int) throw() {}
    void engineView(const dStorm::Image &im, 
                    const dStorm::SmoothedImage &,
                    const dStorm::CandidateTree<Pix> &max,
                    const dStorm::Localization* fits, int numFits ) throw()
    {
        if (prepareImage(im) != true) return;
        const dStorm::Localization* i = fits;
        int motivation = config.motivation();
        for ( typename dStorm::CandidateTree<Pix>::const_iterator cM
                = max.begin(); cM.hasMore(); cM++)
        {
            if (motivation == 0) {
                notry(*cM);
            } else {
                if ( (i != (fits+numFits)) && isNear( *i, cM->second ) ) {
                    motivation = config.motivation();
                    good(*cM);
                    i++;
                } else {
                    bad(*cM);
                    motivation--;
                }
            }
        }
        finishImage();
    }
};

#endif
