#include <stdexcept>
#include <signal.h>
#include "unistd.h"
#include "fcntl.h"
#include "Logger.h"
#include <rpc/RcomClient.h>
#include <rpc/RemoteCamera.h>

static bool quit = false;
static void set_quit(int sig, siginfo_t *info, void *ucontext);
static void quit_on_control_c();

void store_jpeg(rcom::MemBuffer& jpeg, const char *filename)
{
        r_info("File %s, Length %d", filename, (int) jpeg.size());
        
        int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
        if (fd >= 0) {
                auto nwrite = write(fd, jpeg.data().data(), jpeg.size());
                if (nwrite != (ssize_t)jpeg.size())
                    r_info("File %s, failed to write size <%d> wrote <%d>", filename, (int) jpeg.size(), nwrite);
                fsync(fd);
                close(fd);
        } else {
                r_err("Failed to write to %s", filename);
        }
}

char buffer[512];

const char *make_filename(const char *prefix, int index)
{
        snprintf(buffer, sizeof(buffer), "%s_%04d.jpg", prefix, index);
        return buffer;
}

int main(int argc, char **argv)
{
        try {
                int number_of_grabs = 1000;
                if (argc >= 2) {
                        number_of_grabs = atoi(argv[1]);
                }

                const char *requested_filename = nullptr;
                if (argc >= 2) {
                        requested_filename = argv[2];
                }
                
                auto client = romi::RcomClient::create("camera", 10.0);
                romi::RemoteCamera camera(client);

                quit_on_control_c();
                
                for (int i = 0; i < number_of_grabs; i++) {
                        rcom::MemBuffer& jpeg = camera.grab_jpeg();
                        if (jpeg.size() > 0) {
                                const char *filename = requested_filename;
                                if (filename == nullptr)
                                        filename = make_filename("remote", i);
                                store_jpeg(jpeg, filename);
                        }
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
