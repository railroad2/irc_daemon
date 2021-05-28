#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

#include "/storage/irc/GetThermal/source/libuvc/include/libuvc/libuvc.h"
#include "detect_sun.c"
#include "shutter_ctrl.c"

const int nCam = 4;

struct Irc_str {
    int idcam;
    char* serial;
    uvc_context_t* ctx;
    uvc_device_t* dev;
};

int detect_irc(struct Irc_str cam)
{
    uvc_error_t res;
    uvc_device_t* tmp;
    int stat;

    res = uvc_find_device(cam.ctx, &cam.dev, 0x1e4e, 0x0100, cam.serial);
    if (res < 0) {
        uvc_perror(res, "uvc_find_device");
        stat = 0;
    }
    else{
        stat = 1;
    }

    return stat;
}


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


//int stream_proc(struct Irc_str cam)
void *stream_proc(void *camptr)
{ 
    int stat;
    struct Irc_str cam = *((struct Irc_str *) camptr);
    uvc_error_t res; 
    uvc_device_handle_t* devh;
    uvc_stream_ctrl_t ctrl;

    stat = detect_irc(cam);

    res = uvc_open(cam.dev, &devh);
    if (res < 0) {
        uvc_perror(res, "uvc_open");
        uvc_unref_device(cam.dev);
        return (void*)-1;
    }

    res = uvc_get_stream_ctrl_format_size(
            devh, &ctrl, UVC_FRAME_FORMAT_Y16, 
            160, 120, 9);
    if (res < 0) {
        uvc_perror(res, "get_mode");
        uvc_close(devh);
        uvc_unref_device(cam.dev);
        return (void*)-1;
    }

    res = uvc_start_streaming(devh, &ctrl, cb, (void*) &cam.idcam, 0);
    if (res < 0) {
        uvc_perror(res, "start_streaming");
        printf("Error with Cam %d\n", cam.idcam);
        uvc_stop_streaming(devh);
        uvc_close(devh);
        uvc_unref_device(cam.dev);
        return (void*)-1;
    }

    while (1) {
        sleep(5);
        stat = detect_irc(cam);
        if (stat == 0){
            puts("Device not found");
            uvc_stop_streaming(devh);
            uvc_close(devh);
            uvc_unref_device(cam.dev);
            return (void*)-1;
        }
    }

    uvc_close(devh);
    uvc_unref_device(cam.dev);
    return (void*) 0;
}


void *stream_proc_shutter(void *camptr)
{ 
    int stat;
    int sunflag = 0;
    int sunwait = 0 ;

    struct Irc_str cam = *((struct Irc_str *) camptr);
    uvc_error_t res; 
    uvc_device_handle_t* devh;
    uvc_stream_ctrl_t ctrl;

    stat = detect_irc(cam);

    res = uvc_open(cam.dev, &devh);
    if (res < 0) {
        uvc_perror(res, "uvc_open");
        uvc_unref_device(cam.dev);
        return (void*)-1;
    }

STARTSTREAMING:
    res = uvc_get_stream_ctrl_format_size(
            devh, &ctrl, UVC_FRAME_FORMAT_Y16, 
            160, 120, 9);
    if (res < 0) {
        uvc_perror(res, "get_mode");
        uvc_close(devh);
        uvc_unref_device(cam.dev);
        return (void*)-1;
    }

    res = uvc_start_streaming(devh, &ctrl, cb, (void*) &cam.idcam, 0);
    if (res < 0) {
        uvc_perror(res, "start_streaming");
        printf("Error with Cam %d\n", cam.idcam);
        uvc_stop_streaming(devh);
        uvc_close(devh);
        uvc_unref_device(cam.dev);
        return (void*)-1;
    }

    while (1) {
        sleep(1);
        stat = detect_irc(cam);
        if (stat == 0){
            puts("Device not found");
            uvc_stop_streaming(devh);
            uvc_close(devh);
            uvc_unref_device(cam.dev);
            return (void*)-1;
        }
        
        //sunflag = detect_sun();
        sunflag = detect_sun_test(cam.idcam);

        if (sunflag == 1) {
            puts("Sun detected");
            uvc_stop_streaming(devh);
            shutter_ctrl(devh, 0);
            sunwait = 1;
        }
        else if (sunflag == 0) {
            if (sunwait == 1) {
                puts("Sun is gone");
                sunwait = 0;
                shutter_ctrl(devh, 1);
                goto STARTSTREAMING;
            }
        }

    }

    uvc_close(devh);
    uvc_unref_device(cam.dev);
    return (void*) 0;
}


