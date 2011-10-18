#ifndef DSTORM_ENGINE_MAXIMUMLIST_H
#define DSTORM_ENGINE_MAXIMUMLIST_H

#include "Candidate.h"
#include "Image_decl.h"
#include "../data-c++/MergingTree.h"

namespace dStorm {
namespace engine {
    /** The CandidateTree class represents a sorted tree of
     *  candidates. Deriving from the MergingTree, it offers
     *  sorted storage and retrieval of the highest N elements
     *  of a set, with the rest discarded efficiently.
     *
     *  The CandidateTree can fill itself with values from
     *  an image. It will perform non-maximum suppression and
     *  insert the found maximums into itself. */
    template <typename PixelType>
    class CandidateTree 
    : public data_cpp::MergingTree< Candidate<PixelType> > 
    {
      private:
        const int msx, msy, bx, by;
      public:
        typedef Candidate<PixelType> Element;
        typedef dStorm::Image<PixelType,2> Input;

        /** \param msx NMS mask radius in X direction
         *  \param msx NMS mask radius in Y direction
         *  \param bx  Border (not-scanned area at the image borders)
         *             in X direction
         *  \param by  Border (not-scanned area at the image borders)
         *             in Y direction
         **/
        CandidateTree(int msx, int msy, int bx, int by)
        : msx(msx), msy(msy), bx(bx), by(by) {}
        virtual ~CandidateTree();

        /** Perform inclusive non-maximum suppression on the supplied
         *  image. Fill this tree with the results, use the parameters
         *  supplied in constructor. */
        void fill(const Input& findIn);
        /** Perform exclusive non-maximum suppression on the supplied
         *  image. Fill this tree with the results, use the parameters
         *  supplied in constructor. */
        void fillMax(const Input& findIn);

        /** Helper function for MergingTree. Compares \c a and \c b and
         *  returns a value < or = or > 0 if a < or = or > b, 
         *  respectively. */
        virtual int compare(const Candidate<PixelType> &a,
                            const Candidate<PixelType> &b);
        virtual void merge(Candidate<PixelType> &a, 
                           const Candidate<PixelType> &b)
            { a.merge(b); }
    };
}
}

#endif
