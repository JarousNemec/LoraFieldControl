#include <ESP8266HTTPClient.h>
#include "main.h"

void setup() {
    Serial.begin(9600);

    //Load configuration
    LoadConfiguration();

    //Init Transceiver
    ESerial.begin(9600);
    Serial.println("Starting Chat");
    Serial.println(Transceiver.init());
    Transceiver.PrintParameters();

    //Init web server and Wi-Fi ap
    randomSeed(analogRead(A0));
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(ssid + String(StationConfig.Id), password);
    WiFi.softAPConfig(local_ip, gateway, subnet);
    delay(100);
    server.on("/", HandleOnConnect);
    server.onNotFound(HandleNotFound);
    server.begin();
    Serial.println("HTTP server started");

    pinMode(PIN_SUCCESS_LED, OUTPUT);
    pinMode(PIN_SEND_LED, OUTPUT);

    if (!IsWifiAvailable(StationConfig.SSID) || StationConfig.SType != Beacon)
        return;

    // Begin WiFi
    WiFi.begin(String(StationConfig.SSID), String(StationConfig.PSW));

    // Connecting to WiFi...
    Serial.print("Connecting to ");
    Serial.print(StationConfig.SSID);
    while (WiFi.status() != WL_CONNECTED) {
        delay(100);
        Serial.print(".");
    }

    // Connected to Wi-Fi
    Serial.println();
    Serial.println("Connected!");
    Serial.print("IP address for network ");
    Serial.print(StationConfig.SSID);
    Serial.print(" : ");
    Serial.println(WiFi.localIP());
    Serial.print("IP address for network ");
    Serial.print(ssid + String(StationConfig.Id));
    Serial.print(" : ");
    Serial.print(WiFi.softAPIP());


}

bool IsWifiAvailable(const String &wifi_ssid) {
    bool IsAvailable = false;
    if (wifi_ssid == "")
        return false;
    char numberOfNetworks = WiFi.scanNetworks();

    for (int i = 0; i < numberOfNetworks; i++) {
        if (WiFi.SSID(i) == wifi_ssid)
            IsAvailable = true;
        Serial.print("Network name: ");
        Serial.println(WiFi.SSID(i));
        Serial.print("Signal strength: ");
        Serial.println(WiFi.RSSI(i));
        Serial.println("-----------------------");

    }
    return IsAvailable;
}

void loop() {
    server.handleClient();
    if (ESerial.available()) {
        String input = ESerial.readString();
        JsonObject &IncommingObj = jBuffer.parseObject(input);
        if (ValidatePacket(&IncommingObj)) {
            ProcessPacket(&IncommingObj);
        }
    }
    BehaveByStationType();
    ShineControl();
    jBuffer.clear();
}

void ShineControl() {
    if ((millis() - shine_send_time) > LED_TIMEOUT) {
        digitalWrite(PIN_SEND_LED, LOW);
        shine_send_time = 0;
    }

    if ((millis() - shine_success_time) > LED_TIMEOUT) {
        digitalWrite(PIN_SUCCESS_LED, LOW);
        shine_success_time = 0;
    }
}

void ShineSend() {
    shine_send_time = millis();
    digitalWrite(PIN_SEND_LED, HIGH);
}

void ShineSuccess() {
    shine_success_time = millis();
    digitalWrite(PIN_SUCCESS_LED, HIGH);
}

void BehaveByStationType() {
    ValidateTimeouts();
    switch (StationConfig.SType) {
        case Beacon:
            BehaveAsBeacon();
            break;
        case FieldStation:
            BehaveAsFieldStation();
            break;
        case Bridge:
            break;
        case Pinger:
            BehaveAsPinger();
            break;
        case UndefinedS:
            break;
    }
}

void ValidateTimeouts() {
    if (millis() - PingStatus.PingSentWhen > PING_TIMEOUT)
        PingStatus.PingSent = false;
    if (millis() - BeaconDiscoverStatus.DiscoverSentWhen > DISCOVER_TIMEOUT)
        BeaconDiscoverStatus.DiscoverSent = false;
    if (millis() - OtherDataStatus.OtherDataSentWhen > OTHER_DATA_TIMEOUT) {
        OtherDataStatus.OtherDataSent = false;
        if(OtherDataStatus.AttemptsCount > OTHER_DATA_MAX_ATTEMPTS){
            OtherDataStatus.OtherDataCollected = false;
            OtherDataStatus.OtherDataRequested = false;
        }else{
            OtherDataStatus.AttemptsCount++;
        }
    }
}

void BehaveAsBeacon() {
    if (!OtherDataStatus.OtherDataSent &&
        millis() - WaterLevelStatus.collect_water_level_time > WATER_LEVEL_COLLECT_TIME) {
        Serial.println("Water level requested!");
        SendPacket(PacketType::Syn, "OD", StationConfig.Id, StationConfig.WaterLevelFSId);
        OtherDataStatus.OtherDataSent = true;
        OtherDataStatus.OtherDataSentWhen = millis();
        OtherDataStatus.OtherDataTarget = StationConfig.WaterLevelFSId;
    }
}


void BehaveAsPinger() {
    if (BeaconId == "") {
        if (!BeaconDiscoverStatus.DiscoverSent) {
            Serial.println("Discover sent!");
            SendPacket(PacketType::Syn, "DiscB", StationConfig.Id, BeaconDiscoverStatus.DiscoverTarget);
            BeaconDiscoverStatus.DiscoverSent = true;
            BeaconDiscoverStatus.DiscoverSentWhen = millis();
        }
    } else {
        if (!PingStatus.PingSent) {
            Serial.println("Ping sent!");
            SendPacket(PacketType::Syn, "Ping", StationConfig.Id, BeaconId);
            PingStatus.PingSent = true;
            PingStatus.PingSentWhen = millis();
            PingStatus.PingSentTo = BeaconId;
        }
    }
}

void BehaveAsFieldStation() {
    if (BeaconId == "") {
        if (!BeaconDiscoverStatus.DiscoverSent) {
            Serial.println("Discover sent!");
            SendPacket(PacketType::Syn, "DiscB", StationConfig.Id, BeaconDiscoverStatus.DiscoverTarget);
            BeaconDiscoverStatus.DiscoverSent = true;
            BeaconDiscoverStatus.DiscoverSentWhen = millis();
        }
    } else {
        if (!OtherDataStatus.OtherDataSent && OtherDataStatus.OtherDataCollected &&
            OtherDataStatus.OtherDataRequested && StationConfig.FSType == WaterSensor) {
            Serial.println("Other data sent!");
            String data = "OD";
            data += WaterLevelStatus.actualLevel;
            SendPacket(PacketType::SynAck, data, StationConfig.Id, BeaconId);
            OtherDataStatus.OtherDataSent = true;
            OtherDataStatus.OtherDataSentWhen = millis();
            OtherDataStatus.OtherDataTarget = BeaconId;
        }
    }
}

void PrintPacketStructure(JsonObject *pckt) {
    Serial.print("Packet sent:");
    Serial.print((pckt->get<String>("t")));
    Serial.print(": ");
    Serial.print((pckt->get<String>("c")));
    Serial.print(": ");
    Serial.print((pckt->get<String>("s")));
    Serial.print(": ");
    Serial.println((pckt->get<String>("d")));
}

void SendPacket(PacketType type, const String &content, const String &source, const String &destination) {
    JsonObject &PacketToSend = jBuffer.createObject();
    PacketToSend["t"] = (int) type;
    PacketToSend["c"] = content;
    PacketToSend["s"] = source;
    PacketToSend["d"] = destination;
    String output;
    PacketToSend.printTo(output);
    ESerial.print(output);
    PrintPacketStructure(&PacketToSend);
    ShineSend();
}

void ProcessPacket(JsonObject *pckt) {
    String content = pckt->get<String>("c");
    Serial.println(content);
    if (content == "DiscB") {
        ProcessDiscoverBeacon(pckt);
    } else if (content == "Ping") {
        ProcessPing(pckt);
    } else if (content.substring(0, 2) == "OD") {
        ProcessOtherData(pckt);
    }
}

void CollectSensorData() {
    Serial.println("Collecting data...");
    WaterLevelStatus.actualLevel = random(100);
    OtherDataStatus.OtherDataCollected = true;
}

void ProcessOtherData(JsonObject *pckt) {
    auto source = pckt->get<String>("s");
    auto destination = pckt->get<String>("d");
    auto type = (PacketType) pckt->get<int>("t");
    switch (type) {
        case Syn:
            if (destination == StationConfig.Id && source == BeaconId) {
                OtherDataStatus.OtherDataRequested = true;
                OtherDataStatus.AttemptsCount = 0;
                CollectSensorData();
            }
            break;
        case SynAck:
            if (OtherDataStatus.OtherDataSent && OtherDataStatus.OtherDataTarget == source && OtherDataStatus.OtherDataTarget == StationConfig.WaterLevelFSId) {
                Serial.print("Successfully got data from:");
                Serial.println(source);
                OtherDataStatus.OtherDataSent = false;
                OtherDataStatus.OtherDataSentWhen = 0;
                OtherDataStatus.OtherDataTarget = "";
                WaterLevelStatus.collect_water_level_time = millis();
                String content = pckt->get<String>("c");

                if (WiFi.status() == WL_CONNECTED && String(StationConfig.WebServerAddress) != "") { //Check Wi-Fi connection status

                    String url = StationConfig.WebServerAddress;
                    url += WaterLevelStatus.waterLevelServerEndpoint;
                    http.begin(client,url);      //Specify request destination
                    http.addHeader("Auth", "bumbac");
                    http.addHeader("Level", content.substring(2));

                    int httpCode = http.POST("Hello world");   //Send the request
                    String payload = http.getString();                  //Get the response payload

                    Serial.println(httpCode);   //Print HTTP return code
                    Serial.println(payload);    //Print request response payload
                    http.end();  //Close connection

                } else {
                    Serial.println("Error in WiFi connection");
                }

                SendPacket(PacketType::Ack, "OD", StationConfig.Id, source);
                ShineSuccess();
            }
            break;
        case Ack:
            if (OtherDataStatus.OtherDataSent && OtherDataStatus.OtherDataTarget == source) {
                Serial.print("Successfully delivered data to:");
                Serial.println(source);
                OtherDataStatus.OtherDataSent = false;
                OtherDataStatus.OtherDataSentWhen = 0;
                OtherDataStatus.OtherDataTarget = "";
                OtherDataStatus.OtherDataCollected = false;
                OtherDataStatus.OtherDataRequested = false;
                ShineSuccess();
            }
            break;
    }
}

void ProcessPing(JsonObject *pckt) {
    auto source = pckt->get<String>("s");
    auto destination = pckt->get<String>("d");
    auto type = (PacketType) pckt->get<int>("t");
    switch (type) {
        case Syn:
            if (destination == StationConfig.Id) {
                SendPacket(PacketType::SynAck, "Ping", StationConfig.Id, source);
                PingStatus.PingSent = true;
                PingStatus.PingSentTo = source;
                PingStatus.PingSentWhen = millis();
            }
            break;
        case SynAck:
            if (PingStatus.PingSent && PingStatus.PingSentTo == source) {
                Serial.print("Successfully ping id:");
                Serial.println(source);
                PingStatus.PingSent = false;
                PingStatus.PingSentWhen = 0;
                PingStatus.PingSentTo = "all";
                SendPacket(PacketType::Ack, "Ping", StationConfig.Id, source);
                ShineSuccess();
            }
            break;
        case Ack:
            if (PingStatus.PingSent && PingStatus.PingSentTo == source) {
                Serial.print("Successfully ping id:");
                Serial.println(source);
                PingStatus.PingSent = false;
                PingStatus.PingSentWhen = 0;
                PingStatus.PingSentTo = "all";
                ShineSuccess();
            }
            break;
    }
}

void ProcessDiscoverBeacon(JsonObject *pckt) {
    auto source = pckt->get<String>("s");
    auto destination = pckt->get<String>("d");
    auto type = (PacketType) pckt->get<int>("t");
    switch (type) {
        case Syn:
            if (StationConfig.SType == Beacon) {
                SendPacket(PacketType::SynAck, "DiscB", StationConfig.Id, source);
                BeaconDiscoverStatus.DiscoverSent = true;
                BeaconDiscoverStatus.DiscoverTarget = source;
                BeaconDiscoverStatus.DiscoverSentWhen = millis();
            }
            break;
        case SynAck:
            if (BeaconDiscoverStatus.DiscoverSent && source != "" &&
                (StationConfig.SType == Pinger || StationConfig.SType == FieldStation)) {
                Serial.print("Successfully discovered beacon id:");
                Serial.println(source);
                BeaconId = source;
                BeaconDiscoverStatus.DiscoverSent = false;
                BeaconDiscoverStatus.DiscoverSentWhen = 0;
                BeaconDiscoverStatus.DiscoverTarget = "all";
                SendPacket(PacketType::Ack, "DiscB", StationConfig.Id, source);
                ShineSuccess();
            }
            break;
        case Ack:
            if (StationConfig.SType == Beacon && source == BeaconDiscoverStatus.DiscoverTarget &&
                BeaconDiscoverStatus.DiscoverSent) {
                Serial.println("Beacon successfully discovered new station!");
                ShineSuccess();
            }
            break;
    }
}

bool ValidatePacket(JsonObject *pckt) {
    return pckt->containsKey("t") && pckt->containsKey("s") && pckt->containsKey("d") && pckt->containsKey("c");
}

String SendHTML() {
    String ptr = "<!DOCTYPE html>\n"
                 "<html lang=\"en\">\n"
                 "<head>\n"
                 "    <meta charset=\"UTF-8\">\n"
                 "    <title>Station control</title>\n"
                 "</head>\n"
                 "<body>\n"
                 "    <form action=\"/\" method=\"post\">\n"

                 "        <label for=\"ssid\">Gateway SSID (only for beacon):</label>\n"
                 "        <input type=\"text\" id=\"ssid\" name=\"ssid\" value=\"" + String(StationConfig.SSID) +
                 "\"><br>\n"

                 "        <label for=\"psw\">Gateway PSW (only for beacon):</label>\n"
                 "        <input type=\"text\" id=\"psw\" name=\"psw\" value=\"" + String(StationConfig.PSW) +
                 "\"><br>\n"

                 "        <label for=\"wsa\">Web server address (only for beacon):</label>\n"
                 "        <input type=\"text\" id=\"wsa\" name=\"wsa\" value=\"" +
                 String(StationConfig.WebServerAddress) +
                 "\"><br>\n"

                 "        <label for=\"wlfsid\">Water level FS Id (only for beacon):</label>\n"
                 "        <input type=\"text\" id=\"wlfsid\" name=\"wlfsid\" value=\"" +
                 String(StationConfig.WaterLevelFSId) +
                 "\"><br>\n"

                 "        <label for=\"id\">ID:</label>\n"
                 "        <input type=\"text\" id=\"id\" name=\"id\" value=\"" + String(StationConfig.Id) + "\"><br>\n"

                                                                                                            "        <label>Station Type:</label>\n"
                                                                                                            "        <input type=\"radio\" id=\"beacon\" name=\"stype\" value=\"Beacon\" " +
                 (StationConfig.SType == Beacon ? "checked" : "") + " ><label for=\"beacon\" >Beacon</label>\n"
                                                                    "        <input type=\"radio\" id=\"fieldStation\" name=\"stype\" value=\"FieldStation\" " +
                 (StationConfig.SType == FieldStation ? "checked" : "") +
                 " ><label for=\"fieldStation\">FieldStation</label>\n"
                 "        <input type=\"radio\" id=\"bridge\" name=\"stype\" value=\"Bridge\"><label for=\"bridge\" " +
                 (StationConfig.SType == Bridge ? "checked" : "") + " >Bridge</label>\n"
                                                                    "        <input type=\"radio\" id=\"pinger\" name=\"stype\" value=\"Pinger\"><label for=\"pinger\" " +
                 (StationConfig.SType == Pinger ? "checked" : "") + " >Pinger</label><br>\n"

                                                                    "        <label>FieldStation Type:</label>\n"
                                                                    "        <input type=\"radio\" id=\"waterSensor\" name=\"fstype\" value=\"WaterSensor\" " +
                 (StationConfig.FSType == WaterSensor ? "checked" : "") +
                 " ><label for=\"waterSensor\" >WaterSensor</label>\n"
                 "        <input type=\"radio\" id=\"solarPanelController\" name=\"fstype\" value=\"SolarPanelController\" " +
                 (StationConfig.FSType == SolarPanelController ? "checked" : "") +
                 " ><label for=\"solarPanelController\">SolarPanelController</label>\n"

                 "        <input type=\"submit\" value=\"Submit\">"
                 "    </form>\n"
                 "</body>\n"
                 "</html>";
    return ptr;
}

void HandleOnConnect() {
    if (server.method() == HTTP_POST && server.args() == 7) {
        if (server.argName(4) == "id" && server.argName(5) == "stype") {
            String oldSsid = StationConfig.SSID;
            server.arg(0).toCharArray(StationConfig.SSID, 32);
            server.arg(1).toCharArray(StationConfig.PSW, 32);
            server.arg(2).toCharArray(StationConfig.WebServerAddress, 32);
            server.arg(3).toCharArray(StationConfig.WaterLevelFSId, 32);
            server.arg(4).toCharArray(StationConfig.Id, 32);
            StationConfig.SType = GetStationTypeFromString(server.arg(5));
            StationConfig.FSType = GetFieldStationTypeFromString(server.arg(6));
            SaveConfiguration();
            Serial.println("Configuration writed.");
            LoadConfiguration();
            if (oldSsid != StationConfig.SSID) {
                resetFunc();
            }
        }
    }
    server.send(200, "text/html", SendHTML());
}

void HandleNotFound() {
    server.send(404, "text/plain", "Not found");
}


StationType GetStationTypeFromString(const String &text) {
    if (text == "FieldStation") {
        return FieldStation;
    } else if (text == "Beacon") {
        return Beacon;
    } else if (text == "Bridge") {
        return Bridge;
    } else if (text == "Pinger") {
        return Pinger;
    }
    return UndefinedS;
}

String GetStationTypeFromEnum() {
    switch (StationConfig.SType) {
        case Beacon:
            return "Beacon";
        case FieldStation:
            return "FieldStation";
        case Bridge:
            return "Bridge";
        case Pinger:
            return "Pinger";
        default:
            return "UndefinedS";
    }
}

FieldStationType GetFieldStationTypeFromString(const String &text) {
    if (text == "WaterSensor") {
        return WaterSensor;
    } else if (text == "SolarPanelController") {
        return SolarPanelController;
    }
    return UndefinedFS;
}

String GetFieldStationTypeFromEnum() {
    switch (StationConfig.FSType) {
        case WaterSensor:
            return "WaterSensor";
        case SolarPanelController:
            return "SolarPanelController";
        default:
            return "UndefinedFS";
    }
}

void SaveConfiguration() {
    EEPROM.begin(512);
    EEPROM.put(CONFIGURATION_ADDRESS, StationConfig);
    boolean ok = EEPROM.commit();
    Serial.println((ok) ? "Commit OK" : "Commit failed");
    EEPROM.end();
}

void LoadConfiguration() {
    EEPROM.begin(512);
    EEPROM.get(CONFIGURATION_ADDRESS, StationConfig);
    EEPROM.end();
    PrintConfig();
}

void PrintConfig() {
    Serial.println("Loaded configuration...");
    Serial.print("Ssid: ");
    Serial.println(StationConfig.SSID);
    Serial.print("Psw: ");
    Serial.println(StationConfig.PSW);
    Serial.print("Web server address: ");
    Serial.println(StationConfig.WebServerAddress);
    Serial.print("WaterLevelFSId: ");
    Serial.println(StationConfig.WaterLevelFSId);
    Serial.print("Id: ");
    Serial.println(StationConfig.Id);
    Serial.print("SType:");
    Serial.println(GetStationTypeFromEnum());
    Serial.print("FSType:");
    Serial.println(GetFieldStationTypeFromEnum());
    Serial.println("----------------");
}