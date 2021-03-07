/**
 * Name: GPXJSON.h
 * Author: Ethan Rowan (1086586)
 * Date Created: 03/3/2021
 * Last Modified: 03/04/2021
 */

#ifndef GPX_JSON
#define GPX_JSON

#include "LinkedListAPI.h"

/* Raw data types supported by GPXJSON */
#define TYPE_INT 0
#define TYPE_DOUBLE 1
#define TYPE_BOOL 2
#define TYPE_STRING 3
#define TYPE_JSONOBJECT 4
#define TYPE_JSONARRAY 5

/**
 * JSONData is a wrapper for raw data types (int, double,
 * bool, string, JSONObject, JSONArray) that contains extra
 * metadata for handling specific types in JSON objects.
 **/
typedef struct jsondata {
    // The name/key for the data
    char *name;
    // The data itself
    void *value;
    // The type of the data. Must be one of: TYPE_INT, TYPE_DOUBLE,
    // TYPE_BOOL, TYPE_STRING, TYPE_JSONOBJECT, TYPE_JSONARRAY
    int type;
} JSONData;

/**
 * JSONArrData is a wrapper for raw data types (int, double,
 * bool, string, JSONObject, JSONArray), that contains extra
 * metadata for handling specific types in JSON arrays.
 **/
typedef struct jsonarrdata {
    // The data itself
    void *value;
    // The type of the data. Must be one of: TYPE_INT, TYPE_DOUBLE,
    // TYPE_BOOL, TYPE_STRING, TYPE_JSONOBJECT, TYPE_JSONARRAY
    int type;
} JSONArrData;

/**
 * A JSONObject represents a JSON object as a
 * collection of JSON data.
 **/
typedef struct jsonobject {
    // Data must be stored in a JSONData wrapper
    List *data;
    int numElements;
} JSONObject;

/**
 * A JSONArray represents a JSON array as a collection of
 * JSON array data.
 **/
typedef struct jsonarray {
    // Data must be stored in a JSONArrData wrapper
    List *data;
    int numElements;
} JSONArray;

/* JSON object functions */

JSONObject *createJSONObject();
char *jsonObjectToString(JSONObject *json);
char *jsonObjectToStringAndEat(JSONObject *json);
void deleteJSONObject(JSONObject *json);
JSONObject *cloneJSONObject(JSONObject *json);
JSONObject *parseJSONString(char *jsonStr);
void putIntInJSONObject(JSONObject *json, char *name, int value);
void putDoubleInJSONObject(JSONObject *json, char *name, double value);
void putBoolInJSONObject(JSONObject *json, char *name, bool value);
void putStringInJSONObject(JSONObject *json, char *name, char *value);
void putJSONObjectInJSONObject(JSONObject *json, char *name, JSONObject *value);
void putJSONArrayInJSONObject(JSONObject *json, char *name, JSONArray *value);
bool jsonObjectHas(JSONObject *json, char *name);
int getIntFromJSONObject(JSONObject *json, char *name);
double getDoubleFromJSONObject(JSONObject *json, char *name);
bool getBoolFromJSONObject(JSONObject *json, char *name);
char *getStringFromJSONObject(JSONObject *json, char *name);
JSONObject *getJSONObjectFromJSONObject(JSONObject *json, char *name);
JSONArray *getJSONArrayFromJSONObject(JSONObject *json, char *name);

/* JSON array functions */

JSONArray *createJSONArray();
char *jsonArrayToString(JSONArray *json);
char *jsonArrayToStringAndEat(JSONArray *json);
void deleteJSONArray(JSONArray *json);
JSONArray *cloneJSONArray(JSONArray *json);
JSONArray *parseJSONArrayString(char *jsonStr);
void addIntToJSONArray(JSONArray *json, int value);
void addDoubleToJSONArray(JSONArray *json, double value);
void addBoolToJSONArray(JSONArray *json, bool value);
void addStringToJSONArray(JSONArray *json, char *value);
void addJSONObjectToJSONArray(JSONArray *json, JSONObject *value);
void addJSONArrayToJSONArray(JSONArray *json, JSONArray *value);

#endif