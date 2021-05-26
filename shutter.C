#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <pthread.h>

#include "irc_ctrl.C"
#include "/storage/irc/GetThermal/source/libuvc/include/libuvc/libuvc.h"
#include "LEPTON_SDK.h"
#include "LEPTON_SYS.h"

struct Irc_str cam;
static char* shutterPosition2string(LEP_SYS_SHUTTER_POSITION_E shutterPosition);

void exiting()
{
    if (detect_irc(cam)){
        uvc_unref_device(cam.dev);
    }
    uvc_exit(cam.ctx);
    printf("Device closed\n");

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

    pid_t pid[nCam];
    pthread_t pthread[nCam];

    uvc_error_t          res;
    uvc_device_descriptor_t *desc;
    uvc_device_handle_t *devh;

    LEP_CAMERA_PORT_DESC_T m_portDesc;

    signal (SIGINT, INThandler);

    i = 2;

    cam.serial = (char*)serial[i];
    cam.idcam = idCam[i];

    res = uvc_init(&cam.ctx, NULL);
    if (res < 0) {
        uvc_perror(res, "uvc_init");
    }
    puts ("UVC initialized");

    res = uvc_find_device(cam.ctx, &cam.dev, 0x1e4e, 0x0100, cam.serial);
    if (res < 0) {
        uvc_perror(res, "uvc_find_device");
    }

    uvc_get_device_descriptor(cam.dev, &desc);
    printf("Using %s %s with firmware %s\n", desc->manufacturer, desc->product, desc->serialNumber);

    res = uvc_open(cam.dev, &devh);
    if (res < 0) {
        uvc_perror(res, "uvc_open");
        uvc_unref_device(cam.dev);
        return -1;
    }

    m_portDesc.portID = 0;
    m_portDesc.portType = LEP_CCI_UVC;
    m_portDesc.userPtr = (void*) devh;
    //LEP_OpenPort(m_portDesc.portID, m_portDesc.portType, 0, &m_portDesc);
                
    //printf("LEPTON port opened\n");

    
    const uvc_extension_unit_t *units = uvc_get_extension_units(devh);
    /*
    while (units)
    {
        printf("Found extension unit ID %d, controls: %08lx, GUID:", units->bUnitID, units->bmControls); 
        for (i = 0; i < 16; i++)
            printf(" %02x", units->guidExtensionCode[i]);
        printf("\n");
        units = units->next;
    }

    const uvc_format_desc_t *desc1 = uvc_get_format_descs(devh);
    while (desc != NULL)
    {
        int width, height;
        width = desc1->frame_descs[0].wWidth;
        height = desc1->frame_descs[0].wHeight;
        printf("w = %d, h = %d\n", width, height);
        break;
    }
    */

    uvc_device_t* dev_test;
    int bus, addr;

    dev_test = uvc_get_device(devh);
    bus = uvc_get_bus_number(dev_test);
    addr = uvc_get_device_address(dev_test);
    //printf("%d, %d\n", bus, addr);

    dev_test = uvc_get_device((uvc_device_handle_t*) m_portDesc.userPtr);
    bus = uvc_get_bus_number(dev_test);
    addr = uvc_get_device_address(dev_test);
    //printf("%d, %d\n", bus, addr);

    
    LEP_RESULT lres;
    LEP_SYS_SHUTTER_POSITION_E shutterPosition;
    LEP_SYS_FFC_SHUTTER_MODE_OBJ_T shutterModeObj;
    
    puts("Setting the shutter mode");
    lres = LEP_GetSysFfcShutterModeObj(&m_portDesc, &shutterModeObj);
    printf("shutter mode : %d\n", shutterModeObj.shutterMode);

    shutterModeObj.shutterMode = LEP_SYS_FFC_SHUTTER_MODE_MANUAL;
    //shutterModeObj.shutterMode = LEP_SYS_FFC_SHUTTER_MODE_AUTO;
    lres = LEP_SetSysFfcShutterModeObj(&m_portDesc, shutterModeObj);
    puts("shutter mode set!");
    lres = LEP_GetSysFfcShutterModeObj(&m_portDesc, &shutterModeObj);
    printf("shutter mode : %d\n", shutterModeObj.shutterMode);

    lres = LEP_GetSysShutterPosition(&m_portDesc, &shutterPosition);
    printf("%d\t%s\n", lres, shutterPosition2string(shutterPosition));

    lres = LEP_SetSysShutterPosition(&m_portDesc, LEP_SYS_SHUTTER_POSITION_CLOSED);
    //lres = LEP_SetSysShutterPosition(&m_portDesc, LEP_SYS_SHUTTER_POSITION_BRAKE_ON);

    lres = LEP_GetSysShutterPosition(&m_portDesc, &shutterPosition);
    printf("%d\t%s\n", lres, shutterPosition2string(shutterPosition));

    //lres = LEP_SetSysShutterPosition(&m_portDesc, LEP_SYS_SHUTTER_POSITION_BRAKE_ON);

    lres = LEP_GetSysShutterPosition(&m_portDesc, &shutterPosition);
    printf("%d\t%s\n", lres, shutterPosition2string(shutterPosition));

    while(1) {
        sleep (3);

        lres = LEP_SetSysShutterPosition(&m_portDesc, LEP_SYS_SHUTTER_POSITION_CLOSED);
        //lres = LEP_SetSysShutterPosition(&m_portDesc, LEP_SYS_SHUTTER_POSITION_BRAKE_ON);

        lres = LEP_GetSysShutterPosition(&m_portDesc, &shutterPosition);
        printf("%d\t%s\n", lres, shutterPosition2string(shutterPosition));
    }
        
    atexit(exiting);

    return 0;
}


static char* shutterPosition2string(LEP_SYS_SHUTTER_POSITION_E shutterPosition)
{
    static char* pos;

    printf("%d\n", shutterPosition);

    switch (shutterPosition) {
        case -1:
            pos = (char*)"UNKNOWN";
            break;
        case 0:
            pos = (char*)"IDLE";
            break;
        case 1:
            pos = (char*)"OPEN";
            break;
        case 2:
            pos = (char*)"CLOSED";
            break;
        case 3:
            pos = (char*)"BREAK_ON";
            break;
        case 4:
            pos = (char*)"END";
            break;
        default:
            pos = (char*)"DEFAULT?";
            break;
    }

    return pos;
}
