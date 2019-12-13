#include <r.h>
#include "action.h"
#include "script.h"
#include "controller.h"
#include "cnc.h"
#include "virt.h"
#include "ostepper.h"
#include "v.h"

int main(int argc, char **argv)
{
        controller_t *controller;
        cnc_t *cnc;
        
        r_log_init();
        r_log_set_app("oquam");


        double xmax[3] = { 0.7, 0.7, 0.4 };
        double steps[3] = { 200.0, 200.0, 200.0 };
        double rpm[3] = { 600.0, 600.0, 600.0 };
        double displacement[3] = { 0.04, 0.04, 0.002 };

        
        double vmax[3];
        double scale[3];

        for (int i = 0; i < 3; i++) {
                vmax[i] = displacement[i] * rpm[i] / 60.0;
                scale[i] = steps[i] / displacement[i];
        }

        // XCarve: values taken from grbl/defaults.h.  
        // 
        // XY: 500.0*60*60 mm/min² = 500 mm/s² = 0.5 m/s²
        // Z:  50.0*60*60 mm/min² = 500 mm/s² = 0.05 m/s²
        double amax[3] = { 0.5, 0.5, 0.05 };
        
        double period = 0.010;

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

        double xc = controller->xmax[0] / 2.0;
        double yc = controller->xmax[1] / 2.0;
        double r = xc * 0.9;
        int segments = 36;

        cnc_begin_script(cnc, 0.001);
        
        for (int i = 0; i < segments; i++) {
                double angle = 2.0 * M_PI * i / segments;
                double x = xc + r * cosf(angle);
                double y = xc + r * sinf(angle);
                cnc_move(cnc, x, y, 0, 0.3);
        }
        
        cnc_end_script(cnc);
        cnc_run_script(cnc, 0);

        return 0;
}
