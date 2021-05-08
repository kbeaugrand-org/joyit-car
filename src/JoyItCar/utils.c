
#include <errno.h>
#include <unistd.h>
#include <string.h>

#include <applibs/log.h>

void CloseFd(int fd, const char *fdName)
{
    if (fd == -1)
    {
        return;
    }

    int result = close(fd);

    if (result != 0)
    {
        Log_Debug("ERROR: Could not close fd %s: %s (%d).\n", fdName, strerror(errno), errno);
    }
}