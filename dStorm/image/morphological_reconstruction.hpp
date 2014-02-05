#include "dStorm/image/morphological_reconstruction.h"
#include <math.h>
#include <float.h>

#include <stdlib.h>
#include <queue>
#include <cassert>
#include <iostream>
#include <iomanip>

//#define MEASURE_TIME
#ifdef MEASURE_TIME
#define TIME_MEASUREMENT(x)  x
#else
#define TIME_MEASUREMENT(x) 
#endif

using namespace std;

namespace dStorm {
namespace image {

namespace detail {

template <typename InstanceType, int Dim>
bool firstIsBrighter( 
    const dStorm::Image<InstanceType,Dim>& a,
    const dStorm::Image<InstanceType,Dim>& b ) 
{
    unsigned long sz = a.size_in_pixels();
    for (unsigned long i = 0; i < sz; i++)
        if ( a.ptr()[i] < b.ptr()[i] )
            return false;
    return true;
}

template <typename InstanceType>
void add_border_to_image (dStorm::Image<InstanceType,2>& src) 
{
  for (int x = 0; x < src.width_in_pixels(); x++)
    src(x,0) = src(x,src.height_in_pixels()-1) = 0;
  for (int y = 0; y < src.height_in_pixels(); y++)
    src(0,y) = src(src.width_in_pixels()-1,y) = 0;

}

}

//-----------------------------------------------------------------------------
/* Fast Reconstruction Funktion v3.1

   Marko Tscherepanow 28.10.2004
   Changes for CImg: Steve Wolter, 12/2008
   
   L. Vincent, Morpological Grayscale Reconstruction in Image Analysis:
	 Applications and Efficient Algorithms, IEEE Trans. in Image Proc. 

   Input:  Gray level image and mask image of type IplImage*
   Return: Result of reconstruction by dilation, 
           image of type IplImage* 
    
*/
template <typename InstanceType>
void reconstruction_by_dilation(dStorm::Image<InstanceType,2>& src, 
    dStorm::Image<InstanceType,2>& mask,
    dStorm::Image<InstanceType,2>& target)
{
	register int x,y,k;
	InstanceType d,q;
        int step = src.width_in_pixels(), width = step,
            height = src.height_in_pixels();
	TIME_MEASUREMENT( clock_t t1 = clock(); clock_t t2; )
	
        assert( src.size() == mask.size() );
        assert( detail::firstIsBrighter(mask, src) );
	
	detail::add_border_to_image(src);
	detail::add_border_to_image(mask);
	detail::add_border_to_image(target);

	// neighbourhood group forward scan
	int  m[4] = {-(step+1),-(step),-(step)+1,-1};
	// neighbourhood group backward scan
	int  n[4] = {step+1,(step),(step)-1,1}; 
	//int  nr[4][2] = {{1,1},{0,1},{-1,1},{1,0}};
	// complete neighbourhood 
	int  nc[8]= {-(step+1), -(step), -(step)+1,
                     -1, 1, (step)-1, (step), step+1};
	//int  nc[8][2] = {{-1,-1},{0,-1},{1,-1},{-1,0},{1,0},{-1,1},{0,1},{1,1}}; 
	    
        // forward reconstruction
	for ( y = 1; y < height; y++)
          for ( x = 1; x < (int)width; x++)
        {    
            unsigned int base = y*width + x;
            
            // find global maximum of neighbourhood 
            d= src.ptr()[base];
            for (k=0; k< 4;k++)
            {
                        // rand des bildes 0, daher kein randwertproblem
                        d=std::max(src.ptr()[base+m[k]],d); 
            }
            
            // compare mask and maximum value of neighbourhood
            // write this value on position i,j
            
            d = std::min(mask.ptr()[base],d);
            target.ptr()[base] = d;
        }
			
	std::queue<int> fifo;  
			
	// backward reconstruction
	for(y=height-2;y>=0;y--)
            for(x=width-2;x>=0;x--)
            {    
                int base = y*width + x;
		    
                // find global maximum of neighbourhood 
                d=target.ptr()[base];
                for (k=0; k< 4;k++)
                {
                    // rand des bildes 0, daher kein randwertproblem
                    d=std::max(target.ptr()[base+n[k]],d);
                }
		    
                // compare mask and maximum value of neighbourhood
                // write this value on position i,j
		    		    
                d = std::min(mask.ptr()[base],d);
                target.ptr()[base]=d;

                for (k=0; k< 4;k++)
                {
                    /* Check if the current position might be changed in a forward scan */
                    q=target.ptr()[base+n[k]];
                    if((q<d)&&(q<mask.ptr()[base+n[k]]))
                        fifo.push(base);
                } 	
            } 	
	
	while(!(fifo.empty()))
	{
            int base=fifo.front();
            fifo.pop();
	
            d=target.ptr()[base];
            for (k=0; k< 8;k++) 		// complete neighbourhood
            { 	
                    int base_new=base+nc[k];
                    q=target.ptr()[base_new];
                    if((q<d)&&(q < mask.ptr()[base_new]))
                    {
                        target.ptr()[base_new]=
                            std::min(d,mask.ptr()[base_new]);  	
                        fifo.push(base_new);
                    } 
            }	
	}
	
#ifdef MEASURE_TIME
	t2=clock();
 	printf("time for reconstruction (v3.1): %fs\n", (float) (t2-t1)/1000000);
#endif
}

}
}
