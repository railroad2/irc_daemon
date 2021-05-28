#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#include "SolTrack.h"

int sunpos()
{
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    struct Time time;
    struct Location location;
    struct Position position;

    int useDegrees = 1;
    int useNorthEqualsZero = 1; 
    int computeRefrEquatorial = 0; 
    int computeDistance = 1;

    time.year = tm.tm_year + 1900;
    time.month = tm.tm_mon + 1;
    time.day = tm.tm_mday;
    time.hour = tm.tm_hour;
    time.minute = tm.tm_min;
    time.second = tm.tm_sec;

    printf("now: %d-%d-%d %d:%d:%d\n",
       tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday,
       tm.tm_hour, tm.tm_min, tm.tm_sec);

    // Tenerife
    //location.longitude = -16.5124758;
    //location.latitude = 28.300463;
    //Seoul
    location.longitude = 127;
    location.latitude = 37.5;

    SolTrack(time, 
             location, 
             &position, 
             useDegrees, 
             useNorthEqualsZero, 
             computeRefrEquatorial,
             computeDistance);

    //convertEquatorialToHorizontal(location, &position);

    printf("RA = %lf\n", position.rightAscension);
    printf("DEC = %lf\n", position.declination);
    printf("hourangle = %lf\n", position.hourAngle);
    printf("distance = %lf\n", position.distance);
    printf("altitude = %lf\n", position.altitude);
    printf("longitude = %lf\n", position.longitude);
    printf("azimuthRefract = %lf\n", (position.azimuthRefract));// * 180 / PI);
    printf("altitudeRefract= %lf\n", (position.altitudeRefract) );//* 180 / PI);

    return 0;
}

int main()
{
    printf("starting\n");
    sunpos();
}
