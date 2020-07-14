#include "LookupList.h"

LookupList::LookupList(unsigned int slots, unsigned int slotSize) {
  this->slots = slots;
  this->slotSize = slotSize;
  this->usedSlots = 0;
  list = new tLookupEntry[this->slots];
  dataBuffer = new char [this->slots * this->slotSize];
  nextFree = dataBuffer;
}

LookupList::~LookupList() {
  delete list;
  delete dataBuffer;
}

char *LookupList::allocateSpace(unsigned int spaceSize) {
  unsigned long a = nextFree - dataBuffer;
  if (a > (slots * slotSize)) {
    Serial.println(F("No more space left in LookupList buffer"));
    return NULL;
  }
  char *ptr = nextFree;
  nextFree += spaceSize;
  return ptr;
}

int LookupList::lookup(String key, String dataType) {
  for (unsigned int i = 0; i < usedSlots; i++) {
    if (String(list[i].key) == key && String(list[i].dataType) == dataType) {
      return i;
    }
  }
  return -1;
}

char *LookupList::reuseOrAllocate(bool search4key, String text) {
  for (unsigned int i = 0; i < usedSlots; i++) {
    if (search4key) {
      if (String(list[i].key) == text) {
        return list[i].key;
      }
    } else {
      if (String(list[i].dataType) == String(text)) {
        return list[i].dataType;
      }
    }
  }
  int len = text.length()+1;
  return allocateSpace(len);
}

int LookupList::store(String key, String dataType, String value, unsigned int maxValLen) {
  char *ptr;
  for (unsigned int i = 0; i < usedSlots; i++) {
    if (String(list[i].key) == key && String(list[i].dataType) == dataType) {
      if (maxValLen > list[i].maxValLen) {
        if ((ptr = allocateSpace(maxValLen)) == NULL)
          return -1;
        list[i].maxValLen = maxValLen;
        list[i].value = ptr;
      }
      snprintf(list[i].value, list[i].maxValLen, "%s", value.c_str());
      return i;
    }
  }
  if (usedSlots < slots) {
    if ((list[usedSlots].key = reuseOrAllocate(true, key)) == NULL)
      return -1;
    snprintf(list[usedSlots].key, key.length()+1, "%s", key.c_str());
    if ((list[usedSlots].dataType = reuseOrAllocate(false, dataType)) == NULL)
      return -1;
    snprintf(list[usedSlots].dataType, dataType.length()+1, "%s", dataType.c_str());
    int len = value.length()+1;
    if (maxValLen == 0)
      maxValLen = len;
    if ((list[usedSlots].value = allocateSpace(maxValLen)) == NULL)
      return -1;
    snprintf(list[usedSlots].value, (len < (int)maxValLen) ? len : maxValLen, "%s", value.c_str());
    list[usedSlots].maxValLen = maxValLen;
    usedSlots++;
    return usedSlots-1;
  } else {
    Serial.printf_P(PSTR("All %d slots in lookup table used. Can't store additional entry.\n"), slots);
    return -1;
  }
}

void LookupList::cleanList() {
  usedSlots = 0;
  nextFree = dataBuffer;
}
unsigned int LookupList::length() {
  return usedSlots;
}

String LookupList::getValueByIdx(unsigned int i) {
  if (i < usedSlots) {
    return list[i].value;
  } else
    return "Index for LookupList out of range.";
}

String LookupList::getDataTypeByIdx(unsigned int i) {
  if (i < usedSlots)
    return list[i].dataType;
  else
    return "Index for LookupList out of range.";
}

String LookupList::getKeyByIdx(unsigned int i) {
  if (i < usedSlots)
    return list[i].key;
  else
    return "Index for LookupList out of range.";
}
