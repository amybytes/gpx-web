/**
 * Name: GPXHelpers.c
 * Author: Ethan Rowan (1086586)
 * Date Created: 01/19/2021
 * Last Modified: 02/07/2021
 * 
 * Note: Portions of the code in this file are based on sample
 *       code provided by the instructor.
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

/**
 * Calculates the size in bytes needed to represent a given GPXData structure.
 * 
 * Parameters:
 *   gpxData -- the data to be evaluated
 * 
 * Returns:
 *   The size of the GPXData as a string in bytes
 **/
int getGPXDataStringSize(GPXData *gpxData) {
    if (gpxData == NULL) {
        return 0;
    }
    return strlen(gpxData->name) + strlen(gpxData->value) + 5;
}

/**
 * Calculates the size in bytes needed to represent a given Waypoint structure.
 * 
 * Parameters:
 *   waypoint -- the data to be evaluated
 *   indentLevel -- the number of spaces to be used for indenting extra waypoint data
 *                  when representing it as a string (added to total size)
 * 
 * Returns:
 *   The size of the Waypoint as a string in bytes
 **/
int getWaypointStringSize(Waypoint *waypoint, int indentLevel) {
    if (waypoint == NULL) {
        return 0;
    }
    return strlen(waypoint->name) + LAT_MAX_LENGTH + LON_MAX_LENGTH +
        getListStringSize(waypoint->otherData, indentLevel);
}

/**
 * Calculates the size in bytes needed to represent a given Route structure.
 * 
 * Parameters:
 *   route -- the data to be evaluated
 * 
 * Returns:
 *   The size of the Route as a string in bytes
 **/
int getRouteStringSize(Route *route) {
    if (route == NULL) {
        return 0;
    }
    return strlen(route->name) + getListStringSize(route->waypoints, ROUTE_WAYPOINT_INDENT) +
        getListStringSize(route->otherData, GPX_DATA_SPACING);
}

/**
 * Calculates the size in bytes needed to represent a given TrackSegment structure.
 * 
 * Parameters:
 *   trackSegment -- the data to be evaluated
 * 
 * Returns:
 *   The size of the TrackSegment as a string in bytes
 **/
int getTrackSegmentStringSize(TrackSegment *trackSegment) {
    if (trackSegment == NULL) {
        return 0;
    }
    return getListStringSize(trackSegment->waypoints, TRACK_SEGMENT_WAYPOINT_INDENT);
}

/**
 * Calculates the size in bytes needed to represent a given Track structure.
 * 
 * Parameters:
 *   track -- the data to be evaluated
 * 
 * Returns:
 *   The size of the Track as a string in bytes
 **/
int getTrackStringSize(Track *track) {
    if (track == NULL) {
        return 0;
    }
    return strlen(track->name) + getListStringSize(track->segments, TRACK_SEGMENT_INDENT) +
        getListStringSize(track->otherData, GPX_DATA_SPACING);
}

/**
 * Calculates the size in bytes needed to represent a list of data
 * as a string.
 * 
 * Parameters:
 *   list -- the list to be evaluated
 *   indentLevel -- the number of spaces to be used for indenting each element
 *                  in the list when representing them as a string (added to total size)
 * 
 * Returns:
 *   The size of the GPXData as a string in bytes
 **/
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

/**
 * Appends the GPXData elements from otherData to the end of a string
 * to create a chain of attributes. 
 * 
 * Parameters:
 *   otherData -- the list of GPXData to be appended
 *   str -- the string to append the data to (the string itself is modified)
 **/
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

/**
 * Frees the pointer to a GPXData structure.
 * 
 * Parameters:
 *   data -- the pointer to free
 **/
void deleteGpxData(void *data) {
    free(data);
}

/**
 * Creates a string representation of GPXData in the format "name: value".
 * 
 * Parameters:
 *   data -- a pointer to the GPXData
 * 
 * Returns:
 *   A string representing the GPXData
 **/
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
    if (newStr == NULL) {
        return NULL;
    }
    sprintf(newStr, "%s: %s", gpxData->name, gpxData->value);

    return newStr;
}

/** Unused function. Required for compatibility with Linked List API. **/
int compareGpxData(const void *first, const void *second) {
    return 0;
}

/**
 * Frees a pointer to a Waypoint structure and all of its
 * child pointers.
 **/
void deleteWaypoint(void *data) {
    if (data == NULL) {
        return;
    }
    Waypoint *waypoint = (Waypoint *)data;
    free(waypoint->name);
    freeList(waypoint->otherData);
    free(waypoint);
}

/**
 * Creates a string representation of a Waypoint structure.
 * 
 * Parameters:
 *   data -- a pointer to the Waypoint
 * 
 * Returns:
 *   A string representing the Waypoint
 **/
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
    if (newStr == NULL) {
        return NULL;
    }
    sprintf(newStr, "  name: %s  lat: %lf  lon: %lf", waypoint->name,
        waypoint->latitude, waypoint->longitude); // Overhead (22 bytes)

    appendOtherDataToString(waypoint->otherData, newStr);
    
    return newStr;
}

/** Unused function. Required for compatibility with Linked List API. **/
int compareWaypoints(const void *first, const void *second) {
    return 0;
}

/**
 * Frees the pointer to a Route structure and all of its
 * child pointers.
 * 
 * Parameters:
 *   data -- a pointer to the Route
 **/
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

/**
 * Creates a string representation of a Route structure, including
 * all route points and all "other data". The route will be indented
 * by two spaces.
 * 
 * Parameters:
 *   data -- a pointer to the Route
 * 
 * Returns:
 *   A string representation for the Route
 **/
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
    if (newStr == NULL) {
        return NULL;
    }
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

/** Unused function. Required for compatibility with Linked List API. **/
int compareRoutes(const void *first, const void *second) {
    return 0;
}

/**
 * Frees the pointer to a TrackSegment structure and all of its
 * child pointers.
 * 
 * Parameters:
 *   data -- a pointer to the TrackSegment
 **/
void deleteTrackSegment(void *data) {
    if (data == NULL) {
        return;
    }
    TrackSegment *trackSegment = (TrackSegment *)data;
    freeList(trackSegment->waypoints);
    free(trackSegment);
}

/**
 * Creates a string representation of a TrackSegment structure, including
 * all waypoints. The track segment will be indented by eight spaces.
 * 
 * Parameters:
 *   data -- a pointer to the TrackSegment
 * 
 * Returns:
 *   A string representation for the TrackSegment
 **/
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
    if (newStr == NULL) {
        return NULL;
    }
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

/** Unused function. Required for compatibility with Linked List API. **/
int compareTrackSegments(const void *first, const void *second) {
    return 0;
}

/**
 * Frees the pointer to a Track structure and all of its
 * child pointers.
 * 
 * Parameters:
 *   data -- a pointer to the Track
 **/
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

/**
 * Creates a string representation of a Track structure, including
 * all track segments and all "other data". The track will be indented
 * by two spaces.
 * 
 * Parameters:
 *   data -- a pointer to the Track
 * 
 * Returns:
 *   A string representation for the Track
 **/
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
    if (newStr == NULL) {
        return NULL;
    }
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

/** Unused function. Required for compatibility with Linked List API. **/
int compareTracks(const void *first, const void *second) {
    return 0;
}

/**
 * Checks if a string is ONLY empty.
 * 
 * Parameters:
 *   str -- the string to evaluate
 * 
 * Returns:
 *   1 if the string is an empty string, or 0 if
 *   the string is NULL or has a length greater
 *   than zero
 **/
int isEmptyString(char *str) {
    return str != NULL && strequals(str, "");
}

int isNullOrEmptyString(char *str) {
    return str == NULL || isEmptyString(str);
}

/**
 * Checks for equality between two waypoint names.
 * 
 * Parameters:
 *   ptr1 -- the first waypoint to compare
 *   ptr2 -- the second waypoint to compare
 * 
 * Returns:
 *   true (1) if the waypoints have the same name, or
 *   false (0) if the waypoints have different names
 **/
bool compareWaypointsByName(const void *ptr1, const void *ptr2) {
    Waypoint *wpt1;
    Waypoint *wpt2;

    if (ptr1 == NULL || ptr2 == NULL) {
        return false;
    }

    wpt1 = (Waypoint *)ptr1;
    wpt2 = (Waypoint *)ptr2;

    return strequals(wpt1->name, wpt2->name);
}

/**
 * Checks for equality between two route names.
 * 
 * Parameters:
 *   ptr1 -- the first route to compare
 *   ptr2 -- the second route to compare
 * 
 * Returns:
 *   true (1) if the routes have the same name, or
 *   false (0) if the routes have different names
 **/
bool compareRoutesByName(const void *ptr1, const void *ptr2) {
    Route *rte1;
    Route *rte2;

    if (ptr1 == NULL || ptr2 == NULL) {
        return false;
    }

    rte1 = (Route *)ptr1;
    rte2 = (Route *)ptr2;

    return strequals(rte1->name, rte2->name);
}

/**
 * Checks for equality between two track names.
 * 
 * Parameters:
 *   ptr1 -- the first track to compare
 *   ptr2 -- the second track to compare
 * 
 * Returns:
 *   true (1) if the tracks have the same name, or
 *   false (0) if the tracks have different names
 **/
bool compareTracksByName(const void *ptr1, const void *ptr2) {
    Track *trk1;
    Track *trk2;

    if (ptr1 == NULL || ptr2 == NULL) {
        return false;
    }

    trk1 = (Track *)ptr1;
    trk2 = (Track *)ptr2;

    return strequals(trk1->name, trk2->name);
}