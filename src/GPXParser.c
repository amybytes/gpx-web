/**
 * Name: GPXParser.c
 * Author: Ethan Rowan (1086586)
 * Date Created: 01/19/2021
 * Last Modified: 02/22/2021
 */

#include <stdlib.h>
#include "GPXParser.h"
#include "GPXHelpers.h"

#define INVALID_LATITUDE -200
#define INVALID_LONGITUDE -200

xmlNode *createXMLGPXData(GPXData *gpxData, xmlNode *parent) {
    if (gpxData == NULL) {
        return NULL;
    }
    return xmlNewChild(parent, NULL, BAD_CAST gpxData->name, BAD_CAST gpxData->value); 
}

xmlNode *createXMLWaypoint(Waypoint *waypoint, xmlNode *parent, char *name) {
    xmlNode *wpt = NULL;
    char *latitude;
    char *longitude;
    ListIterator it;
    void *element;

    if (waypoint == NULL) {
        return NULL;
    }
    
    latitude = malloc(20);
    sprintf(latitude, "%lf", waypoint->latitude);
    longitude = malloc(20);
    sprintf(longitude, "%lf", waypoint->longitude);

    wpt = xmlNewChild(parent, NULL, BAD_CAST name, NULL);
    if (wpt == NULL) {
        return NULL;
    }
    /* Add latitude and longitude properties to <wpt> */
    xmlNewProp(wpt, BAD_CAST "lat", BAD_CAST latitude);
    xmlNewProp(wpt, BAD_CAST "lon", BAD_CAST longitude);
    free(latitude);
    free(longitude);

    /* Create <name> node and add it to <wpt> */
    if (!isEmptyString(waypoint->name)) {
        xmlNewChild(wpt, NULL, BAD_CAST "name", BAD_CAST waypoint->name);
    }

    /* Add all other data to <wpt> */
    it = createIterator(waypoint->otherData);
    while ((element = nextElement(&it)) != NULL) {
        if (!createXMLGPXData((GPXData *)element, wpt)) {
            xmlFreeNode(wpt);
            return NULL;
        }
    }

    return wpt;
}

xmlNode *createXMLRoute(Route *route, xmlNode *parent) {
    xmlNode *rte = NULL;
    ListIterator it;
    void *element;

    if (route == NULL) {
        return NULL;
    }

    rte = xmlNewChild(parent, NULL, BAD_CAST "rte", NULL);
    if (rte == NULL) {
        return NULL;
    }

    /* Create <name> node and add it to <rte> */
    xmlNewChild(rte, NULL, BAD_CAST "name", BAD_CAST route->name);

    /* Add all other data to <rte> */
    it = createIterator(route->otherData);
    while ((element = nextElement(&it)) != NULL) {
        if (!createXMLGPXData((GPXData *)element, rte)) {
            xmlFreeNode(rte);
            return NULL;
        }
    }

    /* Add all waypoints to <rte> */
    it = createIterator(route->waypoints);
    while ((element = nextElement(&it)) != NULL) {
        if (!createXMLWaypoint((Waypoint *)element, rte, "rtept")) {
            xmlFreeNode(rte);
            return NULL;
        }
    }

    return rte;
}

xmlNode *createXMLTrackSegment(TrackSegment *trackSegment, xmlNode *parent) {
    xmlNode *trkSeg = NULL;
    ListIterator it;
    void *element;

    if (trackSegment == NULL) {
        return NULL;
    }

    trkSeg = xmlNewChild(parent, NULL, BAD_CAST "trkseg", NULL);
    if (trkSeg == NULL) {
        return NULL;
    }

    /* Add all waypoints to <trkseg> */
    it = createIterator(trackSegment->waypoints);
    while ((element = nextElement(&it)) != NULL) {
        if (!createXMLWaypoint((Waypoint *)element, trkSeg, "trkpt")) {
            xmlFreeNode(trkSeg);
            return NULL;
        }
    }

    return trkSeg;
}

xmlNode *createXMLTrack(Track *track, xmlNode *parent) {
    xmlNode *trk = NULL;
    ListIterator it;
    void *element;

    if (track == NULL) {
        return NULL;
    }

    trk = xmlNewChild(parent, NULL, BAD_CAST "trk", NULL);
    if (trk == NULL) {
        return NULL;
    }

    /* Create <name> node and add it to <trk> */
    xmlNewChild(trk, NULL, BAD_CAST "name", BAD_CAST track->name);

    /* Add all other data to <trk> */
    it = createIterator(track->otherData);
    while ((element = nextElement(&it)) != NULL) {
        if (!createXMLGPXData((GPXData *)element, trk)) {
            xmlFreeNode(trk);
            return NULL;
        }
    }

    /* Add all waypoints to <trk> */
    it = createIterator(track->segments);
    while ((element = nextElement(&it)) != NULL) {
        if (!createXMLTrackSegment((TrackSegment *)element, trk)) {
            xmlFreeNode(trk);
            return NULL;
        }
    }

    return trk;
}

xmlDoc *createXMLdoc(GPXdoc *gpxDoc) {
    xmlDoc *xml = NULL;
    xmlNode *root = NULL;
    ListIterator it;
    void *element;
    char *version;

    if (gpxDoc == NULL) {
        return NULL;
    }

    version = malloc(10);
    sprintf(version, "%.1lf", gpxDoc->version);
    xml = xmlNewDoc(BAD_CAST "1.0");
    root = xmlNewNode(NULL, BAD_CAST "gpx");
    xmlDocSetRootElement(xml, root);

    /* Set <gpx> namespace and attributes */
    xmlNs *ns = xmlNewNs(root, BAD_CAST gpxDoc->namespace, NULL);
    xmlSetNs(root, ns);
    xmlSetProp(root, BAD_CAST "version", BAD_CAST version);
    xmlSetProp(root, BAD_CAST "creator", BAD_CAST gpxDoc->creator);
    free(version);

    /* Add waypoint nodes */
    it = createIterator(gpxDoc->waypoints);
    while ((element = nextElement(&it)) != NULL) {
        if (!createXMLWaypoint((Waypoint *)element, root, "wpt")) {
            xmlFreeDoc(xml);
            return NULL;
        }
    }

    /* Add route nodes */
    it = createIterator(gpxDoc->routes);
    while ((element = nextElement(&it)) != NULL) {
        if (!createXMLRoute((Route *)element, root)) {
            xmlFreeDoc(xml);
            return NULL;
        }
    }

    /* Add track nodes */
    it = createIterator(gpxDoc->tracks);
    while ((element = nextElement(&it)) != NULL) {
        if (!createXMLTrack((Track *)element, root)) {
            xmlFreeDoc(xml);
            return NULL;
        }
    }

    return xml;
}

/**
 * Gets the root xmlNode containing the <gpx> tag and returns it.
 * If the <gpx> tag does not exist, returns NULL.
 * 
 * Parameters:
 *   xml -- the first node in the xml document
 * 
 * Returns:
 *   The gpx (root) node
 **/
xmlNode *getGPXRoot(xmlDoc *xml) {
    xmlNode *gpx = xmlDocGetRootElement(xml);
    if (gpx != NULL && strequals((char *)(gpx->name), "gpx")) {
        return gpx;
    }
    return NULL; // no <gpx> tag in the xml file
}

/**
 * Creates a new GPXData structure with a given name and value.
 * 
 * Parameters:
 *   name -- the name of the data
 *   value -- the value of the data
 * 
 * Returns:
 *   A pointer to a new GPXData structure
 **/
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
 * Parses a Waypoint from a <wpt> node.
 * 
 * Parameters:
 *   wptNode -- the xml node containing the waypoint data
 * 
 * Returns:
 *   A pointer to a new Waypoint structure
 **/
Waypoint *createWaypoint(xmlNode *wptNode) {
    Waypoint *wpt = malloc(sizeof(Waypoint));
    if (wpt == NULL) {
        return NULL; // malloc failed; fatal
    }

    int latInitialized = 0; // keep track of whether latitude is initialized or not
    int lonInitialized = 0; // keep track of whether longitude is initialized or not

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
                if (result == 1) {
                    latInitialized = 1;
                }
            }
            else if (strequals((char *)attr->name, "lon")) {
                int result = sscanf((char *)val->content, "%lf", &(wpt->longitude));
                if (result == 1) {
                    lonInitialized = 1;
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
                    if (wpt->name == NULL) {
                        break;
                    }
                    strcpy(wpt->name, (char *)content);
                }
                else {
                    GPXData *data = createGPXData((char *)child->name, (char *)content);
                    if (data != NULL) {
                        insertBack(wpt->otherData, data);
                    }
                    else {
                        deleteWaypoint(wpt);
                        return NULL;
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
    if (!latInitialized || !lonInitialized) {
        deleteWaypoint(wpt);
        return NULL;
    }

    return wpt;
}

/**
 * Parses a route from a <rte> node.
 * 
 * Parameters:
 *   rteNode -- the xml node containing the route data
 * 
 * Returns:
 *   A pointer to a new Route structure
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
                else {
                    deleteRoute(rte);
                    return NULL;
                }
            }
            else {
                // Parse nodes with content
                content = xmlNodeGetContent(child);
                if (content != NULL) {
                    // Parse name
                    if (strequals((char *)child->name, "name")) {
                        rte->name = malloc(xmlStrlen(content)+1);
                        if (rte->name == NULL) {
                            break;
                        }
                        strcpy(rte->name, (char *)content);
                    }
                    else {
                        GPXData *data = createGPXData((char *)child->name, (char *)content);
                        if (data != NULL) {
                            insertBack(rte->otherData, data);
                        }
                        else {
                            deleteRoute(rte);
                            return NULL;
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

/**
 * Parses a track segment from a <trkseg> node.
 * 
 * Parameters:
 *   trkSegNode -- the xml node containing the track segment data
 * 
 * Returns:
 *   A pointer to a new TrackSegment structure
 **/
TrackSegment *createTrackSegment(xmlNode *trkSegNode)
{
    TrackSegment *trkSeg = malloc(sizeof(TrackSegment));
    if (trkSeg == NULL) {
        return NULL; // malloc failed; fatal
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
                else {
                    deleteTrackSegment(trkSeg);
                    return NULL;
                }
            }
        }
        child = child->next;
    }

    return trkSeg;
}

/**
 * Parses a track from a <trk> node.
 * 
 * Parameters:
 *   trkNode -- the xml node containing the track data
 * 
 * Returns:
 *   A pointer to a new Track structure
 **/
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
                else {
                    deleteTrack(trk);
                    return NULL;
                }
            }
            else {
                // Parse nodes with content
                content = xmlNodeGetContent(child);
                if (content != NULL) {
                    // Parse name
                    if (strequals((char *)child->name, "name")) {
                        trk->name = malloc(xmlStrlen(content)+1);
                        if (trk->name == NULL) {
                            break;
                        }
                        strcpy(trk->name, (char *)content);
                    }
                    else {
                        GPXData *data = createGPXData((char *)child->name, (char *)content);
                        if (data != NULL) {
                            insertBack(trk->otherData, data);
                        }
                        else {
                            deleteTrack(trk);
                            return NULL;
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
 * 
 * Parameters:
 *   xmlRoot -- the root xml node (<gpx> tag)
 * 
 * Returns:
 *   A pointer to a new GPXdoc structure, or NULL if the gpx data is invalid,
 *   or a parsing error occurs
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
                if (gpxDoc->creator == NULL) {
                    break;
                }
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

/**
 * Creates a new GPX document from a gpx file.
 * 
 * Parameters:
 *   fileName -- the name of the gpx file
 * 
 * Returns:
 *   A pointer to a new GPXdoc structure, or NULL if a parsing error occurs
 **/
GPXdoc *createGPXdoc(char *fileName) {
    xmlDoc *xml; // Parsed xml document
    xmlNode *xmlRoot; // <gpx> root node

    if (fileName == NULL) {
        return NULL;
    }
    
    xml = xmlReadFile(fileName, NULL, 0);
    if (xml == NULL) {
        xmlCleanupParser();
        return NULL; // xml parsing failed
    }
    xmlRoot = getGPXRoot(xml);
    if (xmlRoot == NULL) {
        xmlFreeDoc(xml);
        xmlCleanupParser();
        return NULL; // File does not contain <gpx> tag
    }

    GPXdoc *gpxDoc = initializeGPXdoc(xmlRoot);
    if (gpxDoc == NULL) {
        xmlFreeDoc(xml);
        xmlCleanupParser();
        return NULL;
    }

    bool parsingErr = 0;

    xmlNode *child = xmlRoot->children;
    while (child) {
        if (child->name != NULL && child->type == XML_ELEMENT_NODE) {
            if (strequals((char *)child->name, "wpt")) {
                Waypoint *wpt = createWaypoint(child);
                if (wpt != NULL) {
                    insertBack(gpxDoc->waypoints, wpt);
                }
                else {
                    parsingErr = 1; // The parsed waypoint is invalid
                }
            }
            else if (strequals((char *)child->name, "rte")) {
                Route *rte = createRoute(child);
                if (rte != NULL) {
                    insertBack(gpxDoc->routes, rte);
                }
                else {
                    parsingErr = 1; // The parsed route is invalid
                }
            }
            else if (strequals((char *)child->name, "trk")) {
                Track *trk = createTrack(child);
                if (trk != NULL) {
                    insertBack(gpxDoc->tracks, trk);
                }
                else {
                    parsingErr = 1; // The parsed track is invalid
                }
            }

            if (parsingErr) {
                xmlFreeDoc(xml);
                xmlCleanupParser();
                deleteGPXdoc(gpxDoc);
                return NULL;
            }
        }
        child = child->next;
    }
    
    xmlFreeDoc(xml);
    xmlCleanupParser();

    return gpxDoc;
}

/**
 * Converts a GPX document to a readable string.
 * 
 * Parameters:
 *   doc -- the GPX document to convert
 * 
 * Returns:
 *   A string representing the GPX document
 **/
char *GPXdocToString(GPXdoc *doc) {
    if (doc == NULL) {
        return NULL;
    }
    if (doc->waypoints == NULL || doc->routes == NULL || doc->tracks == NULL) {
        return NULL;
    }
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

/**
 * Deletes a GPX doc and all of its contents from memory.
 * 
 * Parameters:
 *   doc -- the GPX document to delete
 **/
void deleteGPXdoc(GPXdoc *doc) {
    if (doc == NULL) {
        return; // nothing to do
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

GPXdoc *createValidGPXdoc(char *fileName, char *gpxSchemaFile) {
    GPXdoc *doc = createGPXdoc(fileName);
    if (validateGPXDoc(doc, gpxSchemaFile)) {
        return doc;
    }
    deleteGPXdoc(doc);
    return NULL;
}

bool validateGPXDoc(GPXdoc *doc, char *fileName) {
    xmlDoc *xml;
    xmlSchema *schema;
    xmlSchemaParserCtxt *parserCtxt;
    xmlSchemaValidCtxt *validCtxt;

    parserCtxt = xmlSchemaNewParserCtxt(fileName);
    schema = xmlSchemaParse(parserCtxt);
    xmlSchemaFreeParserCtxt(parserCtxt);
    xml = createXMLdoc(doc);

    int status = 1;
    if (xml == NULL) {
        return false;
    }
    else {
        validCtxt = xmlSchemaNewValidCtxt(schema);
        status = xmlSchemaValidateDoc(validCtxt, xml);
        xmlSchemaFreeValidCtxt(validCtxt);
    }
    if (schema != NULL) {
        xmlSchemaFree(schema);
    }
    xmlFreeDoc(xml);
    xmlSchemaCleanupTypes();
    xmlCleanupParser();

    return status == 0;
}

bool writeGPXdoc(GPXdoc *doc, char *fileName) {
    xmlDoc *xml = createXMLdoc(doc);
    if (xml == NULL) {
        return NULL;
    }
    int status = xmlSaveFormatFileEnc(fileName, xml, "UTF-8", 1);
    xmlFreeDoc(xml);
    xmlCleanupParser();
    return status;
}

/**
 * Gets the total number of waypoints from a GPX document.
 * This number only includes waypoints defined directly in
 * the <gpx> scope.
 * 
 * Parameters:
 *   doc -- the GPX document to evaluate
 * 
 * Returns:
 *   The number of waypoints in the GPX document
 **/
int getNumWaypoints(const GPXdoc *doc) {
    if (doc == NULL) {
        return 0; // doc is invalid; cannot evaluate
    }

    if (doc->waypoints == NULL) {
        return 0; // doc is invalid; cannot evaluate
    }

    return getLength(doc->waypoints);
}

/**
 * Gets the total number of routes from a GPX document.
 * This number only includes routes defined directly in
 * the <gpx> scope.
 * 
 * Parameters:
 *   doc -- the GPX document to evaluate
 * 
 * Returns:
 *   The number of routes in the GPX document
 **/
int getNumRoutes(const GPXdoc *doc) {
    if (doc == NULL) {
        return 0; // doc is invalid; cannot evaluate
    }

    if (doc->routes == NULL) {
        return 0; // doc is invalid; cannot evaluate
    }

    return getLength(doc->routes);
}

/**
 * Gets the total number of tracks from a GPX document.
 * This number only includes waypoints defined directly in
 * the <gpx> scope.
 * 
 * Parameters:
 *   doc -- the GPX document to evaluate
 * 
 * Returns:
 *   The number of tracks in the GPX document
 **/
int getNumTracks(const GPXdoc *doc) {
    if (doc == NULL) {
        return 0; // doc is invalid; cannot evaluate
    }

    if (doc->tracks == NULL) {
        return 0; // doc is invalid; cannot evaluate
    }

    return getLength(doc->tracks);
}

/**
 * Gets the total number of track segments from all tracks in a GPX document.
 * 
 * Parameters:
 *   doc -- the GPX document to evaluate
 * 
 * Returns:
 *   The number of track segments in the GPX document
 **/
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

/**
 * Gets the total number of data elements (GPXData structures) in a list
 * of waypoints.
 * 
 * Parameters:
 *   waypoints -- the list of waypoints to evaluate
 * 
 * Returns:
 *   The number of GPXData structures in the list of waypoints
 **/
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

/**
 * Gets the total number of data elements (GPXData structures) in a list
 * of track segments.
 * 
 * Parameters:
 *   trackSegments -- the list of trackSegments to evaluate
 * 
 * Returns:
 *   The number of GPXData structures in the list of track segments
 **/
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

/**
 * Gets the total number of data elements (GPXData structures) from a GPX document.
 * 
 * Parameters:
 *   doc -- the GPX document to evaluate
 * 
 * Returns:
 *   The number of GPXData structures in the GPX document
 **/
int getNumGPXData(const GPXdoc *doc) {
    if (doc == NULL) {
        return 0;
    }
    if (doc->waypoints == NULL || doc->routes == NULL || doc->tracks == NULL) {
        return 0; // one or more structures are invalid; cannot evaluate
    }

    int numData = 0;
    
    // Add the number of GPXData structures from waypoints
    numData += getNumWaypointsGPXData(doc->waypoints);

    // Add the number of GPXData structures from routes
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

    // Add the number of GPXData structures from tracks
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

/**
 * Gets the first waypoint from a GPXdoc with a given name.
 * 
 * Parameters:
 *   doc -- the GPXdoc to search
 *   name -- the waypoint name to search for
 * 
 * Returns:
 *   A pointer to the first waypoint, or NULL if no waypoint is found
 **/
Waypoint *getWaypoint(const GPXdoc *doc, char *name) {
    if (doc == NULL) {
        return NULL;
    }
    if (doc->waypoints == NULL || name == NULL) {
        return NULL;
    }
    // Build search target
    Waypoint targetWaypoint;
    targetWaypoint.name = malloc(strlen(name)+1);
    if (targetWaypoint.name == NULL) {
        return NULL; // malloc failed; fatal
    }
    strcpy(targetWaypoint.name, name);

    void *result = findElement(doc->waypoints, &compareWaypointsByName, &targetWaypoint);

    free(targetWaypoint.name);
    if (result == NULL) {
        return NULL; // no waypoint found
    }
    return (Waypoint *)result;
}

/**
 * Gets the first track from a GPXdoc with a given name.
 * 
 * Parameters:
 *   doc -- the GPXdoc to search
 *   name -- the track name to search for
 * 
 * Returns:
 *   A pointer to the first track, or NULL if no track is found
 **/
Track *getTrack(const GPXdoc *doc, char *name) {
    if (doc == NULL) {
        return NULL;
    }
    if (doc->tracks == NULL || name == NULL) {
        return NULL;
    }
    // Build search target
    Track targetTrack;
    targetTrack.name = malloc(strlen(name)+1);
    if (targetTrack.name == NULL) {
        return NULL; // malloc failed; fatal
    }
    strcpy(targetTrack.name, name);

    void *result = findElement(doc->tracks, &compareTracksByName, &targetTrack);

    free(targetTrack.name);
    if (result == NULL) {
        return NULL; // no track found
    }
    return (Track *)result;
}

/**
 * Gets the first route from a GPXdoc with a given name.
 * 
 * Parameters:
 *   doc -- the GPXdoc to search
 *   name -- the route name to search for
 * 
 * Returns:
 *   A pointer to the first route, or NULL if no route is found
 **/
Route *getRoute(const GPXdoc *doc, char *name) {
    if (doc == NULL) {
        return NULL;
    }
    if (doc->routes == NULL || name == NULL) {
        return NULL;
    }
    // Build search target
    Route targetRoute;
    targetRoute.name = malloc(strlen(name)+1);
    if (targetRoute.name == NULL) {
        return NULL; // malloc failed; fatal
    }
    strcpy(targetRoute.name, name);

    void *result = findElement(doc->routes, &compareRoutesByName, &targetRoute);

    free(targetRoute.name);
    if (result == NULL) {
        return NULL; // no route found
    }
    return (Route *)result;
}

/**
 * Based on the Haversine formula provided in the assignment description
 * (https://www.movable-type.co.uk/scripts/latlong.html).
 **/
float getDistance(float sourceLat, float sourceLon, float destLat, float destLon) {
    int R = 6371*1000;
    float lat1Rad = sourceLat*M_PI/180.0;
    float lat2Rad = destLat*M_PI/180.0;
    float deltaLat = (destLat - sourceLat)*M_PI/180.0;
    float deltaLon = (destLon - sourceLon)*M_PI/180.0;
    float a = sin(deltaLat/2)*sin(deltaLat/2) + cos(lat1Rad) * cos(lat2Rad) *
              sin(deltaLon/2)*sin(deltaLon/2);
    float c = 2*atan2(sqrt(a), sqrt(1-a));
    return R*c;
}

float getWaypointDistance(Waypoint *wpt1, Waypoint *wpt2) {
    if (wpt1 == NULL || wpt2 == NULL) {
        return 0;
    }
    return getDistance((float)(wpt1->latitude), (float)(wpt1->longitude), (float)(wpt2->latitude), (float)(wpt2->longitude));
}

float getPathDistance(List *waypoints) {
    if (waypoints == NULL) {
        return 0;
    }

    float len = 0;
    Waypoint *lastWpt;
    Waypoint *wpt;
    ListIterator it;

    it = createIterator(waypoints);
    lastWpt = NULL;
    for (int i = 0; i < getLength(waypoints); i++) {
        wpt = (Waypoint *)nextElement(&it);
        if (lastWpt != NULL) {
            len += getWaypointDistance(lastWpt, wpt);
            printf("%f %f  %f %f   ->   %f\n", (float)(lastWpt->latitude), (float)(lastWpt->longitude), (float)(wpt->latitude), (float)(wpt->longitude), getWaypointDistance(lastWpt, wpt));
        }
        lastWpt = wpt;
    }
    return len;
}

float getRouteLen(const Route *rt) {
    if (rt == NULL) {
        return 0;
    }
    return getPathDistance(rt->waypoints);
}

float getTrackLen(const Track *tr) {
    if (tr == NULL) {
        return 0;
    }

    float len = 0;
    TrackSegment *seg;
    TrackSegment *lastSeg;
    ListIterator it;
    void *element ;

    it = createIterator(tr->segments);
    lastSeg = NULL;
    while ((element = nextElement(&it)) != NULL) {
        seg = (TrackSegment *)element;

        len += getPathDistance(seg->waypoints);

        if (lastSeg != NULL) {
            Waypoint *lastPoint = getFromBack(lastSeg->waypoints);
            Waypoint *firstPoint = getFromFront(seg->waypoints);
            len += getWaypointDistance(lastPoint, firstPoint);
        }

        lastSeg = seg;
    }
    return len;
}

float round10(float len) {

}

int numRoutesWithLength(const GPXdoc *doc, float len, float delta) {

}

int numTracksWithLength(const GPXdoc *doc, float len, float delta) {

}

bool isLoopRoute(const Route *route, float delta) {

}

bool isLoopTrack(const Track *tr, float delta) {

}

List *getRoutesBetween(const GPXdoc *doc, float sourceLat, float sourceLong, float destLat, float destLong, float delta) {

}

List *getTracksBetween(const GPXdoc *doc, float sourceLat, float sourceLong, float destLat, float destLong, float delta) {

}
