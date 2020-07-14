#include "ConfigurationEntry.h"
#include <Arduino.h>
#include "ProgConfig.h"

void ConfigurationEntry::deleteEscapes(char *ptr) {
  char *idx = ptr;
  do {
    if (*ptr == '\"' and *(ptr -1) == '\\')
      idx--;
    *idx = *ptr;
    idx++; ptr++;
  } while (*ptr != '\0');
  *idx='\0';
}

char *ConfigurationEntry::trimWhiteSpaces(char *ptr) {
  while (*ptr == ' ' || *ptr == '\t') {
    ptr++;
  }
  return ptr;
}

char *ConfigurationEntry::getNextTextValue(char **ptr) {
  char *sptr = strstr_P(*ptr, PSTR("\""));    // Get starting quote
  if (sptr == NULL) {            // If there is no starting quote check for NULL
    sptr = strstr_P(*ptr, PSTR("NULL"));  // Check if NULL string is configured
    if (sptr == NULL)
      return NULL;              // then quit
    (*ptr) += 4;
    return NULL;
  }
  *ptr = sptr;
  char *valuePtr = (++(*ptr));  // Skip quote so it is not part of the value
  char prevChar;
  do {
    *ptr = strstr_P(*ptr,  PSTR("\""));  // Search for terminating quote
    if (*ptr == NULL)           // If there is no terminating quote
      return NULL;              // then quit
    prevChar = *((*ptr)-1);     // check prev char for escape char
    (*ptr)++;                   // skip quote to search onward
  } while (prevChar == '\\');   // iterate as long as quote is escaped
  *((*ptr)-1) = '\0';           // set terminator instead of last qoute
  deleteEscapes(valuePtr);
  return valuePtr;
}

ConfigurationEntry::~ConfigurationEntry() {
  if (config != NULL && dirty) {
    Serial.println("need to store configentry changes.");
    //config->storeEntry(this);
  }
  free(orgEntryData);
  if (valueS != NULL) {
    Serial.println("freeing up value data buffer.");
    delete valueS;
  }
}


ConfigurationEntry::ConfigurationEntry(ProgConfig *cfg) {
  config = cfg;
}

char *ConfigurationEntry::extractKey(char *orgPtr) {
  // Retrieve key of entry
  key = orgPtr;

  // trim }
  char *ptr = strrchr (orgEntryData, '}');
  if (ptr == NULL) {
    Serial.printf_P(PSTR("No closeing } found in <%s>"), orgEntryData);
    free(orgEntryData);
    return NULL;
  }
  *ptr = ',';  // replace } with seperating , char as value terminator
  // skip =
  ptr = strchr(orgEntryData, '=');
  if (ptr == NULL) {
    Serial.printf_P(PSTR("No seperating = for key & value found in <%s>"), orgEntryData);
    free(orgEntryData);
    return NULL;
  }
  *ptr = '\0';   // terminating key string
  ptr++;        // forward to probably {
  ptr = strchr(ptr, '{');
  if (ptr == NULL) {
    Serial.printf_P(PSTR("No opening { found in <%s>"), orgEntryData);
    free(orgEntryData);
    return NULL;
  }
  ptr++;
  return ptr;
}

#define EXTRACT2VALUES(LIT1,VAL1,LIT2,VAL2,VAR,TEXT) \
  ptr = entry->trimWhiteSpaces(ptr); \
  if (strncmp_P(ptr, PSTR(LIT1),strlen_P(PSTR(LIT1))) == 0) { \
    VAR = VAL1; \
  } else if (strncmp_P(ptr, PSTR(LIT2), strlen_P(PSTR(LIT2))) == 0) { \
    VAR = VAL2; \
  } else { \
    Serial.printf_P(PSTR(TEXT), entry->orgEntryData); \
    free(entry->orgEntryData); \
    return NULL; \
  } \
  ptr = strchr(ptr, ',');


#define EXTRACTTEXTVALUE(VAL,TEXT) \
  ptr = entry->trimWhiteSpaces(++ptr); \
  if (strncmp_P(ptr, PSTR("NULL"), 4) == 0) { \
    VAL = NULL;  \
    ptr+= 4; \
  } else if ( *ptr == '\"') { \
    VAL = entry->getNextTextValue(&ptr); \
  } else { \
    Serial.printf_P(PSTR(TEXT), entry->orgEntryData); \
    free(entry->orgEntryData); \
    return NULL; \
  }

#define EXTRACTINTEGERVALUE(VAL,TEXT) \
  ptr = entry->trimWhiteSpaces(ptr); \
  if (*ptr != ',') { \
    Serial.printf_P(PSTR(TEXT), entry->orgEntryData); \
    free(entry->orgEntryData); \
    return NULL; \
  } \
  ptr++; \
  ptr = entry->trimWhiteSpaces(++ptr); \
  comma = strchr(ptr, ','); \
  if (comma == NULL) { \
    Serial.printf_P(PSTR(TEXT), entry->orgEntryData); \
    free(entry->orgEntryData); \
    return NULL; \
  } \
  *comma = '\0'; \
  if (strncmp_P(ptr, PSTR("NULL_INT"), strlen_P(PSTR("NULL_INT"))) == 0) { \
    VAL = INT32_MIN; \
  } else { \
    VAL = atoi(ptr); \
  } \
  ptr = comma; \
  ptr++;


ConfigurationEntry *ConfigurationEntry::newInstance(ProgConfig *cfg, const char *entryText) {
  //char *property = strdup("WIFICONFSSID=\"{true, \"WIFICONFSSID\", 20, STR, \"ConfigAP SSID\", 0, \"ESP_%ESPID%\", 0, NULL}");

  ConfigurationEntry *entry = new ConfigurationEntry(cfg);
  entry->orgEntryData = strdup(entryText);
  char *ptr, *comma, *value;
  if ((ptr = entry->extractKey(entry->orgEntryData)) == NULL) return NULL;

  // get display flag
  EXTRACT2VALUES("true",true,"false",false,entry->display, "No boolean value true/false found as 1. parameter for display flag found in <%s>")

  // Get entry name put in qoutes, should be identical to key
  EXTRACTTEXTVALUE(entry->name, "No \"text\" or NULL found as 2. parameter for name parameter found in <%s>")

  // Get the max value length
  EXTRACTINTEGERVALUE(entry->valueLen, "No integer value or NULL_INT found as 3. parameter for max value len parameter found in <%s>")

  // Get the values type
  EXTRACT2VALUES("STR",STR,"INT",INT,entry->valueType, "No type value STR/INT found as 4. parameter for value type found in <%s>")

  // Get the description for the webpage
  EXTRACTTEXTVALUE(entry->description, "No \"text\" or NULL found as 5. parameter for web description parameter found in <%s>")

  // Get the default integer value
  EXTRACTINTEGERVALUE(entry->defaultValueI, "No integer value or NULL_INT found as 6. parameter for default integer value parameter found in <%s>")

  // Get the default string value for the webpage
  EXTRACTTEXTVALUE(entry->defaultValueS, "No \"text\" or NULL found as 7. parameter for default string value parameter found in <%s>")

  // Get current integer value
  EXTRACTINTEGERVALUE(entry->valueI, "No integer value or NULL_INT found as 8. parameter for current integer value parameter found in <%s>")

  // Get the default string value for the webpage
  EXTRACTTEXTVALUE(value, "No \"text\" or NULL found as 9. parameter for current string value parameter found in <%s>")

  entry->setValueS(value);

  //Serial.println(entry->asString());

  /* Serial.printf("%s,%s,%d,%s,%s,%d,%s,%d,%s\n", \
    (entry->display) ? "true" : "false",
    (entry->name == NULL) ? "NULL" : entry->name,
    entry->valueLen,
    (entry->valueType == STR) ? "STR" : "INT",
    (entry->description == NULL) ? "NULL" : entry->description,
    entry->defaultValueI,
    (entry->defaultValueS == NULL) ? "NULL" : entry->defaultValueS,
    entry->valueI,
    (entry->valueS == NULL) ? "NULL" : entry->valueS);
*/
  return entry;
}

void ConfigurationEntry::setValueS(const char *value) {
  //Serial.printf("setValueS -> value = %s\n", (value == NULL) ? "NULL" : value);
  //Serial.printf("setValueS -> valueS = %s\n", (valueS == NULL) ? "NULL" : valueS);
  if (valueS != NULL) {
    delete valueS;
    valueS = NULL;
  }
  if (value != NULL) {
    if (valueLen > 0 && valueLen < 256) {
      valueS = new char[valueLen+1];
      strncpy(valueS, value, valueLen + 1);
    } else {
      Serial.printf_P(PSTR("Valuelen out of range %d\n"), valueLen+1);
    }
  }
}

char *ConfigurationEntry::getValueSPtr() {
  if (valueS != NULL) {
    delete valueS;
    valueS = NULL;
  }
  if (valueLen > 0 && valueLen < 256) {
    valueS = new char[valueLen+1];
  } else {
    Serial.printf_P(PSTR("Valuelen out of range %d\n"), valueLen+1);
  }
  return valueS;
}

void ConfigurationEntry::setDirty() {
  dirty = true;
}
void ConfigurationEntry::setValueI(int value) {
  //Serial.printf("setValueS -> value = %s\n", (value == NULL) ? "NULL" : value);
  //Serial.printf("setValueS -> valueS = %s\n", (valueS == NULL) ? "NULL" : valueS);
  valueI = value;
}
