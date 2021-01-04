/*
  romi-rover

  Copyright (C) 2019 Sony Computer Science Laboratories
  Author(s) Peter Hanappe

  romi-rover is collection of applications for the Romi Rover.

  romi-rover is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see
  <http://www.gnu.org/licenses/>.

 */

#include <stdexcept>
#include <jpeglib.h>
#include <png.h>
#include <setjmp.h>
#include <r.h>
#include "ImageIO.h"

namespace romi {

        bool ImageIO::store_jpg(Image& image, const char *filename)
        {
                struct jpeg_compress_struct cinfo;
                struct jpeg_error_mgr jerr;
                FILE* outfile;	 
                JSAMPLE* buffer;
        
                cinfo.err = jpeg_std_error(&jerr);
                jpeg_create_compress(&cinfo);

                if ((outfile = fopen(filename, "wb")) == NULL) {
                        r_err("Failed to open the file: %s\n", filename);
                        return false;
                }
                
                jpeg_stdio_dest(&cinfo, outfile);

                cinfo.image_width = image.width(); 
                cinfo.image_height = image.height();
                
                if (image.type() == Image::BW) {
                        cinfo.input_components = 1;	
                        cinfo.in_color_space = JCS_GRAYSCALE;
                } else { 
                        cinfo.input_components = 3;	
                        cinfo.in_color_space = JCS_RGB; // FIXME: HSV images!!!
                }
        
                jpeg_set_defaults(&cinfo);
                jpeg_set_quality(&cinfo, 90, TRUE);

                jpeg_start_compress(&cinfo, TRUE);

                buffer = (JSAMPLE*) r_alloc(image.channels() * image.width()); 

                size_t index = 0;
                float *data = image.data();
                
                while (cinfo.next_scanline < cinfo.image_height) {
                        if (image.type() == Image::BW) {
                                for (size_t i = 0; i < image.width(); i++)
                                        buffer[i] = (uint8_t) (data[index++] * 255.0f);
                        } else {
                                for (size_t i = 0, j = 0; i < image.width(); i++) {
                                        buffer[j++] = (uint8_t) (data[index++] * 255.0f);
                                        buffer[j++] = (uint8_t) (data[index++] * 255.0f);
                                        buffer[j++] = (uint8_t) (data[index++] * 255.0f);
                                }
                        }
                        jpeg_write_scanlines(&cinfo, &buffer, 1);
                }

                jpeg_finish_compress(&cinfo);
                fclose(outfile);
                
                jpeg_destroy_compress(&cinfo);
                r_free(buffer);

                return true;
        }
        
        bool ImageIO::store_png(Image& image, const char *filename)
        {
                png_structp png_ptr = NULL;
                png_infop info_ptr = NULL;
                size_t x, y, k;
                png_bytepp row_pointers;

                FILE *fp = fopen(filename, "wb");
                if (fp == NULL) {
                        r_err("ImageIO::store_png: Failed to open the file '%s'", filename);
                        return false;
                }

                png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
                if (png_ptr == NULL) {
                        r_err("ImageIO::store_png: png_create_write_struct failed");
                        return false;
                }
                
                info_ptr = png_create_info_struct(png_ptr);
                if (info_ptr == NULL) {
                        png_destroy_write_struct(&png_ptr, NULL);
                        r_err("ImageIO::store_png: png_create_info_struct failed");
                        return false;
                }
                
                if (setjmp(png_jmpbuf(png_ptr))) {
                        png_destroy_write_struct(&png_ptr, &info_ptr);
                        r_err("ImageIO::store_png: setjmp returned from error");
                        return false;
                }
                
                png_set_IHDR(png_ptr, info_ptr,
                             image.width(), image.height(), 
                             8, 
                             (image.type() == Image::BW)? PNG_COLOR_TYPE_GRAY : PNG_COLOR_TYPE_RGB, // FIXME: HSV images?
                             PNG_INTERLACE_NONE,
                             PNG_COMPRESSION_TYPE_DEFAULT,
                             PNG_FILTER_TYPE_DEFAULT);
                
                row_pointers = (png_bytepp) png_malloc(png_ptr, image.height()
                                                       * sizeof(png_bytep));
                
                for (y = 0; y < image.height(); y++) {
                        row_pointers[y] = (png_bytep) png_malloc(png_ptr,
                                                                 image.width()
                                                                 * image.channels());
                }

                float *data = image.data();
                
                if (image.type() == Image::BW) {
                        for (y = 0, k = 0; y < image.height(); y++) {
                                png_bytep row = row_pointers[y];
                                for (x = 0; x < image.width(); x++) {
                                        *row++ = (png_byte) (data[k++] * 255.0f);
                                } 
                        }
                        
                } else {
                        for (y = 0, k = 0; y < image.height(); y++) {
                                png_bytep row = row_pointers[y];
                                for (x = 0; x < image.width(); x++) {
                                        *row++ = (png_byte) (data[k++] * 255.0f);
                                        *row++ = (png_byte) (data[k++] * 255.0f);
                                        *row++ = (png_byte) (data[k++] * 255.0f);
                                }
                        }
                }
                
                png_init_io(png_ptr, fp);
                png_set_rows(png_ptr, info_ptr, row_pointers);
                png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);
                
                for (y = 0; y < image.height(); y++)
                        png_free(png_ptr, row_pointers[y]);
                
                png_free(png_ptr, row_pointers);
                png_destroy_write_struct(&png_ptr, &info_ptr);
                
                fclose(fp);
                
                return true;
        }

        bool ImageIO::load(Image& image, const char *filename)
        {
                bool success = false;
                
                if (is_jpg(filename)) {
                        success = load_jpg(image, filename);
                        
                } else if (is_png(filename)) {
                        success = load_png(image, filename);
                        
                } else {
                        r_warn("Unsupported image type: '%s'", filename);
                }
                
                return success;
        }
        
        bool ImageIO::is_png(const char *filename)
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
        
        bool ImageIO::is_jpg(const char *filename)
        {
                FILE *fp;
                unsigned char buf[3];

                if ((fp = fopen(filename, "rb")) == NULL)
                        return 0;
                ;
                if (fread(buf, 1, 3, fp) != 3) {
                        fclose(fp);
                        return 0;
                }
                
                fclose(fp);
                
                return buf[0] == 0xff && buf[1] == 0xd8 && buf[2] == 0xff;
        }

        
        static jmp_buf setjmp_buffer;

        static void exit_error(j_common_ptr cinfo __attribute__((unused)))
        {
                longjmp(setjmp_buffer, 1);
        }

        bool ImageIO::load_jpg(Image& image, const char *filename)
        {
                struct jpeg_error_mgr pub;
                struct jpeg_decompress_struct cinfo;
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
                        r_err("ImageIO::load_jpg: Failed to load the file: %s", filename);
                        return false;
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
                        image.init(Image::BW, cinfo.output_width, cinfo.output_height);
                        
                } else if (cinfo.output_components == 3
                           && cinfo.out_color_space == JCS_RGB) {
                        //fprintf(stderr, "24-bit RGB JPEG\n");
                        image.init(Image::RGB, cinfo.output_width, cinfo.output_height);
                        
                } else {
                        r_err("Unhandled JPEG format");
                        return false;
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
                        float *data = image.data();
                        
                        if (image.type() == Image::BW) {
                                offset = (cinfo.output_scanline - 1) * cinfo.output_width;
                                for (size_t i = 0; i < cinfo.output_width; i++, offset++) 
                                        data[offset] = (float) p[i] / 255.0f;
                        } else {
                                offset = 3 * (cinfo.output_scanline - 1) * cinfo.output_width;
                                for (size_t i = 0; i < cinfo.output_width; i++) {
                                        data[offset++] = (float) *p++ / 255.0f;
                                        data[offset++] = (float) *p++ / 255.0f;
                                        data[offset++] = (float) *p++ / 255.0f;
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

                return true;
        }

        bool ImageIO::load_png(Image& image, const char *filename)
        {
                size_t x, y, i, k;
                FILE *fp = NULL;
                png_structp png = NULL;
                png_infop info = NULL;
                png_bytep *row_pointers = NULL;
                bool success = false;
                png_byte color_type = 0;
                png_byte bit_depth = 0;
                size_t width = 0;
                size_t height = 0;
                float *data = 0;
                
                png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
                if (png == NULL) {
                        r_err("ImageIO::load_png: png_create_read_struct failed");
                        goto cleanup_and_exit;
                }

                if (setjmp(png_jmpbuf(png))) {
                        r_err("ImageIO::load_png: setjmp returned error");
                        return false;
                }

                info = png_create_info_struct(png);
                if (info == NULL) {
                        r_err("ImageIO::load_png: png_create_info_struct failed");
                        goto cleanup_and_exit;
                }
        
                fp = fopen(filename, "rb");
                if (fp == NULL) {
                        r_err("ImageIO::load_png: Failed to open the file '%s'", filename);
                        goto cleanup_and_exit;
                }
        
                png_init_io(png, fp);
                png_read_info(png, info);

                color_type = png_get_color_type(png, info);
                bit_depth = png_get_bit_depth(png, info);
                width = png_get_image_width(png, info);
                height = png_get_image_height(png, info);

                // Convert 16-bits to 8-bits 
                if (bit_depth == 16)
                        png_set_strip_16(png);

                // Convert < 8-bit gray scale to 8-bit 
                if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
                        png_set_expand_gray_1_2_4_to_8(png);

                if (color_type == PNG_COLOR_TYPE_GRAY) {
                        image.init(Image::BW, width, height);
                        
                } else if (color_type == PNG_COLOR_TYPE_RGB
                           || color_type == PNG_COLOR_TYPE_RGBA) {
                        image.init(Image::RGB, width, height);
                        
                } else {
                        r_err("Unsupported PNG image format");
                        goto cleanup_and_exit;
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

                data = image.data();
                
                if (color_type == PNG_COLOR_TYPE_GRAY) {
                        for (y = 0, k = 0; y < height; y++) {
                                png_bytep row = row_pointers[y];
                                for (x = 0; x < width; x++)
                                        data[k++] = (float) *row++ / 255.0f;
                        }
                } else if (color_type == PNG_COLOR_TYPE_RGB) {
                        for (y = 0, k = 0; y < height; y++) {
                                png_bytep row = row_pointers[y];
                                for (x = 0; x < width; x++) {
                                        data[k++] = (float) *row++ / 255.0f;
                                        data[k++] = (float) *row++ / 255.0f;
                                        data[k++] = (float) *row++ / 255.0f;
                                }
                        }
                } else if (color_type == PNG_COLOR_TYPE_RGBA) {
                        for (y = 0, k = 0; y < height; y++) {
                                png_bytep row = row_pointers[y];
                                for (x = 0, i = 0; x < width; x++, i += 4) {
                                        float alpha = (float) row[i+3] / 255.0f;
                                        data[k++] = alpha * (float) row[i] / 255.0f;
                                        data[k++] = alpha * (float) row[i+1] / 255.0f;
                                        data[k++] = alpha * (float) row[i+2] / 255.0f;
                                }
                        }
                }

                success = true;
                
        cleanup_and_exit:
                
                if (fp)
                        fclose(fp);
                
                if (row_pointers) {
                        for (y = 0; y < height; y++) 
                                if (row_pointers[y])
                                        r_free(row_pointers[y]);
                        r_free(row_pointers);
                }
                
                png_destroy_read_struct(&png, &info, NULL);
        
                return success;
        }

        bool ImageIO::load_jpg(Image& image, const uint8_t *data, size_t len)
        {
                struct jpeg_error_mgr pub;
                struct jpeg_decompress_struct cinfo;
                JSAMPARRAY buffer;
                int row_stride;
                
                cinfo.err = jpeg_std_error(&pub);
                pub.error_exit = exit_error;

                if (setjmp(setjmp_buffer)) {
                        jpeg_destroy_decompress(&cinfo);
                        r_err("Failed to load the data. Not a JPEG?");
                        return false;
                }

                jpeg_create_decompress(&cinfo);
                jpeg_mem_src(&cinfo, data, len);
                jpeg_read_header(&cinfo, TRUE);
                jpeg_start_decompress(&cinfo);

                if (cinfo.output_components == 1
                    && cinfo.jpeg_color_space == JCS_GRAYSCALE) {
                        r_debug("8-bit grayscale JPEG");
                        image.init(Image::BW, cinfo.output_width, cinfo.output_height);
                        
                } else if (cinfo.output_components == 3
                           && cinfo.out_color_space == JCS_RGB) {
                        r_debug("24-bit RGB JPEG");
                        image.init(Image::RGB, cinfo.output_width, cinfo.output_height);
                        
                } else {
                        r_err("load_jpg: Unhandled JPEG format");
                        return false;
                }

                row_stride = cinfo.output_width * cinfo.output_components;
                buffer = (*cinfo.mem->alloc_sarray)
                        ((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

                
                while (cinfo.output_scanline < cinfo.output_height) {
                        jpeg_read_scanlines(&cinfo, buffer, 1);

                        unsigned int offset;
                        unsigned char* p = buffer[0];
                        float *img = image.data();
                        
                        if (image.type() == Image::BW) {
                                offset = (cinfo.output_scanline - 1) * cinfo.output_width;
                                for (size_t i = 0; i < cinfo.output_width; i++, offset++) 
                                        img[offset] = (float) p[i] / 255.0f;
                                
                        } else {
                                offset = 3 * (cinfo.output_scanline - 1) * cinfo.output_width;
                                for (size_t i = 0; i < cinfo.output_width; i++) {
                                        img[offset++] = (float) *p++ / 255.0f;
                                        img[offset++] = (float) *p++ / 255.0f;
                                        img[offset++] = (float) *p++ / 255.0f;
                                }
                        }
                }

                jpeg_finish_decompress(&cinfo);
                jpeg_destroy_decompress(&cinfo);
                
                return true;
        }


#define BLOCKSIZE 4096

        typedef struct _jpeg_dest_t {
                struct jpeg_destination_mgr mgr;
                bytevector *buffer;
        } jpeg_dest_t;


        static void jpeg_bufferinit(j_compress_ptr cinfo)
        {
                jpeg_dest_t* p = (jpeg_dest_t*) cinfo->dest;
                p->buffer->resize(BLOCKSIZE);
                cinfo->dest->next_output_byte = &p->buffer->at(0);
                cinfo->dest->free_in_buffer = BLOCKSIZE;                
        }

        static boolean jpeg_bufferemptyoutput(j_compress_ptr cinfo)
        {
                jpeg_dest_t* p = (jpeg_dest_t*) cinfo->dest;
                size_t size = p->buffer->size();
                p->buffer->resize(size + BLOCKSIZE);
                cinfo->dest->next_output_byte = &p->buffer->at(0) + size;
                cinfo->dest->free_in_buffer = BLOCKSIZE;
                return 1;
        }

        static void jpeg_bufferterminate(j_compress_ptr cinfo)
        {
                jpeg_dest_t* p = (jpeg_dest_t*) cinfo->dest;
                size_t size = p->buffer->size() - cinfo->dest->free_in_buffer;
                p->buffer->resize(size);
        }

        bool ImageIO::store_jpg(Image& image, bytevector& out)
        {
                struct jpeg_compress_struct cinfo;
                struct jpeg_error_mgr jerr;
                jpeg_dest_t* my_mgr;
                JSAMPLE* buffer;
                int index = 0;

                cinfo.err = jpeg_std_error(&jerr);
                jpeg_create_compress(&cinfo);

                cinfo.dest = (struct jpeg_destination_mgr *) 
                        (*cinfo.mem->alloc_small) ((j_common_ptr) &cinfo, JPOOL_PERMANENT,
                                                   sizeof(jpeg_dest_t));       
                cinfo.dest->init_destination = &jpeg_bufferinit;
                cinfo.dest->empty_output_buffer = &jpeg_bufferemptyoutput;
                cinfo.dest->term_destination = &jpeg_bufferterminate;

                my_mgr = (jpeg_dest_t*) cinfo.dest;
                my_mgr->buffer = &out;

                cinfo.image_width = image.width();	
                cinfo.image_height = image.height();
                if (image.type() == Image::BW) {
                        cinfo.input_components = 1;	
                        cinfo.in_color_space = JCS_GRAYSCALE;
                } else {
                        cinfo.input_components = 3;	
                        cinfo.in_color_space = JCS_RGB;
                }

                jpeg_set_defaults(&cinfo);
                jpeg_set_quality(&cinfo, 90, TRUE);

                jpeg_start_compress(&cinfo, TRUE);

                buffer = (JSAMPLE*) r_alloc(image.channels() * image.width()); 

                float *data = image.data();
                
                while (cinfo.next_scanline < cinfo.image_height) {
                        if (image.type() == Image::BW) {
                                for (size_t i = 0; i < image.width(); i++)
                                        buffer[i] = (uint8_t) (data[index++] * 255.0f);
                        } else {
                                for (size_t i = 0, j = 0; i < image.width(); i++) {
                                        buffer[j++] = (uint8_t) (data[index++] * 255.0f);
                                        buffer[j++] = (uint8_t) (data[index++] * 255.0f);
                                        buffer[j++] = (uint8_t) (data[index++] * 255.0f);
                                }
                        }
                        jpeg_write_scanlines(&cinfo, &buffer, 1);
                }

                jpeg_finish_compress(&cinfo);

                jpeg_destroy_compress(&cinfo);
                r_free(buffer);

                return true;
        }
}
