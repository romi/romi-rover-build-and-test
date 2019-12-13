#include "otsu.h"
#include "profiling.h"

static void delete_otsu_module(segmentation_module_t *module);

// see http://www.labbookpages.co.uk/software/imgProc/otsuThreshold.html
static float image_otsu_threshold(fileset_t *fileset, image_t* image)
{
        if (image->type != IMAGE_BW) {
                r_err("image_otsu_threshold: not a BW image");
                return 0.0f;
        }

        // Calculate histogram
        int histogram[256];
        int histogram_length = 256;
        int len = image->width * image->height;

        memset(histogram, 0, sizeof(histogram));
        
        for (int i = 0; i < len; i++) {
                int bin = (int) roundf(image->data[i] * 255.0f);
                if (bin >= 0 && bin < histogram_length)
                        histogram[bin]++;
        }
        
        int total = image->width * image->height;
        float sum = 0.0f;
        for (int i = 0 ; i < histogram_length; i++)
                sum += i * histogram[i];

        if (0) {
                membuf_t *buf = new_membuf();
                for (int t = 0; t < histogram_length; t++)
                        membuf_printf(buf, "%d\t%f\n", t, histogram[t] / (double) total);
                store_text(fileset, "exg-histogram", buf);
                delete_membuf(buf);
        }
        
        float sumB = 0;
        int wB = 0;
        int wF = 0;

        float var_max = 0;
        int threshold = 0;

        for (int t = 0; t < histogram_length; t++) {
                wB += histogram[t];               // Weight Background
                if (wB == 0) continue;

                wF = total - wB;                 // Weight Foreground
                if (wF == 0) break;

                sumB += (float) (t * histogram[t]);

                float mB = sumB / wB;            // Mean Background
                float mF = (sum - sumB) / wF;    // Mean Foreground

                // Calculate Between Class Variance
                float var_between = (float) wB * (float) wF * (mB - mF) * (mB - mF);

                // Check if new maximum found
                if (var_between > var_max) {
                        var_max = var_between;
                        threshold = t;
                }
        }

        return (float) threshold / 255.0f;
}

static image_t *otsu_segmentation_function(segmentation_module_t *module,
                                           image_t *in,
                                           fileset_t *fileset,
                                           membuf_t *message)
{
        if (in->type != IMAGE_RGB) {
                r_err("Expected an RGB input image");
                return NULL;
        }
        
        image_t* exg = image_excess_green(in);
        store_png(fileset, "excess-green", exg);

        float threshold = image_otsu_threshold(fileset, exg);
        
        image_t *mask = image_binary(exg, threshold);
        store_png(fileset, "mask", mask);

        delete_image(exg);
        return mask;
}

segmentation_module_t *new_otsu_module()
{
        segmentation_module_t *module; 
        module = r_new(segmentation_module_t);
        if (module == NULL)
                return NULL;
        module->segment = otsu_segmentation_function;
        module->cleanup = delete_otsu_module;
        return module;
}

void delete_otsu_module(segmentation_module_t *module)
{
        if (module)
                r_delete(module);
}
