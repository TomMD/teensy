
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include "utils.h"

int main(int argc, char **argv) {
    int teensy;
    int err = 0;

    /* Open handle and return if not available */
    teensy = open_teensy(0);
    if (teensy == -1) { return (-1); }

    /* write to the empty list */
    err |= write_file_to_dev(teensy, "data/mid", 28);
    /* write to a singleton list, insert at head */
    err |= write_file_to_dev(teensy, "data/small", 13);
    /* write to a multi-item list, insert at tail */
    err |= write_file_to_dev(teensy, "data/small", 29);
        
    /* read and test value inserted to singleton list */
    err |= compare_dev_to_file(teensy, "data/small", 13);
    
    err |= close(teensy);

    return err;
}

