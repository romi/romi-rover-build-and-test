#include <RSerial.h>
#include <RomiSerialClient.h>

int main()
{
        RSerial serial("/dev/ttyACM0", 115200, 0);
        RomiSerialClient romiserial(&serial, &serial); 
        json_object_t response;
        int successes = 0;
        int errors = 0;
        int num_loops = 1024;
        
        for (int i = 0; i < num_loops; i++) {
                
                response = romiserial.send("a");
                if (json_array_getnum(response, 0) == 0)
                        successes++;
                else
                        errors++;
                json_unref(response);

                // Error: bad number of arguments
                response = romiserial.send("b");
                if (json_array_getnum(response, 0) == 0)
                        successes++;
                else
                        errors++;
                json_unref(response);

                response = romiserial.send("b[1,2]");
                if (json_array_getnum(response, 0) == 0)
                        successes++;
                else
                        errors++;
                json_unref(response);
                
                response = romiserial.send("c[1,\"Dinner's ready\"]");
                if (json_array_getnum(response, 0) == 0)
                        successes++;
                else
                        errors++;
                json_unref(response);
                
                response = romiserial.send("d[1,1]");
                if (json_array_getnum(response, 0) == 0)
                        successes++;
                else
                        errors++;
                json_unref(response);
                
                response = romiserial.send("e[\"he's resting\"]");
                if (json_array_getnum(response, 0) == 0)
                        successes++;
                else
                        errors++;
                json_unref(response);
                
                // Always error
                response = romiserial.send("f");
                if (json_array_getnum(response, 0) == 0)
                        successes++;
                else
                        errors++;
                json_unref(response);
        }

        int r;
        if (successes == 5 * num_loops
            && errors == 2 * num_loops) {
                printf("OK\n");
                r = 0;
        } else {
                printf("FAIL (%d, %d)\n", successes, errors);
                r = 1;
        }
        return r;
}
