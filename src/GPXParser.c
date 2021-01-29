/**
 * Name: GPXParser.c
 * Author: Ethan Rowan (1086586)
 * Date Created: 01/19/2021
 * Last Modified: 01/28/2021
 */

#include <stdlib.h>
#include "GPXParser.h"
#include "GPXHelpers.h"

#define INVALID_LATITUDE -200
#define INVALID_LONGITUDE -200

/**
 * Gets the root xmlNode containing the <gpx> tag and returns it.
 * If the <gpx> tag does not exist, returns NULL.
 **/
xmlNode *getGPXRoot(xmlDoc *xml) {
    xmlNode *gpx = xmlDocGetRootElement(xml);
    if (gpx != NULL && strequals((char *)(gpx->name), "gpx")) {
        return gpx;
    }
    return NULL; // no <gpx> tag in the xml file
}

GPXData *createGPXData(char *name, char *value) {
    GPXData *data = malloc(sizeof(GPXData)+strlen(value)+1);
    if (data == NULL) {
        return NULL; // malloc failed; fatal
    }
    strcpy(data->name, name);
    strcpy(data->value, value);
    return data;
}

/**
 * Parses a Waypoint from a <wpt> node
 **/
Waypoint *createWaypoint(xmlNode *wptNode) {
    Waypoint *wpt = malloc(sizeof(Waypoint));
    if (wpt == NULL) {
        return NULL; // malloc failed; fatal
    }

    // Invalid initializers; will be checked before return
    wpt->name = NULL;
    wpt->latitude = INVALID_LATITUDE;
    wpt->longitude = INVALID_LONGITUDE;

    // Valid initializers
    wpt->otherData = initializeList(gpxDataToString, deleteGpxData, compareGpxData);

    // Parse waypoint attributes (lat and lon)
    xmlAttr *attr = wptNode->properties;
    xmlNode *val;
    while (attr) {
        if (attr->name != NULL) {
            val = attr->children;
            if (strequals((char *)attr->name, "lat")) {
                int result = sscanf((char *)val->content, "%lf", &(wpt->latitude));
                if (result != 1) {
                    wpt->latitude = INVALID_LATITUDE;
                }
            }
            else if (strequals((char *)attr->name, "lon")) {
                int result = sscanf((char *)val->content, "%lf", &(wpt->longitude));
                if (result != 1) {
                    wpt->longitude = INVALID_LONGITUDE;
                }
            }
        }
        attr = attr->next;
    }

    // Parse waypoint children (name and other data)
    xmlNode *child = wptNode->children;
    xmlChar *content;
    while (child) {
        if (child->name != NULL && child->type == XML_ELEMENT_NODE) {
            content = xmlNodeGetContent(child);
            if (content != NULL) {
                if (strequals((char *)child->name, "name")) {
                    wpt->name = malloc(xmlStrlen(content)+1);
                    strcpy(wpt->name, (char *)content);
                }
                else {
                    GPXData *data = createGPXData((char *)child->name, (char *)content);
                    if (data != NULL) {
                        insertBack(wpt->otherData, data);
                    }
                }
                xmlFree(content);
            }
        }
        child = child->next;
    }

    // Recheck invalid initializers
    if (wpt->name == NULL) {
        wpt->name = malloc(1);
        wpt->name[0] = '\0';
    }

    // Recheck invalid initializers (must pass)
    if (wpt->latitude == -200 || wpt->longitude == -200) {
        deleteWaypoint(wpt);
        return NULL;
    }

    return wpt;
}

/**
 * Parses a route from a <rte> node.
 **/
Route *createRoute(xmlNode *rteNode) {
    Route *rte = malloc(sizeof(Route));
    if (rte == NULL) {
        return NULL; // malloc failed; fatal
    }

    // Invalid initializers; will be checked before return
    rte->name = NULL;

    // Valid initializers
    rte->waypoints = initializeList(waypointToString, deleteWaypoint, compareWaypoints);
    rte->otherData = initializeList(gpxDataToString, deleteGpxData, compareGpxData);

    // Parse route children (rtept, name, and other data)
    xmlNode *child = rteNode->children;
    xmlChar *content;
    while (child) {
        if (child->name != NULL && child->type == XML_ELEMENT_NODE) {
            // Parse rtept
            if (strequals((char *)child->name, "rtept")) {
                Waypoint *wpt = createWaypoint(child);
                if (wpt != NULL) {
                    insertBack(rte->waypoints, wpt);
                }
            }
            else {
                // Parse nodes with content
                content = xmlNodeGetContent(child);
                if (content != NULL) {
                    // Parse name
                    if (strequals((char *)child->name, "name")) {
                        rte->name = malloc(xmlStrlen(content)+1);
                        strcpy(rte->name, (char *)content);
                    }
                    else {
                        GPXData *data = createGPXData((char *)child->name, (char *)content);
                        if (data != NULL) {
                            insertBack(rte->otherData, data);
                        }
                    }
                    xmlFree(content);
                }
            }
        }
        child = child->next;
    }

    // Recheck invalid initializer
    if (rte->name == NULL) {
        rte->name = malloc(1);
        rte->name[0] = '\0';
    }

    return rte;
}

TrackSegment *createTrackSegment(xmlNode *trkSegNode)
{
    TrackSegment *trkSeg = malloc(sizeof(TrackSegment));
    if (trkSeg == NULL) {
        return NULL;
    }

    trkSeg->waypoints = initializeList(waypointToString, deleteWaypoint, compareWaypoints);

    // Parse route children (trkpt)
    xmlNode *child = trkSegNode->children;
    while (child) {
        if (child->name != NULL && child->type == XML_ELEMENT_NODE) {
            // Parse trkpt
            if (strequals((char *)child->name, "trkpt")) {
                Waypoint *trkpt = createWaypoint(child);
                if (trkpt != NULL) {
                    insertBack(trkSeg->waypoints, trkpt);
                }
            }
        }
        child = child->next;
    }

    return trkSeg;
}

Track *createTrack(xmlNode *trkNode) {
    Track *trk = malloc(sizeof(Track));
    if (trk == NULL) {
        return NULL; // malloc failed; fatal
    }

    // Invalid initializers; will be checked before return
    trk->name = NULL;

    // Valid initializers
    trk->segments = initializeList(trackSegmentToString, deleteTrackSegment, compareTrackSegments);
    trk->otherData = initializeList(gpxDataToString, deleteGpxData, compareGpxData);

    // Parse route children (trkseg, name, and other data)
    xmlNode *child = trkNode->children;
    xmlChar *content;
    while (child) {
        if (child->name != NULL && child->type == XML_ELEMENT_NODE) {
            // Parse trkseg
            if (strequals((char *)child->name, "trkseg")) {
                TrackSegment *trkSeg = createTrackSegment(child);
                if (trkSeg != NULL) {
                    insertBack(trk->segments, trkSeg);
                }
            }
            else {
                // Parse nodes with content
                content = xmlNodeGetContent(child);
                if (content != NULL) {
                    // Parse name
                    if (strequals((char *)child->name, "name")) {
                        trk->name = malloc(xmlStrlen(content)+1);
                        strcpy(trk->name, (char *)content);
                    }
                    else {
                        GPXData *data = createGPXData((char *)child->name, (char *)content);
                        if (data != NULL) {
                            insertBack(trk->otherData, data);
                        }
                    }
                    xmlFree(content);
                }
            }
        }
        child = child->next;
    }

    // Recheck invalid initializer
    if (trk->name == NULL) {
        trk->name = malloc(1);
        trk->name[0] = '\0';
    }

    return trk;
}

/**
 * Initializes a GPXdoc pointer with the attributes in the <gpx> tag.
 * Returns a pointer to a new GPXdoc structure, or NULL if the <gpx>
 **/
GPXdoc *initializeGPXdoc(xmlNode *xmlRoot) {
    xmlNs *namespace = xmlRoot->ns;
    if (namespace == NULL) {
        return NULL; // Namespace is not set
    }

    GPXdoc *gpxDoc = malloc(sizeof(GPXdoc));
    if (gpxDoc == NULL) {
        return NULL; // malloc() failed; fatal
    }

    // Invalid initializers; will be checked before return (if still invalid, NULL will be returned)
    gpxDoc->version = -1;
    gpxDoc->creator = NULL;

    // Valid initializers
    strcpy(gpxDoc->namespace, (char *)namespace->href);
    gpxDoc->waypoints = initializeList(waypointToString, deleteWaypoint, compareWaypoints);
    gpxDoc->routes = initializeList(routeToString, deleteRoute, compareRoutes);
    gpxDoc->tracks = initializeList(trackToString, deleteTrack, compareTracks);

    // Parse gpx attributes (version and creator)
    xmlAttr *attr = xmlRoot->properties;
    xmlNode *val;
    while (attr) {
        if (attr->name != NULL) {
            val = attr->children;
            if (strequals((char *)attr->name, "version")) {
                int result = sscanf((char *)val->content, "%lf", &(gpxDoc->version));
                if (result != 1) {
                    gpxDoc->version = -1;
                }
            }
            else if (strequals((char *)attr->name, "creator")) {
                gpxDoc->creator = malloc(xmlStrlen(val->content)+1);
                strcpy(gpxDoc->creator, (char *)val->content);
            }
        }
        attr = attr->next;
    }

    // Namespace MUST not be empty
    if (strequals(gpxDoc->namespace, "")) {
        deleteGPXdoc(gpxDoc);
        return NULL;
    }

    // Version MUST be positive
    if (gpxDoc->version == -1) {
        deleteGPXdoc(gpxDoc);
        return NULL; // version and creator must not be NULL
    }

    // Creator MUST not be NULL or an empty string
    if (isNullOrEmptyString(gpxDoc->creator)) {
        deleteGPXdoc(gpxDoc);
        return NULL;
    }
    
    return gpxDoc;
}

GPXdoc *createGPXdoc(char *fileName) {
    xmlDoc *xml; // Parsed xml document
    xmlNode *xmlRoot; // <gpx> root node
    
    xml = xmlReadFile(fileName, NULL, 0);
    if (xml == NULL) {
        return NULL; // Parsing failed
    }
    xmlRoot = getGPXRoot(xml);
    if (xmlRoot == NULL) {
        return NULL; // File does not contain <gpx> tag
    }

    GPXdoc *gpxDoc = initializeGPXdoc(xmlRoot);
    if (gpxDoc == NULL) {
        return NULL;
    }

    xmlNode *child = xmlRoot->children;
    while (child) {
        if (child->name != NULL && child->type == XML_ELEMENT_NODE) {
            if (strequals((char *)child->name, "wpt")) {
                Waypoint *wpt = createWaypoint(child);
                if (wpt != NULL) {
                    insertBack(gpxDoc->waypoints, wpt);
                }
            }
            else if (strequals((char *)child->name, "rte")) {
                Route *rte = createRoute(child);
                if (rte != NULL) {
                    insertBack(gpxDoc->routes, rte);
                }
            }
            else if (strequals((char *)child->name, "trk")) {
                Track *trk = createTrack(child);
                if (trk != NULL) {
                    insertBack(gpxDoc->tracks, trk);
                }
            }
        }
        child = child->next;
    }
    
    xmlFreeDoc(xml);
    xmlCleanupParser();

    return gpxDoc;
}

char *GPXdocToString(GPXdoc *doc) {
    char *waypoints = toString(doc->waypoints);
    char *routes = toString(doc->routes);
    char *tracks = toString(doc->tracks);
    if (waypoints == NULL || routes == NULL || tracks == NULL) {
        return NULL;
    }

    int size = strlen(doc->namespace) + sizeof(doc->version) +
            strlen(doc->creator) + strlen(waypoints) +
            strlen(routes) + strlen(tracks) + 64;
    char *str = malloc(size);
    if (str == NULL) {
        return NULL; // malloc failed; fatal
    }
    sprintf(str, "NAMESPACE: %s\n", doc->namespace);
    sprintf(str + strlen(str), "VERSION: %lf\n", doc->version);
    sprintf(str + strlen(str), "CREATOR: %s\n", doc->creator);
    sprintf(str + strlen(str), "WAYPOINTS: %s\n", waypoints);
    sprintf(str + strlen(str), "ROUTES: %s\n", routes);
    sprintf(str + strlen(str), "TRACKS: %s", tracks);
    free(waypoints);
    free(routes);
    free(tracks);
    return str;
}

void deleteGPXdoc(GPXdoc *doc) {
    if (doc == NULL) {
        return;
    }
    if (doc->creator != NULL) {
        free(doc->creator);
    }
    if (doc->waypoints != NULL) {
        freeList(doc->waypoints);
    }
    if (doc->routes != NULL) {
        freeList(doc->routes);
    }
    if (doc->tracks != NULL) {
        freeList(doc->tracks);
    }
    free(doc);
}

int getNumWaypoints(const GPXdoc *doc) {
    if (doc == NULL) {
        return 0;
    }

    if (doc->waypoints == NULL) {
        return 0;
    }

    return getLength(doc->waypoints);
}

int getNumRoutes(const GPXdoc *doc) {
    if (doc == NULL) {
        return 0;
    }

    if (doc->routes == NULL) {
        return 0;
    }

    return getLength(doc->routes);
}

int getNumTracks(const GPXdoc *doc) {
    if (doc == NULL) {
        return 0;
    }

    if (doc->tracks == NULL) {
        return 0;
    }

    return getLength(doc->tracks);
}

int getNumSegments(const GPXdoc *doc) {
    if (doc == NULL) {
        return 0;
    }

    if (doc->tracks == NULL) {
        return 0;
    }

    int numSegments = 0;
    void *element;
    Track *track;
    ListIterator it = createIterator(doc->tracks);
    while ((element = nextElement(&it))) {
        track = (Track *)element;
        if (track->segments != NULL) {
            numSegments += getLength(track->segments);
        }
    }

    return numSegments;
}

int getNumWaypointsGPXData(List *waypoints) {
    if (waypoints == NULL) {
        return 0;
    }
    int numData = 0;
    void *element;
    Waypoint *waypoint;
    ListIterator it = createIterator(waypoints);
    while ((element = nextElement(&it))) {
        waypoint = (Waypoint *)element;
        if (strlen(waypoint->name) > 0) {
            numData++;
        }
        if (waypoint->otherData != NULL) {
            numData += getLength(waypoint->otherData);
        }
    }
    return numData;
}

int getNumTrackSegmentsGPXData(List *trackSegments) {
    if (trackSegments == NULL) {
        return 0;
    }
    int numData = 0;
    void *element;
    TrackSegment *trackSegment;
    ListIterator it = createIterator(trackSegments);
    while ((element = nextElement(&it))) {
        trackSegment = (TrackSegment *)element;
        numData += getNumWaypointsGPXData(trackSegment->waypoints);
    }
    return numData;
}

int getNumGPXData(const GPXdoc *doc) {
    if (doc == NULL) {
        return 0;
    }

    if (doc->waypoints == NULL || doc->routes == NULL || doc->tracks == NULL) {
        return 0;
    }

    int numData = 0;
    
    numData += getNumWaypointsGPXData(doc->waypoints);

    void *element;
    Route *route;
    ListIterator it = createIterator(doc->routes);
    while ((element = nextElement(&it))) {
        route = (Route *)element;
        if (strlen(route->name) > 0) {
            numData++;
        }
        numData += getNumWaypointsGPXData(route->waypoints);
        if (route->otherData != NULL) {
            numData += getLength(route->otherData);
        }
    }

    Track *track;
    it = createIterator(doc->tracks);
    while ((element = nextElement(&it))) {
        track = (Track *)element;
        if (strlen(track->name) > 0) {
            numData++;
        }
        if (track->otherData != NULL) {
            numData += getLength(track->otherData);
        }
        numData += getNumTrackSegmentsGPXData(track->segments);
    }

    return numData;
}

Waypoint *getWaypoint(const GPXdoc *doc, char *name) {
    if (doc == NULL) {
        return NULL;
    }
    if (doc->waypoints == NULL) {
        return NULL;
    }
    Waypoint *waypoint;
    void *element;
    ListIterator it = createIterator(doc->waypoints);
    while ((element = nextElement(&it))) {
        waypoint = (Waypoint *)element;
        if (strequals(waypoint->name, name)) {
            return waypoint;
        }
    }
    return NULL;
}

Track *getTrack(const GPXdoc *doc, char *name) {
    if (doc == NULL) {
        return NULL;
    }
    if (doc->tracks == NULL) {
        return NULL;
    }
    Track *track;
    void *element;
    ListIterator it = createIterator(doc->tracks);
    while ((element = nextElement(&it))) {
        track = (Track *)element;
        if (strequals(track->name, name)) {
            return track;
        }
    }
    return NULL;
}

Route *getRoute(const GPXdoc *doc, char *name) {
    if (doc == NULL) {
        return NULL;
    }
    if (doc->routes == NULL) {
        return NULL;
    }
    Route *route;
    void *element;
    ListIterator it = createIterator(doc->routes);
    while ((element = nextElement(&it))) {
        route = (Route *)element;
        if (strequals(route->name, name)) {
            return route;
        }
    }
    return NULL;
}
