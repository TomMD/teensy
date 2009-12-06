
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

    /* write to the empty list */
    err |= write_file_to_dev(teensy, "data/mid", 28);
    /* write to a singleton list, insert at head */
    err |= write_file_to_dev(teensy, "data/small", 13);
        
    /* try to delete a node from the list */
    err |= ioctl(teensy, TEENSY_IOCERASE_IDX, 13);
        
    /* try to read the deleted value */
    err |= compare_dev_to_file(teensy, "data/small", 13);

    /* error -EAGAIN indicates device is aborting properly */
    if (err == EAGAIN) { err = 0; } else { err = (-1); }
    
    err |= close(teensy);

    return err;
}

