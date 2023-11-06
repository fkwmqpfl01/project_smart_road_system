#include <SoftwareSerial.h>
#include <LiquidCrystal.h>
#include <ArduinoJson.h>
// 핀 번호 (RS, E, DB4, DB5, DB6, DB7)
LiquidCrystal lcd(44, 45, 46, 47, 48, 49);  // LCD 연결

void setup() {
  lcd.begin(16,2);  //LCD 초기화
  lcd.setCursor(0, 0);  // 0행 0열로 이동
}

#include <SPI.h>

using namespace ArduinoJson::Parser;

#define SSID "<ssid>" // insert your SSID
#define PASS "<password>" // insert your password
#define LOCATIONID "jTD5BEAbCsM2ddtTbZqVrWgTXWlikqIJdhTgKJ9l5ZOha1%2FAjCes9bir%2FTIPqBiuOWraRPpgXfpVpheBjcI0ww%3D%3D" // location id
#define DST_IP "61.43.246.153" //공공데이터 주소
SoftwareSerial dbgSerial(2, 3); // RX, TX for debugging
JsonParser<32> parser;

void setup()
{
Serial.begin(9600);
Serial.setTimeout(5000);
dbgSerial.begin(115200); // for debuging
dbgSerial.println("Init");
lcd.setCursor(0, 0);  // 0행 0열로 이동

Serial.println("AT+RST"); // restet and test if module is redy
delay(1000);
if(Serial.find("ready")) {
dbgSerial.println("WiFi – Module is ready");

lcd.write("WiFi – Module is ready");
}else{
dbgSerial.println("Module dosn’t respond.");

lcd.write("Module dosn’t respond.");
lcd.write("Please reset.");
while(1);
}
delay(1000);
// try to connect to wifi
boolean connected=false;
for(int i=0;i<5;i++){
if(connectWiFi()){
connected = true;

lcd.println("Connected to WiFi…");
break;
}
}
if (!connected){

lcd.write("Coudn’t connect to WiFi.");
while(1);
}
delay(5000);
Serial.println("AT+CIPMUX=0"); // set to single connection mode
}


void printBusInfo(JsonObject& item)
{
  uint8_t station1 = item["station1"];
  uint8_t min1 = ceil((float)item["min1"] / 60);
  String lineid = item["lineid"];
  Serial.print("[No."); Serial.print(lineid); Serial.print("] "); Serial.print(station1); Serial.print(" stops before, arrives in "); Serial.print(min1); Serial.println(" minutes");
}



void loop()
{
String cmd = "AT+CIPSTART=\"TCP\",\"";
cmd += DST_IP;
cmd += "\",80";
Serial.println(cmd);
dbgSerial.println(cmd);
if(Serial.find("Error")) return;
cmd = "GET /openapi-data/service/busanBIMS2/stopArr?bstopid=175620101&serviceKey=";
cmd += LOCATIONID;
cmd += " HTTP/1.0\r\nHost: 61.43.246.153\r\n\r\n";
Serial.print("AT+CIPSEND=");
Serial.println(cmd.length());
if(Serial.find(">")){
dbgSerial.print(">");
}else{
Serial.println("AT+CIPCLOSE");
dbgSerial.println("connection timeout");

lcd.setCursor(2, 2);
lcd.write("connection timeout");
delay(1000);
return;
}
Serial.print(cmd);
unsigned int i = 0; //timeout counter
int n = 1; // char counter
char json[100]="{";
while (!Serial.find("\"main\":{")){} // find the part we are interested in.
while (i<60000) {
if(Serial.available()) {
char c = Serial.read();
json[n]=c;
if(c=='}') break;
n++;
i=0;
}
i++;
}
dbgSerial.println(json);

DynamicJsonBuffer jsonBuffer;
JsonObject& root = jsonBuffer.parseObject(httpResponse);

JsonObject& body = root["response"]["body"];

JsonObject& items = body["items"];
 for(int i = 0;i < 3;i++)
  {
    JsonObject& item = items["item"][i];
    printBusInfo(item);
  }
}

boolean connectWiFi()
{
Serial.println("AT+CWMODE=1");
String cmd="AT+CWJAP=\"";
cmd+=SSID;
cmd+="\",\"";
cmd+=PASS;
cmd+="\"";
dbgSerial.println(cmd);
Serial.println(cmd);
delay(2000);
if(Serial.find("OK")){
dbgSerial.println("OK, Connected to WiFi.");
return true;
}else{
dbgSerial.println("Can not connect to the WiFi.");
return false;
}
}
