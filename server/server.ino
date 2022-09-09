#include <SerialCommands.h>
#include <EEPROM.h>
#include <WiFi.h>

bool wifi_connected;
TaskHandle_t wifi_task_handle = NULL;
WiFiServer server(80);

char ssid[32];
char password[32];

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

SerialCommand cmd_ssid_("ssid", cmd_ssid);

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

SerialCommand cmd_password_("password", cmd_password);

char serial_command_buffer_[64];
SerialCommands serial_commands_(&Serial, serial_command_buffer_, sizeof(serial_command_buffer_), "\n", " ");

void cmd_unrecognized(SerialCommands* sender, const char* cmd){
	sender->GetSerial()->print("ERROR: Unrecognized command [");
	sender->GetSerial()->print(cmd);
	sender->GetSerial()->println("]");
    sender->GetSerial()->println("Commands:\nssid <ssid>\npassword <password>");
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

void setup(){
    EEPROM.begin(sizeof(ssid)+sizeof(password));
    Serial.begin(9600);
    serial_commands_.AddCommand(&cmd_ssid_);
    serial_commands_.AddCommand(&cmd_password_);
 	serial_commands_.SetDefaultHandler(&cmd_unrecognized);
    readEEPROM(0,ssid,sizeof(ssid));
    readEEPROM(1,password,sizeof(password));
    Serial.println();
    Serial.println("Water temperature server init");
    Serial.println();
    reconnect_wifi();
    Serial.println("Ready!");
}

void loop(){
	serial_commands_.ReadSerial();
    if(wifi_connected){
        server.begin();
        wifi_connected = false;
    }

}

