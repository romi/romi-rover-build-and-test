#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jpeglib.h>
#include <png.h>
#include <setjmp.h>
#include <math.h>

#include <r.h>
#include "romi/image.h"

image_t *new_image(int type, int width, int height)
{
        image_t *image = r_new(image_t);
        if (image == NULL) return NULL;

        image->type = type;
        image->channels = (type == IMAGE_BW)? 1 : 3;
        image->width = width;
        image->height = height;
        image->data = (float *) malloc(image->channels * width * height * sizeof(float));
        if (image->data == NULL) {
                free(image);
                return NULL;
        }
        memset(image->data, 0, image->channels * width * height * sizeof(float));
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

void delete_image(image_t *image)
{
        if (image) {
                if (image->data)
                        free(image->data);
                r_delete(image);
        }
}

image_t *image_clone(image_t *im)
{
        image_t *image = (image_t *) malloc(sizeof(image_t));
        if (image == NULL) return NULL;

        image->type = im->type;
        image->channels = im->channels;
        image->width = im->width;
        image->height = im->height;
        image->data = (float *) malloc(im->channels * im->width * im->height * sizeof(float));
        if (image->data == NULL) {
                free(image);
                return NULL;
        }
        memcpy(image->data, im->data, im->channels * im->width * im->height * sizeof(float));
        return image;
}

void image_clear(image_t *image)
{
        memset(image->data, 0, image->channels * image->width * image->height);
}

void image_fill(image_t *image, int channel, float color)
{
        int stride = image->channels;
        int len = image->channels * image->width * image->height;
        for (int i = channel; i < len; i += stride)
                image->data[i] = color;
}

void image_offset(image_t *image, float offset, int channel)
{
        int stride = image->channels;
        int len = image->channels * image->width * image->height;
        for (int i = channel; i < len; i += stride)
                image->data[i] += offset;
}

void image_circle(image_t *image, float xc, float yc, float radius, float* color)
{
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

static void exit_error(j_common_ptr cinfo)
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
        
        cinfo.err = jpeg_std_error(&jerr);
        jpeg_create_compress(&cinfo);

        if ((outfile = fopen(filename, "wb")) == NULL) {
                fprintf(stderr, "Failed to open the file: %s\n", filename);
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

        buffer = (JSAMPLE*) malloc(image->channels * image->width); 
        if (buffer == NULL) {
                fprintf(stderr, "Out of memory\n");
                return -1;
        }

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
        free(buffer);

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

        if (membuf_assure(membuf, BLOCKSIZE) != 0)
                return 0;
        
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

        buffer = (JSAMPLE*) malloc(image->channels * image->width); 
        if (buffer == NULL) {
                fprintf(stderr, "Out of memory\n");
                return -1;
        }

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
        free(buffer);

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

void flush_png_data(png_structp png_ptr)
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
        char buf[4];

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
        if (image->type != IMAGE_BW) {
                fprintf(stderr, "image_otsu_threshold: not a BW image\n");
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


// FIXME
image_t *FIXME_image_crop(image_t *image, int x, int y, int width, int height)
{
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
        if (image->type == IMAGE_BW) {
                return image_clone(image);
        }
        image_t *bw = new_image_bw(image->width, image->height);
        if (bw == NULL) {
                // FIXME
                return NULL;
        }
        int len = image->width * image->height;
        for (int i = 0, j = 0; i < len; i++) {
                float r = image->data[j++];
                float g = image->data[j++];
                float b = image->data[j++];
                float v = 0.2989f * r + 0.5870f * g + 0.1140f * b;
                bw->data[i] = v;
        }
        return bw;
}

image_t* image_excess_green(image_t* image)
{
        if (image->type != IMAGE_RGB) {
                fprintf(stderr, "image_excess_green: not a RGB image\n");
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

image_t *convert_to_image(uint8_t* rgb, int width, int height)
{
        image_t *image = new_image_rgb(width, height);
        if (image == NULL)
                return NULL;
        float *p = image->data;
        int len = width * height;
        for (int i = 0; i < len; i++)
                *p++ = (float) *rgb / 255.0f;
        return image;
}

