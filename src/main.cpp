#include <Arduino.h>
#include "LookupList.h"
#include "ProgConfig.h"



/******************************************************************************/
unsigned long freeHeap() {
/******************************************************************************/
  uint32_t freeHeap;
  ESP.getHeapStats(&freeHeap);
  return freeHeap;
}

#define HEAP_BEGIN(A) \
unsigned long heapStartSize = freeHeap(); \
unsigned long heapStartTime = millis(); \
Serial.print(F("--> ")); Serial.print(A); Serial.printf_P(PSTR(": free heap (%ld)...\n"), heapStartSize);

#define HEAP_END(A) \
Serial.print(F("<-- ")); Serial.print(A); Serial.printf_P(PSTR(": free heap (%ld) in %ldms ...\n"), heapStartSize - freeHeap(), millis() - heapStartTime);

#define ASSERT(A,B) \
if (A != B) { \
  Serial.printf_P(PSTR(#A " returned %d. Expected %d\n"), A, B); \
  Serial.println(F(".... aborting test ....")); \
  return; \
} else { \
  Serial.printf_P(PSTR(#A " returned %d as expected %d ==> OK (%d)\n"), A, B, freeHeap()); \
}

#define ASSERT_STR(A,B) \
if (String(A) != String(B)) { \
  Serial.printf_P(PSTR(#A " returned %s. Expected %s\n"), String(A).c_str(), String(B).c_str()); \
  Serial.println(F(".... aborting test ....")); \
  return; \
} else { \
  Serial.printf_P(PSTR(#A " returned %s as expected %s ==> OK (%d)\n"), String(A).c_str(), String(B).c_str(), freeHeap()); \
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println();
  Serial.println("Test programm for LookupList");
  {
    HEAP_BEGIN(F("First Test - the easy path"))
    {
      unsigned int slots = 5;
      unsigned int slotsize = 20;
      Serial.printf_P(PSTR("Create LookupList with %d slots and %d bytes space for each\n"), slots, slotsize);
      LookupList ll(slots,slotsize);
      ASSERT( ll.getTotalSlots(), slots)
      int usedSlot = ll.store("Key1", "DF", "Testvalue", 20);
      ASSERT(ll.getFreeBuffer(), 72)
      ASSERT(usedSlot, 0);
      ASSERT(ll.store("Key1", "DF", "Value", 20), 0)
      ASSERT(ll.getFreeBuffer(), 72)

      usedSlot = ll.store("Key2", "DF", "false", 6);
      ASSERT(usedSlot, 1);
      ASSERT(ll.getFreeBuffer(), 61)
      ASSERT(ll.store("Key2", "DD", "Value", 7), 2)
      ASSERT(ll.getFreeBuffer(), 51)
      ASSERT(ll.store("Key2", "DD", "MyValue", 7), 2)
      ASSERT(ll.getFreeBuffer(), 51)
      ASSERT(ll.store("Key4", "DD", "MyValue", 7), 3)
      ASSERT(ll.getFreeBuffer(), 39)
      ASSERT(ll.store("Key5", "DD", "MyValue", 6), 4)
      ASSERT(ll.getFreeBuffer(), 28)
      ASSERT(ll.store("Key6", "DD", "MyValue", 20), -1)   // No more slots
      ASSERT(ll.store("Key4", "DF", "MyValue", 30), -1)   // No more space
      ASSERT(ll.getFreeBuffer(), 28)

      ASSERT(ll.lookup("Key4", "DF"), -1)
      ASSERT(ll.lookup("Key4", "DD"), 3)

      ASSERT_STR(ll.getValueByIdx(ll.lookup("Key4", "DD")), "MyValu")
      ASSERT_STR(ll.getDataTypeByIdx(ll.lookup("Key4", "DD")), "DD")
      ASSERT_STR(ll.getKeyByIdx(ll.lookup("Key4", "DD")), "Key4")
      ll.cleanList();
      Serial.println(F("Cleaned list."));
      ASSERT(ll.getFreeBuffer(), 100);
      ASSERT(ll.length(), 0);
    }
    HEAP_END(F("First Test - the easy path"))
  }
  HEAP_BEGIN(F("Second Test - ProgConfig"))
  {
    HEAP_BEGIN(F("Second Test - ProgConfig"))
    {
      ProgConfig progConf(220, 10, "factory.conf");
      progConf.llStat();

      ASSERT_STR(progConf.getValue("NTPSERVER"), "NULL")
      ASSERT_STR(progConf.getMetaData("NTPSERVER", C_DATATYPE_DEFSTRING), "europe.pool.ntp.org")
      ASSERT_STR(progConf.getMetaData("PWR_PIN", C_DATATYPE_DEFINTEGER), "5")

      ASSERT(progConf.setValue("PWR_PIN", "9"), 143);
      ASSERT(progConf.setMetaData("PWR_PIN", C_DATATYPE_DEFSTRING,  "9"), 142);
      ASSERT(progConf.writeConfig(), 1);
      ASSERT(progConf.readConfig(), 25);

      progConf.llStat();
      //progConf.llDump();
    }
    HEAP_END(F("Second Test - ProgConfig"))
  }
  HEAP_END(F("Second Test - ProgConfig"))
  // put your setup code here, to run once:
}

void loop() {
  // put your main code here, to run repeatedly:
}
