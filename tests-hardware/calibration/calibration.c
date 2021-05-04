#include <string.h>
#include <romi.h>
#include <rcom.h>

typedef struct {
        double x, y;
} xy_t;

static int n_measures = 0;
static xy_t cnc[32];
static xy_t ball[32];
static double z0 = -0.15f;
static float min[] = { 130.0f * 2.0f, 197.0f / 255.0f, 109.0f / 255.0f};
static float max[] = { 154.0f * 2.0f, 255.0f / 255.0f, 255.0f / 255.0f};
static int min_pixel_count = 400;
static messagelink_t *messagelink_cnc = NULL;

static int status_error(json_object_t reply, membuf_t *message)
{
        int r = -1;
        
        const char *status = json_object_getstr(reply, "status");
        if (status == NULL) {
                membuf_printf(message, "Invalid status message");
                
        } else if (rstreq(status, "ok")) {
                r = 0;
                
        } else if (rstreq(status, "error")) {
                const char *s = json_object_getstr(reply, "message");
                membuf_printf(message, "%s", s);
        }
        return r;
}

static int moveto(double x, double y, double z)
{
        int r = 0;
        json_object_t reply;
        membuf_t *message = new_membuf();
        
        reply = messagelink_send_command_f(messagelink_cnc,
                                           "{\"command\": \"moveto\", "
                                           "\"x\": %0.3f, \"y\": %0.3f, \"z\": %0.3f}",
                                           x, y, z);
        if (status_error(reply, message) != 0) {
                r_err("moveto returned error: %s", membuf_data(message));
                r = -1;
        }
        
        delete_membuf(message);
        return r;
}

static image_t *grab_camera()
{
        image_t *image = NULL;        
        response_t *response = NULL;
        
        int err = client_get_data("camera", "camera.jpg", &response);
        if (err == 0) {
                membuf_t *body = response_body(response);
                image_t *image = image_load_from_mem((const unsigned char *)membuf_data(body),
                                                     membuf_len(body));
                if (image == NULL)
                        r_err("Failed to decompress the image");
        } else {
                r_err("Failed to obtain the camera image");
        }
        
        delete_response(response);
        
        return image;
}

static int init_com()
{
        int r = 0;
        messagelink_cnc = registry_open_messagelink("weeder", "cnc", (messagelink_onmessage_t) NULL, NULL);
        if (messagelink_cnc == NULL) {
                r_err("Failed to create the messagelink");
                r = -1;
        };
        return r;
}

static void add_measure(int count, double cnc_x, double cnc_y, double ball_x, double ball_y)
{
        printf("n=%d, cnc(%0.3f, %0.3f) -> ball(%0.3f, %0.3f)\n",
               count, cnc_x, cnc_y, ball_x, ball_y);
        if (n_measures < 32) {
                cnc[n_measures].x = cnc_x;
                cnc[n_measures].y = cnc_y;
                ball[n_measures].x = ball_x;
                ball[n_measures].y = ball_y;
                n_measures++;
        }
}

static void detect_ball(image_t *image, double x, double y)
{
        int32_t count;
        float ball_x, ball_y;
        
        image_t *hsv = image_convert_hsv(image);
        image_range_stats(hsv, min, max, &count, &ball_x, &ball_y);
                        
        if (count > min_pixel_count)
                add_measure(count, x, y, (double) ball_x, (double) ball_y);
        
        delete_image(hsv);
}

static int detect_ball_at(double x, double y, double z)
{
        int r = -1;
        
        if (moveto(x, y, z0) != 0) {
                r_err("moveto failed");
        } else {
                image_t *image = grab_camera();
                if (image == NULL) {
                        r_err("grab_camera failed");
                } else {
                        detect_ball(image, x, y);
                        delete_image(image);
                        r = 0;
                }
        }
        
        return r;
}

static void regression(xy_t *xy_cnc, xy_t *xy_ball, int len, double *a, double *b)
{
        a[0] = 1.0;
        a[1] = 2.0;
        b[0] = 3.0;
        b[1] = 4.0;
}

static double compute_error(xy_t *xy_cnc, xy_t *xy_ball, int len, double *a, double *b)
{
        return 0.0;
}

int main(int argc, char **argv)
{
        int err = 0;

        app_init(&argc, argv);
        
        err = init_com();
        if (err != 0) {
                r_err("init() failed");
                app_set_quit();
        }

        if (moveto(0.0f, 0.0f, z0) != 0) {
                r_err("moveto failed");
                app_set_quit();
        } else {
                
                for (double x = 0.0f; x <= 0.56f; x += 0.08f) {
                        for (double y = 0.0f; y <= 0.560f; y += 0.08f) {
                                if (detect_ball_at(x, y, z0) != 0)
                                        break;
                        }
                }

                if (n_measures < 4) {
                        r_err("too few measurements (%d) to make a good estimate", n_measures);
                } else {
                        double a[2];
                        double b[2];
                        regression(cnc, ball, n_measures, a, b);
                        printf("{'a': [%f, %f], 'b': [%f, %f]}\n", a[0], a[1], b[0], b[1]);

                        double e = compute_error(cnc, ball, n_measures, a, b);
                        printf("Error: %f\n", e);
                }
        }
        
        return 0;
}
