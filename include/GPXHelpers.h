/**
 * Name: GPXHelpers.h
 * Author: Ethan Rowan (1086586)
 * Date Created: 01/19/2021
 * Last Modified: 02/1/2021
 */

#ifndef GPXHELPERS_H
#define GPXHELPERS_H

#include <stdbool.h>
#include "GPXParser.h"

#define strequals(a, b) (!strcmp(a, b))

/* Required list API functions */

void deleteGpxData(void *data);
char *gpxDataToString(void *data);
int compareGpxData(const void *first, const void *second);

void deleteWaypoints(void *data);
char *waypointToString(void *data);
int compareWaypoints(const void *first, const void *second);

void deleteRoute(void *data);
char *routeToString(void *data);
int compareRoutes(const void *first, const void *second);

void deleteTrackSegment(void *data);
char *trackSegmentToString(void *data);
int compareTrackSegments(const void *first, const void *second);

void deleteTrack(void *data);
char *trackToString(void *data);
int compareTracks(const void *first, const void *second);

/* Custom helper functions */

int isEmptyString(char *str);
int isNullOrEmptyString(char *str);
int getGPXDataStringSize(GPXData *gpxData);
int getWaypointStringSize(Waypoint *waypoint, int indentLevel);
int getRouteStringSize(Route *route);
int getTrackSegmentStringSize(TrackSegment *trackSegment);
int getTrackStringSize(Track *track);
int getListStringSize(List *list, int indentLevel);

/* Custom list API functions */

bool compareWaypointsByName(const void *ptr1, const void *ptr2);
bool compareRoutesByName(const void *ptr1, const void *ptr2);
bool compareTracksByName(const void *ptr1, const void *ptr2);

#endif