#include <stdexcept>
#include <r.h>
#include <RPCServer.h>
#include <picamera/PiCamera.h>
#include <picamera/PiCameraSettings.h>
#include <rpc/CameraAdaptor.h>

static bool quit = false;
static void set_quit(int sig, siginfo_t *info, void *ucontext);
static void quit_on_control_c();

int main()
{
        try {
                romi::V2CameraSettings settings(romi::kV2HalfWidth,
                                                romi::kV2HalfHeight);
                romi::PiCamera camera(settings);
                romi::CameraAdaptor adaptor(camera);
                rcom::RPCServer server(adaptor, "notused", "camera");

                quit_on_control_c();
                
                while (!quit) {
                        server.handle_events();
                        clock_sleep(0.050);
                }

                

        } catch (std::exception& e) {
                r_err("RomiCamera: caught exception: %s", e.what());
        }
}

static void set_quit(int sig, siginfo_t *info, void *ucontext)
{
        (void) sig;
        (void) info;
        (void) ucontext;
        quit = true;
}

static void quit_on_control_c()
{
        struct sigaction act;
        memset(&act, 0, sizeof(struct sigaction));

        act.sa_flags = SA_SIGINFO;
        act.sa_sigaction = set_quit;
        if (sigaction(SIGINT, &act, nullptr) != 0) {
                perror("init_signal_handler");
                exit(1);
        }
}
