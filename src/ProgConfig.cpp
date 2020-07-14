#include "ProgConfig.h"
#include "LookupList.h"
#include "ConfigurationEntry.h"

ProgConfig::ProgConfig(unsigned int maxSize, unsigned int slotSize, String factoryFile) {
    this->maxSize = maxSize;
    this->dirtyList = new int[maxSize];
    this->dirtyListCnt = 0;
    this->factoryFile = factoryFile;
    this->ll = new LookupList(maxSize, slotSize);
    readConfig();
}

ProgConfig::~ProgConfig() {
    delete dirtyList;
    delete ll;
}

String ProgConfig::getValue(String key) {
  return getMetaData(key, C_DATATYPE_VALUE);
}

String ProgConfig::getMetaData(String key, String metaData) {
  int idx = ll->lookup(key, metaData);
  return (idx == -1) ? PSTR("Key not found.") : ll->getValueByIdx((unsigned int)idx);
}

int ProgConfig::setValue(String key, String value, unsigned int maxValLen) {
    int idx = setMetaData(key, C_DATATYPE_VALUE, value, maxValLen);
    if (idx != -1 ) {
      unsigned int i = 0;
      while (i < dirtyListCnt && dirtyList[i] != idx)
        i++;
      if (i == dirtyListCnt) {
        dirtyList[dirtyListCnt++] = idx;
      }
    }
    return idx;
}

int ProgConfig::setMetaData(String key, String metaData, String value, unsigned int maxValLen) {
    return ll->store(key, metaData, value, maxValLen);
}


int ProgConfig::openFile(String filename, String mode, File &file) {
  int ret = -1;
  if (filename == NULL) {
    Serial.println(F("ERROR: filename parameter is NULL. Abort."));
  } else {
    LittleFSConfig cfg;
    cfg.setAutoFormat(false);
    LittleFS.setConfig(cfg);
    LittleFS.begin();
    file = LittleFS.open(filename, mode.c_str());
    if (!file) {
      LittleFS.end();
      Serial.printf_P(PSTR("Failed to open %s for %s.\n"), filename.c_str(), ((mode == PSTR("r")) ? PSTR("reading") : PSTR("writing")));
    } else {
      ret = 0;
    }
  }
  return ret;
}

int ProgConfig::readLine(File *file) {
  bufIdx = 0;
  char c;
  while (bufIdx < 255 && (c = file->read()) != '\n') {
    buf[bufIdx++] = c;
  }
  if (bufIdx == 255)
    return -1;
  buf[bufIdx] = '\0';
  //Serial.printf_P(PSTR("after read %s\n"), buf);
  return bufIdx;
}

void ProgConfig::closeFile(File &file) {
  file.close();
  LittleFS.end();
}

int ProgConfig::readAndProcessFile(String filename, void(ProgConfig::*pfn)(String)) {
  File file;
  if (openFile(filename, PSTR("r"), file))
    return -1;
  int count = 0;
  while(file.available()) {
    if (readLine(&file) == -1)
        break;
    String text = buf;
    (this->*pfn)(text);
    count++;
  }
  closeFile(file);
  return count;
}

int ProgConfig::readConfig() {
  ll->cleanList();
  int count = 0, a = 0;
  if ((count = readAndProcessFile(factoryFile, &ProgConfig::processEntry)) == -1)
    return -1;
  if ((a = readAndProcessFile(factoryFile+".curr", &ProgConfig::processEntryKeyVal)) == -1)
    a = 0;
  return count + a;
}

int ProgConfig::writeConfig() {
  if (dirtyListCnt == 0) {
    return 0;
  }
  File file;
  if (openFile(factoryFile+".curr", "w", file))
    return -1;

  for (unsigned int i = 0; i < dirtyListCnt; i++) {
    String key = ll->getKeyByIdx(dirtyList[i]);
    String value = ll->getValueByIdx(dirtyList[i]);
    file.write(key.c_str()); file.write("={\""); file.write(value.c_str()); file.write("\"}\n");
  }
  closeFile(file);
  return dirtyListCnt;
}

void ProgConfig::processEntryKeyVal(String text) {
  unsigned int idx = 0;
  String key = getKey(text, idx);
  idx = move2FirstPara(text, idx);
  String val = getText(text, idx);
  //Serial.printf_P(PSTR("<%s>=<%s>\n"), key.c_str(), val.c_str());
  setMetaData(key, C_DATATYPE_VALUE, val);
}


void ProgConfig::processEntry(String text) {
  //String text = "WIFICONFSSID=\"{ true , \"WIFICO\\\"NFSSID\", 20, STR, \"ConfigAP SSID\", 0, \"ESP_%ESPID%\", NULL_INT, NULL}";
  unsigned int idx = 0;
  String key = getKey(text, idx);
  idx = move2FirstPara(text, idx);

  String boolConst[] = {C_BOOLTRUE, C_BOOLFALSE};
  String val = getConst(text, boolConst, idx);
  setMetaData(key, C_DATATYPE_DISPLAYFLG, val);

  val = getText(text, idx);
  setMetaData(key, C_DATATYPE_KEY, val);

  int maxValLen = getInteger(text, idx);
  setMetaData(key, C_DATATYPE_MAXVALLEN, "" + String(maxValLen));

  String dtConst[] = {C_DT_STRING, C_DT_INTEGER};
  String dtype = getConst(text, dtConst, idx);
  setMetaData(key, C_DATATYPE_DATATYPE, dtype);

  val = getText(text, idx); setMetaData(key, C_DATATYPE_DISPLAYTEXT, val);

  int defVal = getInteger(text, idx);
  setMetaData(key, C_DATATYPE_DEFINTEGER, "" + String(defVal));

  val = getText(text, idx); setMetaData(key, C_DATATYPE_DEFSTRING, val);

  int ival = getInteger(text, idx);
  val = getText(text, idx);
  if (dtype == String(C_DT_STRING))
    setMetaData(key,C_DATATYPE_VALUE, val, maxValLen);
  else
    setMetaData(key,C_DATATYPE_VALUE, "" + String(ival), 12);
}

String ProgConfig::getKey(String text, unsigned int &idx) {
  int pos = text.indexOf('=', idx);
  if (pos == -1) {
    Serial.println(F("No key terminator = found."));
    return "";
  }
  unsigned int old = idx;
  idx = (unsigned int) pos;
  return text.substring(old, pos);
}

unsigned int ProgConfig::move2FirstPara(String text, unsigned int idx) {
  int pos = text.indexOf('{', idx);
  if (pos == -1 || pos == ((int)text.length() - 1)) {
    Serial.println(F("No opening { found or no following content."));
    return INT16_MAX;
  }
  return pos + 1;
}

String ProgConfig::getConst(String text, String consts[], unsigned int &idx) {
  int pos = text.indexOf(',', idx);
  if (pos == -1) {
    pos = text.indexOf('}', idx);
    if (pos == -1) {
      Serial.println(F("No key terminator = found."));
      return "";
    }
  }
  for (unsigned int i = 0; i < sizeof(consts); i++) {
    String val = text.substring(idx, pos);
    val.trim();
    if (val == consts[i]) {
      idx = pos + 1;
      return consts[i];
    }
  }
  return "";
}

long ProgConfig::getInteger(String text, unsigned int &idx) {
  String consts[] = {PSTR("NULL_INT")};
  if (getConst(text, consts, idx) != "") {
    return INT32_MIN;
  }
  int pos = text.indexOf(',', idx);
  if (pos == -1) {
    pos = text.indexOf('}', idx);
    if (pos == -1) {
      Serial.println(F("No key terminator = found."));
      return INT32_MIN;
    }
  }
  String val = text.substring(idx, pos);
  idx = pos + 1;
  val.trim();
  return val.toInt();
}

String ProgConfig::getText(String text, unsigned int &idx) {
  String consts[] = {PSTR("NULL")};
  if (getConst(text, consts, idx) != "")
    return PSTR("NULL");
  int pos;
  String val;
  bool firstQuote = true;
  while ((pos = text.indexOf('\"', idx)) != -1) {
    //Serial.printf("Idx/Pos = %d/%d %s, ", idx, pos, text.substring(idx, pos).c_str() );
    if (pos != 0) {
      if (text.charAt(pos - 1) != '\\' && !firstQuote) {
        val += text.substring(idx, pos);
        break;
      }
      if (!firstQuote) {
        val += text.substring(idx, pos - 1);
        val += '\"';
      }
      firstQuote = false;
    }
    idx = pos + 1;
  }
  pos = text.indexOf(',', idx);
  if (pos == -1) {
    pos = text.indexOf('}', idx);
    if (pos == -1) {
      Serial.println(F("End of Text reached."));
      pos = text.length();
    }
  }
  idx = pos + 1;
  return val;
}

 void ProgConfig::llStat() {
   Serial.printf_P(PSTR("LookupList: used slots=%d, free space=%d\n"), ll->length(), ll->getFreeBuffer());
 }

void ProgConfig::llDump() {
  for (unsigned int i = 0; i < ll->length(); i++) {
    Serial.printf_P(PSTR("%03d. Key=<%s> dataType=<%s> value=<%s>\n"), i, ll->getKeyByIdx(i).c_str(), ll->getDataTypeByIdx(i).c_str(), ll->getValueByIdx(i).c_str());
  }
}
