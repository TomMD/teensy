
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include "../commands.h"
#include "utils.h"

int main(int argc, char **argv) {
    int teensy;
    int err = 0;

    /* Open handle with nonblocking option set, return if not available */
    teensy = open_teensy(O_NONBLOCK);
    if (teensy == -1) { return (-1); }

    /* try to delete a node from an empty list */
    err |= ioctl(teensy, TEENSY_IOCERASE_IDX, 7);

    err |= close(teensy);

    return err;
}

