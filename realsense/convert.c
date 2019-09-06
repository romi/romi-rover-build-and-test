#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <stdint.h>
#include <jpeglib.h>
#include <png.h>
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



void append_png_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
        membuf_t *out = (membuf_t*) png_get_io_ptr(png_ptr);
        membuf_append(out, (const char*) data, length);
}

void flush_png_data(png_structp png_ptr)
{
}

int convert_to_png_16bit_grayscale(const uint16_t* image, int width, int height, membuf_t *out)
{
        png_structp png_ptr = NULL;
        png_infop info_ptr = NULL;
        size_t x, y, k;
        png_bytepp row_pointers;

        png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
        if (png_ptr == NULL) {
                return -1;
        }

        info_ptr = png_create_info_struct(png_ptr);
        if (info_ptr == NULL) {
                png_destroy_write_struct(&png_ptr, NULL);
                return -1;
        }

        if (setjmp(png_jmpbuf(png_ptr))) {
                png_destroy_write_struct(&png_ptr, &info_ptr);
                return -1;
        }

        png_set_IHDR(png_ptr, info_ptr,
                     width, height, 
                     16, // bit depth
                     PNG_COLOR_TYPE_GRAY, 
                     PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

        row_pointers = (png_bytepp) png_malloc(png_ptr, height * sizeof(png_bytep));
        for (y = 0; y < (size_t) height; y++)
                row_pointers[y] = png_malloc(png_ptr, width * 2);

        for (y = 0, k = 0; y < (size_t) height; y++) {
                png_bytep row = row_pointers[y];
                for (x = 0; x < (size_t) width; x++) {
                        uint16_t color = image[k++];
                        *row++ = (png_byte)(color >> 8);
                        *row++ = (png_byte)(color & 0xFF);
                }
        }

        png_set_write_fn(png_ptr, out, append_png_data, flush_png_data);
        
        png_set_rows(png_ptr, info_ptr, row_pointers);
        png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);


        for (y = 0; y < (size_t) height; y++) {
                png_free(png_ptr, row_pointers[y]);
        }
        png_free(png_ptr, row_pointers);
        png_destroy_write_struct(&png_ptr, &info_ptr);

        return 0;
}
