#include <stdexcept>
#include <r.h>
#include <picamera/PiCamera.h>
#include <picamera/PiCameraSettings.h>

static bool quit = false;
static void set_quit(int sig, siginfo_t *info, void *ucontext);
static void quit_on_control_c();

void store_jpeg(rpp::MemBuffer& jpeg, const char *name_prefix, int index)
{
        char filename[512];
        snprintf(filename, sizeof(filename), "%s_%04d.jpg", name_prefix, index);
        r_info("File %s, Length %d", filename, (int) jpeg.size());
        
        int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
        if (fd >= 0) {
                write(fd, jpeg.data().data(), jpeg.size());
                fsync(fd);
                close(fd);
        } else {
                r_err("Failed to write to %s", filename);
        }
}

int main()
{
        try {
                romi::V2CameraSettings settings(romi::kV2HalfWidth,
                                                romi::kV2HalfHeight);
                romi::PiCamera camera(settings);

                quit_on_control_c();
                
                for (int i = 0; !quit; i++) {
                        rpp::MemBuffer& jpeg = camera.grab_jpeg();
                        store_jpeg(jpeg, "test", i);
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
