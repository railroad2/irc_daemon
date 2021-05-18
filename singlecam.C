#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include "/storage/irc/GetThermal/source/libuvc/include/libuvc/libuvc.h"

static double threshold;
static int thresholdN;

void cb(uvc_frame_t *frame, void *ptr);

void cb(uvc_frame_t *frame, void *ptr)
{
    int seq = frame->sequence;

    /* capture every 9 seconds */
    if (seq % 9 != 0)
    {
        return;
    }

    const int len    = frame->data_bytes;
    const int width  = frame->width;
    const int height = frame->height;

    unsigned short *pix = (unsigned short*) frame->data;
    int i;

    struct timeval time;
    gettimeofday(&time, NULL);

    char fileName[64];
    snprintf(fileName, 64, "log/%ld.%06ld-cam%d", time.tv_sec, time.tv_usec, *(int*) ptr);

    FILE *file  = fopen(fileName, "a+");
    if (file == NULL)
    {
        printf ("[Error] Cannot open logging file\n");
        return ; 
    }

    fwrite(pix, len , 1, file);
    fclose(file);
}


int main(int argc, char **argv)
{
    threshold = 0.0; // in degeree celcius
    thresholdN = threshold * 100 + 27315;

    int  i;
    int  ndev = 0;
    char *serial = argv[1];
    int  idcam = atoi(argv[2]);

    uvc_context_t*       ctx;
    uvc_device_t*        dev;
    uvc_device_handle_t* devh;
    uvc_stream_ctrl_t    ctrl;
    uvc_error_t          res;

    printf("serial = %s\n", serial); 

    /* Initialize device */
    res = uvc_init(&ctx, NULL);

    if (res < 0) {
        uvc_perror(res, "uvc_init");

        return res;
    }
    puts("UVC initialized");


    /* Find device */
    res = uvc_find_device(ctx, &dev, 0x1e4e, 0x0100, serial);

    if (res < 0) {
        uvc_perror(res, "uvc_find_device");
        return res;
    }
    puts("Devices found");


    /* Open device */
    res = uvc_open(dev, &devh);

    if (res < 0) {
        uvc_perror(res, "uvc_open");
    }
    puts("Devices opened");


    /* Print device information */
    //uvc_print_diag(devh, stderr);


    /* Stream control */
    res = uvc_get_stream_ctrl_format_size(
            devh, &ctrl, UVC_FRAME_FORMAT_Y16, 
            160, 120, 9);

    //uvc_print_stream_ctrl(&ctrl, stderr);
    if (res < 0) {
        uvc_perror(res, "get_mode");
        return res;
    }


    /* Start streaming */
    res = uvc_start_streaming(devh, &ctrl, cb, (void*) &idcam, 0);
    if (res < 0)
    {
        uvc_perror(res, "start_streaming");
        printf("Error with Cam %d\n", idcam);
        return res;
    }
    puts("Streaming...");

    /* Set auto-exposure mode 
     * UVC_AUTO_EXPOSURE_MODE_MANUAL (1) - manual exposure time, manual iris
     * UVC_AUTO_EXPOSURE_MODE_AUTO (2) - auto exposure time, auto iris
     * UVC_AUTO_EXPOSURE_MODE_SHUTTER_PRIORITY (4) - manual exposure time, auto iris
     * UVC_AUTO_EXPOSURE_MODE_APERTURE_PRIORITY (8) - auto exposure time, manual iris */
    uvc_set_ae_mode(devh, 2);

    /* Waiting ... */
    sleep(10);

    /* Closing */
    uvc_stop_streaming(devh);
    puts("Done streaming.");

    uvc_close(devh);
    puts("Device closed");
    uvc_unref_device(dev);

    uvc_exit(ctx);

    puts("UVC exited");

    return 0;
}

