#include <SerialCommands.h>
#include <EEPROM.h>
#include <WiFi.h>
#include <WebServer.h>
#include <RHReliableDatagram.h>
#include <RH_RF95.h>
#include <HardwareSerial.h>

void cmd_ssid(SerialCommands*);
void cmd_password(SerialCommands*);
void cmd_status(SerialCommands*);

const int rh_server_address = 1;
const int rh_client_address = 2;
const int port = 80;
bool server_started = false;
bool wifi_connected = false;
TaskHandle_t wifi_task_handle = NULL;
float water_temp = -9999;
float ambient_temp = -9999;
char ssid[32];
char password[32];
char serial_command_buffer_[64];
char rh_buf[8];

WebServer server(port);
HardwareSerial SerialPort(2);
RH_RF95<HardwareSerial> driver(SerialPort);
RHReliableDatagram manager(driver, rh_server_address);
SerialCommand cmd_ssid_("ssid", cmd_ssid);
SerialCommand cmd_password_("password", cmd_password);
SerialCommand cmd_status_("status", cmd_status);
SerialCommands serial_commands_(&Serial, serial_command_buffer_, sizeof(serial_command_buffer_), "\n", " ");

void handle_NotFound() {
    server.send(404, "text/plain", "404 not found");
}

void handle_temp() {
    Serial.println("Received /temp HTTP request");
    server.send(200, "text/html", SendHTML(ambient_temp,water_temp));
}

String SendHTML(float ambient_temp, float water_temp) {
    String ptr = "<!DOCTYPE html> <html>\n";
    ptr += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
    ptr += "<title>Water temperature sensor</title>\n";
    ptr += "</head>\n";
    ptr += "<body>\n";
    ptr += "<div id=\"div1\">\n";
    ptr += "<h1>Temperature sensor readings:</h1>\n";
    ptr += "<pre>Ambient temperature [C] : " + String(ambient_temp) + "</pre>\n";
    ptr += "<pre>Water temperature   [C] : " + String(water_temp) + "</pre>\n";
    ptr += "</div>\n";
    ptr += "</body>\n";
    ptr += "</html>\n";
    ptr += "<script>\n";
    ptr += "var today = new Date();\n";
    ptr += "var date = today.getFullYear() + '/' + (today.getMonth()+1) + '/' + today.getDate();\n";
    ptr += "var time = today.getHours()    + ':' + today.getMinutes()   + ':' + today.getSeconds();\n";
    ptr += "const para = document.createElement(\"pre\");\n";
    ptr += "const node = document.createTextNode(\"Timestamp               : \" + date + \" \" + time);\n";
    ptr += "para.appendChild(node);\n";
    ptr += "document.getElementById(\"div1\").appendChild(para);\n";
    ptr += "</script>\n";
//    Serial.println(ptr);
    return ptr;
}

void writeEEPROM(int offset, char * str, size_t len) {
    int i;
    offset = offset * len;
    for (i=0; i<strlen(str); i++){
        EEPROM.write(offset+i, str[i]);
    }
    EEPROM.write(offset+i, 0);
    EEPROM.commit();
}

void readEEPROM(int offset, char * str, size_t len) {
    offset = offset * len;
    for (int i=0; i<len; i++){
        str[i]=EEPROM.read(offset+i);
        if(str[i] == 0){
            break;
        }
    }
}

void cmd_ssid(SerialCommands* sender){
	//Do not use Serial.Print!
	//Use sender->GetSerial this allows sharing the callback method with multiple Serial Ports
    char *ssid_str = sender->Next();
    if (ssid_str == NULL){
        sender->GetSerial()->print("SSID is \"");
        sender->GetSerial()->print(ssid);
        sender->GetSerial()->println("\"");
        Serial.println();
		return;
	}
    strcpy(ssid,ssid_str);
    writeEEPROM(0,ssid,sizeof(ssid));
	sender->GetSerial()->print("SSID is \"");
    sender->GetSerial()->print(ssid);
    sender->GetSerial()->println("\"");
    Serial.println();
}

void cmd_password(SerialCommands* sender){
	//Do not use Serial.Print!
	//Use sender->GetSerial this allows sharing the callback method with multiple Serial Ports
    char *password_str = sender->Next();
    if (password_str == NULL){
        sender->GetSerial()->print("WiFi password is \"");
        sender->GetSerial()->print(password);
        sender->GetSerial()->println("\"");
        Serial.println();
		return;
	}
    strcpy(password,password_str);
    writeEEPROM(1,password,sizeof(password));
	sender->GetSerial()->print("WiFi password is \"");
    sender->GetSerial()->print(password);
    sender->GetSerial()->println("\"");
    Serial.println();
    reconnect_wifi();
}

void cmd_status(SerialCommands* sender){
	//Do not use Serial.Print!
	//Use sender->GetSerial this allows sharing the callback method with multiple Serial Ports
    sender->GetSerial()->print("ssid: ");
    sender->GetSerial()->println(ssid);
    sender->GetSerial()->print("password: ");
    sender->GetSerial()->println(password);
    sender->GetSerial()->print("IP address: ");
    sender->GetSerial()->println(WiFi.localIP());
    sender->GetSerial()->print("port: ");
    sender->GetSerial()->println(port);
    sender->GetSerial()->print("water_temp: ");
    sender->GetSerial()->println(water_temp);
    sender->GetSerial()->print("ambient_temp: ");
    sender->GetSerial()->println(ambient_temp);
    sender->GetSerial()->print("server_started: ");
    sender->GetSerial()->println(server_started);
    sender->GetSerial()->print("wifi_connected: ");
    sender->GetSerial()->println(wifi_connected);
}

void cmd_unrecognized(SerialCommands* sender, const char* cmd){
	sender->GetSerial()->print("ERROR: Unrecognized command [");
	sender->GetSerial()->print(cmd);
	sender->GetSerial()->println("]");
    sender->GetSerial()->println("Available commands:\nssid <ssid>\npassword <password>\nstatus");
    Serial.println();
}

void connect_wifi(void * parameter){
    Serial.println("Connecting to WiFi network");
    Serial.print("SSID: \"");
    Serial.print(ssid);
    Serial.println("\"");
    Serial.print("Password: ");
    Serial.print(password);
    Serial.println("\"");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
    }
    // Print local IP address and start web server
    Serial.println("");
    Serial.println("WiFi connected.");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    wifi_connected = true;
    vTaskDelete(NULL);
}

void reconnect_wifi(){
    if(wifi_task_handle != NULL){
        vTaskDelete(wifi_task_handle);
    }
    xTaskCreate(connect_wifi,"TaskOne",10000,NULL,1,&wifi_task_handle);
}

float float_from_bytes(uint8_t b3, uint8_t b2, uint8_t b1, uint8_t b0){
    union my_union {
        float float_variable;
        char bytes_array[4];
    };
    union my_union u;
    u.bytes_array[0] = b0;
    u.bytes_array[1] = b1;
    u.bytes_array[2] = b2;
    u.bytes_array[3] = b3;
    return u.float_variable;
}

void setup(){
    Serial.begin(9600);
    Serial.println();
    Serial.println("Water sensor server setup start");
    Serial.println();
    SerialPort.setPins(16,17);
    EEPROM.begin(sizeof(ssid)+sizeof(password));
    serial_commands_.AddCommand(&cmd_ssid_);
    serial_commands_.AddCommand(&cmd_password_);
    serial_commands_.AddCommand(&cmd_status_);
 	serial_commands_.SetDefaultHandler(&cmd_unrecognized);
    readEEPROM(0,ssid,sizeof(ssid));
    readEEPROM(1,password,sizeof(password));
    reconnect_wifi();
    server.on("/", handle_temp);
    server.onNotFound(handle_NotFound);
    if (manager.init()) {
        Serial.println("RH ready");
    } else {
        Serial.println("RH init failed");
    }
    Serial.println("Setup done!");
}

void loop(){
	serial_commands_.ReadSerial();
    if(wifi_connected){
        server.begin();
        Serial.print("HTTP server running on port ");
        Serial.println(port);
        server_started = true;
        wifi_connected = false;
    }
    if(server_started){
        server.handleClient();
    }
    if (manager.available()) {
        uint8_t len = sizeof(rh_buf);
        uint8_t from;
        if (manager.recvfromAck((uint8_t*)rh_buf, &len, &from)) {
            Serial.print("RH got request from: 0x");
            Serial.print(from, HEX);
            Serial.print(": ");
            for(int i=0; i<len; i++){
                uint8_t rbyte = *(rh_buf+i);
                Serial.print("0x");
                Serial.print(rbyte,HEX);
                Serial.print(" ");
            }
            Serial.println();
            water_temp   = float_from_bytes(rh_buf[3],rh_buf[2],rh_buf[1],rh_buf[0]);
            ambient_temp = float_from_bytes(rh_buf[7],rh_buf[6],rh_buf[5],rh_buf[4]);
            Serial.print("water_temp: ");
            Serial.println(water_temp);
            Serial.print("ambient_temp: ");
            Serial.println(ambient_temp);
        }
    }
}

