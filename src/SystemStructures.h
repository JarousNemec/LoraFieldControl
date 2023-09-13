#ifndef LORAFIELDCONTROL_SYSTEMSTRUCTURES_H
#define LORAFIELDCONTROL_SYSTEMSTRUCTURES_H

#include <WString.h>

enum PacketType {
    Syn = 1, SynAck = 2, Ack = 3
};

enum StationType {
    Beacon, FieldStation, Bridge, UndefinedS, Pinger
};
enum FieldStationType {
    UndefinedFS, WaterSensor, SolarPanelController
};

struct ConfigStruct {
    char Id[32]{};
    char SSID[32]{};
    char PSW[32]{};
    char WebServerAddress[32]{};
    char WaterLevelFSId[32]{};
    StationType SType = UndefinedS;
    FieldStationType FSType = UndefinedFS;
    unsigned short MaximumAnalogLevel{};
    unsigned short MinimumAnalogLevel{};
};

struct PingStatusStruct {
    bool PingSent = false;
    String PingSentTo = "";
    unsigned long PingSentWhen = 0;
};

struct BeaconDiscoverStatusStruct {
    bool DiscoverSent = false;
    unsigned long DiscoverSentWhen = 0;
    String DiscoverTarget = "all";
};

struct OtherDataStatusStruct {
    bool OtherDataSent = false;
    bool OtherDataRequested = false;
    bool OtherDataCollected = false;
    unsigned long OtherDataSentWhen = 0;
    String OtherDataTarget = "";
    String OtherData = "";
    int AttemptsCount = 0;
};


struct WaterLevelStatusStruct {
    unsigned long collect_water_level_time = 0;
    int actualLevelPercentage = 0;
    const String waterLevelServerEndpoint = "api/water-level/set";
};


#endif //LORAFIELDCONTROL_SYSTEMSTRUCTURES_H
