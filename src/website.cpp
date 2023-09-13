#include "website.h"

String AssembleHTMLPage(ConfigStruct *stConfig) {
    String ptr = "<!DOCTYPE html>\n"
                 "<html lang=\"en\">\n"
                 "<head>\n"
                 "    <meta charset=\"UTF-8\">\n"
                 "    <title>Station control</title>\n"
                 "</head>\n"
                 "<body>\n"
                 "    <form action=\"/\" method=\"post\">\n"

                 "        <label for=\"ssid\">Gateway SSID (only for beacon):</label>\n"
                 "        <input type=\"text\" id=\"ssid\" name=\"ssid\" value=\"" + String(stConfig->SSID) +
                 "\"><br>\n"

                 "        <label for=\"psw\">Gateway PSW (only for beacon):</label>\n"
                 "        <input type=\"text\" id=\"psw\" name=\"psw\" value=\"" + String(stConfig->PSW) +
                 "\"><br>\n"

                 "        <label for=\"wsa\">Web server address (only for beacon):</label>\n"
                 "        <input type=\"text\" id=\"wsa\" name=\"wsa\" value=\"" +
                 String(stConfig->WebServerAddress) +
                 "\"><br>\n"

                 "        <label for=\"wlfsid\">Water level FS Id (only for beacon):</label>\n"
                 "        <input type=\"text\" id=\"wlfsid\" name=\"wlfsid\" value=\"" +
                 String(stConfig->WaterLevelFSId) +
                 "\"><br>\n"

                 "        <label for=\"id\">ID:</label>\n"
                 "        <input type=\"text\" id=\"id\" name=\"id\" value=\"" +
                 String(stConfig->Id) +
                 "\"><br>\n"

                 "        <label>Station Type:</label>\n"
                 "        <input type=\"radio\" id=\"beacon\" name=\"stype\" value=\"Beacon\" " +
                 (stConfig->SType == Beacon ? "checked" : "") + " ><label for=\"beacon\" >Beacon</label>\n"
                                                                    "        <input type=\"radio\" id=\"fieldStation\" name=\"stype\" value=\"FieldStation\" " +
                 (stConfig->SType == FieldStation ? "checked" : "") +
                 " ><label for=\"fieldStation\">FieldStation</label>\n"
                 "        <input type=\"radio\" id=\"bridge\" name=\"stype\" value=\"Bridge\"><label for=\"bridge\" " +
                 (stConfig->SType == Bridge ? "checked" : "") + " >Bridge</label>\n"
                                                                    "        <input type=\"radio\" id=\"pinger\" name=\"stype\" value=\"Pinger\"><label for=\"pinger\" " +
                 (stConfig->SType == Pinger ? "checked" : "") + " >Pinger</label><br>\n"

                                                                    "        <label>FieldStation Type:</label>\n"
                                                                    "        <input type=\"radio\" id=\"waterSensor\" name=\"fstype\" value=\"WaterSensor\" " +
                 (stConfig->FSType == WaterSensor ? "checked" : "") +
                 " ><label for=\"waterSensor\" >WaterSensor</label>\n"
                 "        <input type=\"radio\" id=\"solarPanelController\" name=\"fstype\" value=\"SolarPanelController\" " +
                 (stConfig->FSType == SolarPanelController ? "checked" : "") +
                 " ><label for=\"solarPanelController\">SolarPanelController</label>\n"
                 "<br>\n"
                 "<br>\n"

                 "        <label>AnalogRead value from A0: "+String(analogRead(A0))+"</label><br>\n"
                 "        <label for=\"wlamaxv\">Water level analog maximal value(only for FS water sensor): </label>\n"
                 "        <input type=\"text\" id=\"wlamaxv\" name=\"wlamaxv\" value=\"" +
                 String(stConfig->MaximumAnalogLevel) +"\"><br>\n"

                 "        <label for=\"wlaminv\">Water level analog minimal value(only for FS water sensor): </label>\n"
                 "        <input type=\"text\" id=\"wlaminv\" name=\"wlaminv\" value=\"" +
                 String(stConfig->MinimumAnalogLevel) +
                 "\"><br>\n"

                 "        <input type=\"submit\" value=\"Save\">"
                 "    </form>\n"
                 "</body>\n"
                 "</html>";
    return ptr;
}
