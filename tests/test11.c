
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

    /* write a node */
    err |= write_file_to_dev(teensy, "data/mid", 28);
    /* write a different value to the same index */
    err |= write_file_to_dev(teensy, "data/small", 28);
        
    /* read from the index */
    err |= compare_dev_to_file(teensy, "data/small", 28);
    
    err |= close(teensy);

    return err;
}

