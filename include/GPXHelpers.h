/**
 * Name: GPXHelpers.h
 * Author: Ethan Rowan (1086586)
 * Date Created: 01/19/2021
 * Last Modified: 01/22/2021
 */

#ifndef GPXHELPERS_H
#define GPXHELPERS_H

#define strequals(a, b) (!strcmp(a, b))

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


int isEmptyString(char *str);
int isNullOrEmptyString(char *str);
int getGPXDataSizeInBytes(void *data);
int getWaypointSizeInBytes(void *data);
int getRouteSizeInBytes(void *data);
int getTrackSegmentSizeInBytes(void *data);
int getTrackSizeInBytes(void *data);
int getListSizeInBytes(List *list, int (*size_fn)(void *));

#endif