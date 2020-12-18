/*
  libromi

  Copyright (C) 2019 Sony Computer Science Laboratories
  Author(s) Peter Hanappe

  Libromi provides common abstractions and functions for ROMI
  applications.

  libromi is free software: you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation, either version 3 of the
  License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see
  <http://www.gnu.org/licenses/>.

 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jpeglib.h>
#include <png.h>
#include <setjmp.h>
#include <math.h>

#include <r.h>
#include "image_impl.h"

typedef struct {
    float r;       // a fraction between 0 and 1
    float g;       // a fraction between 0 and 1
    float b;       // a fraction between 0 and 1
} rgb_t;

typedef struct {
    float h;       // angle in degrees
    float s;       // a fraction between 0 and 1
    float v;       // a fraction between 0 and 1
} hsv_t;


static int channels_per_type(int type)
{
        int r = -1;
        switch (type) {
        case IMAGE_BW:
                r = 1;
                break;
        case IMAGE_RGB:
                r = 3;
                break;
        case IMAGE_HSV:
                r = 3;
                break;
        }
        return r;
}

image_t *new_image(int type, int width, int height)
{
        image_t *image = r_new(image_t);
        image->type = type;
        image->channels = channels_per_type(type);
        if (image->channels <= 0) {
                r_err("Invalid type");
                r_delete(image);
        }
        image->width = width;
        image->height = height;
        int len = width * height;
        if (len > 0) {
                image->data = (float *) r_alloc(image->channels * len * sizeof(float));
                memset(image->data, 0, image->channels * len * sizeof(float));
        } else {
                image->data = NULL;
        }
        return image;
}

image_t *new_image_bw(int width, int height)
{
        return new_image(IMAGE_BW, width, height);
}

image_t *new_image_rgb(int width, int height)
{
        return new_image(IMAGE_RGB, width, height);
}

image_t *new_image_hsv(int width, int height)
{
        return new_image(IMAGE_HSV, width, height);
}

void delete_image(image_t *image)
{
        if (image) {
                if (image->data)
                        r_free(image->data);
                r_delete(image);
        }
}

int image_width(image_t *image)
{
        return image? image->width : 0;
}

int image_height(image_t *image)
{
        return image? image->height : 0;
}

image_t *image_clone(image_t *im)
{
        if (im == NULL) {
                r_warn("image_clone: image is null");
                return NULL;
        }
        image_t *image = (image_t *) r_alloc(sizeof(image_t));

        image->type = im->type;
        image->channels = im->channels;
        image->width = im->width;
        image->height = im->height;
        image->data = r_array(float, im->channels * im->width * im->height);
        memcpy(image->data, im->data, im->channels * im->width * im->height * sizeof(float));
        return image;
}

void image_clear(image_t *image)
{
        if (image == NULL) {
                r_warn("image_clear: image is null");
        } else {
                memset(image->data, 0, image->channels * image->width * image->height);
        }
}

void image_fill(image_t *image, int channel, float color)
{
        if (image == NULL) {
                r_warn("image_fill: image is null");
                return;
        }        
        int stride = image->channels;
        int len = image->channels * image->width * image->height;
        for (int i = channel; i < len; i += stride)
                image->data[i] = color;
}

void image_offset(image_t *image, float offset, int channel)
{
        if (image == NULL) {
                r_warn("image_circle: image is null");
                return;
        }
        int stride = image->channels;
        int len = image->channels * image->width * image->height;
        for (int i = channel; i < len; i += stride)
                image->data[i] += offset;
}

void image_circle(image_t *image, float xc, float yc, float radius, float* color)
{
        if (image == NULL) {
                r_warn("image_circle: image is null");
                return;
        }
        int ymin = ceilf(yc - radius);
        int ymax = floorf(yc + radius);
        int r2 = radius * radius;

        if (ymin >= image->height || ymax < 0)
                return;
        if (xc - radius >= image->width || xc + radius < 0)
                return;

        if (ymin < 0)
                ymin = 0;
        if (ymax >= image->height)
                ymax = image->height - 1;
        
        for (int y = ymin; y <= ymax; y++) {
                float _y = (float) y - yc;
                float _x = sqrtf(r2 - (float) (_y * _y));
                int xmin = (int) roundf(xc - _x);
                int xmax = (int) roundf(xc + _x);
                
                if (xmin >= image->width || xmax < 0)
                        continue;
                if (xmin < 0)
                        xmin = 0;
                if (xmax >= image->width)
                        xmax = image->width-1;
                
                for (int x = xmin; x <= xmax; x++) {
                        for (int channel = 0; channel < image->channels; channel++)
                                image_set(image, x, y, channel, color[channel]);
                }
        }
}

void image_bell(image_t *image, float xc, float yc, float stddev)
{
        if (image == NULL) {
                r_warn("image_bell: image is null");
                return;
        }
        float r = 3 * stddev;
        int ymin = ceilf(yc - r);
        int ymax = floorf(yc + r);
        float var = stddev * stddev;
        float r2 = r * r;

        if (ymin >= image->height || ymax < 0)
                return;
        if (xc - r >= image->width || xc + r < 0)
                return;

        if (ymin < 0)
                ymin = 0;
        if (ymax >= image->height)
                ymax = image->height - 1;
        
        for (int y = ymin; y <= ymax; y++) {
                float _y = (float) y - yc;
                float _x = sqrtf(r2 - (float) (_y * _y));
                int xmin = (int) roundf(xc - _x);
                int xmax = (int) roundf(xc + _x);
                
                if (xmin >= image->width || xmax < 0)
                        continue;
                if (xmin < 0)
                        xmin = 0;
                if (xmax >= image->width)
                        xmax = image->width-1;
                
                for (int x = xmin; x <= xmax; x++) {
                        // For normalised values, the color value
                        // should be divided by 2.pi.variance. I leave
                        // it as is so that the maximum color is
                        // white.
                        _x = (float) x - xc;
                        float color = expf(-(_x * _x + _y * _y) / (2.0f * var));
                        for (int channel = 0; channel < image->channels; channel++)
                                image_set(image, x, y, channel, color);
                }
        }
}


/*****************************************************/
/* JPEG                                              */

static jmp_buf setjmp_buffer;

static void exit_error(j_common_ptr cinfo __attribute__((unused)))
{
        longjmp(setjmp_buffer, 1);
}

image_t *image_load_jpeg(const char *filename)
{
        struct jpeg_error_mgr pub;
        struct jpeg_decompress_struct cinfo;
        image_t *image = NULL;
        JSAMPARRAY buffer;
        int row_stride;
        FILE *infile;
        
        if ((infile = fopen(filename, "rb")) == NULL) {
                fprintf(stderr, "Failed to open the file %s\n", filename);
                return NULL;
        }

        /* Step 1: allocate and initialize JPEG decompression object */
		
        /* We set up the normal JPEG error routines, then
         * override error_exit. */
        cinfo.err = jpeg_std_error(&pub);
        pub.error_exit = exit_error;

        /* Establish the setjmp return context for
         * exit_error to use. */
        if (setjmp(setjmp_buffer)) {
                jpeg_destroy_decompress(&cinfo);
                fclose(infile);
                fprintf(stderr, "Failed to load the file. Not a JPEG file?\n");
                return NULL;
        }

        /* Now we can initialize the JPEG decompression object. */
        jpeg_create_decompress(&cinfo);

        /* Step 2: specify data source (eg, a file) */
        jpeg_stdio_src(&cinfo, infile);

        /* Step 3: read file parameters with jpeg_read_header() */
        jpeg_read_header(&cinfo, TRUE);

        /* Step 4: set parameters for decompression */
		
        /* In this example, we don't need to change any of the
         * defaults set by jpeg_read_header(), so we do
         * nothing here.
         */
		
        /* Step 5: Start decompressor */		
        jpeg_start_decompress(&cinfo);

        /* We may need to do some setup of our own at this
         * point before reading the data.  After
         * jpeg_start_decompress() we have the correct scaled
         * output image dimensions available, as well as the
         * output colormap if we asked for color quantization.
         * In this example, we need to make an output work
         * buffer of the right size.
         */
        if (cinfo.output_components == 1
            && cinfo.jpeg_color_space == JCS_GRAYSCALE) {
                //fprintf(stderr, "8-bit grayscale JPEG\n");
                image = new_image_bw(cinfo.output_width, cinfo.output_height);
        } else if (cinfo.output_components == 3
            && cinfo.out_color_space == JCS_RGB) {
                //fprintf(stderr, "24-bit RGB JPEG\n");
                image = new_image_rgb(cinfo.output_width, cinfo.output_height);
        } else {
                fprintf(stderr, "Unhandled JPEG format\n");
                return NULL;
        }

        /* JSAMPLEs per row in output buffer */
        row_stride = cinfo.output_width * cinfo.output_components;

        /* Make a one-row-high sample array that will go away
         * when done with image */
        buffer = (*cinfo.mem->alloc_sarray)
                ((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

        /* Step 6: while (scan lines remain to be read) */
        /*           jpeg_read_scanlines(...); */

        /* Here we use the library's state variable
         * cinfo.output_scanline as the loop counter, so that
         * we don't have to keep track ourselves.
         */
        while (cinfo.output_scanline < cinfo.output_height) {
                /* jpeg_read_scanlines expects an array of
                 * pointers to scanlines.  Here the array is
                 * only one element long, but you could ask
                 * for more than one scanline at a time if
                 * that's more convenient.
                 */
                (void) jpeg_read_scanlines(&cinfo, buffer, 1);

                unsigned int offset;
                unsigned char* p = buffer[0];
                
                if (image->type == IMAGE_BW) {
                        offset = (cinfo.output_scanline - 1) * cinfo.output_width;
                        for (size_t i = 0; i < cinfo.output_width; i++, offset++) 
                                image->data[offset] = (float) p[i] / 255.0f;
                } else {
                        offset = 3 * (cinfo.output_scanline - 1) * cinfo.output_width;
                        for (size_t i = 0; i < cinfo.output_width; i++) {
                                image->data[offset++] = (float) *p++ / 255.0f;
                                image->data[offset++] = (float) *p++ / 255.0f;
                                image->data[offset++] = (float) *p++ / 255.0f;
                        }
                }
        }

        /* Step 7: Finish decompression */

        jpeg_finish_decompress(&cinfo);

        /* We can ignore the return value since suspension is
         * not possible with the stdio data source.
         */

        /* Step 8: Release JPEG decompression object */

        /* This is an important step since it will release a
         * good deal of memory. */
        jpeg_destroy_decompress(&cinfo);

        /* After finish_decompress, we can close the input
         * file.  Here we postpone it until after no more JPEG
         * errors are possible, so as to simplify the setjmp
         * error logic above.  (Actually, I don't think that
         * jpeg_destroy can do an error exit, but why assume
         * anything...)
         */
        fclose(infile);

        /* At this point you may want to check to see whether
         * any corrupt-data warnings occurred (test whether
         * jerr.pub.num_warnings is nonzero).
         */

        /* And we're done! */
        return image;
}

image_t *image_load_from_mem(const unsigned char *data, int len)
{
        struct jpeg_error_mgr pub;
        struct jpeg_decompress_struct cinfo;
        image_t *image = NULL;
        JSAMPARRAY buffer;
        int row_stride;

        /* Step 1: allocate and initialize JPEG decompression object */
		
        /* We set up the normal JPEG error routines, then
         * override error_exit. */
        cinfo.err = jpeg_std_error(&pub);
        pub.error_exit = exit_error;

        /* Establish the setjmp return context for
         * exit_error to use. */
        if (setjmp(setjmp_buffer)) {
                jpeg_destroy_decompress(&cinfo);
                fprintf(stderr, "Failed to load the data. Not a JPEG?\n");
                return NULL;
        }

        /* Now we can initialize the JPEG decompression object. */
        jpeg_create_decompress(&cinfo);

        /* Step 2: specify data source */
        jpeg_mem_src(&cinfo, data, len);

        
        /* Step 3: read file parameters with jpeg_read_header() */
        jpeg_read_header(&cinfo, TRUE);

        /* Step 4: set parameters for decompression */
		
        /* In this example, we don't need to change any of the
         * defaults set by jpeg_read_header(), so we do
         * nothing here.
         */
		
        /* Step 5: Start decompressor */		
        jpeg_start_decompress(&cinfo);

        /* We may need to do some setup of our own at this
         * point before reading the data.  After
         * jpeg_start_decompress() we have the correct scaled
         * output image dimensions available, as well as the
         * output colormap if we asked for color quantization.
         * In this example, we need to make an output work
         * buffer of the right size.
         */
        if (cinfo.output_components == 1
            && cinfo.jpeg_color_space == JCS_GRAYSCALE) {
                fprintf(stderr, "8-bit grayscale JPEG\n");
                image = new_image_bw(cinfo.output_width, cinfo.output_height);
        } else if (cinfo.output_components == 3
            && cinfo.out_color_space == JCS_RGB) {
                fprintf(stderr, "24-bit RGB JPEG\n");
                image = new_image_rgb(cinfo.output_width, cinfo.output_height);
        } else {
                fprintf(stderr, "Unhandled JPEG format\n");
                return NULL;
        }

        /* JSAMPLEs per row in output buffer */
        row_stride = cinfo.output_width * cinfo.output_components;

        /* Make a one-row-high sample array that will go away
         * when done with image */
        buffer = (*cinfo.mem->alloc_sarray)
                ((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

        /* Step 6: while (scan lines remain to be read) */
        /*           jpeg_read_scanlines(...); */

        /* Here we use the library's state variable
         * cinfo.output_scanline as the loop counter, so that
         * we don't have to keep track ourselves.
         */
        while (cinfo.output_scanline < cinfo.output_height) {
                /* jpeg_read_scanlines expects an array of
                 * pointers to scanlines.  Here the array is
                 * only one element long, but you could ask
                 * for more than one scanline at a time if
                 * that's more convenient.
                 */
                (void) jpeg_read_scanlines(&cinfo, buffer, 1);

                unsigned int offset;
                unsigned char* p = buffer[0];
                
                if (image->type == IMAGE_BW) {
                        offset = (cinfo.output_scanline - 1) * cinfo.output_width;
                        for (size_t i = 0; i < cinfo.output_width; i++, offset++) 
                                image->data[offset] = (float) p[i] / 255.0f;
                } else {
                        offset = 3 * (cinfo.output_scanline - 1) * cinfo.output_width;
                        for (size_t i = 0; i < cinfo.output_width; i++) {
                                image->data[offset++] = (float) *p++ / 255.0f;
                                image->data[offset++] = (float) *p++ / 255.0f;
                                image->data[offset++] = (float) *p++ / 255.0f;
                        }
                }
        }

        /* Step 7: Finish decompression */

        jpeg_finish_decompress(&cinfo);

        /* We can ignore the return value since suspension is
         * not possible with the stdio data source.
         */

        /* Step 8: Release JPEG decompression object */

        /* This is an important step since it will release a
         * good deal of memory. */
        jpeg_destroy_decompress(&cinfo);

        /* At this point you may want to check to see whether
         * any corrupt-data warnings occurred (test whether
         * jerr.pub.num_warnings is nonzero).
         */

        /* And we're done! */
        return image;
}

int image_store_jpeg(image_t* image, const char *filename)
{
        struct jpeg_compress_struct cinfo;
        struct jpeg_error_mgr jerr;
        FILE* outfile;	 
        JSAMPLE* buffer;
        int index = 0;

        if (image == NULL) {
                r_warn("image_store_jpeg: image is null");
                return -1;
        }
        
        cinfo.err = jpeg_std_error(&jerr);
        jpeg_create_compress(&cinfo);

        if ((outfile = fopen(filename, "wb")) == NULL) {
                r_err("Failed to open the file: %s\n", filename);
                return -1;
        }
        jpeg_stdio_dest(&cinfo, outfile);

        cinfo.image_width = image->width; 
        cinfo.image_height = image->height;
        if (image->type == IMAGE_BW) {
                cinfo.input_components = 1;	
                cinfo.in_color_space = JCS_GRAYSCALE;
        } else {
                cinfo.input_components = 3;	
                cinfo.in_color_space = JCS_RGB;
        }
        
        jpeg_set_defaults(&cinfo);
        jpeg_set_quality(&cinfo, 90, TRUE);

        jpeg_start_compress(&cinfo, TRUE);

        buffer = (JSAMPLE*) r_alloc(image->channels * image->width); 

        while (cinfo.next_scanline < cinfo.image_height) {
                if (image->type == IMAGE_BW) {
                        for (int i = 0; i < image->width; i++)
                                buffer[i] = (unsigned char) (image->data[index++] * 255.0f);
                } else {
                        for (int i = 0, j = 0; i < image->width; i++) {
                                buffer[j++] = (unsigned char) (image->data[index++] * 255.0f);
                                buffer[j++] = (unsigned char) (image->data[index++] * 255.0f);
                                buffer[j++] = (unsigned char) (image->data[index++] * 255.0f);
                        }
                }
                jpeg_write_scanlines(&cinfo, &buffer, 1);
        }

        jpeg_finish_compress(&cinfo);
        fclose(outfile);
        jpeg_destroy_compress(&cinfo);
        r_free(buffer);

        return 0;
}

#define BLOCKSIZE 4096

typedef struct _jpeg_dest_t {
        struct jpeg_destination_mgr mgr;
        membuf_t* membuf;
} jpeg_dest_t;


static void jpeg_bufferinit(j_compress_ptr cinfo)
{
        jpeg_dest_t* my_mgr = (jpeg_dest_t*) cinfo->dest;
        membuf_t* membuf = my_mgr->membuf;

        membuf_clear(membuf);
        membuf_assure(membuf, BLOCKSIZE);
        cinfo->dest->next_output_byte = (unsigned char*) membuf_data(membuf);
        cinfo->dest->free_in_buffer = membuf_available(membuf);
}

static boolean jpeg_bufferemptyoutput(j_compress_ptr cinfo)
{
        jpeg_dest_t* my_mgr = (jpeg_dest_t*) cinfo->dest;
        membuf_t* membuf = my_mgr->membuf;

        membuf_set_len(membuf, membuf_size(membuf));

        membuf_assure(membuf, BLOCKSIZE);
        
        cinfo->dest->next_output_byte = (unsigned char*) (membuf_data(membuf)
                                                          + membuf_len(membuf));
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

int image_store_to_mem_jpeg(image_t* image, membuf_t *out)
{
        struct jpeg_compress_struct cinfo;
        struct jpeg_error_mgr jerr;
	jpeg_dest_t* my_mgr;
        JSAMPLE* buffer;
        int index = 0;

        if (image == NULL) {
                r_warn("image_store_to_mem_jpeg: image is null");
                return -1;
        }

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

        cinfo.image_width = image->width;	
        cinfo.image_height = image->height;
        if (image->type == IMAGE_BW) {
                cinfo.input_components = 1;	
                cinfo.in_color_space = JCS_GRAYSCALE;
        } else {
                cinfo.input_components = 3;	
                cinfo.in_color_space = JCS_RGB;
        }

        jpeg_set_defaults(&cinfo);
        jpeg_set_quality(&cinfo, 90, TRUE);

        jpeg_start_compress(&cinfo, TRUE);

        buffer = (JSAMPLE*) r_alloc(image->channels * image->width); 

        while (cinfo.next_scanline < cinfo.image_height) {
                if (image->type == IMAGE_BW) {
                        for (int i = 0; i < image->width; i++)
                                buffer[i] = (unsigned char) (image->data[index++] * 255.0f);
                } else {
                        for (int i = 0, j = 0; i < image->width; i++) {
                                buffer[j++] = (unsigned char) (image->data[index++] * 255.0f);
                                buffer[j++] = (unsigned char) (image->data[index++] * 255.0f);
                                buffer[j++] = (unsigned char) (image->data[index++] * 255.0f);
                        }
                }
                jpeg_write_scanlines(&cinfo, &buffer, 1);
        }

        jpeg_finish_compress(&cinfo);

        jpeg_destroy_compress(&cinfo);
        r_free(buffer);

        return 0;
}

/*****************************************************/
/* PNG                                               */

image_t *image_load_png(const char *filename)
{
        int x, y, i, k;
        FILE *fp = NULL;
        png_structp png = NULL;
        png_infop info = NULL;
        png_bytep *row_pointers = NULL;
        
        png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
        if (png == NULL) {
                r_err("Memory allocation failed");
                goto cleanup_and_exit;
        }

        if (setjmp(png_jmpbuf(png))) {
                r_err("setjmp failed");
                return NULL;
        }

        info = png_create_info_struct(png);
        if (info == NULL) {
                r_err("Memory allocation failed");
                goto cleanup_and_exit;
        }
        
        fp = fopen(filename, "rb");
        if (fp == NULL) {
                r_err("Failed to open the file '%s'", filename);
                goto cleanup_and_exit;
        }
        
        png_init_io(png, fp);
        png_read_info(png, info);

        png_byte color_type = png_get_color_type(png, info);
        png_byte bit_depth = png_get_bit_depth(png, info);
        int width = png_get_image_width(png, info);
        int height = png_get_image_height(png, info);
        image_t *image = NULL;

        // Convert 16-bits to 8-bits 
        if (bit_depth == 16)
                png_set_strip_16(png);

        // Convert < 8-bit gray scale to 8-bit 
        if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
                png_set_expand_gray_1_2_4_to_8(png);

        if (color_type == PNG_COLOR_TYPE_GRAY) {
                image = new_image_bw(width, height);
        } else if (color_type == PNG_COLOR_TYPE_RGB
                   || color_type == PNG_COLOR_TYPE_RGBA) {
                image = new_image_rgb(width, height);
        } else {
                r_err("Unsupported PNG image format");
                return NULL;
        }
                
        png_read_update_info(png, info);

        row_pointers = (png_bytep*) r_alloc(sizeof(png_bytep) * height);
        if (row_pointers == NULL) 
                goto cleanup_and_exit;
        
        for (y = 0; y < height; y++) {
                row_pointers[y] = (png_byte*) r_alloc(png_get_rowbytes(png, info));
                if (row_pointers[y] == NULL) 
                        goto cleanup_and_exit;
        }
        
        png_read_image(png, row_pointers);

        if (color_type == PNG_COLOR_TYPE_GRAY) {
                for (y = 0, k = 0; y < height; y++) {
                        png_bytep row = row_pointers[y];
                        for (x = 0; x < width; x++)
                                image->data[k++] = (float) *row++ / 255.0f;
                }
        } else if (color_type == PNG_COLOR_TYPE_RGB) {
                for (y = 0, k = 0; y < height; y++) {
                        png_bytep row = row_pointers[y];
                        for (x = 0; x < width; x++) {
                                image->data[k++] = (float) *row++ / 255.0f;
                                image->data[k++] = (float) *row++ / 255.0f;
                                image->data[k++] = (float) *row++ / 255.0f;
                        }
                }
        } else if (color_type == PNG_COLOR_TYPE_RGBA) {
                for (y = 0, k = 0; y < height; y++) {
                        png_bytep row = row_pointers[y];
                        for (x = 0, i = 0; x < width; x++, i += 4) {
                                float alpha = (float) row[i+3] / 255.0f;
                                image->data[k++] = alpha * (float) row[i] / 255.0f;
                                image->data[k++] = alpha * (float) row[i+1] / 255.0f;
                                image->data[k++] = alpha * (float) row[i+2] / 255.0f;
                        }
                }
        }

cleanup_and_exit:
        if (fp) fclose(fp);
        if (row_pointers) {
                for (y = 0; y < height; y++) 
                        if (row_pointers[y])
                                r_free(row_pointers[y]);
                r_free(row_pointers);
        }
        png_destroy_read_struct(&png, &info, NULL);
        
        return image;
}

int image_store_png(image_t* image, const char *filename)
{
        png_structp png_ptr = NULL;
        png_infop info_ptr = NULL;
        size_t x, y, k;
        png_bytepp row_pointers;

        if (image == NULL) {
                r_warn("image_store_png: image is null");
                return -1;
        }

        FILE *fp = fopen(filename, "wb");
        if (fp == NULL) {
                r_err("Failed to open the file '%s'", filename);
                return -1;
        }

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
                     image->width, image->height, 
                     8, 
                     (image->type == IMAGE_BW)? PNG_COLOR_TYPE_GRAY : PNG_COLOR_TYPE_RGB, 
                     PNG_INTERLACE_NONE,
                     PNG_COMPRESSION_TYPE_DEFAULT,
                     PNG_FILTER_TYPE_DEFAULT);

        row_pointers = (png_bytepp) png_malloc(png_ptr, image->height * sizeof(png_bytep));
        for (y = 0; y < (size_t) image->height; y++)
                row_pointers[y] = png_malloc(png_ptr, image->width * image->channels);

        if (image->type == IMAGE_BW) {
                for (y = 0, k = 0; y < (size_t) image->height; y++) {
                        png_bytep row = row_pointers[y];
                        for (x = 0; x < (size_t) image->width; x++) {
                                *row++ = (png_byte)(image->data[k++] * 255.0f);
                        }
                }
        } else {
                for (y = 0, k = 0; y < (size_t) image->height; y++) {
                        png_bytep row = row_pointers[y];
                        for (x = 0; x < (size_t) image->width; x++) {
                                *row++ = (png_byte)(image->data[k++] * 255.0f);
                                *row++ = (png_byte)(image->data[k++] * 255.0f);
                                *row++ = (png_byte)(image->data[k++] * 255.0f);
                        }
                }
        }
        
        png_init_io(png_ptr, fp);
        png_set_rows(png_ptr, info_ptr, row_pointers);
        png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

        for (y = 0; y < (size_t) image->height; y++)
                png_free(png_ptr, row_pointers[y]);

        png_free(png_ptr, row_pointers);
        png_destroy_write_struct(&png_ptr, &info_ptr);

        fclose(fp);
        
        return 0;
}

void append_png_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
        membuf_t *out = (membuf_t*) png_get_io_ptr(png_ptr);
        membuf_append(out, (const char*) data, length);
}

void flush_png_data(png_structp png_ptr __attribute__((unused)))
{
}

int image_store_to_mem_png(image_t* image, membuf_t *out)
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
                     image->width, image->height, 
                     8, 
                     (image->type == IMAGE_BW)? PNG_COLOR_TYPE_GRAY : PNG_COLOR_TYPE_RGB, 
                     PNG_INTERLACE_NONE,
                     PNG_COMPRESSION_TYPE_DEFAULT,
                     PNG_FILTER_TYPE_DEFAULT);

        row_pointers = (png_bytepp) png_malloc(png_ptr, image->height * sizeof(png_bytep));
        for (y = 0; y < (size_t) image->height; y++)
                row_pointers[y] = png_malloc(png_ptr, image->width * image->channels);

        if (image->type == IMAGE_BW) {
                for (y = 0, k = 0; y < (size_t) image->height; y++) {
                        png_bytep row = row_pointers[y];
                        for (x = 0; x < (size_t) image->width; x++) {
                                float v = image->data[k++];
                                if (v < 0.0f)
                                        v = 0.0f;
                                else if (v > 1.0f)
                                        v = 1.0f;
                                *row++ = (png_byte)(v * 255.0f);
                        }
                }
        } else {
                for (y = 0, k = 0; y < (size_t) image->height; y++) {
                        png_bytep row = row_pointers[y];
                        for (x = 0; x < (size_t) image->width; x++) {
                                float r = image->data[k++];
                                float g = image->data[k++];
                                float b = image->data[k++];
                                if (r < 0.0f)
                                        r = 0.0f;
                                else if (r > 1.0f)
                                        r = 1.0f;
                                if (g < 0.0f)
                                        g = 0.0f;
                                else if (g > 1.0f)
                                        g = 1.0f;
                                if (b < 0.0f)
                                        b = 0.0f;
                                else if (b > 1.0f)
                                        b = 1.0f;
                                *row++ = (png_byte)(r * 255.0f);
                                *row++ = (png_byte)(g * 255.0f);
                                *row++ = (png_byte)(b * 255.0f);
                        }
                }
        }
        
        png_set_write_fn(png_ptr, out, append_png_data, flush_png_data);
        
        png_set_rows(png_ptr, info_ptr, row_pointers);
        png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

        for (y = 0; y < (size_t) image->height; y++) {
                png_free(png_ptr, row_pointers[y]);
        }
        png_free(png_ptr, row_pointers);
        png_destroy_write_struct(&png_ptr, &info_ptr);

        return 0;
}


/*****************************************************/

int image_store(image_t* image, const char *filename, const char *type)
{
        if (image == NULL) {
                r_warn("image_store: image is null");
                return -1;
        }
        if (rstreq(type, "jpg"))
                return image_store_jpeg(image, filename);
        else if (rstreq(type, "png"))
                return image_store_png(image, filename);
        else {
                r_warn("Unsupported image type: '%s'", type);
                return -1;
        }
}

int image_store_to_mem(image_t* image, membuf_t *out, const char *format)
{
        if (image == NULL) {
                r_warn("image_store_to_mem: image is null");
                return -1;
        }
        if (rstreq(format, "jpg"))
                return image_store_to_mem_jpeg(image, out);
        else if (rstreq(format, "png"))
                return image_store_to_mem_png(image, out);
        else {
                r_warn("Unsupported image format: '%s'", format);
                return -1;
        }
}

int image_type_png(const char *filename)
{
        FILE *fp;
        unsigned char buf[4];

        if ((fp = fopen(filename, "rb")) == NULL)
                return 0;

        if (fread(buf, 1, 4, fp) != 4) {
                fclose(fp);
                return 0;
        }
        
        fclose(fp);
        return (png_sig_cmp(buf, 0, 4) == 0)? 1 : 0;
}

int image_type_jpeg(const char *filename)
{
        FILE *fp;
        unsigned char buf[3];

        if ((fp = fopen(filename, "rb")) == NULL)
                return 0;

        if (fread(buf, 1, 3, fp) != 3) {
                fclose(fp);
                return 0;
        }
        
        fclose(fp);

        return buf[0] == 0xff && buf[1] == 0xd8 && buf[2] == 0xff;
}

const char *image_type(const char *filename)
{
        if (image_type_jpeg(filename)) {
                return "jpg";
        } else if (image_type_png(filename)) {
                return "png";
        }
        return NULL;
}

const char *image_mimetype(const char *format)
{
        if (rstreq(format, "jpg")) {
                return "image/jpeg";
        } else if (rstreq(format, "png")) {
                return "image/png";
        } else {
                r_warn("Unsupported image format: '%s'", format);
                return NULL;
        }
}

image_t *image_load(const char *filename)
{
        const char *type = image_type(filename);
        if (type == NULL) {
                r_warn("Unsupported image type");
                return NULL;
        } else if (rstreq(type, "jpg")) {
                return image_load_jpeg(filename);
        } else if (rstreq(type, "png")) {
                return image_load_png(filename);
        } else {
                r_warn("Unsupported image type: '%s'", type);
                return NULL;
        }
}

/*****************************************************/

image_t *image_binary(image_t* image, float threshold)
{
        if (image == NULL) {
                r_warn("image_binary: image is null");
                return NULL;
        }
        if (image->type != IMAGE_BW) {
                fprintf(stderr, "image_binary: not a BW image\n");
                return NULL;
        }

        int len = image->width * image->height;
        image_t *binary = new_image_bw(image->width, image->height);
        
        for (int i = 0; i < len; i++) {
                if (image->data[i] < threshold)
                        binary->data[i] = 0.0f;
                else
                        binary->data[i] = 1.0f;
        }

        return binary;
}

image_t *image_in_range_1(image_t* image, float *min, float *max)
{
        image_t *res = image_clone(image);
        int len = image->width * image->height;
        for (int i = 0; i < len; i++) {
                if (image->data[i] < min[0] || image->data[i] > max[0])
                        res->data[i] = 0.0f;
        }
        return res;
}

image_t *image_in_range_n(image_t* image, float *min, float *max)
{
        image_t *res = image_clone(image);
        int len = image->width * image->height;
        for (int i = 0, j = 0; i < len; i++, j += image->channels) {
                int in_range = 1;
                for (int c = 0; c < image->channels; c++) {
                        if (image->data[j+c] < min[c]
                            || image->data[j+c] > max[c]) {
                                in_range = 0;
                                break;
                        }
                }
                if (!in_range) {
                        for (int c = 0; c < image->channels; c++)
                                res->data[j+c] = 0.0;
                }
        }
        return res;
}

image_t *image_in_range(image_t* image, float *min, float *max)
{
        image_t *r = NULL;
        if (image->channels == 1)
                r = image_in_range_1(image, min, max);
        else
                r = image_in_range_n(image, min, max);
        return r;
}

void image_range_stats(image_t* image,
                       float *min, float *max,
                       int32_t *count,
                       float *cx, float *cy)
{
        int32_t n = 0;
        double x = 0.0;
        double y = 0.0;
        int32_t j = 0;
        for (int32_t yi = 0; yi < image->height; yi++) {
                for (int32_t xi = 0; xi < image->width; xi++) {
                        int in_range = 1;
                        for (int32_t c = 0; c < image->channels; c++) {
                                if (image->data[j+c] < min[c]
                                    || image->data[j+c] > max[c]) {
                                        in_range = 0;
                                        break;
                                }
                        }
                        if (in_range) {
                                n++;
                                x += (double) xi;
                                y += (double) yi;
                        }
                        j += image->channels;
                }
        }
        *count = n;
        if (n > 0) {
                *cx = (float) (x / (double) n);
                *cy = (float) (y / (double) n);
        }
}


// FIXME
image_t *FIXME_image_crop(image_t *image, int x, int y, int width, int height)
{
        if (image == NULL) {
                r_warn("image_circle: image is null");
                return NULL;
        }
        // FIXME: check boundaries! The rest of this code supposes that
        // the rectangle fits in the original image. Beware!
        image_t *cropped = new_image(image->type, width, height);
        for (int y_ = 0; y_ < height; y_++) {
                int len = cropped->width * cropped->channels * sizeof(float);
                int crop_offset = y_ * width * image->channels;
                int im_offset = ((y_ + y) * image->width + x) * image->channels;
                memcpy(&cropped->data[crop_offset], &image->data[im_offset], len); 
        }
        return cropped;
}

image_t *image_rotate(image_t *image, float xc, float yc, double radians)
{
        if (image == NULL) {
                r_warn("image_rotate: image is null");
                return NULL;
        }
        
        image_t *rot = new_image(image->type, image->width, image->height);

        float c = cosf(radians);
        float s = sinf(radians);
        
        for (int y = 0; y < image->height; y++) {
                for (int x = 0; x < image->width; x++) {
                        float x_ = (float) x - xc;
                        float y_ = (float) y - yc;
                        float xp = xc + x_ * c - y_ * s;
                        float yp = yc + x_ * s + y_ * c;

                        int x0 = (xp >= 0.0f)? (int) xp : -1 + (int) xp;
                        int x1 = x0 + 1;
                        float bx = xp - x0;
                        float ax = 1.0f - bx;
                        
                        int y0 = (yp >= 0.0f)? (int) yp : -1 + (int) yp;
                        int y1 = y0 + 1;
                        float by = yp - y0;
                        float ay = 1.0f - by;

                        if (image->channels == 3) {
                                float r00 = image_get(image, x0, y0, 0);
                                float r10 = image_get(image, x1, y0, 0);
                                float r01 = image_get(image, x0, y1, 0);
                                float r11 = image_get(image, x1, y1, 0);
                                
                                float r = (ax * ay * r00 + bx * ay * r10
                                           + ax * by * r01 + bx * by * r11);

                                
                                float g00 = image_get(image, x0, y0, 1);
                                float g10 = image_get(image, x1, y0, 1);
                                float g01 = image_get(image, x0, y1, 1);
                                float g11 = image_get(image, x1, y1, 1);
                                
                                float g = (ax * ay * g00 + bx * ay * g10
                                           + ax * by * g01 + bx * by * g11);

                                float b00 = image_get(image, x0, y0, 2);
                                float b10 = image_get(image, x1, y0, 2);
                                float b01 = image_get(image, x0, y1, 2);
                                float b11 = image_get(image, x1, y1, 2);
                                
                                float b = (ax * ay * b00 + bx * ay * b10
                                           + ax * by * b01 + bx * by * b11);
                                
                                int off = 3 * (y * rot->width + x);
                                rot->data[off] = r;
                                rot->data[off + 1] = g;
                                rot->data[off + 2] = b;
                        } else {
                                float v00 = image_get(image, x0, y0, 0);
                                float v10 = image_get(image, x1, y0, 0);
                                float v01 = image_get(image, x0, y1, 0);
                                float v11 = image_get(image, x1, y1, 0);
                                
                                float v = (ax * ay * v00 + bx * ay * v10
                                           + ax * by * v01 + bx * by * v11);
                                
                                rot->data[y * rot->width + x] = v;
                        }
                }
        }
        return rot;
}

// FIXME
image_t *FIXME_image_scale(image_t *image, int n)
{
        if (image == NULL) {
                r_warn("image_scale: image is null");
                return NULL;
        }
        int w = image->width / n;
        int h = image->height / n;
        image_t *scaled = new_image(image->type, w, h);

        for (int y = 0; y < h; y++) {
                int im_offset_y = n * y * image->width;
                int scaled_offset_y = y * scaled->width;
                for (int x = 0; x < w; x++) {
                        int im_offset_xy = (im_offset_y + n * x) * image->channels;
                        int scaled_offset_xy = (scaled_offset_y + x) * scaled->channels;
                        for (int channel = 0; channel < image->channels; channel++) {
                                scaled->data[scaled_offset_xy + channel] = image->data[im_offset_xy + channel];
                        }
                }
        }
        return scaled;
}


// FIXME
image_t *image_scale(image_t *image, int width, int height)
{
        if (image == NULL) {
                r_warn("image_scale: image is null");
                return NULL;
        }
        image_t *scaled = new_image(image->type, width, height);
        float scale_x = (float) image->width / width;
        float scale_y = (float) image->height / height;
        
        for (int y = 0; y < height; y++) {
                float yi = (float) y * scale_y;
                int y0 = (int) yi;
                int y1 = y0 + 1;
                float dy1 = yi - (int) y0;
                float dy0 = 1.0f - dy1;
                int im_offset_y0 = y0 * image->width;
                int im_offset_y1 = y1 * image->width;
                int scaled_offset_y = y * scaled->width;
                for (int x = 0; x < width; x++) {
                        float xi = (float) x * scale_x;
                        int x0 = (int) xi;
                        int x1 = x0 + 1;
                        float dx1 = xi - x0;
                        float dx0 = 1.0f - dx1;
                        int ix0y0 = (im_offset_y0 + x0) * image->channels;
                        int ix0y1 = (im_offset_y1 + x0) * image->channels;
                        int ix1y0 = (im_offset_y0 + x1) * image->channels;
                        int ix1y1 = (im_offset_y1 + x1) * image->channels;
                        int offset_xy = (scaled_offset_y + x) * scaled->channels;
                        
                        for (int channel = 0; channel < image->channels; channel++) {
                                // FIXME: dx1 * dy1 is out-of-range if x=width-1 and y=height-1
                                scaled->data[offset_xy + channel] =
                                        dx0 * dy0 * image->data[ix0y0 + channel]
                                        + dx0 * dy1 * image->data[ix0y1 + channel]
                                        + dx1 * dy0 * image->data[ix1y0 + channel]
                                        + dx1 * dy1 * image->data[ix1y1 + channel];
                        }
                }
        }
        return scaled;
}

int image_split_rgb(image_t *rgb_in, image_t **rgb_out)
{
        if (rgb_in == NULL) {
                r_warn("image_split_rgb: image is null");
                return -1;
        }
        memset(rgb_out, 0, 3 * sizeof(image_t *));
        
        if (rgb_in->type == IMAGE_RGB) {
                rgb_out[0] = new_image_bw(rgb_in->width, rgb_in->height);
                rgb_out[1] = new_image_bw(rgb_in->width, rgb_in->height);
                rgb_out[2] = new_image_bw(rgb_in->width, rgb_in->height);

                if (rgb_out[0] == NULL
                    || rgb_out[1] == NULL
                    || rgb_out[2] == NULL)
                        return -1;
                
                int len = rgb_in->width * rgb_in->height;
                for (int i = 0; i < len; i++) {
                        rgb_out[0]->data[i] = rgb_in->data[3*i];
                        rgb_out[1]->data[i] = rgb_in->data[3*i+1];
                        rgb_out[2]->data[i] = rgb_in->data[3*i+2];
                }
                
                return 0;
        }
        return -1;
}

image_t *image_convert_bw(image_t *image)
{
        if (image == NULL) {
                r_warn("image_convert_bw: image is null");
                return NULL;
        }
        image_t *bw = NULL;
        
        if (image->type == IMAGE_BW) {
                bw = image_clone(image);
        } else if (image->type == IMAGE_RGB) {
                bw = new_image_bw(image->width, image->height);
                int len = image->width * image->height;
                for (int i = 0, j = 0; i < len; i++) {
                        float r = image->data[j++];
                        float g = image->data[j++];
                        float b = image->data[j++];
                        float v = 0.2989f * r + 0.5870f * g + 0.1140f * b;
                        bw->data[i] = v;
                }
        } else if (image->type == IMAGE_HSV) {
                r_err("image_convert_bw: cannot convert a HSV image, yet");
        }  else {
                r_err("image_convert_bw: unknown image type");
        }
        return bw;
}

// https://stackoverflow.com/questions/3018313/algorithm-to-convert-rgb-to-hsv-and-hsv-to-rgb-in-range-0-255-for-both
static hsv_t rgb2hsv(rgb_t in)
{
    hsv_t out;
    float min, max, delta;

    min = in.r < in.g ? in.r : in.g;
    min = min  < in.b ? min  : in.b;

    max = in.r > in.g ? in.r : in.g;
    max = max  > in.b ? max  : in.b;

    out.v = max;                                // v
    delta = max - min;
    
    if (delta < 0.00001f) {
            out.s = 0;
            out.h = 0; // undefined, maybe nan?
            return out;
    }
    if (max > 0.0f) { // NOTE: if Max is == 0, this divide would cause a crash
            out.s = (delta / max);                  // s
    } else {
            // if max is 0, then r = g = b = 0              
            // s = 0, h is undefined
            out.s = 0.0f;
            out.h = NAN;                            // its now undefined
            return out;
    }
    if (in.r >= max)                           // > is bogus, just keeps compilor happy
            out.h = (in.g - in.b) / delta;        // between yellow & magenta
    else if (in.g >= max)
            out.h = 2.0f + ( in.b - in.r ) / delta;  // between cyan & yellow
    else
            out.h = 4.0f + ( in.r - in.g ) / delta;  // between magenta & cyan

    out.h *= 60.0f;                              // degrees

    if (out.h < 0.0f)
            out.h += 360.0f;

    return out;
}

// https://stackoverflow.com/questions/3018313/algorithm-to-convert-rgb-to-hsv-and-hsv-to-rgb-in-range-0-255-for-both
static rgb_t hsv2rgb(hsv_t in)
{
        float hh, p, q, t, ff;
        long i;
        rgb_t out;

        if (in.s <= 0.0f) {       // < is bogus, just shuts up warnings
                out.r = in.v;
                out.g = in.v;
                out.b = in.v;
                return out;
        }
    
        hh = in.h;
        if(hh >= 360.0f)
                hh = 0.0f;
        hh /= 60.0f;
        i = (long) hh;
        ff = hh - i;
        p = in.v * (1.0f - in.s);
        q = in.v * (1.0f - (in.s * ff));
        t = in.v * (1.0f - (in.s * (1.0f - ff)));

        switch(i) {
        case 0:
                out.r = in.v;
                out.g = t;
                out.b = p;
                break;
        case 1:
                out.r = q;
                out.g = in.v;
                out.b = p;
                break;
        case 2:
                out.r = p;
                out.g = in.v;
                out.b = t;
                break;

        case 3:
                out.r = p;
                out.g = q;
                out.b = in.v;
                break;
        case 4:
                out.r = t;
                out.g = p;
                out.b = in.v;
                break;
        case 5:
        default:
                out.r = in.v;
                out.g = p;
                out.b = q;
                break;
        }
        return out;     
}

image_t *image_convert_hsv(image_t *image)
{
        image_t *res = NULL;
        
        if (image->type == IMAGE_HSV) {
                res = image_clone(image);
        } else if (image->type == IMAGE_BW) {
                r_err("image_convert_hsv: cannot convert a BW image");
        } else if (image->type == IMAGE_RGB) {
                res = new_image_hsv(image->width, image->height);
                int len = image->width * image->height;
                rgb_t *in = (rgb_t *) image->data;
                hsv_t *out = (hsv_t *) res->data;
                for (int i = 0; i < len; i++)
                        out[i] = rgb2hsv(in[i]);
        }  else {
                r_err("image_convert_hsv: unknown image type");
        }
        return res;
}

image_t *image_convert_rgb(image_t *image)
{
        image_t *res = NULL;
        
        if (image->type == IMAGE_RGB) {
                res = image_clone(image);
        } else if (image->type == IMAGE_BW) {
                r_err("image_convert_rgb: cannot convert a BW image");
        } else if (image->type == IMAGE_HSV) {
                res = new_image_rgb(image->width, image->height);
                int len = image->width * image->height;
                hsv_t *in = (hsv_t *) image->data;
                rgb_t *out = (rgb_t *) res->data;
                for (int i = 0; i < len; i++)
                        out[i] = hsv2rgb(in[i]);
        }  else {
                r_err("image_convert_rgb: unknown image type");
        }
        return res;
}

image_t* image_excess_green(image_t* image)
{
        if (image == NULL) {
                r_warn("image_excess_green: image is null");
                return NULL;
        }
        if (image->type != IMAGE_RGB) {
                r_err("image_excess_green: not an RGB image");
                return NULL;
        }

        image_t* exg = new_image_bw(image->width, image->height);
        int len = image->width * image->height;

        float r_max = 0.0f, g_max = 0.0f, b_max = 0.0f;
        for (int i = 0, j = 0; i < len; i++) {
                float r = image->data[j++];
                float g = image->data[j++];
                float b = image->data[j++];
                if (r > r_max) r_max = r;
                if (g > g_max) g_max = g;
                if (b > b_max) b_max = b;
        }

        if (r_max == 0.0f)
                r_max = 1.0f;
        if (g_max == 0.0f)
                g_max = 1.0f;
        if (b_max == 0.0f)
                b_max = 1.0f;

        float exg_min = 2.0f;
        float exg_max = -2.0f;
        
        for (int i = 0, j = 0; i < len; i++) {
                float rn = image->data[j++] / r_max;
                float gn = image->data[j++] / g_max;
                float bn = image->data[j++] / b_max;

                if (0) {
                        exg->data[i] = 3.0f * gn / (rn + gn + bn) - 1.0f;
                } else {
                        float n = rn + gn + bn;
                        float r = rn / n;
                        float g = gn / n;
                        float b = bn / n;
                        float v = 2.0f * g - r - b;

                        if (v < exg_min) exg_min = v;
                        if (v > exg_max) exg_max = v;

                        exg->data[i] = v;
                }
        }
        
        float norm = exg_max - exg_min;
        for (int i = 0; i < len; i++) {
                exg->data[i] = (exg->data[i] - exg_min) / norm;
        }
        
        return exg;
}

int convert_to_jpeg(uint8_t* rgb, int width, int height, int quality, membuf_t *out)
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
                row_pointer[0] = (unsigned char *) rgb + offset;
                jpeg_write_scanlines(&cinfo, row_pointer, 1);
        }

        jpeg_finish_compress(&cinfo);
        jpeg_destroy_compress(&cinfo);

        return 0;
}

void image_import(image_t *image, uint8_t* rgb)
{
        float *p = image->data;
        int len = 3 * image->width * image->height;
        for (int i = 0; i < len; i++)
                *p++ = (float) *rgb++ / 255.0f;
}

image_t *convert_to_image(uint8_t* rgb, int width, int height)
{
        image_t *image = new_image_rgb(width, height);
        image_import(image, rgb);
        return image;
}

