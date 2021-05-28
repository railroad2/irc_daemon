#include <stdio.h>
#include <time.h>

#include "/storage/irc/GetThermal/source/libuvc/include/libuvc/libuvc.h"
#include "LEPTON_SDK.h"
#include "LEPTON_SYS.h"

int shutter_ctrl(uvc_device_handle_t *devh, int onoff) 
{
    LEP_RESULT lres;
    LEP_CAMERA_PORT_DESC_T m_portDesc;
    LEP_SYS_SHUTTER_POSITION_E shutterPosition;
    LEP_SYS_FFC_SHUTTER_MODE_OBJ_T shutterModeObj;

    m_portDesc.portID = 0;
    m_portDesc.portType = LEP_CCI_UVC;
    m_portDesc.userPtr = (void*) devh;

    // close
    if (onoff == 0) { 
CLOSE:
        lres = LEP_GetSysFfcShutterModeObj(&m_portDesc, &shutterModeObj);
        if (shutterModeObj.shutterMode != LEP_SYS_FFC_SHUTTER_MODE_MANUAL) {
            shutterModeObj.shutterMode = LEP_SYS_FFC_SHUTTER_MODE_MANUAL;
            lres = LEP_SetSysFfcShutterModeObj(&m_portDesc, shutterModeObj);
            lres = LEP_GetSysFfcShutterModeObj(&m_portDesc, &shutterModeObj);
            if (shutterModeObj.shutterMode != LEP_SYS_FFC_SHUTTER_MODE_MANUAL) {
                puts ("Failed to change the shutter mode\n");
                goto CLOSE;
                return -1;
            }
        }
        lres = LEP_SetSysShutterPosition(&m_portDesc, LEP_SYS_SHUTTER_POSITION_CLOSED);
        lres = LEP_GetSysShutterPosition(&m_portDesc, &shutterPosition);
        if (shutterPosition != LEP_SYS_SHUTTER_POSITION_CLOSED) {
            puts ("Failed to change the shutter position\n");
            goto CLOSE;
            return -1;
        }
        lres = LEP_SetSysShutterPosition(&m_portDesc, LEP_SYS_SHUTTER_POSITION_BRAKE_ON);
        lres = LEP_GetSysShutterPosition(&m_portDesc, &shutterPosition);
        if (shutterPosition != LEP_SYS_SHUTTER_POSITION_BRAKE_ON) {
            puts ("Failed to change the shutter position\n");
            goto CLOSE;
            return -1;
        }
        puts ("Shutter CLOSED\n");
    }   
    // open
    else {
OPEN:
        lres = LEP_GetSysFfcShutterModeObj(&m_portDesc, &shutterModeObj);
        if (shutterModeObj.shutterMode != LEP_SYS_FFC_SHUTTER_MODE_AUTO) {
            shutterModeObj.shutterMode = LEP_SYS_FFC_SHUTTER_MODE_AUTO;
            lres = LEP_SetSysFfcShutterModeObj(&m_portDesc, shutterModeObj);
            lres = LEP_GetSysFfcShutterModeObj(&m_portDesc, &shutterModeObj);
            if (shutterModeObj.shutterMode != LEP_SYS_FFC_SHUTTER_MODE_AUTO) {
                puts ("Failed to change the shutter mode\n");
                goto OPEN;
                return -1;
            }
        }
        lres = LEP_SetSysShutterPosition(&m_portDesc, LEP_SYS_SHUTTER_POSITION_OPEN);
        lres = LEP_GetSysShutterPosition(&m_portDesc, &shutterPosition);
        if (shutterPosition != LEP_SYS_SHUTTER_POSITION_OPEN) {
            puts ("Failed to change the shutter position\n");
            goto OPEN;
            return -1;
        }
        puts ("Shutter OPENED\n");
    }

    return 0;
}
