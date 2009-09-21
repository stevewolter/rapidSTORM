/** \file  read_sif.h
 *  \brief Header file defining functions for reading Andor SIF image files.
 *
 * The public interface consists of a number of structs
 * in this file, which are all READ-ONLY for the user,
 * and some access methods for SIF files at the end of
 * this header.
 *
 * Code by Alan Murray
 * reorganized by Steve Wolter
 * 
 */
#ifndef READSIF_H
#define READSIF_H

#define _FILE_OFFSET_BITS 64
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef READSIF_HEADER
#define READSIF_HEADER extern
#endif

#ifdef __cplusplus
extern "C" {
#endif
   typedef int SIF_BYTE;

   typedef struct _readsif_TShutter {
      char type;
      char mode;
      char custom_bg_mode;
      char custom_mode;
      float closing_time;
      float opening_time;
   } readsif_TShutter;

   typedef struct _readsif_TShamrockSave {
      int IsActive;
      int WavePresent;
      float Wave;
      int GratPresent;
      int GratIndex;
      float GratLines;
      char GratBlaze[32];
      char SpectrographName[32];
      int SlitPresent;
      float SlitWidth;
      int FlipperPresent;
      int FlipperPort;
      int FilterPresent;
      int FilterIndex;
      char FilterString[32];
      int AccessoryPresent;
      int Port1State;
      int Port2State;
      int Port3State;
      int Port4State;
      int OutputSlitPresent;
      float OutputSlitWidth;
      int IsStepAndGlue;
   } readsif_TShamrockSave;

   typedef struct _readsif_TSpectrographSave {
      int IsActive;
      float Wave;
      float GratLines;
      char SpectrographName[32];
   } readsif_TSpectrographSave;

   typedef struct _readsif_TInstaImage {
      long subversion[5];

      SIF_BYTE head;                 // <which head 1 2 etc
      SIF_BYTE store_type;           // <single background reference source etc
      SIF_BYTE data_type;            // <X,XY,XYZ do not know if this should be here
      SIF_BYTE mode;      				//<realtime singlescan etc
      SIF_BYTE trigger_source;
      SIF_BYTE sync;              // <Internal or external
      SIF_BYTE read_pattern;        // <LIS MT or RANDOM
      SIF_BYTE shutter_delay;     // <ON or OFF

      unsigned int type;                 // <int long float
      unsigned int active;               // <does it contain valid data
      unsigned int structure_version;
      int no_integrations;
      int no_points;         // <must be tied to image_format
      int fast_track_height;
      int gain;              // <should this be an index or actual gain
      int track_height;		// <must be tied to TImage. Not valid in LIS or CI
      int series_length;         // <must be tied to TImage class
      int operation_mode;         // <InstaSpec II,IV,V
      int  mt_offset;
      int st_centre_row;
      int FlipX, FlipY, Clock, AClock, Gain, MCP, Prop, IOC;
      float Freq;

      char head_model[270];
      int detector_format_x;
      int detector_format_z;
      time_t timedate;
      char filename[270];	// <MAXPATH for 32 bit is 260
      float temperature;
      float unstabilizedTemperature;
      float trigger_level;
      float exposure_time;
      float delay;
      float integration_cycle_time;
      float kinetic_cycle_time;
      float gate_delay;
      float gate_width;
      float GateStep;
      float pixel_readout_time;
      char* user_text;
      readsif_TShutter shutter;
      readsif_TShamrockSave shamrock_save;
      readsif_TSpectrographSave spec_save;

      int VertClockAmp;
      float data_v_shift_speed;

      float PreAmpGain;
      int OutputAmp, Serial;

      int NumPulses;
      int mFrameTransferAcqMode;
      int mBaselineClamp;
      int mPreScan;
      int mEMRealGain;
      int mBaselineOffset;
      unsigned long mSWVersion;
      int miGateMode;

      unsigned long mSWDllVer;
      unsigned long mSWDllRev;
      unsigned long mSWDllRel;
      unsigned long mSWDllBld;
   } readsif_TInstaImage;

   typedef struct _readsif_TCalibImage {
      SIF_BYTE x_type;
      SIF_BYTE x_unit;
      SIF_BYTE y_type;
      SIF_BYTE y_unit;
      SIF_BYTE z_type;
      SIF_BYTE z_unit;
      float x_cal[4];
      float y_cal[4];
      float z_cal[4];
      char* x_text;
      char* y_text;
      char* z_text;
      float rayleigh_wavelength;
      float pixel_length;
      float pixel_height;
   } readsif_TCalibImage;

   typedef struct _readsif_LONG_RECT {
      int left;
      int top;
      int right;
      int bottom;
   } readsif_LONG_RECT;

   typedef struct _readsif_TSubImage {
      int left;
      int top;
      int right;
      int bottom;
      int vertical_bin;
      int horizontal_bin;
   } readsif_TSubImage;

   /** This struct defines the type of image data in a data set. 
    *  It will be filled regardless of the SIF file version. */
   typedef struct _readsif_TImage {
      /** This vector of structs gives the sizes for the subimages. */
      readsif_TSubImage *position;
      /** This structure gives the size for the base image that contains
       *  the subimages. Warning: This is NOT the size of the image itself.
       *  It is rather the size of the base images that all subimages are
       *  on. */
      readsif_LONG_RECT image_format;
      /** \brief The number of subimages per image
       *
       *  An image might have subimages; I have not seen a SIF file
       *  with more than one.
       *  */
      int no_subimages;
      /** \brief The number of images in the data set and the ones
       *         preceding in this spooling set.
       *
       *  The format of all images is identical and given in \a position .
       *  This parameters gives their number. */
      int no_images;
      /** \brief Vector with offsets of the different subimages */
      unsigned long *subimage_offset;
      unsigned long *time_stamps;
      /** \brief The size of one image in pixels. */
      unsigned long image_length;
      /** \brief The size of all images in this file in pixels. 
       *
       * Try to avoid this variable, since it is prone to overflow. */
      unsigned long total_length;
   } readsif_TImage;

   /** The representation of an open SIF data set. A data set consists of
    *  a series of images, each of which may contain a number of subimages.
    *
    *  All of the variables in this structure are free to be read, but
    *  behaviour is undefined if you manipulate these data apart from the
    *  functions provided in this header.
    *  */
   typedef struct _readsif_DataSet {
      /** This flag is set to non-zero if the data set is present. If
       *  zero, no further fields in the data set are valid. */
      int data_present;

      /** This struct contains MUCH information about the way the images of
       *  this data set got acquired. */
      readsif_TInstaImage instaImage;
      /** This struct contains MUCH information about the way the images of
       *  this data set got acquired. */
      readsif_TCalibImage calibImage;
      /** A structure with information about the image size. */
      readsif_TImage image;

      /** The underlying stream containing the data set. */
      FILE *src;
      /** The ftello() where the data section of this data set starts. */
      off64_t start_of_data;
      /** The pixel the file stream is currently positioned at. */
      unsigned long currentPixel;
   } readsif_DataSet;

   /** The representation of an open SIF file. A file is a container for
    *  multiple data sets, which hold the actual data. */
   typedef struct _readsif_File {
      int version;
      int currentDataSet;

      FILE *src;
   } readsif_File;

   /** \fn readsif_File* readsif_open_File( FILE *src )
    *  \brief Start reading a SIF file from the stream \a src.
    *  \return If successful, a pointer to a new structure describing the
    *          file. If any error occurs, it is printed to stderr and this
    *          function returns NULL. */
   READSIF_HEADER readsif_File* readsif_open_File( FILE *src );
   /** \brief Deallocate a structure returned from open_SIF_File(). 
    *  This function must be called for each non-NULL return value of open_SIF_File().
    *  It will not close the stream that was provided at opening.
    *  */
   READSIF_HEADER void readsif_destroy_File( readsif_File *m );

   /** \brief Start reading a data set from an open SIF file.
    *  
    *  Images in SIF files are organized into up to 5 data sets.
    *  After opening a file or reading all images from the previous
    *  data set, this function may be used to open the next data set.
    *  
    *  \return A pointer to a new structure describing the data set and
    *          the acquisition that was used to generate it. If an error
    *          occurs or there is no next data set, NULL is returned. */
    READSIF_HEADER readsif_DataSet* readsif_read_DataSet( readsif_File *file );
    /** \brief Read past the end of the given data set.
     *  
     *  This method needs to be called after a data set was read if the
     *  caller will read another data set. It does not need to be called
     *  for the last data set read.
     *
     *  \return 0 if the operation succeeded.
     */
    READSIF_HEADER int readsif_seek_past_DataSet( readsif_DataSet *m );
    /** Deallocate a data set constructed by readDataSet(). */
    READSIF_HEADER void readsif_destroy_DataSet( readsif_DataSet *m );
    /** \brief This function determines whether there \e might be more data in the
     *         file.
     *  \return 0 if subsequent calls to readDataSet are guaranteed to fail. 
     **/
    READSIF_HEADER int readsif_mightContainMoreDataSets( readsif_File *file );

    /** \brief Returns the size of a single image in this data set in pixels. */
    READSIF_HEADER unsigned long readsif_imageSize( readsif_DataSet *dataset );
    /** \brief Returns the number of images in a data set. */
    READSIF_HEADER int readsif_numberOfImages( readsif_DataSet *dataset );
    /** \brief Returns the number of subimages each image has. */
    READSIF_HEADER int readsif_numberOfSubimages(readsif_DataSet *dataset);

    /** \brief Returns the width of a subimage in this data set in pixels.
     *  \param sub_im Subimage index between 0 and readsif_numberOfImages()-1 */
    READSIF_HEADER int readsif_imageWidth( readsif_DataSet *dataset, int sub_im );
    /** \brief Returns the height of a subimage in this data set in pixels.
     *  \param sub_im Subimage index between 0 and readsif_numberOfImages()-1 */
    READSIF_HEADER int readsif_imageHeight( readsif_DataSet *dataset, int sub_im );

    /** \brief Reads the next image into the provided buffer
     *  \param image_buffer  Pointer to a buffer which must large enough to
     *                       hold imageSize() floats. 
     *  \return 0 if successful, 1 if there are no images left in the data set. */
    READSIF_HEADER int readsif_getNextImage( readsif_DataSet *dataset,
                                            float *image_buffer );
    /** \brief Seek to a given image in the data set and read it.
     *
     *  Subsequent calls to readNextImage will return the images \a image_no + 1,
     *  \a image_no + 2 etc.
     *
     *  \param image_no  Read this image. Seeks to the correct position, which may
     *                   result in an error on non-seekable streams and will cause
     *                   havoc on files larger than 2 GB.
     *  \return 0 if successful.
     **/
    READSIF_HEADER int readsif_getImage( readsif_DataSet *dataset, int image_no,
                                            float *image_buffer );

    extern char readsif_error[];

#ifdef __cplusplus
}
#endif

#endif
