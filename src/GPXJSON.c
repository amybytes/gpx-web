/**
 * Name: GPXJSON.c
 * Author: Ethan Rowan (1086586)
 * Date Created: 03/03/2021
 * Last Modified: 03/03/2021
 * 
 * GPXJSON is a small library to simplify the creation and parsing of JSON.
 * The following data types are supported: int, float, bool, char*, JSONObject,
 * JSONArray.
 **/

#include "GPXJSON.h"

#define JSON_BUFF_SIZE 512

#define strequals(a, b) (!strcmp(a, b))

int intLen(int n) {
    if (n == 0) {
        return 1;
    }
    int len = 0;
    while (n != 0) {
        len++;
        n /= 10;
    }
    return len;
}

char *jsonRawToString(void *value, int type) {
    char *str = NULL;
    int size = 0;
    if (type == TYPE_INT) {
        int rawData = *((int *)(value));
        size = intLen(rawData) + 5;
        str = malloc(size);
        sprintf(str, "%d", rawData);
    }
    else if (type == TYPE_FLOAT) {
        float rawData = *((float *)(value));
        size = 25;
        str = malloc(size);
        sprintf(str, "%.1f", rawData);
    }
    else if (type == TYPE_BOOL) {
        bool rawData = *((bool *)(value));
        size = 10;
        str = malloc(size);
        sprintf(str, "%s", rawData ? "true" : "false");
    }
    else if (type == TYPE_STRING) {
        char *rawData = (char *)(value);
        size = strlen(rawData) + 7;
        str = malloc(size);
        sprintf(str, "\"%s\"", rawData);
    }
    else if (type == TYPE_JSONOBJECT) {
        JSONObject *rawData = (JSONObject *)(value);
        char *objStr = jsonObjectToString(rawData);
        size = strlen(objStr) + 4;
        str = malloc(size);
        sprintf(str, "%s", objStr);
        free(objStr);
    }
    else if (type == TYPE_JSONARRAY) {
        JSONArray *rawData = (JSONArray *)(value);
        char *arrStr = jsonArrayToString(rawData);
        size = strlen(arrStr) + 4;
        str = malloc(size);
        sprintf(str, "%s", arrStr);
        free(arrStr);
    }
    return str;
}

char *jsonDataToString(void *data) {
    JSONData *jsonData = (JSONData *)data;
    char *name = jsonData->name;
    char *rawValue = jsonRawToString(jsonData->value, jsonData->type);
    int size = strlen(name) + strlen(rawValue) + 5;
    char *str = malloc(size);
    sprintf(str, "\"%s\":%s", name, rawValue);
    free(rawValue);
    return str;
}

char *jsonArrDataToString(void *data) {
    JSONArrData *jsonArrData = (JSONArrData *)data;
    return jsonRawToString(jsonArrData->value, jsonArrData->type);
}

void deleteJSONData(void *data) {
    JSONData *jsonData = (JSONData *)data;
    if (jsonData->type == TYPE_JSONOBJECT) {
        deleteJSONObject(jsonData->value);
    }
    else if (jsonData->type == TYPE_JSONARRAY) {
        deleteJSONArray(jsonData->value);
    }
    else {
        free(jsonData->value);
    }
    free(jsonData->name);
    free(jsonData);
}

void deleteJSONArrData(void *data) {
    JSONArrData *jsonArrData = (JSONArrData *)data;
    if (jsonArrData->type == TYPE_JSONOBJECT) {
        deleteJSONObject(jsonArrData->value);
    }
    else if (jsonArrData->type == TYPE_JSONARRAY) {
        deleteJSONArray(jsonArrData->value);
    }
    else {
        free(jsonArrData->value);
    }
    free(jsonArrData);
}

void deleteJSONObject(JSONObject *json) {
    freeList(json->data);
    free(json);
}

void deleteJSONArray(JSONArray *json) {
    freeList(json->data);
    free(json);
}

int compareJSONData(const void *first, const void *second) {
    return 0;
}

int compareJSONArrData(const void *first, const void *second) {
    return 0;
}

size_t getTypeSize(void *data, int type) {
    switch (type) {
        case TYPE_INT: return sizeof(int);
        case TYPE_FLOAT: return sizeof(float);
        case TYPE_BOOL: return sizeof(bool);
        case TYPE_STRING: return strlen((char *)data)+1;
        default: return 1;
    }
}

JSONObject *createJSONObject() {
    JSONObject *json = malloc(sizeof(JSONObject));
    json->data = initializeList(&jsonDataToString, &deleteJSONData, &compareJSONData);
    json->numElements = 0;
    return json;
}

JSONArray *createJSONArray() {
    JSONArray *json = malloc(sizeof(JSONArray));
    json->data = initializeList(&jsonArrDataToString, &deleteJSONArrData, &compareJSONArrData);
    json->numElements = 0;
    return json;
}

JSONObject *cloneJSONObject(JSONObject *json) {
    JSONObject *clone = createJSONObject();
    clone->numElements = json->numElements;
    ListIterator it = createIterator(json->data);
    JSONData *jsonData;
    JSONData *innerData;
    while ((jsonData = (JSONData *)nextElement(&it)) != NULL) {
        innerData = malloc(sizeof(JSONData));
        innerData->name = malloc(strlen(jsonData->name)+1);
        strcpy(innerData->name, jsonData->name);
        innerData->type = jsonData->type;
        if (jsonData->type == TYPE_JSONOBJECT) {
            innerData->value = cloneJSONObject((JSONObject *)(jsonData->value));
        }
        else if (jsonData->type == TYPE_JSONARRAY) {
            innerData->value = cloneJSONArray((JSONArray *)(jsonData->value));
        }
        else {
            size_t size = getTypeSize(jsonData->value, jsonData->type);
            innerData->value = malloc(size);
            memcpy(innerData->value, jsonData->value, size);
        }
        insertBack(clone->data, innerData);
    }
    return clone;
}

JSONArray *cloneJSONArray(JSONArray *json) {
    JSONArray *clone = createJSONArray();
    clone->numElements = json->numElements;
    ListIterator it = createIterator(json->data);
    JSONArrData *jsonArrData;
    JSONArrData *innerData;
    while ((jsonArrData = (JSONArrData *)nextElement(&it)) != NULL) {
        innerData = malloc(sizeof(JSONArrData));
        innerData->type = jsonArrData->type;
        if (jsonArrData->type == TYPE_JSONOBJECT) {
            innerData->value = cloneJSONObject((JSONObject *)(jsonArrData->value));
        }
        else if (jsonArrData->type == TYPE_JSONARRAY) {
            innerData->value = cloneJSONArray((JSONArray *)(jsonArrData->value));
        }
        else {
            size_t size = getTypeSize(jsonArrData->value, jsonArrData->type);
            innerData->value = malloc(size);
            memcpy(innerData->value, jsonArrData->value, size);
        }
        insertBack(clone->data, innerData);
    }
    return clone;
}

void putJSONData(JSONObject *json, char *name, void *value, int type) {
    ListIterator it;
    JSONData *tempJsonData;
    it = createIterator(json->data);
    while ((tempJsonData = (JSONData *)nextElement(&it)) != NULL) {
        if (!strcmp(tempJsonData->name, name)) {
            deleteDataFromList(json->data, (void *)tempJsonData); // overwrite old data with new (PUT)
        }
    }

    JSONData *jsonData = malloc(sizeof(JSONData));
    jsonData->name = malloc(strlen(name)+1);
    strcpy(jsonData->name, name);
    if (type == TYPE_JSONOBJECT) {
        JSONObject *clone = cloneJSONObject((JSONObject *)value);
        jsonData->value = clone;
    }
    else if (type == TYPE_JSONARRAY) {
        JSONArray *clone = cloneJSONArray((JSONArray *)value);
        jsonData->value = clone;
    }
    else {
        size_t size = getTypeSize(value, type);
        jsonData->value = malloc(size);
        memcpy(jsonData->value, value, size);
    }
    jsonData->type = type;
    insertBack(json->data, jsonData);
    json->numElements++;
}

void putIntInJSONObject(JSONObject *json, char *name, int value) {
    putJSONData(json, name, (void *)(&value), TYPE_INT);
}

void putFloatInJSONObject(JSONObject *json, char *name, float value) {
    putJSONData(json, name, (void *)(&value), TYPE_FLOAT);
}

void putBoolInJSONObject(JSONObject *json, char *name, bool value) {
    putJSONData(json, name, (void *)(&value), TYPE_BOOL);
}

void putStringInJSONObject(JSONObject *json, char *name, char *value) {
    putJSONData(json, name, (void *)value, TYPE_STRING);
}

void putJSONObjectInJSONObject(JSONObject *json, char *name, JSONObject *value) {
    putJSONData(json, name, (void *)value, TYPE_JSONOBJECT);
}

void putJSONArrayInJSONObject(JSONObject *json, char *name, JSONArray *value) {
    putJSONData(json, name, (void *)value, TYPE_JSONARRAY);
}

void addJSONArray(JSONArray *json, void *value, int type) {
    JSONArrData *jsonArrData = malloc(sizeof(JSONArrData));
    if (type == TYPE_JSONOBJECT) {
        JSONObject *clone = cloneJSONObject((JSONObject *)value);
        jsonArrData->value = clone;
    }
    else if (type == TYPE_JSONARRAY) {
        JSONArray *clone = cloneJSONArray((JSONArray *)value);
        jsonArrData->value = clone;
    }
    else {
        size_t size = getTypeSize(value, type);
        jsonArrData->value = malloc(size);
        memcpy(jsonArrData->value, value, size);
    }
    jsonArrData->type = type;
    insertBack(json->data, jsonArrData);
    json->numElements++;
}

void addIntToJSONArray(JSONArray *json, int value) {
    addJSONArray(json, (void *)(&value), TYPE_INT);
}

void addFloatToJSONArray(JSONArray *json, float value) {
    addJSONArray(json, (void *)(&value), TYPE_FLOAT);
}

void addBoolToJSONArray(JSONArray *json, bool value) {
    addJSONArray(json, (void *)(&value), TYPE_BOOL);
}

void addStringToJSONArray(JSONArray *json, char *value) {
    addJSONArray(json, (void *)value, TYPE_STRING);
}

void addJSONObjectToJSONArray(JSONArray *json, JSONObject *value) {
    addJSONArray(json, (void *)value, TYPE_JSONOBJECT);
}

void addJSONArrayToJSONArray(JSONArray *json, JSONArray *value) {
    addJSONArray(json, (void *)value, TYPE_JSONARRAY);
}

char *jsonObjectToString(JSONObject *json) {
    ListIterator it;
    JSONData *jsonData;
    char *str = malloc(JSON_BUFF_SIZE);
    strcpy(str, "{");
    int stringSize = JSON_BUFF_SIZE;
    char *objStr;
    int i = 0;

    it = createIterator(json->data);
    while ((jsonData = (JSONData *)nextElement(&it)) != NULL) {
        objStr = jsonDataToString(jsonData);
        while (strlen(objStr)+1 > stringSize-strlen(str)-1) {
            stringSize += JSON_BUFF_SIZE;
            str = realloc(str, stringSize);
        }
        strcat(str, objStr);
        if (i+1 < json->numElements) {
            strcat(str, ",");
        }
        free(objStr);
        i++;
    }
    strcat(str, "}");
    return str;
}

char *jsonArrayToString(JSONArray *json) {
    ListIterator it;
    JSONArrData *jsonArrData;
    char *str = malloc(JSON_BUFF_SIZE);
    strcpy(str, "[");
    int stringSize = JSON_BUFF_SIZE;
    char *objStr;
    int i = 0;

    it = createIterator(json->data);
    while ((jsonArrData = (JSONArrData *)nextElement(&it)) != NULL) {
        objStr = jsonArrDataToString(jsonArrData);
        while (strlen(objStr)+1 > stringSize-strlen(str)-1) {
            stringSize += JSON_BUFF_SIZE;
            str = realloc(str, stringSize);
        }
        strcat(str, objStr);
        if (i+1 < json->numElements) {
            strcat(str, ",");
        }
        free(objStr);
        i++;
    }
    strcat(str, "]");
    return str;
}

char *jsonObjectToStringAndEat(JSONObject *json) {
    char *jsonStr = jsonObjectToString(json);
    deleteJSONObject(json);
    return jsonStr;
}

char *jsonArrayToStringAndEat(JSONArray *json) {
    char *jsonStr = jsonArrayToString(json);
    deleteJSONArray(json);
    return jsonStr;
}

JSONObject *parseJSONString(char *jsonStr) {
    JSONObject *json = createJSONObject();
    if (jsonStr[0] != '{') {
        return NULL;
    }

    JSONData *jsonData;
    bool nameRead = false;
    char *name = NULL;
    int j = 0;
    for (int i = 1; jsonStr[i] != '\0'; i++) {
        // Start of name or string value
        if (jsonStr[i] == '\"') {
            // Start of name
            if (!nameRead) {
                for (j = i+1; jsonStr[j] != '\"' && jsonStr[j] != '\0'; j++)
                    ;
                name = malloc(j-i);
                strncpy(name, jsonStr+i+1, j-i-1); // set the name
                name[j-i-1] = '\0';
                nameRead = true;
                i = j+1; // increment past the quote and colon ("name":...)
            }
            // Start of string value
            else {
                // TODO
            }
        }
        // Start of nested JSON object
        else if (jsonStr[i] == '{') {
            int level = 1;
            for (j = i+1; level > 0 && jsonStr[j] != '\0'; j++) {
                if (jsonStr[j] == '{') {
                    level++;
                }
                else if (jsonStr[j] == '}') {
                    level--;
                }
            }
            char *tempStr = malloc(j-i+1);
            strncpy(tempStr, jsonStr+i, j-i);
            JSONObject *obj = parseJSONString(tempStr);
            free(tempStr);
            jsonData = malloc(sizeof(JSONData));
            jsonData->name = malloc(strlen(name)+1);
            strcpy(jsonData->name, name);
            jsonData->type = TYPE_JSONOBJECT;
            jsonData->value = obj;
            insertBack(json->data, jsonData);
        }
        // Start of JSON array
        else if (nameRead && jsonStr[i] == '[') {
            // TODO
        }
        // Start of value (int, float, bool)
        else if (nameRead) {
            bool isNumeric = true;
            bool hasDecimal = false;
            for (j = i; jsonStr[j] != '}' && jsonStr[j] != '\0'; j++) {
                if (jsonStr[j] == '.') {
                    hasDecimal = true;
                }
                else if (jsonStr[j] < '0' || jsonStr[j] > '9') {
                    isNumeric = false;
                }
            }
            int type;
            if (!isNumeric) {
                type = TYPE_BOOL;
            }
            else if (hasDecimal) {
                type = TYPE_FLOAT;
            }
            else {
                type = TYPE_INT;
            }
            printf("type: %d\n", type);
            printf("name: %s\n", name);
            jsonData = malloc(sizeof(JSONData));
            jsonData->name = malloc(strlen(name)+1);
            strcpy(jsonData->name, name);
            jsonData->type = type;
            int size = getTypeSize(NULL, type);
            printf("size: %d\n", size);
            jsonData->value = malloc(size);
            char *data = malloc(j-i+1);
            strncpy(data, jsonStr+i, j-i);
            if (type == TYPE_INT) {
                int value = atoi(data);
                printf("val: %d\n", value);
                memcpy(jsonData->value, &value, size);
            }
            else if (type == TYPE_FLOAT) {
                float value = (float)atof(data);
                memcpy(jsonData->value, &value, size);
            }
            else if (type == TYPE_BOOL) {
                bool value = false;
                if (strequals(data, "true")) {
                    value = true;
                }
                memcpy(jsonData->value, &value, size);
            }
            insertBack(json->data, jsonData);
            i = j;
        }
    }
    return json;
}