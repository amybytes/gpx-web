/**
 * Name: GPXHelpers.c
 * Author: Ethan Rowan (1086586)
 * Date Created: 01/19/2021
 * Last Modified: 01/22/2021
 **/

#include "GPXParser.h"
#include "GPXHelpers.h"

void deleteGpxData(void *data) {
    free(data);
}

char *gpxDataToString(void *data) {
    char *newStr;
    GPXData *gpxData;

    if (data == NULL) {
        return NULL;
    }

    gpxData = (GPXData *)data;
    newStr = malloc(sizeof(char)*(strlen(gpxData->name)+strlen(gpxData->value)+4));
    sprintf(newStr, "%s %s", gpxData->name, gpxData->value);

    return newStr;
}

int compareGpxData(const void *first, const void *second) {
    GPXData *data1;
    GPXData *data2;

    if (first == NULL || second == NULL) {
        return 0;
    }

    data1 = (GPXData *)first;
    data2 = (GPXData *)second;

    return strcmp((char *)data1->name, (char *)data2->name);
}

void deleteWaypoint(void *data) {
    if (data == NULL) {
        return;
    }
    Waypoint *waypoint = (Waypoint *)data;
    free(waypoint->name);
    freeList(waypoint->otherData);
    free(waypoint);
}

char *waypointToString(void *data) {
    char *newStr;
    Waypoint *waypoint;

    if (data == NULL) {
        return NULL;
    }

    waypoint = (Waypoint *)data;
    newStr = malloc(sizeof(char)*(strlen(waypoint->name)+3));
    sprintf(newStr, "%s  ", waypoint->name);

    return newStr;
}

int compareWaypoints(const void *first, const void *second) {
    
}

void deleteRoute(void *data) {

}

char *routeToString(void *data) {

}

int compareRoutes(const void *first, const void *second) {
    
}

void deleteTrackSegment(void *data) {

}

char *trackSegmentToString(void *data) {

}

int compareTrackSegments(const void *first, const void *second) {
    
}

void deleteTrack(void *data) {

}

char *trackToString(void *data) {
    return "";
}

int compareTracks(const void *first, const void *second) {
    return -1;
}

int isEmptyString(char *str) {
    return str != NULL && strequals(str, "");
}

int isNullOrEmptyString(char *str) {
    return str == NULL || isEmptyString(str);
}