#ifndef LETTUCETHINK_CONVERT_H
#define LETTUCETHINK_CONVERT_H

#ifdef __cplusplus
extern "C" {
#endif

/* int convert_to_jpeg(const void* image, int width, int height, int quality, membuf_t *out); */
int convert_to_png_16bit_grayscale(const uint16_t* image, int width, int height, membuf_t *out);
        
#ifdef __cplusplus
}
#endif

#endif // LETTUCETHINK_CONVERT_H
