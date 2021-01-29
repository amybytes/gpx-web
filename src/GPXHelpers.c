/**
 * Name: GPXHelpers.c
 * Author: Ethan Rowan (1086586)
 * Date Created: 01/19/2021
 * Last Modified: 01/29/2021
 **/

#include "GPXParser.h"
#include "GPXHelpers.h"

#define LAT_MAX_LENGTH 19  // maximum number of digits in a latitude string
#define LON_MAX_LENGTH 19  // maximum number of digits in a longitude string

#define GPX_DATA_SPACING 2
#define WAYPOINT_INDENT 2
#define ROUTE_INDENT 2
#define ROUTE_WAYPOINT_INDENT 4
#define TRACK_SEGMENT_INDENT 6
#define TRACK_SEGMENT_WAYPOINT_INDENT 8

int getGPXDataStringSize(void *data) {
    if (data == NULL) {
        return 0;
    }
    GPXData *gpxData = (GPXData *)data;
    return strlen(gpxData->name) + strlen(gpxData->value) + 5;
}

int getWaypointStringSize(void *data, int indentLevel) {
    if (data == NULL) {
        return 0;
    }
    Waypoint *waypoint = (Waypoint *)data;
    // return strlen(waypoint->name) + LAT_MAX_LENGTH + LON_MAX_LENGTH +
    //     getListSizeInBytes(waypoint->otherData, &getGPXDataSizeInBytes);
    return strlen(waypoint->name) + LAT_MAX_LENGTH + LON_MAX_LENGTH +
        getListStringSize(waypoint->otherData, indentLevel);
}

int getRouteStringSize(void *data) {
    if (data == NULL) {
        return 0;
    }
    Route *route = (Route *)data;
    return strlen(route->name) + getListStringSize(route->waypoints, ROUTE_WAYPOINT_INDENT) +
        getListStringSize(route->otherData, GPX_DATA_SPACING);
}

int getTrackSegmentStringSize(void *data) {
    if (data == NULL) {
        return 0;
    }
    TrackSegment *trackSegment = (TrackSegment *)data;
    return getListStringSize(trackSegment->waypoints, TRACK_SEGMENT_WAYPOINT_INDENT);
}

int getTrackStringSize(void *data) {
    if (data == NULL) {
        return 0;
    }
    Track *track = (Track *)data;
    return strlen(track->name) + getListStringSize(track->segments, TRACK_SEGMENT_INDENT) +
        getListStringSize(track->otherData, GPX_DATA_SPACING);
}

int getListStringSize(List *list, int indentLevel) {
    int size = 0;
    void *element;
    ListIterator it = createIterator(list);
    while ((element = nextElement(&it))) {
        char *elementStr = list->printData(element);
        size += strlen(elementStr) + indentLevel+1;
        free(elementStr);
    }
    return size+1;
}

void appendOtherDataToString(List *otherData, char *str) {
    void *element;
    char *dataStr;
    ListIterator it = createIterator(otherData);
    while ((element = nextElement(&it))) {
        dataStr = gpxDataToString(element);
        sprintf(str+strlen(str), "  %s", dataStr);
        free(dataStr);
    }
}

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
    int size = getGPXDataStringSize(data);
    if (size == 0) {
        return NULL;
    }
    newStr = malloc(size);
    sprintf(newStr, "%s: %s", gpxData->name, gpxData->value);

    return newStr;
}

int compareGpxData(const void *first, const void *second) {
    // GPXData *data1;
    // GPXData *data2;

    // if (first == NULL || second == NULL) {
    //     return 0;
    // }

    // data1 = (GPXData *)first;
    // data2 = (GPXData *)second;

    // return strcmp((char *)data1->name, (char *)data2->name);
    return 0;
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
    int size = getWaypointStringSize(waypoint, WAYPOINT_INDENT);
    if (size == 0) {
        return NULL;
    }
    int overhead = 24;
    newStr = malloc(size+overhead);
    sprintf(newStr, "  name: %s  lat: %lf  lon: %lf", waypoint->name,
        waypoint->latitude, waypoint->longitude); // Overhead (22 bytes)

    appendOtherDataToString(waypoint->otherData, newStr);
    
    return newStr;
}

int compareWaypoints(const void *first, const void *second) {
    
}

void deleteRoute(void *data) {
    if (data == NULL) {
        return;
    }
    Route *route = (Route *)data;
    free(route->name);
    freeList(route->waypoints);
    freeList(route->otherData);
    free(route);
}

char *routeToString(void *data) {
    char *newStr;
    Route *route;

    if (data == NULL) {
        return NULL;
    }

    route = (Route *)data;
    int size = getRouteStringSize(route);
    if (size == 0) {
        return NULL;
    }
    int overhead = 24;
    newStr = malloc(size+overhead);
    sprintf(newStr, "  name: %s", route->name); // Overhead (8 bytes)
    
    appendOtherDataToString(route->otherData, newStr);

    sprintf(newStr+strlen(newStr), "\n    WAYPOINTS:"); // Overhead (14 bytes)

    // Append indented waypoints (4 levels of indenting)
    void *element;
    char *elementStr;
    ListIterator it = createIterator(route->waypoints);
    while ((element = nextElement(&it))) {
        elementStr = route->waypoints->printData(element);
        sprintf(newStr+strlen(newStr), "\n    %s", elementStr);
        free(elementStr);
    }

    return newStr;
}

int compareRoutes(const void *first, const void *second) {
    
}

void deleteTrackSegment(void *data) {
    if (data == NULL) {
        return;
    }
    TrackSegment *trackSegment = (TrackSegment *)data;
    freeList(trackSegment->waypoints);
    free(trackSegment);
}

char *trackSegmentToString(void *data) {
    char *newStr;
    TrackSegment *trackSegment;

    if (data == NULL) {
        return NULL;
    }

    trackSegment = (TrackSegment *)data;
    int size = getTrackSegmentStringSize(trackSegment);
    if (size == 0) {
        return NULL;
    }
    int overhead = 11;
    newStr = malloc(size+overhead);
    newStr[0] = '\0';

    sprintf(newStr+strlen(newStr), "WAYPOINTS:"); // Overhead (10 bytes)

    // Append indented waypoints (8 levels of indenting)
    void *element;
    char *elementStr;
    ListIterator it = createIterator(trackSegment->waypoints);
    while ((element = nextElement(&it))) {
        elementStr = trackSegment->waypoints->printData(element);
        sprintf(newStr+strlen(newStr), "\n        %s", elementStr);
        free(elementStr);
    }

    return newStr;
}

int compareTrackSegments(const void *first, const void *second) {
    
}

void deleteTrack(void *data) {
    if (data == NULL) {
        return;
    }
    Track *track = (Track *)data;
    free(track->name);
    freeList(track->segments);
    freeList(track->otherData);
    free(track);
}

char *trackToString(void *data) {
    char *newStr;
    Track *track;

    if (data == NULL) {
        return NULL;
    }

    track = (Track *)data;
    int size = getTrackStringSize(track);
    if (size == 0) {
        return NULL;
    }
    int overhead = 25;
    newStr = malloc(size+overhead);
    sprintf(newStr, "  name: %s", track->name); // overhead (8 bytes)
    
    appendOtherDataToString(track->otherData, newStr);

    sprintf(newStr+strlen(newStr), "\n    SEGMENTS:"); // overhead (14 bytes)

    // Append indented segments (6 levels of indenting)
    void *element;
    char *elementStr;
    ListIterator it = createIterator(track->segments);
    while ((element = nextElement(&it))) {
        elementStr = track->segments->printData(element);
        sprintf(newStr+strlen(newStr), "\n      %s", elementStr);
        free(elementStr);
    }

    return newStr;
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