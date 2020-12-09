#include <string.h>
#include <romi.h>
#include <rcom.h>

static float min[] = { 130.0f * 2.0f, 197.0f / 255.0f, 109.0f / 255.0f};
static float max[] = { 154.0f * 2.0f, 255.0f / 255.0f, 255.0f / 255.0f};

int main(int argc, char **argv)
{
        for (int n = 1; n < argc; n++) {
                const char *filename = argv[n];
                
                image_t *im = image_load(filename);
                image_t *hsv = image_convert_hsv(im);


                int32_t count;
                float cx;
                float cy;
                image_range_stats(hsv, min, max, &count, &cx, &cy);

                if (count > 400) {
                        printf("%s: n=%d, c=(%f, %f)\n", filename, count, cx, cy);
                }
                
                if (0) {
                        image_t *mask = image_in_range(hsv, min, max);
                        image_t *rgb = image_convert_rgb(mask);
                        image_store(rgb, "output.jpg", "jpg");
                        delete_image(mask);
                        delete_image(rgb);
                }
        
                delete_image(im);
                delete_image(hsv);
        }
        
        return 0;
}
