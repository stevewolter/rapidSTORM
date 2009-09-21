#include "read_sif.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

void print_instaImage(const char *prefix, const readsif_TInstaImage* t, FILE* stream)
{
    char sp[40];
    int i;
    sprintf(sp, "%s  ", prefix);

    for (i = 0; i < 5; i++)
        fprintf(stream, "%sVersion #%i: %lu\n", sp, i, t->subversion[i]);
    fprintf(stream, "%sHead:\t%i\n", sp, t->head);
    fprintf(stream, "%sStore type:\t%i\n", sp, t->store_type);
    fprintf(stream, "%sData type:\t%i\n", sp, t->data_type);
    fprintf(stream, "%sType:\t%i\n", sp, t->type);
    fprintf(stream, "%sActive:\t%i\n", sp, t->active);
    fprintf(stream, "%sStructure version:\t%i\n", sp, t->structure_version);
    fprintf(stream, "%sNo. integrations:\t%i\n", sp, t->no_integrations);
    fprintf(stream, "%sNo. points:\t%i\n", sp, t->no_points);
    fprintf(stream, "%sSeries length:\t%i\n", sp, t->series_length);
    fprintf(stream, "%sMT offset:\t%i\n", sp, t->mt_offset);
    fprintf(stream, "%sST centre row:\t%i\n", sp, t->st_centre_row);
}

void print_longrect(const char *prefix, const readsif_LONG_RECT* t, FILE* stream) {
    char sp[40];
    sprintf(sp, "%s  ", prefix);

    fprintf(stream, "%sX range: %i - %i\n", sp, t->left, t->right);
    fprintf(stream, "%sY range: %i - %i\n", sp, t->bottom, t->top);
}

void print_subimage(const char *prefix, const readsif_TSubImage* t, FILE* stream) {
    char sp[40];
    sprintf(sp, "%s  ", prefix);

    fprintf(stream, "%sX range: %i - %i\n", sp, t->left, t->right);
    fprintf(stream, "%sY range: %i - %i\n", sp, t->bottom, t->top);
    fprintf(stream, "%sHorizontal binning: %i\n", sp, t->horizontal_bin);
    fprintf(stream, "%sVertical binning: %i\n", sp, t->vertical_bin);
}

void print_image(const char *prefix, const readsif_TImage* t, FILE* stream) {
    char sp[40];
    int i = 0;
    sprintf(sp, "%s  ", prefix);

    printf("%sPosition:\n", sp);
    print_subimage(sp, t->position, stream);
    printf("%sImage format:\n", sp);
    print_longrect(sp, &t->image_format, stream);
    printf("%sNumber of subimages:\t%i\n", sp, t->no_subimages);
    printf("%sNumber of images:\t%i\n", sp, t->no_images);
    for (i = 0; i < t->no_subimages; i++) {
        printf("%sSubimage %i offset:\t%lu\n", sp,i,t->subimage_offset[i]);
        printf("%sSubimage %i timestamp:\t%lu\n", sp,i, t->time_stamps[i]);
    }
    printf("%sImage length:\t%lu\n", sp, t->image_length);
    printf("%sTotal length:\t%lu\n", sp, t->total_length);
    /*printf("%s:\t%i\n", sp, t->head);*/
}


int main(int argc, char *argv[]) {
    FILE *stream;
    readsif_File *rf;
    readsif_DataSet* ds;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s [file]\n", argv[0]);
        exit(1);
    }

    stream = fopen(argv[1], "r");
    if (stream == NULL) {
        fprintf(stderr, "Unable to open %s: %s\n",
                         argv[1], strerror(errno));
        exit(1);
    }

    rf = readsif_open_File(stream);
    
    printf("Version:\t%i\n", rf->version);
    while ( readsif_mightContainMoreDataSets(rf) ) {
        ds = readsif_read_DataSet(rf);

        if (ds != NULL) {
            if ( ds->data_present ) {
                fprintf(stdout, "InstaImage substructure:\n");
                print_instaImage("  ", &ds->instaImage, stdout);
                fprintf(stdout, "Image substructure:\n");
                print_image("  ", &ds->image, stdout);
                readsif_seek_past_DataSet( ds );
            }
            readsif_destroy_DataSet(ds);
        } else {
            fprintf(stderr, "%s\n", readsif_error);
            break;
        }
    }
    return 0;
}
