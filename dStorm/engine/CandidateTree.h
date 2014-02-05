#ifndef DSTORM_ENGINE_MAXIMUMLIST_H
#define DSTORM_ENGINE_MAXIMUMLIST_H

#include "dStorm/engine/Candidate.h"
#include "dStorm/engine/Image_decl.h"
#include <vector>

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
    {
      private:
        std::vector< Candidate<PixelType> > elements;
        const int msx, msy, bx, by;
        unsigned limit_;
        inline void insert( const Candidate<PixelType>, PixelType& );
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
        : msx(msx), msy(msy), bx(bx), by(by), limit_(10) {}
        virtual ~CandidateTree();
        void setLimit( int limit ) { limit_ = limit; }

        /** Perform inclusive non-maximum suppression on the supplied
         *  image. Fill this tree with the results, use the parameters
         *  supplied in constructor. */
        void fill(const Input& findIn);
        /** Perform exclusive non-maximum suppression on the supplied
         *  image. Fill this tree with the results, use the parameters
         *  supplied in constructor. */
        void fillMax(const Input& findIn);

        typedef typename std::vector< Candidate<PixelType> >::const_iterator const_iterator;
        const_iterator begin() const { return elements.begin(); }
        const_iterator end() const { return elements.end(); }

        bool reached_size_limit() const { return elements.size() == limit_; }
    };
}
}

#endif
