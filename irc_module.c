#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

#include "/storage/irc/GetThermal/source/libuvc/include/libuvc/libuvc.h"

static double threshold;
static int thresholdN;

uvc_context_t*       ctx;
uvc_device_t*        dev;
uvc_device_handle_t* devh;
uvc_stream_ctrl_t    ctrl;
uvc_error_t          res;

void INThandler(int);
void cb(uvc_frame_t *frame, void *ptr);
void exiting(void);

void cb(uvc_frame_t *frame, void *ptr)
{
    int seq = frame->sequence;

    //printf("%d ", seq);
    /* capture every 9 seconds */
    if (seq % 9 != 0)
    {
        return;
    }

    const int len    = frame->data_bytes;
    const int width  = frame->width;
    const int height = frame->height;

    unsigned short *pix = (unsigned short*) frame->data;
    //printf("%d %d %d %d\n", *pix, len, width, height);
    int i;

    struct timeval time;
    gettimeofday(&time, NULL);

    char fileName[64];
    snprintf(fileName, 64, "/home/kmlee/log/%ld.%06ld-cam%d", time.tv_sec, time.tv_usec, *(int*) ptr);

    FILE *file  = fopen(fileName, "a+");
    if (file == NULL)
    {
        printf ("[Error] Cannot open logging file\n");
        return ; 
    }

    fwrite(pix, len , 1, file);
    fclose(file);
}


void INThandler(int sig)
{
    char c;
    
    signal(sig, SIG_IGN);
    exiting();
    exit(0);
}


void exiting(void)
{
    uvc_stop_streaming(devh);
    uvc_close(devh);
    uvc_unref_device(dev);
    uvc_exit(ctx);
    puts("Device closed");
}


int main(int argc, char **argv)
{
    int  i;
    int  ndev = 0;
    char *serial = argv[1];
    int  idcam = atoi(argv[2]);

    signal(SIGINT, INThandler);

    threshold = 0.0; // in degeree celcius
    thresholdN = threshold * 100 + 27315;

    printf("serial = %s\n", serial); 

    /* Initialize device */
    res = uvc_init(&ctx, NULL);

    if (res < 0) {
        uvc_perror(res, "uvc_init");

        return res;
    }
    puts("UVC initialized");


FindDevice:
    /* Find device */
    res = uvc_find_device(ctx, &dev, 0x1e4e, 0x0100, serial);

    if (res < 0) {
        uvc_perror(res, "uvc_find_device");
        sleep (5);
        goto FindDevice;
    }
    puts("Device found");


    /* Open device */
    res = uvc_open(dev, &devh);

    if (res < 0) {
        uvc_perror(res, "uvc_open");
        //sleep (5);
        //goto FindDevice;
    }
    puts("Device opened");


    /* Print device information */
    uvc_print_diag(devh, stderr);


    /* Stream control */
    res = uvc_get_stream_ctrl_format_size(
            devh, &ctrl, UVC_FRAME_FORMAT_Y16, 
            160, 120, 9);

    //uvc_print_stream_ctrl(&ctrl, stderr);
    if (res < 0) {
        uvc_perror(res, "get_mode");
        sleep (5);
        goto FindDevice;
        //return res;
    }

    /* set status callback */
    int *user_ptr;
    //uvc_set_status_callback(devh, cb, (void*) user_ptr);
    

    /* Start streaming */
    res = uvc_start_streaming(devh, &ctrl, cb, (void*) &idcam, 0);
    if (res < 0)
    {
        uvc_perror(res, "start_streaming");
        printf("Error with Cam %d\n", idcam);
        goto FindDevice;
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
    while (1) {
        sleep(5); 
        
        res = uvc_find_device(ctx, &dev, 0x1e4e, 0x0100, serial);

        if (res < 0) {
            uvc_perror(res, "uvc_find_device");
            puts("Device not found");
            goto FindDevice;
        }
        //pause();
    }

    /* Closing */
    atexit(exiting);


    return 0;
}

