#ifndef LookupList_h
#define LookupList_h
#include <Arduino.h>

struct tLookupEntry {
  char *key;
  char *dataType;
  char *value;
  byte maxValLen;
};

class LookupList {
public:
  LookupList(unsigned int slots = 10, unsigned int slotSize = 50);
  ~LookupList();
  int lookup(String key, String dataType);
  int store(String key, String dataType, String value, unsigned int maxValLen);
  void cleanList();
  unsigned int length();
  unsigned int getTotalSlots() { return slots; }
  unsigned int getFreeBuffer() { return (long)(slots * slotSize) - (nextFree - dataBuffer); }
  String getValueByIdx(unsigned int i);
  String getDataTypeByIdx(unsigned int i);
  String getKeyByIdx(unsigned int i);
private:
  char *allocateSpace(unsigned int spaceSize);
  char *reuseOrAllocate(bool search4key, String text);
private:
  tLookupEntry *list;
  char *dataBuffer;
  char *nextFree;
  unsigned int slots;
  unsigned int slotSize;
  unsigned int usedSlots;
};
#endif
