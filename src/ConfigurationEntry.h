#ifndef ConfigurationEntry_h
#define ConfigurationEntry_h

#include <Arduino.h>
enum vType {STR, INT};
#define MAXENTRYLEN 256

class ProgConfig;

class ConfigurationEntry {
  char *key;
  bool display;
  const char *name;
  int valueLen = 0;
  vType valueType; /* 1=String, 2=int */
  const char *description;
  int defaultValueI;
  const char *defaultValueS;
  int valueI;
  char *valueS = NULL;
  char *orgEntryData;
  bool dirty = false;
  ProgConfig *config;

//  char *getNextFromUntil(char **ptr, const char *firstTerm, const char *secTerm);
//  char *getNextUntil(char *ptr, int terminator);
  char *getNextTextValue(char **ptr);
//  char *skipSeperators(char *ptr, const char *seperators);

  char *extractKey(char *orgPtr);
  char *trimWhiteSpaces(char *ptr);

  void deleteEscapes(char *ptr);
  ConfigurationEntry(ProgConfig *cfg);

public:
  static ConfigurationEntry *newInstance(ProgConfig *cfg, const char *entry);
  ~ConfigurationEntry();
  const char *getKey() { return key; };
  bool getDisplay() { return display; };
  const char *getName() { return name; }
  int getValueLen() { return valueLen; }
  vType getValueType() { return valueType; } /* 1=String, 2=int */
  const char *getDescription() { return description; }
  int getDefaultValueI() { return defaultValueI; }
  const char *getDefaultValueS() { return defaultValueS; }
  int getValueI() { return (valueI == INT32_MIN) ? defaultValueI : valueI; }
  void setValue(int value) { valueI = value; }
  const char *getValueS() { if (valueS == NULL) return (defaultValueS == NULL) ? "NULL" : defaultValueS; else return valueS; }
  void setValueS(const char *value);
  char *getValueSPtr();
  void setValueI(int value);
  void setDirty();
  String asString() { return String((display) ? "true" : "false") + "," +
    String((name == NULL) ? "NULL" : name) + "," +
    String(valueLen) + "," +
    String((valueType == STR) ? "STR" : "INT") + "," +
    String((description == NULL) ? "NULL" : description) + "," +
    String(defaultValueI) + "," +
    String((defaultValueS == NULL) ? "NULL" : defaultValueS) + "," +
    String(valueI) + "," +
    String((valueS == NULL) ? "NULL" : valueS); }
};
#endif
