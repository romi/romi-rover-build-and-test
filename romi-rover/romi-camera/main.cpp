#include <stdexcept>
#include <r.h>
#include <RegistryServer.h>
#include <picamera/PiCamera.h>
#include <picamera/PiCameraSettings.h>
#include <rpc/CameraAdaptor.h>
#include <rpc/RcomServer.h>

static bool quit = false;
static void set_quit(int sig, siginfo_t *info, void *ucontext);
static void quit_on_control_c();

int main(int argc, char **argv)
{
        if (argc >= 2) {
                r_info("Using registry IP %s", argv[1]);
                rcom::RegistryServer::set_address(argv[1]);
        }
        
        try {
                romi::V2CameraSettings settings(romi::kV2HalfWidth,
                                                romi::kV2HalfHeight);
                romi::PiCamera camera(settings);
                romi::CameraAdaptor adaptor(camera);
                auto server = romi::RcomServer::create("camera", adaptor);

                quit_on_control_c();
                
                while (!quit) {
                        server->handle_events();
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
