/**
 * Name: GPXParser.c
 * Author: Ethan Rowan (1086586)
 * Date Created: 01/19/2021
 * Last Modified: 01/22/2021
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

/**
 * Parses a Waypoint from a <wpt> node
 **/
Waypoint *createWaypoint(xmlNode *wptNode) {
    printf("CREATING WAYPOINT\n");
    Waypoint *wpt = malloc(sizeof(Waypoint));
    if (wpt == NULL) {
        return NULL; // malloc failed; fatal
    }

    wpt->name = NULL;
    wpt->latitude = INVALID_LATITUDE;
    wpt->longitude = INVALID_LONGITUDE;

    wpt->otherData = initializeList(gpxDataToString, deleteGpxData, compareGpxData);

    xmlAttr *attr = wptNode->properties;
    xmlNode *val;
    while (attr) {
        if (attr->name != NULL) {
            val = attr->children;
            if (strequals((char *)attr->name, "latitude")) {
                int result = sscanf((char *)val->content, "%lf", &(wpt->latitude));
                if (result != 1) {
                    wpt->latitude = INVALID_LATITUDE;
                }
            }
            else if (strequals((char *)attr->name, "longitude")) {
                int result = sscanf((char *)val->content, "%lf", &(wpt->longitude));
                if (result != 1) {
                    wpt->longitude = INVALID_LONGITUDE;
                }
            }
        }
        attr = attr->next;
    }

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
                    insertBack(wpt->otherData, (char *)content);
                }
                xmlFree(content);
            }
        }
        child = child->next;
    }

    if (wpt->name == NULL) {
        deleteWaypoint(wpt);
        return NULL;
    }

    return wpt;
}

Route *createRoute(xmlNode *rteNode) {
    // TODO: allocate a Route structure pointer and return it
    printf("CREATING ROUTE\n");
}

Track *createTrack(xmlNode *trkNode) {
    // TODO: allocate a Track structure pointer and return it
    printf("CREATING TRACK\n");
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

    // Invalid initializers; will be checked before return (otherwise NULL will be returned)
    gpxDoc->version = -1;
    gpxDoc->creator = NULL;

    // Valid initializers
    strcpy(gpxDoc->namespace, (char *)namespace->href);
    gpxDoc->waypoints = initializeList(waypointToString, deleteWaypoint, compareWaypoints);
    gpxDoc->routes = initializeList(routeToString, deleteRoute, compareRoutes);
    gpxDoc->tracks = initializeList(trackToString, deleteTrack, compareTracks);

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
            printf("%s\n", child->name);
            if (strequals((char *)child->name, "wpt")) {
                Waypoint *wpt = createWaypoint(child);
                if (wpt != NULL) {
                    insertBack(gpxDoc->waypoints, wpt);
                }
            }
            else if (strequals((char *)child->name, "rte")) {
                Route *rte = createRoute(child);
                if (rte != NULL) {
                    // insertBack(gpxDoc->routes, rte);
                }
            }
            else if (strequals((char *)child->name, "trk")) {
                Track *trk = createTrack(child);
                if (trk != NULL) {
                    // insertBack(gpxDoc->waypoints, trk);
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
    sprintf(str, "namespace: %s\n", doc->namespace);
    sprintf(str + strlen(str), "version: %lf\n", doc->version);
    sprintf(str + strlen(str), "creator: %s\n", doc->creator);
    sprintf(str + strlen(str), "waypoints: %s\n", waypoints);
    sprintf(str + strlen(str), "routes: %s\n", routes);
    sprintf(str + strlen(str), "tracks: %s", tracks);
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
