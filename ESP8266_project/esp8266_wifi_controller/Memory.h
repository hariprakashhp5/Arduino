#ifndef Memory_h
#define Memory_h


#include "Arduino.h"
#include "EEPROM.h"

class Memory : public EEPROMClass{
    public:
        Memory();
        bool isBridge();
        bool resetMemory();
        String getMeshSSID();
        String getMeshPSWD();
        String getWiFiSSID();
        String getWiFiPSWD();
        bool commitToEEPROM();
        bool isCredsAvailable();
        String getCredentials();
        bool setMeshSSID(String ssid);
        bool setMeshPSWD(String pswd);
        bool setWiFiSSID(String ssid);
        bool setWiFiPSWD(String pswd);
        bool setCredsState(char state);
        bool setBridge(char state);

    private:
        void println(String content);
};


#endif
