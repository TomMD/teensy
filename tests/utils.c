#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "../commands.h"

/* Open the teensy device for reading and writing with any additional options 
specified in flags (such as NON-BLOCKING) */
int open_teensy(int flags) {
    int teensy, err;

    teensy = open("/dev/ttyUSB001", O_RDWR | flags);

    if (!teensy) { 
        err = errno;
        printf("Could not open device.\n");
        printf("Error code: %s\n", strerror(err));
    }

    return teensy;
}

/* Read a node from the device and compare its contents to the contents */
/* of a particular file.  A file-size that is greater than the maximum */
/* bytes allowed by the device is considered an error. */
int compare_dev_to_file(int dev_handle, const char* file, int index) {
    struct command *rreq = malloc(sizeof(struct command));
    void *filedata;
    int handle;
    int datasize;
    int err;

    /* open the specified file for reading */
    handle = open(file, O_RDONLY);
    if (handle == (-1)) { return (-1); }

    /* verify that the file is an appropriate size for comparison */
    datasize    = filesize(handle);
    if (datasize > MAXDATASIZE) { err = (-1); goto fail_before_alloc; }

    /* fill in the request structure */
    rreq->index    = index;
    rreq->cmd_type = CMD_READ;
    /* FUTURE: alloc data field --- rreq->data   = malloc(datasize); */

    /* read the file */
    filedata       = malloc(datasize);
    err            = read(handle, filedata, datasize);
    if (err != datasize) { err = (-1); goto done; }

    /* read from the device */
    err = read(dev_handle, rreq, 1);

    /* check for errors and validate the data we read against the file */
    if (err == (-1)) { err = errno; }
    else if (err == 1 && !(err = memcmp(filedata, &(rreq->data), datasize))) {
      err = 0;
    }

    /* FUTURE: compare data directly
    else if (err == 1 && !(err = memcmp(filedata, rreq->data, datasize))) { 
        err = 0;
    } */

    /* cleanup */
  done: 
    /* FUTURE: free data array --- free(rreq->data);*/
    free(rreq);
  fail_before_alloc:
    close(handle);
    free(filedata);

    return err;
}

/* Write the contents of a particular file to an open sstore device at a */
/* particular index. Returns 0 to indicate success. */
int write_file_to_dev(int dev_handle, const char* file, int index) {
    struct command *wreq = malloc(sizeof(struct command));
    int handle;
    int datasize;
    int err;

    /* open the specified file for reading */
    handle = open(file, O_RDONLY);
    if (handle == (-1)) { return (-1); }

    /* verify that the file is an appropriate size for writing */
    datasize    = filesize(handle);
    if (datasize > MAXDATASIZE) { err = (-1); goto fail_before_alloc; }

    /* fill in the request structure */
    wreq->index    = index;
    wreq->cmd_type = CMD_STORE;
    /* FUTURE: alloc data field and fill in size of array to write
    wreq->size     = datasize;
    wreq->data     = malloc(datasize); */
    err            = read(handle, wreq->data, datasize);
    if (err != datasize) { err = (-1); goto done; }

    /* perform the write to the device */
    err = write(dev_handle, wreq, 1);

    /* set error return */
    if (err == (-1)) { err = errno; }
    else if (err == 1) { err = 0; }

    /* cleanup */
  done:
    /* FUTURE: free data array --- free(wreq->data);*/
    free(wreq);
  fail_before_alloc:
    close(handle);

    return err;
}

int filesize(int handle) {
    struct stat *stats = malloc(sizeof(struct stat));
    int result, err;

    /* get size from stat then free intermediate structure */
    err = fstat(handle, stats);
    result = stats->st_size;
    if (err) { result = errno; }
    free(stats);

    return result;
}
