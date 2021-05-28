#include <stdio.h>
#include <time.h>

#include "SolTrack.h"

int sunpos(struct Position* pos)
{
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    struct Time time;
    struct Location loc;

    int useDegrees = 1;
    int useNorthEqualsZero = 1;
    int computeRefrEquatorial = 1; 
    int computeDistance = 0;

    time.year = tm.tm_year + 1900;
    time.month = tm.tm_mon + 1;
    time.day = tm.tm_mday;
    time.hour = tm.tm_hour;
    time.minute = tm.tm_min;
    time.second = tm.tm_sec;

    // Teide observatory
    //location.longitude = -16.5124758;
    //location.latitude = 28.300463;

    // Seoul
    loc.longitude = 127.0;
    loc.latitude = 37.6;

    SolTrack(time, 
             loc,
             pos,
             useDegrees, 
             useNorthEqualsZero,
             computeRefrEquatorial,
             computeDistance);

    return 0;
}

int detect_sun(int idcam)
{
    int res;
    int sunflag;
    struct Position pos;

    double al, az;

    res = sunpos(&pos);

    az = pos.azimuthRefract;
    al = pos.altitudeRefract;

    if (al > 61.5) {
        printf("altitude=%lf\n", al);
        sunflag = 1;
    }
    else {
        sunflag = 0;
    }
        
    return sunflag;
}
    

int detect_sun_test(int idcam)
{
    int res;
    int sunflag;
    double az_min, az_max;
    struct Position pos;

    double al, az;

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    res = sunpos(&pos);

    az = (int)(((tm.tm_sec % 60)/60.0*360.0*3)) % 360;
    al = pos.altitudeRefract;

    switch (idcam) {
        case 1:
            az_min = 0;
            az_max = 90;
            break;
        case 2:
            az_min = 90;
            az_max = 180;
            break;
        case 3:
            az_min = 180; 
            az_max = 270;
            break;
        case 4:
            az_min = 270; 
            az_max = 359;
            break;
        default:
            return -1;
    }

    if (az > az_min && az < az_max) {
        printf("id=%d, azimuth=%lf\n", idcam, az);
        //printf("altitude=%lf\n", al);
        sunflag = 1;
    }
    else {
        sunflag = 0;
    }
        
    return sunflag;
}

