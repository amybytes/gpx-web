/**
 * Name: GPXParser.c
 * Author: Ethan Rowan (1086586)
 * Date Created: 01/19/2021
 * Last Modified: 02/22/2021
 */

#include <stdlib.h>
#include <dirent.h>
#include "GPXParser.h"
#include "GPXHelpers.h"
#include "GPXJSON.h"

#define INVALID_LATITUDE -200
#define INVALID_LONGITUDE -200
#define DEFAULT_NAMESPACE "http://www.topografix.com/GPX/1/1"

/**
 * Creates an XML node representation of a GPXData structure.
 * This is used to construct a complete GPXdoc XML tree.
 **/
xmlNode *createXMLGPXData(GPXData *gpxData, xmlNode *parent) {
    if (gpxData == NULL) {
        return NULL;
    }
    return xmlNewChild(parent, NULL, BAD_CAST gpxData->name, BAD_CAST gpxData->value); 
}

/**
 * Creates an XML node representation of a Waypoint structure.
 * This is used to construct a complete GPXdoc XML tree.
 **/
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

/**
 * Creates an XML node representation of a Route structure.
 * This is used to construct a complete GPXdoc XML tree.
 **/
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

/**
 * Creates an XML node representation of a TrackSegment structure.
 * This is used to construct a complete GPXdoc XML tree.
 **/
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

/**
 * Creates an XML node representation of a Track structure.
 * This is used to construct a complete GPXdoc XML tree.
 **/
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

/**
 * Creates an XML document representation of a GPXdoc structure.
 * This XML tree can be used to validate a GPX document using
 * validateGPXDoc(), or to write it to a file using writeGPXdoc().
 **/
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

    if (!fileExtensionsEqual(fileName, ".gpx")) {
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

/**
 * Creates a new *valid* GPX document from a gpx file.
 * 
 * Parameters:
 *   fileName -- the name of the gpx file
 * 
 * Returns:
 *   A pointer to a new GPXdoc structure, or NULL if a parsing error occurs
 **/
GPXdoc *createValidGPXdoc(char *fileName, char *gpxSchemaFile) {
    GPXdoc *doc = createGPXdoc(fileName);
    if (validateGPXDoc(doc, gpxSchemaFile)) {
        return doc;
    }
    deleteGPXdoc(doc);
    return NULL;
}

/**
 * Validates a GPX doc against an XML schema file.
 * 
 * Parameters:
 *   doc -- the GPX document to validate
 *   fileName -- the name of the schema file; must end in ".xsd"
 **/
bool validateGPXDoc(GPXdoc *doc, char *fileName) {
    xmlDoc *xml;
    xmlSchema *schema;
    xmlSchemaParserCtxt *parserCtxt;
    xmlSchemaValidCtxt *validCtxt;

    if (doc == NULL || fileName == NULL) {
        return false;
    }

    if (!fileExtensionsEqual(fileName, ".xsd")) {
        return false;
    }

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

/**
 * Writes a GPX document to a file on the disk.
 * 
 * Parameters:
 *   doc -- the GPX document to write
 *   fileName -- the name of the file to write to
 * 
 * Returns:
 *   true if the write is successful, or false if the GPX document
 *   is invalid or the write is unsuccessful
 **/
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

/**
 * Gets the distance between two waypoints.
 * 
 * Parameters:
 *   wpt1 -- the first waypoint
 *   wpt2 -- the second waypoint
 * 
 * Returns:
 *   The distance, in meters, between the two waypoints
 **/
float getWaypointDistance(Waypoint *wpt1, Waypoint *wpt2) {
    if (wpt1 == NULL || wpt2 == NULL) {
        return 0;
    }
    return getDistance((float)(wpt1->latitude), (float)(wpt1->longitude), (float)(wpt2->latitude), (float)(wpt2->longitude));
}

/**
 * Gets the distance between a waypoint and another point (lat,lon).
 * 
 * Parameters:
 *   wpt -- the first waypoint
 *   lat -- the latitude of the second point
 *   lon -- the longitude of the second point
 * 
 * Returns:
 *   The distance, in meters, between the waypoint and the other point
 **/
float getWaypointPointDistance(Waypoint *wpt, float lat, float lon) {
    if (wpt == NULL) {
        return 0;
    }
    return getDistance((float)(wpt->latitude), (float)(wpt->longitude), lat, lon);
}

/**
 * Calculates the total distance of a list of waypoints from the first waypoint
 * in the list to the last waypoint in list.
 **/
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
        }
        lastWpt = wpt;
    }
    return len;
}

/**
 * Gets the total length of a route.
 * 
 * Parameters:
 *   rt -- the route to evaluate
 * 
 * Returns:
 *   The total length of the route in meters.
 **/
float getRouteLen(const Route *rt) {
    if (rt == NULL) {
        return 0;
    }
    return getPathDistance(rt->waypoints);
}

/**
 * Gets the total length of a track.
 * 
 * Parameters:
 *   tr -- the track to evaluate
 * 
 * Returns:
 *   The total length of the track in meters.
 **/
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

/**
 * Rounds a floating point number to the nearest ten. The length
 * must be non-negative.
 * 
 * Parameters:
 *   len -- the length to be rounded; must be non-negative
 * 
 * Returns:
 *   The length, rounded to the nearest ten
 **/
float round10(float len) {
    int base = (int)(len/10)*10; // Remove ones digit and fractional part
    float r = len - base;
    if (r >= 5) {
        return base+10; // Round up
    }
    else {
        return base; // Round down
    }
}

/**
 * Gets the number of routes from a GPX document that have a given
 * length, within a range delta.
 **/
int numRoutesWithLength(const GPXdoc *doc, float len, float delta) {
    if (doc == NULL || len < 0 || delta < 0) {
        return 0;
    }

    if (doc->routes == NULL) {
        return 0;
    }

    int numRoutes = 0;
    ListIterator it;
    Route *route;

    it = createIterator(doc->routes);
    while ((route = (Route *)nextElement(&it)) != NULL) {
        if (fabs(getRouteLen(route) - len) <= delta) {
            numRoutes++;
        }
    }
    return numRoutes;
}

/**
 * Gets the number of tracks from a GPX document that have a given
 * length, within a range delta.
 **/
int numTracksWithLength(const GPXdoc *doc, float len, float delta) {
    if (doc == NULL || len < 0 || delta < 0) {
        return 0;
    }

    if (doc->tracks == NULL) {
        return 0;
    }

    int numTracks = 0;
    ListIterator it;
    Track *track;

    it = createIterator(doc->tracks);
    while ((track = (Track *)nextElement(&it)) != NULL) {
        if (fabs(getTrackLen(track) - len) <= delta) {
            numTracks++;
        }
    }
    return numTracks;
}

/**
 * Checks if a loop exists between the first and last waypoints
 * in a list of waypoints. The list must contain at least 4 waypoints.
 * 
 * Parameters:
 *   waypoints -- the list of waypoints; must have at least 4
 *   delta -- the maximum distance that the start and end waypoints
 *            can be from one another
 **/
bool hasLoop(List *waypoints, float delta) {
    if (waypoints == NULL) {
        return false;
    }

    if (getLength(waypoints) < 4) {
        return false;
    }

    Waypoint *firstWpt = getFromFront(waypoints);
    Waypoint *lastWpt = getFromBack(waypoints);

    return getWaypointDistance(firstWpt, lastWpt) <= delta;
}

/**
 * Checks if a route with at least four waypoints starts and ends
 * at the same point within a range delta.
 * 
 * Parameters:
 *   tr -- the route to evaluate
 *   delta -- the maximum distance that the start and end points
 *            can be from one another
 * 
 * Returns:
 *   true if the route loops or false if it does not loop
 **/
bool isLoopRoute(const Route *route, float delta) {
    if (route == NULL || delta < 0) {
        return false;
    }
    return hasLoop(route->waypoints, delta);
}

/**
 * Checks if a track with at least four waypoints starts and ends
 * at the same point within a range delta.
 * 
 * Parameters:
 *   tr -- the track to evaluate
 *   delta -- the maximum distance that the points can be
 *            from one another
 * 
 * Returns:
 *   true if the track loops or false if it does not loop
 **/
bool isLoopTrack(const Track *tr, float delta) {
    if (tr == NULL || delta < 0) {
        return false;
    }

    if (tr->segments == NULL) {
        return false;
    }

    TrackSegment *firstSegment = getFromFront(tr->segments);
    TrackSegment *lastSegment = getFromBack(tr->segments);

    if (firstSegment == NULL || lastSegment == NULL) {
        return false;
    }

    Waypoint *firstPoint = getFromFront(firstSegment->waypoints);
    Waypoint *lastPoint = getFromBack(lastSegment->waypoints);

    if (firstPoint == NULL || lastPoint == NULL) {
        return false;
    }

    return getWaypointDistance(firstPoint, lastPoint) <= delta;
}

/**
 * Creates a list of all the routes in a GPX doc that start and end at
 * the given source and dest locations.
 * 
 * Parameters:
 *   doc -- the gpx doc containing the routes
 *   sourceLat -- the latitude of the starting point
 *   sourceLong -- the longitude of the starting point
 *   destLat -- the latitude of the ending point
 *   destLong -- the longitude of the ending point
 *   delta -- the maximum allowable distance from the start/end point
 * 
 * Returns:
 *   A list of routes that are between the starting point and the ending point,
 *   or NULL if the doc is NULL or there are no routes between
 **/
List *getRoutesBetween(const GPXdoc *doc, float sourceLat, float sourceLong, float destLat, float destLong, float delta) {
    if (doc == NULL) {
        return NULL;
    }

    if (doc->routes == NULL) {
        return NULL;
    }

    List *routes;
    ListIterator it;
    Route *route;
    Waypoint *startPoint;
    Waypoint *endPoint;

    routes = initializeList(routeToString, deleteDummy, compareRoutes);
    it = createIterator(doc->routes);
    while ((route = (Route *)nextElement(&it)) != NULL) {
        startPoint = getFromFront(route->waypoints);
        endPoint = getFromBack(route->waypoints);
        if (getWaypointPointDistance(startPoint, sourceLat, sourceLong) <= delta &&
                getWaypointPointDistance(endPoint, destLat, destLong) <= delta) {
            insertBack(routes, route);
        }
    }

    if (getLength(routes) == 0) {
        freeList(routes);
        return NULL;
    }

    return routes;
}

// Checks if a track is between given start and end locations
bool isTrackBetween(Track *track, float sourceLat, float sourceLong, float destLat, float destLong, float delta) {
    if (track == NULL) {
        return false;
    }

    if (track->segments == NULL) {
        return false;
    }

    TrackSegment *firstSeg = getFromFront(track->segments);
    TrackSegment *lastSeg = getFromBack(track->segments);

    if (firstSeg == NULL || lastSeg == NULL) {
        return false;
    }

    Waypoint *firstPoint = getFromFront(firstSeg->waypoints);
    Waypoint *lastPoint = getFromBack(lastSeg->waypoints);

    if (firstPoint == NULL || lastPoint == NULL) {
        return false;
    }
    
    return (getWaypointPointDistance(firstPoint, sourceLat, sourceLong) <= delta &&
                getWaypointPointDistance(lastPoint, destLat, destLong) <= delta);
}

/**
 * Creates a list of all the tracks in a GPX doc that start and end at
 * the given source and dest locations.
 * 
 * Parameters:
 *   doc -- the gpx doc containing the tracks
 *   sourceLat -- the latitude of the starting point
 *   sourceLong -- the longitude of the starting point
 *   destLat -- the latitude of the ending point
 *   destLong -- the longitude of the ending point
 *   delta -- the maximum allowable distance from the start/end point
 * 
 * Returns:
 *   A list of tracks that are between the starting point and the ending point,
 *   or NULL if the doc is NULL or there are no tracks between
 **/
List *getTracksBetween(const GPXdoc *doc, float sourceLat, float sourceLong, float destLat, float destLong, float delta) {
    if (doc == NULL) {
        return NULL;
    }

    if (doc->tracks == NULL) {
        return NULL;
    }

    List *tracks;
    ListIterator it;
    Track *track;

    tracks = initializeList(trackToString, deleteDummy, compareTracks);
    it = createIterator(doc->tracks);
    while ((track = (Track *)nextElement(&it)) != NULL) {
        if (isTrackBetween(track, sourceLat, sourceLong, destLat, destLong, delta)) {
            insertBack(tracks, track);
        }
    }

    if (getLength(tracks) == 0) {
        freeList(tracks);
        return NULL;
    }

    return tracks;
}

/* 
 * Internal function for converting a route to a JSONObject. This is an
 * intermediate step for converting a route to a JSON string (conversion
 * is handled by the GPXJSON library).
 */
JSONObject *routeToJSONObject(const Route *rt) {
    JSONObject *json = createJSONObject();
    
    if (rt == NULL) {
        return json;
    }

    char *name = rt->name;
    int numPoints = getLength(rt->waypoints);
    float routeLen = round10(getRouteLen(rt));
    bool loop = isLoopRoute(rt, 10);

    putStringInJSONObject(json, "name", !isEmptyString(name) ? name : "None");
    putIntInJSONObject(json, "numPoints", numPoints);
    putDoubleInJSONObject(json, "len", (double)routeLen);
    putBoolInJSONObject(json, "loop", loop);

    return json;
}

/* 
 * Internal function for converting a track to a JSONObject. This is an
 * intermediate step for converting a track to a JSON string (conversion
 * is handled by the GPXJSON library).
 */
JSONObject *trackToJSONObject(const Track *tr) {
    JSONObject *json = createJSONObject();

    if (tr == NULL) {
        return json;
    }

    char *name = tr->name;
    int trackLen = round10(getTrackLen(tr));
    bool loop = isLoopTrack(tr, 10);

    putStringInJSONObject(json, "name", !isEmptyString(name) ? name : "None");
    putDoubleInJSONObject(json, "len", (float)trackLen);
    putBoolInJSONObject(json, "loop", loop);

    return json;
}

/**
 * Creates a JSON array string containing summaries of each route.
 * 
 * Parameters:
 *   list -- a list of routes
 * 
 * Returns:
 *   A JSON array string where each entry is a route summary (see routeToJSON()).
 *   If the list is NULL, an empty JSON array string is returned
 **/
char *routeListToJSON(const List *list) {
    JSONArray *jsonArr = createJSONArray();

    if (list == NULL) {
        return jsonArrayToStringAndEat(jsonArr);
    }

    JSONObject *routeJson;
    ListIterator it;
    Route *route;
    
    it = createIterator((List *)list);
    while ((route = (Route *)nextElement(&it)) != NULL) {
        routeJson = routeToJSONObject(route);
        addJSONObjectToJSONArray(jsonArr, routeJson);
        deleteJSONObject(routeJson);
    }
    
    return jsonArrayToStringAndEat(jsonArr);
}

/**
 * Creates a JSON array string containing summaries of each track.
 * 
 * Parameters:
 *   list -- a list of tracks
 * 
 * Returns:
 *   A JSON array string where each entry is a track summary (see trackToJSON()).
 *   If the list is NULL, an empty JSON array string is returned
 **/
char *trackListToJSON(const List *list) {
    JSONArray *jsonArr = createJSONArray();

    if (list == NULL) {
        return jsonArrayToStringAndEat(jsonArr);
    }

    JSONObject *trackJson;
    ListIterator it;
    Track *track;
    
    it = createIterator((List *)list);

    while ((track = (Track *)nextElement(&it)) != NULL) {
        trackJson = trackToJSONObject(track);
        addJSONObjectToJSONArray(jsonArr, trackJson);
        deleteJSONObject(trackJson);
    }

    return jsonArrayToStringAndEat(jsonArr);
}

/**
 * Creates a JSON string with a summary of a Track.
 * 
 * Parameters:
 *   tr -- the track to convert
 * 
 * Returns:
 *   A JSON string containing the track name, length, and whether the track
 *   is a loop track. If the track is NULL, an empty JSON string is returned
 **/
char *trackToJSON(const Track *tr) {
    return jsonObjectToStringAndEat(trackToJSONObject(tr));
}

/**
 * Creates a JSON string with a summary of a Route.
 * 
 * Parameters:
 *   rt -- the route to convert
 * 
 * Returns:
 *   A JSON string containing the route name, number of points, length,
 *   and whether the route is a loop route. If the route is NULL, an empty
 *   JSON string is returned
 **/
char *routeToJSON(const Route *rt) {
    return jsonObjectToStringAndEat(routeToJSONObject(rt));
}

/**
 * Creates a JSON string with a summary of the GPX document.
 * 
 * Parameters:
 *   gpx -- the gpx document to convert
 * 
 * Returns:
 *   A JSON string containing the GPX document version, creator, number
 *   of waypoints, number of routes, and number of tracks. If the GPX
 *   document is NULL, an empty JSON string is returned
 **/
char *GPXtoJSON(const GPXdoc *gpx) {
    JSONObject *json = createJSONObject();

    if (gpx == NULL) {
        return jsonObjectToStringAndEat(json);
    }

    putDoubleInJSONObject(json, "version", gpx->version);
    putStringInJSONObject(json, "creator", gpx->creator);
    putIntInJSONObject(json, "numWaypoints", getLength(gpx->waypoints));
    putIntInJSONObject(json, "numRoutes", getLength(gpx->routes));
    putIntInJSONObject(json, "numTracks", getLength(gpx->tracks));

    return jsonObjectToStringAndEat(json);
}

/**
 * Adds a waypoint to the end of a route.
 * 
 * Parameters:
 *   rt -- the route to add the waypoint to
 *   pt -- the waypoint to be added
 **/
void addWaypoint(Route *rt, Waypoint *pt) {
    if (rt == NULL || pt == NULL) {
        return;
    }

    if (rt->waypoints == NULL) {
        return;
    }

    insertBack(rt->waypoints, pt);
}

/**
 * Adds a route to the end of the GPXdoc.
 * 
 * Parameters:
 *   doc -- the doc to add the route to
 *   rt -- the route to be added
 **/
void addRoute(GPXdoc *doc, Route *rt) {
    if (doc == NULL || rt == NULL) {
        return;
    }

    if (doc->routes == NULL) {
        return;
    }

    insertBack(doc->routes, rt);
}

/**
 * Creates a basic GPX document from a JSON string. If the supplied values are
 * all valid and the returned GPX document is non-null, the returned GPX document
 * will always pass validation with validateGPXDoc().
 * 
 * Parameters:
 *   gpxString -- the JSON string in the format {"version": <double>, "creator", <string>}
 * 
 * Returns:
 *   A pointer to a new GPXdoc structure if the JSON is valid, or NULL if the
 *   JSON is invalid
 **/
GPXdoc *JSONtoGPX(const char *gpxString) {
    if (gpxString == NULL) {
        return NULL;
    }

    JSONObject *json = parseJSONString((char *)gpxString);
    GPXdoc *doc = malloc(sizeof(GPXdoc));
    doc->version = -1;
    doc->creator = NULL;
    strcpy(doc->namespace, DEFAULT_NAMESPACE);
    doc->waypoints = initializeList(waypointToString, deleteWaypoint, compareWaypoints);
    doc->routes = initializeList(routeToString, deleteRoute, compareRoutes);
    doc->tracks = initializeList(trackToString, deleteTrack, compareTracks);

    if (jsonObjectHas(json, "version")) {
        doc->version = getDoubleFromJSONObject(json, "version");
    }
    if (jsonObjectHas(json, "creator")) {
        char *creator = getStringFromJSONObject(json, "creator");
        if (creator != NULL) {
            doc->creator = malloc(strlen(creator)+1);
            strcpy(doc->creator, creator);
        }
    }
    
    deleteJSONObject(json);
    
    if (doc->version == -1 || doc->creator == NULL) {
        if (doc->creator != NULL) {
            free(doc->creator);
        }
        free(doc);
        return NULL;
    }

    return doc;
}

/**
 * Creates a basic Waypoint structure from a JSON string.
 * 
 * Parameters:
 *   gpxString -- the JSON string in the format {"lat": <double>, "lon": <double>}
 * 
 * Returns:
 *   A pointer to a new Waypoint structure if the JSON is valid, or NULL
 *   if the JSON is invalid
 **/
Waypoint *JSONtoWaypoint(const char *gpxString) {
    if (gpxString == NULL) {
        return NULL;
    }

    JSONObject *json = parseJSONString((char *)gpxString);
    Waypoint *waypoint = malloc(sizeof(Waypoint));
    waypoint->name = malloc(2);
    waypoint->name[0] = '\0';
    waypoint->latitude = INVALID_LATITUDE;
    waypoint->longitude = INVALID_LONGITUDE;
    waypoint->otherData = initializeList(gpxDataToString, deleteGpxData, compareGpxData);

    // Get waypoint attributes from JSON
    if (jsonObjectHas(json, "lat")) {
        waypoint->latitude = getDoubleFromJSONObject(json, "lat");
    }
    if (jsonObjectHas(json, "lon")) {
        waypoint->longitude = getDoubleFromJSONObject(json, "lon");
    }
    
    deleteJSONObject(json);
    
    // Waypoint creation fails if attributes are not set in JSON
    if (waypoint->latitude == INVALID_LATITUDE ||
            waypoint->longitude == INVALID_LONGITUDE) {
        deleteWaypoint(waypoint);
        return NULL;
    }

    return waypoint;
}

/**
 * Creates a basic Route structure from a JSON string.
 * 
 * Parameters:
 *   gpxString -- the JSON string in the format {"name": <string>}
 * 
 * Returns:
 *   A pointer to a new Route structure if the JSON is valid, or
 *   NULL if the JSON is invalid
 **/
Route *JSONtoRoute(const char *gpxString) {
    if (gpxString == NULL) {
        return NULL;
    }

    JSONObject *json = parseJSONString((char *)gpxString);
    Route *route = malloc(sizeof(Route));
    route->name = NULL;
    route->waypoints = initializeList(waypointToString, deleteWaypoint, compareWaypoints);
    route->otherData = initializeList(gpxDataToString, deleteGpxData, compareGpxData);

    if (jsonObjectHas(json, "name")) {
        char *name = getStringFromJSONObject(json, "name");
        if (name != NULL) {
            route->name = malloc(strlen(name)+1);
            strcpy(route->name, name);
        }
    }
    
    deleteJSONObject(json);
    
    if (route->name == NULL) {
        deleteRoute(route);
        return NULL;
    }

    return route;
}

int createGPXFileFromJSON(char *gpxString, char *path, char *schema) {
    JSONObject *json = parseJSONString(gpxString);
    char *jsonStr = jsonObjectToStringAndEat(json);
    GPXdoc *gpxDoc = JSONtoGPX(jsonStr);
    free(jsonStr);
    int status = validateGPXDoc(gpxDoc, schema);
    if (status == 1) {
        status = writeGPXdoc(gpxDoc, path);
    }
    deleteGPXdoc(gpxDoc);
    return status;
}

char *getAllValidGPXFilesAsJSON(char *dirname, char *schema) {
    JSONArray *jsonArr;
    JSONObject *docJsonObj;
    GPXdoc *doc;
    char *docJson;
    DIR *dir = NULL;
    struct dirent *entry;
    char *filename;

    jsonArr = createJSONArray();

    dir = opendir(dirname);
    if (dir != NULL) {
        while ((entry = readdir(dir)) != NULL) {
            if (!strequals(entry->d_name, ".") && !strequals(entry->d_name, "..")) {
                filename = malloc(strlen(dirname) + strlen(entry->d_name) + 2);
                sprintf(filename, "%s/%s", dirname, entry->d_name);
                doc = createValidGPXdoc(filename, schema);
                if (doc != NULL) {
                    docJson = GPXtoJSON(doc);
                    docJsonObj = parseJSONString(docJson);
                    putStringInJSONObject(docJsonObj, "name", entry->d_name);
                    if (!isEmptyJSONObject(docJsonObj)) {
                        addJSONObjectToJSONArray(jsonArr, docJsonObj);
                    }
                    deleteGPXdoc(doc);
                    free(docJson);
                    deleteJSONObject(docJsonObj);
                }
                free(filename);
            }
        }
        closedir(dir);
    }
    return jsonArrayToStringAndEat(jsonArr);
}

int validateGPXFile(char *filename, char *schema) {
    GPXdoc *gpxDoc = createValidGPXdoc(filename, schema);
    if (gpxDoc == NULL) {
        return 0;
    }
    deleteGPXdoc(gpxDoc);
    return 1;
}

char *getGPXFileAsJSON(char *filename, char *name) {
    GPXdoc *gpxDoc;
    JSONObject *json;
    
    gpxDoc = createGPXdoc(filename);
    char *jsonStr = GPXtoJSON(gpxDoc);
    deleteGPXdoc(gpxDoc);
    json = parseJSONString(jsonStr);
    free(jsonStr);

    putStringInJSONObject(json, "name", name);

    return jsonObjectToStringAndEat(json);
}

char *getGPXRoutesAsJSON(char *filename) {
    GPXdoc *gpxDoc = createGPXdoc(filename);
    if (gpxDoc == NULL) {
        JSONObject *json = createJSONObject();
        return jsonObjectToStringAndEat(json);
    }
    char *routes = routeListToJSON(gpxDoc->routes);
    deleteGPXdoc(gpxDoc);
    return routes;
}

char *getGPXTracksAsJSON(char *filename) {
    GPXdoc *gpxDoc = createGPXdoc(filename);
    if (gpxDoc == NULL) {
        JSONObject *json = createJSONObject();
        return jsonObjectToStringAndEat(json);
    }
    char *tracks = trackListToJSON(gpxDoc->tracks);
    deleteGPXdoc(gpxDoc);
    return tracks;
}

Route *getRouteFromWaypointsJSON(char *waypoints) {
    JSONArray *json = parseJSONArrayString(waypoints);
    Route *route = malloc(sizeof(Route));
    route->name = malloc(5);
    strcpy(route->name, "None");
    route->waypoints = initializeList(waypointToString, deleteWaypoint, compareWaypoints);
    route->otherData = initializeList(gpxDataToString, deleteGpxData, compareGpxData);

    JSONObject *waypointJson;
    Waypoint *wpt;
    for (int i = 0; i < json->numElements; i++) {
        waypointJson = getJSONObjectFromJSONArrayAt(json, i);
        char *waypointStr = jsonObjectToString(waypointJson);
        wpt = JSONtoWaypoint(waypointStr);
        free(waypointStr);
        addWaypoint(route, wpt);
    }
    deleteJSONArray(json);
    return route;
}

char *getRouteAsJSON(char *waypoints) {
    Route *route = getRouteFromWaypointsJSON(waypoints);
    char *routeJson = routeToJSON(route);
    deleteRoute(route);
    return routeJson;
}

int addRouteToGPXFile(char *filename, char *waypoints) {
    GPXdoc *gpxDoc = createGPXdoc(filename);
    Route *route = getRouteFromWaypointsJSON(waypoints);

    if (gpxDoc == NULL || route == NULL) {
        return 0;
    }

    addRoute(gpxDoc, route);
    int status = writeGPXdoc(gpxDoc, filename);
    deleteGPXdoc(gpxDoc);

    return status;
}

char *getRoutesBetweenAsJSON(char *dirname, float sourceLat, float sourceLon, float destLat, float destLon) {
    GPXdoc *gpxDoc;
    List *routesList;
    JSONArray *routesJson;
    DIR *dir = NULL;
    struct dirent *entry;
    char *filename;

    routesJson = createJSONArray();

    dir = opendir(dirname);
    if (dir != NULL) {
        while ((entry = readdir(dir)) != NULL) {
            if (!strequals(entry->d_name, ".") && !strequals(entry->d_name, "..")) {
                filename = malloc(strlen(dirname) + strlen(entry->d_name) + 2);
                sprintf(filename, "%s/%s", dirname, entry->d_name);
                gpxDoc = createGPXdoc(filename);
                routesList = getRoutesBetween(gpxDoc, sourceLat, sourceLon, destLat, destLon, 10);
                if (routesList != NULL) {
                    char *routeListStr = routeListToJSON(routesList);
                    JSONArray *routesListJson = parseJSONArrayString(routeListStr);
                    free(routeListStr);
                    if (getJSONArraySize(routesListJson) > 0) {
                        for (int i = 0; i < getJSONArraySize(routesListJson); i++) {
                            JSONObject *route = getJSONObjectFromJSONArrayAt(routesListJson, i);
                            addJSONObjectToJSONArray(routesJson, route);
                        }
                    }
                    freeList(routesList);
                    deleteJSONArray(routesListJson);
                }
                deleteGPXdoc(gpxDoc);
                free(filename);
            }
        }
        closedir(dir);
    }
    return jsonArrayToStringAndEat(routesJson);
}

char *getTracksBetweenAsJSON(char *dirname, float sourceLat, float sourceLon, float destLat, float destLon) {
    GPXdoc *gpxDoc;
    List *tracksList;
    JSONArray *tracksJson;
    DIR *dir = NULL;
    struct dirent *entry;
    char *filename;

    tracksJson = createJSONArray();

    dir = opendir(dirname);
    if (dir != NULL) {
        while ((entry = readdir(dir)) != NULL) {
            if (!strequals(entry->d_name, ".") && !strequals(entry->d_name, "..")) {
                filename = malloc(strlen(dirname) + strlen(entry->d_name) + 2);
                sprintf(filename, "%s/%s", dirname, entry->d_name);
                gpxDoc = createGPXdoc(filename);
                tracksList = getTracksBetween(gpxDoc, sourceLat, sourceLon, destLat, destLon, 10);
                if (tracksList != NULL) {
                    char *trackListStr = trackListToJSON(tracksList);
                    JSONArray *tracksListJson = parseJSONArrayString(trackListStr);
                    free(trackListStr);
                    if (getJSONArraySize(tracksListJson) > 0) {
                        for (int i = 0; i < getJSONArraySize(tracksListJson); i++) {
                            JSONObject *track = getJSONObjectFromJSONArrayAt(tracksListJson, i);
                            addJSONObjectToJSONArray(tracksJson, track);
                        }
                    }
                    freeList(tracksList);
                    deleteJSONArray(tracksListJson);
                }
                deleteGPXdoc(gpxDoc);
                free(filename);
            }
        }
        closedir(dir);
    }
    return jsonArrayToStringAndEat(tracksJson);
}
