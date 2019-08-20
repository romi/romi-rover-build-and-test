#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <stdint.h>
#include <jpeglib.h>
#include <rcom.h>
#include "convert.h"

#define BLOCKSIZE 4096

typedef struct _jpeg_dest_t {
        struct jpeg_destination_mgr mgr;
        membuf_t* membuf;
} jpeg_dest_t;


static void jpeg_bufferinit(j_compress_ptr cinfo)
{
        jpeg_dest_t* my_mgr = (jpeg_dest_t*) cinfo->dest;
        membuf_t* membuf = my_mgr->membuf;

        cinfo->dest->next_output_byte = (unsigned char*) membuf_data(membuf);
        cinfo->dest->free_in_buffer = membuf_available(membuf);
}

static boolean jpeg_bufferemptyoutput(j_compress_ptr cinfo)
{
        jpeg_dest_t* my_mgr = (jpeg_dest_t*) cinfo->dest;
        membuf_t* membuf = my_mgr->membuf;
        
        if (membuf_assure(membuf, BLOCKSIZE) != 0)
                return 0;
        
        cinfo->dest->next_output_byte = (unsigned char*) membuf_data(membuf) + membuf_len(membuf);
        cinfo->dest->free_in_buffer = membuf_available(membuf);

        return 1;
}

static void jpeg_bufferterminate(j_compress_ptr cinfo)
{
        jpeg_dest_t* my_mgr = (jpeg_dest_t*) cinfo->dest;
        membuf_t* membuf = my_mgr->membuf;
        int len = membuf_size(membuf) - cinfo->dest->free_in_buffer;
        membuf_set_len(membuf, len);
}

int convert_to_jpeg(const void* image, int width, int height, int quality, membuf_t *out)
{
        struct jpeg_compress_struct cinfo;
        struct jpeg_error_mgr jerr;
	jpeg_dest_t* my_mgr;

        JSAMPROW row_pointer[1];

        cinfo.err = jpeg_std_error(&jerr);
        jpeg_create_compress(&cinfo);

        cinfo.dest = (struct jpeg_destination_mgr *) 
                (*cinfo.mem->alloc_small) ((j_common_ptr) &cinfo, JPOOL_PERMANENT,
                                           sizeof(jpeg_dest_t));       
        cinfo.dest->init_destination = &jpeg_bufferinit;
        cinfo.dest->empty_output_buffer = &jpeg_bufferemptyoutput;
        cinfo.dest->term_destination = &jpeg_bufferterminate;

        my_mgr = (jpeg_dest_t*) cinfo.dest;
        my_mgr->membuf = out;
        membuf_assure(out, BLOCKSIZE);

        cinfo.image_width = width;	
        cinfo.image_height = height;
        cinfo.input_components = 3;
        cinfo.in_color_space = JCS_RGB;

        jpeg_set_defaults(&cinfo);
        jpeg_set_quality(&cinfo, quality, TRUE);

        jpeg_start_compress(&cinfo, TRUE);

        // feed data
        while (cinfo.next_scanline < cinfo.image_height) {
                unsigned int offset = cinfo.next_scanline * cinfo.image_width *  cinfo.input_components;
                row_pointer[0] = (unsigned char *)image + offset;
                jpeg_write_scanlines(&cinfo, row_pointer, 1);
        }

        jpeg_finish_compress(&cinfo);

        jpeg_destroy_compress(&cinfo);

        return 0;
}


