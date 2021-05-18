#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <pthread.h>

#include "/storage/irc/GetThermal/source/libuvc/include/libuvc/libuvc.h"
#include "irc_ctrl.C"

struct Irc_str cams[nCam];

void exiting()
{
    for (int i=0; i<nCam; i++) {
        if (detect_irc(cams[i])){
            uvc_unref_device(cams[i].dev);
        }
        uvc_exit(cams[i].ctx);
        printf("Device #%d closed\n", i);
    }
    exit(0);
}


void INThandler(int sig)
{
    signal(sig, SIG_IGN);
    exiting();
}

int main()
{

    int i;
    const char *serial[4] = {
	    "0013001c-5113-3437-3335-373400000000",  // 1
        "0015002c-5119-3038-3732-333700000000",  // 2
	    "8010800b-5113-3437-3335-373400000000",  // 3
	    "00070029-5102-3038-3835-393400000000",  // 4
        }; 

    int idCam[nCam] = {1, 2, 3, 4};

    int irc_flag[nCam]  = {0, };
    int proc_flag[nCam] = {0, };

    pid_t pid[nCam];
    pthread_t pthread[nCam];

    uvc_error_t          res;

    signal (SIGINT, INThandler);

    for (i=0; i<4; i++) {
        cams[i].serial = (char*)serial[i];
        cams[i].idcam = idCam[i];
        res = uvc_init(&cams[i].ctx, NULL);
        if (res < 0) {
            uvc_perror(res, "uvc_init");
        }
    }
    puts ("UVC initialized");


    while (1) {
        for (i=0; i<4; i++) {
            res = uvc_find_device(cams[i].ctx, &cams[i].dev, 0x1e4e, 0x0100, cams[i].serial);
            if (res < 0) {
                uvc_perror(res, "uvc_find_device");
                irc_flag[i] = 0;
                proc_flag[i] = 0;
            }
            else {
                if (irc_flag[i] == 0) {
                    res = uvc_init(&cams[i].ctx, NULL);
                    res = uvc_find_device(cams[i].ctx, &cams[i].dev, 0x1e4e, 0x0100, cams[i].serial);
                    irc_flag[i] = 1;
                    proc_flag[i] = 0;
                }
            }
        }

        for (i=0; i<4; i++) {
            if (irc_flag[i] == 1) {
                if (proc_flag[i] == 0) {
                    printf("starting process for device #%d\n", i);
                    pid[i] = pthread_create(&pthread[i], NULL, stream_proc, (void*) &cams[i]);
                    if (pid[i] < 0) {
                        perror("process create error");
                        proc_flag[i] = 0;
                    }
                    else {
                        printf("process %d created \n", pid[i]);
                        proc_flag[i] = 1;
                        pthread_detach(pthread[i]);
                    }
                }
                else {
                    printf("streaming for device #%d\n", i);
                }
            }
            else if (irc_flag[i] == 0) {
                if (proc_flag[i] == 1) {
                    printf("stopping process for device #%d\n", i);
                    proc_flag[i] = 0;
                }
                else {
                    printf("waiting for device #%d\n", i);
                    proc_flag[i] = 0;
                }
            }
        }
        printf("\n");
        sleep (3);
    }

    atexit(exiting);

    return 0;
}
