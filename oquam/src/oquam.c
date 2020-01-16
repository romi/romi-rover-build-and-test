#include <r.h>
#include "action.h"
#include "script.h"
#include "controller.h"
#include "cnc.h"
#include "virt.h"
#include "ostepper.h"
#include "v.h"

 
int use_virtual_controller = 0;

static void trigger(void *userdata, cnc_t *cnc, int16_t arg)
{
        printf("*** trigger %d ***\n", (int) arg);
}


int main(int argc, char **argv)
{
        controller_t *controller;
        cnc_t *cnc;
        
        r_log_init();
        r_log_set_app("oquam");


        double xmax[3] = { 0.6, 0.6, 0.4 };
        double steps[3] = { 200.0, 200.0, 200.0 };
        //double gears[3] = { 1.0, 1.0, 1.0 };
        double gears[3] = { 15.0, 15.0, 15.0 };
        double rpm[3] = { 300.0, 300.0, 300.0 };
        double displacement[3] = { 0.04, 0.04, 0.002 };
        double microsteps[3] = { 8.0, 8.0, 1.0 };
        
        double vmax[3];
        double scale[3];

        for (int i = 0; i < 3; i++) {
                vmax[i] = displacement[i] * rpm[i] / 60.0 / gears[i];
                scale[i] = gears[i] * microsteps[i] * steps[i] / displacement[i];
        }

        // XCarve: values taken from grbl/defaults.h.  
        // 
        // XY: 500.0*60*60 mm/min² = 500 mm/s² = 0.5 m/s²
        // Z:  50.0*60*60 mm/min² = 500 mm/s² = 0.05 m/s²
        //double amax[3] = { 0.5, 0.5, 0.05 };
        double amax[3] = { 0.3, 0.3, 0.03 };

        vdiv(amax, amax, gears);
        
        double period = 0.014;

        if (use_virtual_controller) 
                controller = new_virtual_stepper_controller("test1",
                                                            xmax, vmax, amax,
                                                            scale, period);
        else
                controller = new_oquam_stepper_controller("/dev/ttyACM0",
                                                          xmax, vmax, amax,
                                                          scale, period);
        if (controller == NULL) {
                r_err("Failed to create the virtual controller");
                return 1;
        }
        
        cnc = new_cnc(controller);
        if (cnc == NULL) {
                r_err("Failed to create the CNC");
                return 1;
        }


        ///
        
        if (0) {
                cnc_begin_script(cnc, 0.001);
                double xc = controller->xmax[0] / 2.0;
                double yc = controller->xmax[1] / 2.0;
                cnc_move(cnc, xc, yc, 0.0, 0.1);
                cnc_move(cnc, 2 * xc, 0.0, 0.0, 0.1);
                cnc_end_script(cnc);
                cnc_run_script(cnc, 0);
        }
        

        ///
        
        if (0) {
                cnc_begin_script(cnc, 0.001);
                double dx = 0.2 * controller->xmax[0];
                double dy = 0.2 * controller->xmax[1];
                cnc_move(cnc, dx, 0.0, 0.0, 0.1);
                cnc_move(cnc, dx,  dy, 0.0, 0.1);
                cnc_move(cnc, 0.0, dy, 0.0, 0.1);
                cnc_end_script(cnc);
                cnc_run_script(cnc, 0);
        }
        
        ///

        // Circle around the center of the workspace
        if (1) {
                cnc_begin_script(cnc, 0.001);
                double xc = controller->xmax[0] / 2.0;
                double yc = controller->xmax[1] / 2.0;
                double r = xc * 0.6;
                int segments = 36;
                //printf("xc=%f, yc=%f, r=%f\n", xc, yc, r);
                for (int i = 0; i < segments; i++) {
                        double angle = 2.0 * M_PI * i / segments;
                        double x = xc - r * cosf(angle);
                        double y = xc + r * sinf(angle);
                        cnc_move(cnc, x, y, 0, 0.15);
                        //printf("%f, %f\n", x, y);
                        cnc_trigger(cnc, trigger, NULL, i);
                }
                cnc_move(cnc, 0, 0, 0, 0.15);
                cnc_trigger(cnc, trigger, NULL, 36);
                cnc_end_script(cnc);
                cnc_run_script(cnc, 0);                
        }

        ///
                
        if (0) {
                cnc_begin_script(cnc, 0.001);
                double dx = 0.2 * controller->xmax[0];
                double dy = 0.2 * controller->xmax[1];
                cnc_move(cnc, dx, 0.0, 0.0, 0.1);
                cnc_move(cnc, dx,  dy, 0.0, 0.1);
                cnc_move(cnc, 0.0, dy, 0.0, 0.1);
                //cnc_move(cnc, 0.0, 0.0, 0.0, 0.1);
                cnc_end_script(cnc);
                cnc_run_script(cnc, 0);                
        }

        ///
                
        if (0) {
                cnc_begin_script(cnc, 0.001);
                for (int i = 1; i < 4; i++) {
                        cnc_move(cnc, i * 0.1, 0.0, 0.0, 0.1);
                        cnc_move(cnc, i * 0.1, i * 0.1, 0.0, 0.1);
                        cnc_move(cnc, 0.0, i * 0.1, 0.0, 0.1);
                        cnc_move(cnc, 0.0, 0.0, 0.0, 0.1);
                }
                cnc_end_script(cnc);
                cnc_run_script(cnc, 0);
        }

        
        ///
        
        if (0) {
                cnc_begin_script(cnc, 0.001);
                for (int i = 1; i < 7; i++) {
                        cnc_move(cnc, i * 0.1, i * 0.1, 0.0, 0.07);
                        cnc_move(cnc, 0.0, 0.0, 0.0, 0.07);
                }
                cnc_end_script(cnc);
                cnc_run_script(cnc, 0);
        }

        ///
                
        if (0) {
                cnc_begin_script(cnc, 0.001);
                double xc = controller->xmax[0] / 2.0;
                double yc = controller->xmax[1] / 2.0;
                cnc_move(cnc, xc, yc, 0.0, 0.1);
                cnc_move(cnc, 2 * xc, 0.0, 0.0, 0.1);
                cnc_end_script(cnc);
                cnc_run_script(cnc, 0);
        }
        

        return 0;
}
