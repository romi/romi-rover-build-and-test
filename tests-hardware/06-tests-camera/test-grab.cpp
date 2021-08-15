#include <stdexcept>
#include <r.h>
#include <Clock.h>
#include <ClockAccessor.h>
#include <picamera/PiCamera.h>
#include <picamera/PiCameraSettings.h>

static bool quit = false;
static void set_quit(int sig, siginfo_t *info, void *ucontext);
static void quit_on_control_c();

void store_jpeg(rpp::MemBuffer& jpeg, const char *name_prefix, int index)
{
        char filename[512];
        snprintf(filename, sizeof(filename), "%s_%04d.jpg", name_prefix, index);
        // r_info("File %s, Length %d", filename, (int) jpeg.size());
        
        int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
        if (fd >= 0) {
                auto nwrite = write(fd, jpeg.data().data(), jpeg.size());
                if (nwrite != (ssize_t) jpeg.size())
                    r_info("File %s, failed to write size <%d> wrote <%d>",
			   filename, (int) jpeg.size(), nwrite);
                fsync(fd);
                close(fd);
        } else {
                r_err("Failed to write to %s", filename);
        }
}

int main()
{
        try {
                // romi::V2VideoCameraSettings settings(640, 480, 10);
                //romi::V2VideoCameraSettings settings(1920, 1080, 10);
                romi::V2VideoCameraSettings settings(640, 480, 50);
                // romi::V2StillCameraSettings settings(romi::kV2HalfWidth,
                //                                      romi::kV2HalfHeight);
                auto camera = romi::PiCamera::create(settings);

                quit_on_control_c();
                
                for (int i = 0; !quit; i++) {
                        auto clock = rpp::ClockAccessor::GetInstance();
                        double start_time = clock->time();
                        rpp::MemBuffer& jpeg = camera->grab_jpeg();
                        r_info("Grab: %f s", clock->time() - start_time);
                        store_jpeg(jpeg, "test", i);
                        r_info("Store: %f s", clock->time() - start_time);
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
