#ifndef ProgConfig_h
#define ProgConfig_h
#include <Arduino.h>
#include <LittleFS.h>

#define C_BOOLTRUE PSTR("true")
#define C_BOOLFALSE PSTR("false")
#define C_DATATYPE_DISPLAYFLG PSTR("DISPLAYFLAG")
#define C_DATATYPE_KEY PSTR("KEY")
#define C_DATATYPE_MAXVALLEN PSTR("MAXVALLEN")
#define C_DT_STRING PSTR("STR")
#define C_DT_INTEGER PSTR("INT")
#define C_DATATYPE_DATATYPE PSTR("DATATYPE")
#define C_DATATYPE_DISPLAYTEXT PSTR("DISPLAYTEXT")
#define C_DATATYPE_DEFINTEGER PSTR("DEFAULTINTEGER")
#define C_DATATYPE_DEFSTRING PSTR("DEFAULTSTRING")
#define C_DATATYPE_VALUE PSTR("VALUE")

class LookupList;

class ProgConfig {
public:
  ProgConfig(unsigned int maxSize, unsigned int slotSize, String factoryFile);
  ~ProgConfig();
  String getValue(String key);
  String getMetaData(String key, String metaData);
  int setValue(String key, String value, unsigned int maxValLen = 0);
  int setMetaData(String key, String metaData, String value, unsigned int maxValLen = 0);
  int readConfig();
  int writeConfig();
  void llStat();
  void llDump();
private:
  int *dirtyList;
  unsigned int dirtyListCnt;
  int maxSize;
  String factoryFile;
  LookupList *ll;
  char buf[256];
  unsigned int bufIdx;
private:
  int openFile(String filename, String mode, File &file);
  void closeFile(File &file);
  int readAndProcessFile(String filename, void(ProgConfig::*pfn)(String));
  int readLine(File *file);
  void processEntryKeyVal(String text);
  void processEntry(String text);
  String getKey(String text, unsigned int &idx);
  unsigned int move2FirstPara(String text, unsigned int idx);
  String getConst(String text, String consts[], unsigned int &idx);
  long getInteger(String text, unsigned int &idx);
  String getText(String text, unsigned int &idx);
};

#endif
