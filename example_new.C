#include <stdio.h>
#include <unistd.h>
#include <time.h>

#include "/storage/irc/GetThermal/source/libuvc/include/libuvc/libuvc.h"

static double threshold;
static int thresholdN;

void cb(uvc_frame_t *frame, void *ptr);

void cb(uvc_frame_t *frame, void *ptr)
{
    int seq = frame->sequence;

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
    threshold = 0.00; // in degeree celcius
    thresholdN = threshold * 100 + 27315;

    const int nCam = 4;
    const int idCam[nCam] = {1, 2, 3, 4};
    const char *serial_all[nCam] = {"0015002c-5119-3038-3732-333700000000",  // 1
                                 "0013001c-5113-3437-3335-373400000000",  // 2
                                 "00070029-5102-3038-3835-393400000000",  // 3
                                 "8010800b-5113-3437-3335-373400000000"}; // 4

    int i;
    int ndev = 0;
    char *serial[nCam] = {};

    uvc_device_t*        dev_tmp;
    uvc_device_handle_t* devh_tmp;

    uvc_context_t*       ctx[nCam];
    uvc_device_t*        dev[nCam];
    uvc_device_handle_t* devh[nCam];
    uvc_stream_ctrl_t    ctrl[nCam];
    uvc_error_t          res[nCam];

    puts("All device serials");

    for (i=0; i<4; i++) {
        printf("   %s\n", serial_all[i]);
    }

    for (i=0; i<nCam; i++) {
        res[i] = uvc_init(&ctx[i], NULL);

        if (res[i] < 0) {
            uvc_perror(res[i], "uvc_init");

            return res[i];
        }
    }

    puts("UVC initialized");


    for (i=0; i<nCam; i++) {
        printf("%d\n", i);
        res[i] = uvc_find_device(ctx[i], &dev_tmp, 0x1e4e, 0x0100, serial_all[i]);

        if (res[i] < 0) {
            uvc_perror(res[i], "uvc_find_device");
        }
        else {
            serial[ndev] = (char*) serial_all[i];
            dev[ndev] = dev_tmp;
            ndev++;
        }
    }

    printf("%d Devices found\n", ndev);

    for (i=0; i<ndev; i++) {
        res[i] = uvc_open(dev[i], &devh[i]);

        if (res[i] < 0) {
            uvc_perror(res[i], "uvc_open");
        }
    }

    printf("%d Devices opened\n", ndev);

    for (i=0; i<ndev; i++) {
        uvc_print_diag(devh[i], stderr);
    }

    for (i=0; i<ndev; i++) {
        res[i] = uvc_get_stream_ctrl_format_size(
        devh[i], &ctrl[i], UVC_FRAME_FORMAT_Y16, 
        160, 120, 9);
    }

    for (i=0; i<ndev; i++) {
        uvc_print_stream_ctrl(&ctrl[i], stderr);
        if (res[i] < 0) {
            uvc_perror(res[i], "get_mode");
            return res[i];
        }
    }

    for (i=0; i<ndev; i++) {
        res[i] = uvc_start_streaming(devh[i], &ctrl[i], cb, (void*) &idCam[i], 0);
        if (res[i] < 0)
        {
            uvc_perror(res[i], "start_streaming");
            printf("Error with Cam %d\n", idCam[i]);
            return res[i];
        }
        puts("Streaming...");
        uvc_set_ae_mode(devh[i], 2);

        sleep(3);

        uvc_stop_streaming(devh[i]);
        puts("Done streaming.");
    }

    for (i=0; i<ndev; i++) {
        uvc_close(devh[i]);
        puts("Device closed");
        uvc_unref_device(dev[i]);
    }

    for (i=0; i<nCam; i++) 
        uvc_exit(ctx[i]);

    puts("UVC exited");

    return 0;
}

