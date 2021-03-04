#ifndef GPX_JSON
#define GPX_JSON

#include "LinkedListAPI.h"

#define TYPE_INT 0
#define TYPE_FLOAT 1
#define TYPE_BOOL 2
#define TYPE_STRING 3
#define TYPE_JSONOBJECT 4
#define TYPE_JSONARRAY 5

typedef struct jsondata {
    char *name;
    void *value;
    int type;
} JSONData;

typedef struct jsonarrdata {
    void *value;
    int type;
} JSONArrData;

typedef struct jsonobject {
    List *data;
    int numElements;
} JSONObject;

typedef struct jsonarray {
    List *data;
    int numElements;
} JSONArray;

JSONObject *createJSONObject();
char *jsonObjectToString(JSONObject *json);
char *jsonObjectToStringAndEat(JSONObject *json);
void deleteJSONObject(JSONObject *json);
JSONObject *cloneJSONObject(JSONObject *json);
JSONObject *parseJSONString(char *jsonStr);
void putIntInJSONObject(JSONObject *json, char *name, int value);
void putFloatInJSONObject(JSONObject *json, char *name, float value);
void putBoolInJSONObject(JSONObject *json, char *name, bool value);
void putStringInJSONObject(JSONObject *json, char *name, char *value);
void putJSONObjectInJSONObject(JSONObject *json, char *name, JSONObject *value);
void putJSONArrayInJSONObject(JSONObject *json, char *name, JSONArray *value);

JSONArray *createJSONArray();
char *jsonArrayToString(JSONArray *json);
char *jsonArrayToStringAndEat(JSONArray *json);
void deleteJSONArray(JSONArray *json);
JSONArray *cloneJSONArray(JSONArray *json);
void addIntToJSONArray(JSONArray *json, int value);
void addFloatToJSONArray(JSONArray *json, float value);
void addBoolToJSONArray(JSONArray *json, bool value);
void addStringToJSONArray(JSONArray *json, char *value);
void addJSONObjectToJSONArray(JSONArray *json, JSONObject *value);
void addJSONArrayToJSONArray(JSONArray *json, JSONArray *value);

#endif