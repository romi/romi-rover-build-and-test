#include "ILinux.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

namespace rpp {
        class LinuxMock : public ILinux {
        public:
                MOCK_METHOD2(open, int(const char *pathname, int flags));
                MOCK_METHOD1(close, int(int fd));
                MOCK_METHOD1(exit, void(int status));

                MOCK_METHOD2(signal, sighandler_t(int signum, sighandler_t handler));
                MOCK_METHOD2(stat, int(const char *path, struct stat *buf));
                MOCK_METHOD3(waitpid, pid_t(pid_t pid, int * status, int options));
                MOCK_METHOD3(execve, int(const char *filename, char *const argv[], char *const envp[]));

                MOCK_METHOD0(fork, pid_t(void));
                MOCK_METHOD2(kill, int (pid_t pid, int sig));
                MOCK_METHOD1(system, int(const char *command));

                MOCK_METHOD2(fopen, FILE * ( const char *filepath, const char *mode ));
                MOCK_METHOD1(fclose, int (FILE *fp));
                MOCK_METHOD1(opendir, DIR* (const char *path));
                MOCK_METHOD1(closedir, int (DIR *directory));
                MOCK_METHOD1(readdir, dirent *(DIR *directory));
                MOCK_METHOD1(remove, int(const char *filename));
                MOCK_METHOD1(sleep, unsigned int(unsigned int seconds));
                MOCK_METHOD3(ioctl, int(int fd, unsigned long request, void *argp));
                MOCK_METHOD3(poll, int(struct pollfd *fds, nfds_t nfds, int timeout));
                MOCK_METHOD3(read, ssize_t(int fd, void *buf, size_t count));
        };
}
