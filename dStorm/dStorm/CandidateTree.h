#ifndef DSTORM_MAXIMUMLIST_H
#define DSTORM_MAXIMUMLIST_H

#include "Candidate.h"
#include <data-c++/MergingTree.h>
#include "CImg_forward.h"

namespace dStorm {
    template <typename PixelType>
    class CandidateTree 
    : public data_cpp::MergingTree< Candidate<PixelType> > 
    {
      private:
        const int msx, msy, bx, by;
      public:
        typedef Candidate<PixelType> Element;

        CandidateTree(const Config &config) throw();
        CandidateTree(int msx, int msy, int bx, int by) throw()
        : msx(msx), msy(msy), bx(bx), by(by) {}
        virtual ~CandidateTree() throw();

        void fill(const cimg_library::CImg<PixelType>& findIn) throw();
        void fillMax(const cimg_library::CImg<PixelType>& findIn) throw();
        virtual int compare(const Candidate<PixelType> &a,
                            const Candidate<PixelType> &b) throw();
        virtual void merge(Candidate<PixelType> &a, 
                           const Candidate<PixelType> &b) throw()
            { a.merge(b); }
    };
}

#endif
