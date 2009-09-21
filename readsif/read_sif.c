/** \file read_sif.c

CLASS:			NA
NAME:			Alan Murray, Steve Wolter
PROTECTION: 		NA
RETURN:       	        NA
Version 		1.4
Last Modified		10/01/2008
DESCRIPTION:		This object provides functions to read both
                        the data and the meta information contained
                        in an Andor SIF file.
LICENSE:                Unknown for Alan. Steve considers this codes
                        free to use.
**********************************************************************/
#define _FILE_OFFSET_BITS 64

#include "config.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>

#define MAXSTRING 128
#define MAXUSERTEXT 513

#define READSIF_HEADER 
#include "read_sif.h"

#define HIWORD(l)   ((short) (((long) (l) >> 16) & 0xFFFF))
#define LOWORD(l)   ((short) (l))

#include <setjmp.h>

static const char *char_name(char c) {
    static char char_name_buf[40];
    if (c == '\n')
        sprintf(char_name_buf, "line feed");
    else if (c == '\r')
        sprintf(char_name_buf, "carriage return");
    else if ( isprint(c) )
        sprintf(char_name_buf, "'%c'", c);
    else 
        sprintf(char_name_buf, "with ASCII code %hhi", c);
    return char_name_buf;
}

/** If any of the read* functions encounters an error, they will longjmp()
 *  to this mark. It is set in readDataSet(). */
static jmp_buf error_handling;

/* Helper functions for various data. Refer to the definition for documentation. */
static void read_string(FILE *source, char terminator,char *buffer, int len);
static long read_int(FILE *source, char terminator);
static void read_image(FILE *source, long rd_image_length,float *image_buffer);

static off64_t find_end_of_file(FILE *source, off64_t begin);
static void compensate_missing_data(FILE *source, readsif_DataSet* dataSet);
static int check_for_missing_data(float *image_buffer, long rd_image_length) {
    unsigned char *buffer = (unsigned char*)image_buffer;
    return (rd_image_length >= 1 && buffer[0] != 0 && buffer[3] == 0);
}

/* Functions to read the different parts of a SIF file. Each fills the structure
 * given in the second argument from the stream in the first. */
static void read_instaimage(FILE *, readsif_TInstaImage*);
static void read_calibimage(FILE *, readsif_TCalibImage*);
static void read_image_structure(FILE *, readsif_TImage*);

char readsif_error[4096];

/* \see read_sif.h */
readsif_File* readsif_open_File( FILE *src ) 
{
   char title[MAXSTRING];
   readsif_File* file = (readsif_File*)malloc(sizeof(readsif_File));

   readsif_error[0] = 0;
   read_string(src, '\n', title, MAXSTRING);
   if (strcmp("Andor Technology Multi-Channel File",title) != 0 &&
       strcmp("Oriel Instruments Multi-Channel File",title) != 0 &&
       strcmp("Andor Technology Multi-Channel File\n",title) != 0) 
      { snprintf(readsif_error, 4095,
                "Selected file has wrong title: %s", title);
        free(file);
        return NULL; }

   file->version = read_int(src, ' '); //Version
   file->currentDataSet = 0;
   file->src = src;

   return file;
}

/* \see read_sif.h */
readsif_DataSet* readsif_read_DataSet( readsif_File *file ) {
   readsif_DataSet *dataSet;
   FILE *source = file->src;

   readsif_error[0] = 0;
   if (file->currentDataSet >= 5) return NULL;

   dataSet = (readsif_DataSet*)malloc(sizeof(readsif_DataSet));
   file->currentDataSet++;
   if( !feof(source) && setjmp(error_handling) == 0 ) {
      dataSet->data_present = read_int(source, '\n');
      // Following section only is repeated number_of_images times.
      if(dataSet->data_present==1){
            read_instaimage(source, &dataSet->instaImage);
            read_calibimage(source, &dataSet->calibImage);
            read_image_structure(source, &dataSet->image);
      }
   } else {
      return NULL;
   }

   dataSet->src = source;
   dataSet->start_of_data = ftello64(source);
   dataSet->currentPixel = 0;
   return dataSet;
}

int readsif_seek_past_DataSet( readsif_DataSet *m ) {
    unsigned long long need_to_read =
            m->image.total_length - m->currentPixel;
    if (need_to_read > 0) {
      if ( fseeko64( m->src, need_to_read * 4, SEEK_CUR ) != 0 ) {
          sprintf(readsif_error, "Dataset was read incompletely and "
              "input is not seekable.");
          return 1;
      }
    }
    return 1;
}

/* \see read_sif.h */
int readsif_mightContainMoreDataSets( readsif_File *file ) {
   return file->currentDataSet < 5;
}

/* \see read_sif.h */
unsigned long readsif_imageSize( readsif_DataSet *dataset ) {
   return dataset->image.image_length;
}
/* \see read_sif.h */
int readsif_numberOfSubimages( readsif_DataSet *dataset ) {
    return dataset->image.no_subimages;
}

/* \see read_sif.h */
int readsif_imageWidth( readsif_DataSet *dataset, int sub_im ) {
    const readsif_TSubImage *r = &dataset->image.position[sub_im];
    return r->right - r->left + 1;
}

/* \see read_sif.h */
int readsif_imageHeight( readsif_DataSet *dataset, int sub_im ) {
    const readsif_TSubImage *r = &dataset->image.position[sub_im];
    return r->top - r->bottom + 1;
}

/* \see read_sif.h */
int readsif_numberOfImages( readsif_DataSet *dataset ) {
   return dataset->image.total_length / dataset->image.image_length;
}

/* \see read_sif.h */
int readsif_getNextImage( readsif_DataSet *dataset, float *imageBuffer ) {
    if (dataset->currentPixel >= dataset->image.total_length)
        return 1;
    read_image(dataset->src, dataset->image.image_length, imageBuffer);
    if ( check_for_missing_data( imageBuffer, dataset->image.image_length) ) {
        compensate_missing_data( dataset->src, dataset );
        read_image(dataset->src, dataset->image.image_length, imageBuffer);
    }
        
    dataset->currentPixel+= dataset->image.image_length;
    return 0;
}

/* \see read_sif.h */
int readsif_getImage( readsif_DataSet *dataset, int i, float *imageBuffer ) {
   int rv = 0;

   long long targetPixel = i * dataset->image.image_length;

   if (targetPixel != dataset->currentPixel ) {
    /* Stream not seekable */
    if (dataset->start_of_data == -1) {
        sprintf(readsif_error,
            "ftello returned -1: Stream is not seekable.");
        return 1;
    }
    
    off64_t delta = targetPixel - dataset->currentPixel;
    rv = fseeko64(dataset->src, delta * 4, SEEK_CUR);
    if (rv == -1) {
        sprintf(readsif_error,
                "fseeko returned -1. Image number %u of length %lu",
                i, dataset->image.image_length);
        return 1;
    } else 
        dataset->currentPixel = targetPixel;
   }

   return readsif_getNextImage( dataset, imageBuffer);
}

/* \see read_sif.h */
void readsif_destroy_File( readsif_File *m ) {
   free(m);
}

/* \see read_sif.h */
void readsif_destroy_DataSet( readsif_DataSet *m ) {
    if ( m->data_present ) {
        free(m->instaImage.user_text);
        free(m->calibImage.x_text);
        free(m->calibImage.y_text);
        free(m->calibImage.z_text);
        free(m->image.position);
        free(m->image.subimage_offset);
        free(m->image.time_stamps);
    }
    free(m);
}

/** Read a terminated string of characters into a provided buffer. 
 *  \param      source     Input stream to read from.
 *  \param      terminator Read characters until the terminator is found.
 *  \param[out] buffer     Store the characters in this buffer
 *  \param      len        Store at most these many characters in the buffer,
 *                         including terminating zero
 */
void read_string(FILE *source, char terminator,char *buffer, int len)
{
   int ch;
   int i;

   i=0;
   ch ='a';

   assert(buffer != NULL);

   while (ch != terminator){
      ch = fgetc(source);
      if (i < len-1) {
         buffer[i]= ch;  // Add the charachter to the buffer
         i++;
      }
   }
   buffer[i] = 0; /* Change terminating character to 0. */
}

/** Read a single long integer from the given file, skipping leading blanks.
 *  \param terminator When encountering this character, discard it and stop
 *                    parsing with success. */
long read_int(FILE *source, char terminator) {
   long value = 0;
   int ch;
   int leading = 1;
   int sgn = 1;

   do {
      ch = fgetc(source);
      if (leading && isspace(ch)) {
         /* Skip */;
      } else if (ch == terminator) {
            break;
      } else if (isdigit(ch)) {
         value = value * 10 - '0' + ch;
         leading = 0;
      } else if (ch == '-' && leading) {
         leading = 0;
         sgn = -1;
      } else {
         sprintf(readsif_error,
                 "Error in reading SIF file: Found %s at byte %llu when "
                         "looking for an integer number.", char_name(ch),
                         ftello64(source)-1);
         longjmp(error_handling, 1);
      }
   } while ( 1 );
   return value * sgn;
}

/** Read a single floating point number from the given file, skipping leading
 *  blanks.
 *  \param terminator When encountering this character, discard it and stop
 *                    parsing with success. */
float read_float(FILE *source, char terminator) {
   static const int max_float_buffer = 64;
   float value;
   char buf[max_float_buffer];
   int i = 0;
   int leading = 1;

   while ( 1 ) {
      int ch = fgetc(source);
      if (isspace(ch) && leading) {
         /* Ignore */ ;
      } else if (ch == terminator) {
         break;
      } else if ( (ch < '0' || ch > '9') && ch != 'e' && ch != 'E' && 
                 ch != '+' && ch != '-' && ch != '.') 
      {
         sprintf(readsif_error,
            "error in reading sif file: found %s at byte %llu when "
            "looking for a float number.", char_name(ch),
            ftello64(source)-1);
         longjmp(error_handling, 1);
      } else if ( i >= max_float_buffer-1 ) {
         sprintf(readsif_error,
            "Maximum size of buffer for reading floating point number (%i) "
            "exceeded.", max_float_buffer);
         longjmp(error_handling, 1);
      } else {
         leading = 0;
         buf[i++] = ch;
      }
   }
   buf[i] = 0;

   value = 0;
   if ( sscanf(buf, "%g", &value) <= 0 ) {
        sprintf(readsif_error,
            "Could not parse '%s' as a float number.\n", buf);
        longjmp(error_handling, 1);
   }

   return value;
}



/** Read a single byte from the given file and discard the following byte. */
int read_byte_and_skip_terminator(FILE *source)
{
   int ch;
   ch = fgetc(source);
   fgetc(source);       // Skip terminator
   return ch;
}

/** Read a single byte from the given file and discard the following byte. */
char read_char_and_skip_terminator(FILE *source)

{
   int ch;
   ch = fgetc(source);
   fgetc(source);       // Skip terminator
   return ch;
}


/** Read a sequence of chars from a file and store the result in a buffer.
 *
 * \param string_length    The number of characters to read. Only this many characters
 *                         will be read, no terminator.
 * \param len_chars_buffer A buffer to store the characters in. A terminating 0 byte
 *                         will be appended.
 * \param buffer_size      The buffer size available, including the terminating 0.
 *                         If smaller or equal to len_chars_buffer, remaining chars
 *                         will be read, but not stored.
 * */
void read_len_chars(FILE *source, int string_length,
   char *len_chars_buffer, int buffer_size) 
{
   int rest = 0;
   if (string_length > (buffer_size-1)) {
      rest = string_length - (buffer_size-1);
      string_length = (buffer_size-1);
   }
   
   fread(len_chars_buffer, 1, string_length, source);
   len_chars_buffer[string_length] = 0;
   while (rest-- > 0) fgetc(source);
}

/** \brief Read an image from the source into the buffer provided.
 *  \param rd_image_length   The size of the image in pixels
 */
static void read_image(FILE *source, long rd_image_length,float *image_buffer)
{
    /* An image is stored in a SIF file as a sequence of floating-point numbers
    *  in binary 4-byte IEEE format. Thus, a simple fread is the best way to do
    *  the job. */

    size_t actually;
    actually = fread(image_buffer,4,(size_t)rd_image_length, source);
}

static off64_t find_end_of_file(FILE *source, off64_t begin) {
    int term = 0, fseek_rv;
    size_t i = 0, real_read, buffer_size = 100000;
    char *buffer;
    off64_t rv = -1;

    fseek_rv = fseeko64(source, begin, SEEK_SET);
    if (fseek_rv != 0) {
        fprintf(stderr, "Jumping to file position %lli failed.\n", (long long)begin);
        fflush(stderr);
        return rv;
    } else {
        fprintf(stderr, "Jumped to file position %lli.\n", (long long)begin);
        fflush(stderr);
    }
    
    buffer = (char*)malloc(buffer_size);
    while (term != 8) {
        real_read = fread(buffer, 1, buffer_size, source);
        if (real_read == 0) break;
        for (i = 0; i < real_read; i++) {
            char c = buffer[i];
            switch(c) {
                case '0': 
                    if (term % 2 == 0) term++; else term = 1; break;
                case '\n':
                    if (term % 2 == 1) {
                        term++;
                        if (term == 8) goto success;
                    } else term = 0; break;
                default:
                    term = 0; break;
            }
        }
    }
    fprintf(stderr, "No terminator found.\n");
    fflush(stderr);
    goto end;
   success:
    rv = ftello64(source)-buffer_size+i-term+1;
   end: 
    free(buffer);
    return rv;
}
static void compensate_missing_data(FILE *source, readsif_DataSet* dataSet) {
    off64_t guess_for_last_image, end_of_file = -1;
    off64_t offset, difference, new_length;
    int fac = 1;
    fprintf(stderr, 
        "SIF file seems to be missing data. Trying to compensate.\n");
    fflush(stderr);
    guess_for_last_image = dataSet->start_of_data + dataSet->image.total_length*4;
    while ( end_of_file == -1 ) {
        guess_for_last_image -= dataSet->image.image_length * 4 * fac;
        fac *= 2;
        if ( guess_for_last_image < dataSet->start_of_data )
            guess_for_last_image = dataSet->start_of_data;
        end_of_file = find_end_of_file(source, guess_for_last_image);
        if ( guess_for_last_image == dataSet->start_of_data ) {
            fprintf(stderr, "No terminator found in file. Giving up.\n");
            fflush(stderr);
            return;
        }
    }

    difference = end_of_file - dataSet->start_of_data;
    offset = (difference % (4*dataSet->image.image_length));
    new_length = (difference - offset) / 4;
    fprintf(stderr, "SIF file is missing %lli bytes of data (%i images).\n", 
            (long long)((4*dataSet->image.image_length) - offset),
            (int)((dataSet->image.total_length - new_length) / dataSet->image.image_length));
    fflush(stderr);
    dataSet->start_of_data += offset;
    dataSet->image.total_length = new_length;
    fseeko64(source, dataSet->start_of_data, SEEK_SET);
}

/** Read a TUserText struct, which consists of an integer giving a string length
 *  and a string.
 *
 *  \param buffer A buffer large enough to store MAXUSERTEXT chars. */
static void read_usertext(FILE *source, char *buffer)
{
  int result;
  result=read_int(source, '\n');
  read_len_chars(source, (int)result, buffer, MAXUSERTEXT);
}

void check_version(unsigned long found, unsigned long can, 
                   const char* structure)
{
    if ( found > can ) {
        snprintf(readsif_error, 4095,
            "%s structure has version %li, "
            "but only version %li and earlier is supported.",
            structure, found, can);
        longjmp(error_handling, 1);
    } else if ( found < 0x10000L ) {
        snprintf(readsif_error, 4095,
            "%s structure has version %li, "
            "but expected version 65536 or higher.",
            structure, found);
        longjmp(error_handling, 1);
    }
}

static void read_shutter(FILE *source, readsif_TShutter *shutter, long version[]) {
  if( (HIWORD(version[0]) >= 1) && (LOWORD(version[0]) >=4) ) {
    version[2] = read_int(source, ' ');
    check_version( version[2], 65538L, "Shutter" );
    shutter->type = read_char_and_skip_terminator(source);
    shutter->mode = read_char_and_skip_terminator(source);
    shutter->custom_bg_mode = read_char_and_skip_terminator(source);
    shutter->custom_mode = read_char_and_skip_terminator(source);
    shutter->closing_time = read_float(source, ' ');
    shutter->opening_time = read_float(source, '\n');
  }
}

static void read_shamrockSave(FILE *source, readsif_TShamrockSave *shamrock_save, 
   long version[]) 
{
  if( (HIWORD(version[0])>1) || ((HIWORD(version[0])==1) && (LOWORD(version[0]) >=12)) ){
    int len;
    version[3] = read_int(source, ' ');
    check_version( version[3], 65540L, "Shamrock save" );
    shamrock_save->IsActive = read_int(source, ' ');
    shamrock_save->WavePresent = read_int(source, ' ');
    shamrock_save->Wave = read_float(source, ' ');
    shamrock_save->GratPresent = read_int(source, ' ');
    shamrock_save->GratIndex = read_int(source, ' ');
    shamrock_save->GratLines = read_float(source, ' ');
    read_string(source, '\n',shamrock_save->GratBlaze, MAXSTRING);
    shamrock_save->SlitPresent = read_int(source, ' ');
    if ( version[3] >= (1 << 16) + 4 )
        shamrock_save->SlitWidth = read_float(source, '\n');
    else
        shamrock_save->SlitWidth = read_float(source, ' ');
    shamrock_save->FlipperPresent = read_int(source, ' ');
    shamrock_save->FlipperPort = read_int(source, ' ');
    shamrock_save->FilterPresent = read_int(source, ' ');
    shamrock_save->FilterIndex = read_int(source, ' ');
    len = read_int(source, ' ');
    read_len_chars(source, len, shamrock_save->FilterString, 32);
    shamrock_save->AccessoryPresent = read_int(source, ' ');
    shamrock_save->Port1State = read_int(source, ' ');
    shamrock_save->Port2State = read_int(source, ' ');
    shamrock_save->Port3State = read_int(source, ' ');
    shamrock_save->Port4State = read_int(source, ' ');
    shamrock_save->OutputSlitPresent = read_int(source, ' ');
    if ( version[3] >= (1 << 16) + 4 )
        shamrock_save->OutputSlitWidth = read_float(source, '\n');
    else
        shamrock_save->OutputSlitWidth = read_float(source, ' ');

    if (LOWORD(version[3]) >= 1 || HIWORD(version[3]) > 1) {
      shamrock_save->IsStepAndGlue = read_int(source, ' ');
    }

    if (LOWORD(version[3]) >= 2 || HIWORD(version[3]) > 1) {
      read_string(source, '\n',shamrock_save->SpectrographName, MAXSTRING);
    }

    if (LOWORD(version[3]) >= 4 || HIWORD(version[3]) > 1) {
        read_int(source, ' ');
        read_int(source, '\n');
        read_int(source, ' ');
        read_int(source, '\n');
        read_int(source, ' ');
        read_int(source, ' ');
    }
  }
}

static void read_spectrographSave(FILE *source, readsif_TSpectrographSave *spec_save, 
   long version[]) 
{
  if( (HIWORD(version[0])>1) || ((HIWORD(version[0])==1) && (LOWORD(version[0]) >=22)) ){
    version[4] = read_int(source, ' ');
    check_version( version[4], 65536L, "Spectrograph save" );
    spec_save->IsActive = read_int(source, ' ');
    spec_save->Wave = read_float(source, ' ');
    spec_save->GratLines = read_float(source, '\n');
    spec_save->GratLines = read_float(source, ' ');
    read_string(source, '\n',spec_save->SpectrographName, MAXSTRING);
  }
}

void read_instaimage(FILE *source, readsif_TInstaImage *InstaImage) {
   char instaimage_title[MAXSTRING];
   long version[5];
   int i;

    version[0] = read_int(source, ' '); //Version
    check_version( version[0], 65559L, "InstaImage base" );

   InstaImage->user_text = (char*)malloc(MAXUSERTEXT);

   InstaImage->type = (unsigned int)read_int(source, ' ');
   InstaImage->active = (unsigned int)read_int(source, ' ');
   InstaImage->structure_version = (unsigned int)read_int(source, ' ');
   InstaImage->timedate = read_int(source, ' ');
   InstaImage->temperature = read_float(source, ' ');
   InstaImage->head = read_byte_and_skip_terminator(source);
   InstaImage->store_type = read_byte_and_skip_terminator(source);
   InstaImage->data_type = read_byte_and_skip_terminator(source);
   InstaImage->mode = read_byte_and_skip_terminator(source);
   InstaImage->trigger_source = read_byte_and_skip_terminator(source);
   InstaImage->trigger_level = read_float(source, ' ');
   InstaImage->exposure_time = read_float(source, ' ');
   InstaImage->delay = read_float(source, ' ');
   InstaImage->integration_cycle_time = read_float(source, ' ');
   InstaImage->no_integrations = (int)read_int(source, ' ');
   InstaImage->sync = read_byte_and_skip_terminator(source);
   InstaImage->kinetic_cycle_time = read_float(source, ' ');
   InstaImage->pixel_readout_time = read_float(source, ' ');
   InstaImage->no_points = (int)read_int(source, ' ');
   InstaImage->fast_track_height = (int)read_int(source, ' ');
   InstaImage->gain = (int)read_int(source, ' ');
   InstaImage->gate_delay = read_float(source, ' ');
   InstaImage->gate_width = read_float(source, ' ');

  if( (HIWORD(version[0]) >= 1) && (LOWORD(version[0]) >=6) )
    InstaImage->GateStep = read_float(source, ' ');

  InstaImage->track_height = (int)read_int(source, ' ');
  InstaImage->series_length =(int) read_int(source, ' ');
  InstaImage->read_pattern = read_byte_and_skip_terminator(source);
  InstaImage->shutter_delay = read_byte_and_skip_terminator(source);

  if( (HIWORD(version[0]) >= 1) && (LOWORD(version[0]) >=7) ) {
    InstaImage->st_centre_row = (int)read_int(source, ' ');
    InstaImage->mt_offset = (int)read_int(source, ' ');
    InstaImage->operation_mode = (int)read_int(source, ' ');
  }

  if( (HIWORD(version[0]) >= 1) && (LOWORD(version[0]) >=8) ) {
    InstaImage->FlipX = (int)read_int(source, ' ');
    InstaImage->FlipY = (int)read_int(source, ' ');
    InstaImage->Clock = (int)read_int(source, ' ');
    InstaImage->AClock = (int)read_int(source, ' ');
    InstaImage->MCP = (int)read_int(source, ' ');
    InstaImage->Prop = (int)read_int(source, ' ');
    InstaImage->IOC = (int)read_int(source, ' ');
    InstaImage->Freq = (int)read_int(source, ' ');
  }

  if((HIWORD(version[0]) >= 1) && (LOWORD(version[0]) >=9)) {
    InstaImage->VertClockAmp = (int)read_int(source, ' ');
    InstaImage->data_v_shift_speed = read_float(source, ' ');
  }

  if((HIWORD(version[0]) >= 1) && (LOWORD(version[0]) >=10)) {
    InstaImage->OutputAmp = (int)read_int(source, ' ');
    InstaImage->PreAmpGain = read_float(source, ' ');
  }

  if((HIWORD(version[0]) >= 1) && (LOWORD(version[0]) >=11)) {
    InstaImage->Serial = (int)read_int(source, ' ');
  }

  if((HIWORD(version[0]) >= 1) && (LOWORD(version[0]) >=13)) {
    InstaImage->NumPulses = (int)read_int(source, ' ');
  }

  if((HIWORD(version[0]) >= 1) && (LOWORD(version[0]) >=14)) {
    InstaImage->mFrameTransferAcqMode = (int)read_int(source, ' ');
  }

  if((HIWORD(version[0]) >= 1) && (LOWORD(version[0]) >=15)) {
    InstaImage->unstabilizedTemperature = (int)read_float(source, ' ');
    InstaImage->mBaselineClamp = (int)read_int(source, ' ');
  }

  if((HIWORD(version[0]) >= 1) && (LOWORD(version[0]) >=16))
    InstaImage->mPreScan = (int)read_int(source, ' ');

  if((HIWORD(version[0]) >= 1) && (LOWORD(version[0]) >=17))
    InstaImage->mEMRealGain = (int)read_int(source, ' ');

  if((HIWORD(version[0]) >= 1) && (LOWORD(version[0]) >=18))
    InstaImage->mBaselineOffset = (int)read_int(source, ' ');

  if((HIWORD(version[0]) >= 1) && (LOWORD(version[0]) >=19))
    InstaImage->mSWVersion = (unsigned long)read_int(source, ' ');

  if((HIWORD(version[0]) >= 1) && (LOWORD(version[0]) >=20))
    InstaImage->miGateMode = (int)read_int(source, ' ');

  if((HIWORD(version[0]) >= 1) && (LOWORD(version[0]) >=21)) {
    InstaImage->mSWDllVer = (unsigned long)read_int(source, ' ');
    InstaImage->mSWDllRev = (unsigned long)read_int(source, ' ');
    InstaImage->mSWDllRel = (unsigned long)read_int(source, ' ');
    InstaImage->mSWDllBld = (unsigned long)read_int(source, ' ');
  }

  if((HIWORD(version[0]) >= 1) && (LOWORD(version[0]) >=23)) {
    /* Unknown semantics of these values. */
    read_int(source, ' ');
    read_int(source, ' ');
  }

  if( (HIWORD(version[0]) >= 1) && (LOWORD(version[0]) >=5) ){
    read_int(source, '\n');
    read_string(source, '\n',instaimage_title, MAXSTRING);
    strcpy(InstaImage->head_model,instaimage_title);
    InstaImage->detector_format_x = (int)read_int(source, ' ');
    InstaImage->detector_format_z = (int)read_int(source, ' ');
  }
  else if( (HIWORD(version[0]) >= 1) && (LOWORD(version[0]) >=3) ){
    unsigned long head_model = (int)read_int(source, ' ');
    sprintf(InstaImage->head_model,"%lu",head_model);
    InstaImage->detector_format_x = (int)read_int(source, ' ');
    InstaImage->detector_format_z = (int)read_int(source, ' ');
  }
  else {
	 strcpy(InstaImage->head_model,"Unknown");
	 InstaImage->detector_format_x = 1024u;
	 InstaImage->detector_format_z = 256u;
  }

  read_int(source, '\n');
  read_string(source, '\n',instaimage_title, MAXSTRING);
  strcpy(InstaImage->filename,instaimage_title);

    version[1] = read_int(source, ' ');
    check_version( version[1], 65538L, "InstaImage user text" );
    read_usertext(source, InstaImage->user_text);
    read_shutter(source, &InstaImage->shutter, version);
    read_shamrockSave(source, &InstaImage->shamrock_save, version);
    read_spectrographSave(source, &InstaImage->spec_save, version);

    for (i = 0; i < 5; i++)
        InstaImage->subversion[i] = version[i];
}


void read_calibimage(FILE *source, readsif_TCalibImage* CalibImage) {
  long version;
  int len;
  char calibimage_title[MAXSTRING];

  CalibImage->x_text = (char*)malloc(MAXSTRING);
  CalibImage->y_text = (char*)malloc(MAXSTRING);
  CalibImage->z_text = (char*)malloc(MAXSTRING);
  version = read_int(source, ' ');
  CalibImage->x_type = read_byte_and_skip_terminator(source);
  CalibImage->x_unit = read_byte_and_skip_terminator(source);
  CalibImage->y_type = read_byte_and_skip_terminator(source);
  CalibImage->y_unit = read_byte_and_skip_terminator(source);
  CalibImage->z_type = read_byte_and_skip_terminator(source);
  CalibImage->z_unit = read_byte_and_skip_terminator(source);
  CalibImage->x_cal[0] = read_float(source, ' ');
  CalibImage->x_cal[1] = read_float(source, ' ');
  CalibImage->x_cal[2] = read_float(source, ' ');
  CalibImage->x_cal[3] = read_float(source, '\n');
  CalibImage->y_cal[0] = read_float(source, ' ');
  CalibImage->y_cal[1] = read_float(source, ' ');
  CalibImage->y_cal[2] = read_float(source, ' ');
  CalibImage->y_cal[3] = read_float(source, '\n');
  CalibImage->z_cal[0] = read_float(source, ' ');
  CalibImage->z_cal[1] = read_float(source, ' ');
  CalibImage->z_cal[2] = read_float(source, ' ');
  CalibImage->z_cal[3] = read_float(source, '\n');

  if( (HIWORD(version) >= 1) && (LOWORD(version) >=3) ){
    CalibImage->rayleigh_wavelength = read_float(source, '\n');
    CalibImage->pixel_length = read_float(source, '\n');
    CalibImage->pixel_height = read_float(source, '\n');
  }

  len = (int)read_int(source, '\n');
  read_len_chars(source, len,calibimage_title, MAXSTRING);
  strcpy(CalibImage->x_text,calibimage_title);
  len = (int)read_int(source, '\n');
  read_len_chars(source, len,calibimage_title, MAXSTRING);
  strcpy(CalibImage->y_text,calibimage_title);
  len = (int)read_int(source, '\n');
  read_len_chars(source, len,calibimage_title, MAXSTRING);
  strcpy(CalibImage->z_text,calibimage_title);
}

void read_image_structure(FILE *source, readsif_TImage *Image) {
   long version1,*version2;
   int j, k;

   version1 = read_int(source, ' ');
   Image->image_format.left = (int)read_int(source, ' ');
   Image->image_format.top = (int)read_int(source, ' ');
   Image->image_format.right = (int)read_int(source, ' ');
   Image->image_format.bottom = (int)read_int(source, ' ');
   Image->no_images = (int)read_int(source, ' ');
   Image->no_subimages = (int)read_int(source, ' ');
   Image->total_length = read_int(source, ' ');
   Image->image_length = read_int(source, '\n');
   version2 = (long*)malloc((sizeof(long))*Image->no_subimages);
   Image->position = (readsif_TSubImage*)malloc((sizeof(int))*Image->no_subimages*6);
   Image->subimage_offset = 
      (unsigned long*)malloc((sizeof(unsigned long))*Image->no_subimages);
   for(j=0;j<Image->no_subimages;j++){ //Repeat no_subimages times
      version2[j] = read_int(source, ' ');
      Image->position[j].left= (int)read_int(source, ' ');
      Image->position[j].top = (int)read_int(source, ' ');
      Image->position[j].right = (int)read_int(source, ' ');
      Image->position[j].bottom = (int)read_int(source, ' ');
      Image->position[j].vertical_bin = (int)read_int(source, ' ');
      Image->position[j].horizontal_bin = (int)read_int(source, ' ');
      Image->subimage_offset[j] = read_int(source, '\n');
   }

   Image->time_stamps = 
      (unsigned long*)malloc((sizeof(unsigned long))*Image->no_images);
   for(k=0;k<Image->no_images;k++){
      Image->time_stamps[k] = read_int(source, '\n');
   }

   free(version2);
}

