#include "Reconstruction.hh"
#include <CImg.h>
#include <math.h>
#include <string.h>
#include <float.h>
#include <stdio.h>

#include <stdlib.h>
#include <list>
#include <vector>
#include <cassert>
#include <iostream>
#include <iomanip>

#ifndef MINIMUM
	#define MINIMUM(A,B) ((A<B)?(A):(B))
#endif

#ifndef MAXIMUM
	#define MAXIMUM(A,B) ((A>B)?(A):(B))
#endif

//#define MEASURE_TIME
#ifdef MEASURE_TIME
#define TIME_MEASUREMENT(x)  x
#else
#define TIME_MEASUREMENT(x) 
#endif

using namespace std;
using namespace cimg_library;

namespace dStorm {

#ifdef InstanceType
#undef InstanceType
#endif
#define InstanceType RECONSTRUCTION_IMPL_TYPE

template <>
bool firstIsBrighter( 
    const CImg<InstanceType>& a,
    const CImg<InstanceType>& b ) 
throw() 
{
    bool is = true;
    unsigned long sz = a.size();
    for (unsigned long i = 0; i < sz; i++)
        if ( a.data[i] < b.data[i] )
            is = false;
    return is;
}

template <>
CImg<InstanceType> add_border_to_image
    (const CImg<InstanceType>& src, int n) throw()
{
  return src.get_resize( src.width + 2*n, src.height + 2*n, src.depth, src.dim,
                         0 /* Fill new pixels with: */, 0, true /*center*/);
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
template <>
void ReconstructionByDilationII(const CImg<InstanceType>& src, 
    const CImg<InstanceType>& mask,
    CImg<InstanceType>& target)
    throw()
{
	register int i,j,k;
	InstanceType d,q;
	CImg<InstanceType> temp_i, mask_i;	
	TIME_MEASUREMENT( clock_t t1 = clock(); clock_t t2; )
	
        assert( src.size() == mask.size() );
        assert( firstIsBrighter(mask, src) );
	
	temp_i=add_border_to_image(src,1);
	mask_i=add_border_to_image(mask,1);

	// neighbourhood group forward scan
	int  m[4] = {-(temp_i.width+1),-(temp_i.width),-(temp_i.width)+1,-1};
	// neighbourhood group backward scan
	int  n[4] = {temp_i.width+1,(temp_i.width),(temp_i.width)-1,1}; 
	//int  nr[4][2] = {{1,1},{0,1},{-1,1},{1,0}};
	// complete neighbourhood 
	int  nc[8]= {-(temp_i.width+1), -(temp_i.width), -(temp_i.width)+1,
                     -1, 1, (temp_i.width)-1, (temp_i.width), temp_i.width+1};
	//int  nc[8][2] = {{-1,-1},{0,-1},{1,-1},{-1,0},{1,0},{-1,1},{0,1},{1,1}}; 
	    
        // forward reconstruction
	cimg_forXY( src, i, j )
        {    
            unsigned int base = (j+1)*temp_i.width + i+1;
            
            // find global maximum of neighbourhood 
            d= temp_i.data[base];
            for (k=0; k< 4;k++)
            {
                        // rand des bildes 0, daher kein randwertproblem
                        d=MAXIMUM(temp_i.data[base+m[k]],d); 
            }
            
            // compare mask and maximum value of neighbourhood
            // write this value on position i,j
            
            d = MINIMUM(mask_i.data[base],d);
            temp_i.data[base] = d;
        }
			
	list<int> fifo;  
			
	// backward reconstruction
	for(j=src.height-1;j>=0;j--)
            for(i=src.width-1;i>=0;i--)
            {    
                int base = (j+1)*temp_i.width + i+1;
		    
                // find global maximum of neighbourhood 
                d=temp_i.data[base];
                for (k=0; k< 4;k++)
                {
                    // rand des bildes 0, daher kein randwertproblem
                    d=MAXIMUM(temp_i.data[base+n[k]],d);
                }
		    
                // compare mask and maximum value of neighbourhood
                // write this value on position i,j
		    		    
                d = MINIMUM(mask_i.data[base],d);
                temp_i.data[base]=d;

                for (k=0; k< 4;k++)
                {
                    q=temp_i.data[base+n[k]];
                    if((q<d)&&(q<temp_i.data[base+n[k]]))
                        fifo.push_back(base);
                } 	
            } 	
	
	while(!(fifo.empty()))
	{
            int base=fifo.front();
            fifo.pop_front();
	
            d=temp_i.data[base];
            for (k=0; k< 8;k++) 		// complete neighbourhood
            { 	
                    int base_new=base+nc[k];
                    q=temp_i.data[base_new];
                    if((q<d)&&(q!=mask_i.data[base_new]))
                    {
                        temp_i.data[base_new]=
                            MINIMUM(d,mask_i.data[base_new]);  	
                        fifo.push_back(base_new);
                    } 
            }	
	}
	
	
        cimg_forY(target, y)
            memcpy(target.ptr(0,y), temp_i.ptr(1,y+1), 
                   target.width * sizeof(InstanceType));
        CImg<InstanceType> dst = temp_i.get_crop(1,1, temp_i.width-2, temp_i.height-2);
	
#ifdef MEASURE_TIME
	t2=clock();
 	printf("time for reconstruction (v3.1): %fs\n", (float) (t2-t1)/1000000);
#endif
}

}
