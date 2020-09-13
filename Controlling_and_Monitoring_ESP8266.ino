#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <DHT11.h>
#include <ArduinoJson.h>
#include "Gsender.h"
#define DHTTYPE DHT11 

const char ssid[] = "XXXX"; //INPUT SSID
const char pass[] = "XXXX"; //INPUT PASS

boolean TRUE;
bool executed = false;

const int PIN1 = D0;     //Controlling 1 Fan/Lamp
const int PIN2 = D1;    //Controlling 2 Fan/Lamp
const int DIGITAL_PIN = 12; 
const int ldrPin = A0;      //Monitoring LDR
const int rainpin = D5;     //Monitoring Rainy Sensor
const int setpoint = D8;         
int pin = D4;               //Monitoring DHT11 

WiFiServer server(80);
WiFiClient client;
DHT11 dht(pin);
int pfLDR;
String pfRain;
int ex  = 0;
int vol = -1;

double Fahrenheit(double celsius) 
{return ((double)(9 / 5) * celsius) + 32;}

double Kelvin(double celsius) 
{return celsius + 273.15;}

JsonObject& prepareResponse(JsonBuffer& jsonBuffer){
  JsonObject& root = jsonBuffer.createObject();
  JsonArray& LDRValues = root.createNestedArray("LDR Status");
  pfLDR = analogRead(ldrPin);
  LDRValues.add(pfLDR);
  JsonArray& RainValues = root.createNestedArray("Rain Status");
  RainValues.add(pfRain);    
  return root;}

#pragma region Globals                    
uint8_t connection_state = 0;                    
uint16_t reconnect_interval = 10000;             
#pragma endregion Globals

uint8_t WiFiConnect(const char* nSSID = nullptr, const char* nPassword = nullptr) {
   static uint16_t attempt = 0; 
   
   uint8_t i = 0;       
   ++attempt;
   return true;
    }                                   

void Awaits(){
    uint32_t ts = millis();
    while(!connection_state)
    {
        delay(50);
        if(millis() > (ts + reconnect_interval) && !connection_state){
            connection_state = WiFiConnect();
            ts = millis();}}
} 

void setup() {
  pinMode(D2, OUTPUT);
  pinMode(D3, OUTPUT);
  pinMode(D6, OUTPUT);
  pinMode(D7, OUTPUT);
  
  initHardWare();
  connectWiFi();
  server.begin();
  stop_all_now();
  setupMDNS();
  Awaits();
  digitalWrite(D0, LOW);
  digitalWrite(D1, LOW);
  }

void loop() {
  WiFiClient client = server.available();

  int err;
  float temp, humi;

  dht.read(humi, temp);
  int currentState = digitalRead(D5); 
  Serial.println (temp);
  //Serial.println (currentState);
  int currentSetpoint = digitalRead(D8);
  int currentLamp = digitalRead(D0);  
  Serial.println (currentSetpoint);
  Serial.println (currentLamp);
  delay (5000);
  if ( temp > 45.00 && currentLamp < 1 && currentSetpoint < 1){
    digitalWrite (PIN1, HIGH);
    delay (30000);
    digitalWrite (PIN1, LOW);}
    
  if (currentState == 1 && executed == false) {
     email2();
     executed = true;}
        
  if (currentState == 0 && executed == true) {
     email1();
     executed = false;}
  
  String req = client.readStringUntil('\r');
  Serial.println(req);
  client.flush();

  int val=0;
  if (req.indexOf("/stop") != -1){
    val = 0;
         client.flush();
     String s = "HTTP/1.1 200 OK\r\n";
     s += "Content Type: text/html\r\n\r\n";
     s += "<!DOCTYPE HTML>\r\n<html>\r\n";
           s += "Lamp is ";
           s += (vol)?"Mati":"Nyala";
           s += "</html>\n";
           client.print(s);    //this prob
           delay(1);}
           
  else if (req.indexOf("/maju") != -1){
    val = 1;
             client.flush();
     String s = "HTTP/1.1 200 OK\r\n";
     s += "Content Type: text/html\r\n\r\n";
     s += "<!DOCTYPE HTML>\r\n<html>\r\n";
           s += "Lamp is ";
           s += (vol)?"Mati":"Nyala";
           s += "</html>\n";
           client.print(s);    //this prob
           delay(1);}
           
  else if (req.indexOf("/mundur") != -1){
  val = 2;
           client.flush();
     String s = "HTTP/1.1 200 OK\r\n";
     s += "Content Type: text/html\r\n\r\n";
     s += "<!DOCTYPE HTML>\r\n<html>\r\n";
           s += "Lampu Sedang ";
           s += (vol)?"Mati":"Nyala";
           s += "</html>\n";
           client.print(s);    //this prob
           delay(1);}

  if(val==1) maju();
  if(val==2) mundur();
  if(val==0) stop_all_now();
  if ((val<=0) || (val>=3))
  stop_all_now();
  val=0;  

  if (req.indexOf("/lampu/0") != -1){
    vol = 0;
    ex = 1;
    digitalWrite(PIN1, vol);
    
    client.flush();
    String s = "HTTP/1.1 200 OK\r\n";
    s += "Content Type: text/html\r\n\r\n";
    s += "<!DOCTYPE HTML>\r\n<html>\r\n"; 
       if (ex>=1 && ex <=2) {
           s += "Lampu Sedang ";
           s += (vol)?"Mati":"Nyala";}
           s += "</html>\n";
           client.print(s);    //this prob
           delay(1);}
         
  else if (req.indexOf("/lampu/1") != -1){
     vol = 1;
     ex = 2;
     digitalWrite(PIN1, vol);
     
     client.flush();
     String s = "HTTP/1.1 200 OK\r\n";
     s += "Content Type: text/html\r\n\r\n";
     s += "<!DOCTYPE HTML>\r\n<html>\r\n";
        if (ex>=1 && ex <=2) {
           s += "Lampu Sedang ";
           s += (vol)?"Mati":"Nyala";}
           s += "</html>\n";
           client.print(s);    //this prob
           delay(1);}
    
  else if (req.indexOf("/kipas/0") != -1){
     vol = 0;
     ex = 3;
     digitalWrite(PIN2, vol);
  
   
     client.flush();
     String s = "HTTP/1.1 200 OK\r\n";
     s += "Content Type: text/html\r\n\r\n";
     s += "<!DOCTYPE HTML>\r\n<html>\r\n";
        if (ex>=3 && ex<=4) {
          s += "Kipas Sedang ";
          s += (vol)?"Mati":"Nyala";}
          s += "</html>\n";
          client.print(s);    //this prob
          delay(1);}
  
  else if (req.indexOf("/kipas/1") != -1){
     vol = 1;
     ex = 4;
     digitalWrite(PIN2, vol);
       
       
     client.flush();
     String s = "HTTP/1.1 200 OK\r\n";
     s += "Content Type: text/html\r\n\r\n";
     s += "<!DOCTYPE HTML>\r\n<html>\r\n";
         if (ex>=3 && ex<=4) {
           s += "Kipas Sedang ";
           s += (vol)?"Mati":"Nyala";}
           s += "</html>\n";
           client.print(s);    //this prob
           delay(1); }
    
    //pid
     else if (req.indexOf("/setpoint/0") != -1){
     vol = 0;
     ex = 5;
     digitalWrite(setpoint, vol);
           client.flush();
           String s = "HTTP/1.1 200 OK\r\n";
           s += "Content Type: text/html\r\n\r\n";
           s += "<!DOCTYPE HTML>\r\n<html>\r\n";
           if (ex>=5 && ex <=6) {
              s += "setpoint Sedang ";
              s += (vol)?"Mati":"Nyala";}
              s += "</html>\n";
           client.print(s);    //this prob
           delay(1);}
               
     else if (req.indexOf("/setpoint/1") != -1){
     vol = 1;
     ex = 6;
     digitalWrite(setpoint, vol);
           client.flush();
           String s = "HTTP/1.1 200 OK\r\n";
           s += "Content Type: text/html\r\n\r\n";
           s += "<!DOCTYPE HTML>\r\n<html>\r\n";
           if (ex>=5 && ex <=6) {
              s += "setpoint Sedang ";
              s += (vol)?"Mati":"Nyala";}
              s += "</html>\n";
           client.print(s);    //this prob
           delay(1);}
    //end pid
  

  else if (req.indexOf("/ldrandrain") != -1){
//          int ldrStatus = analogRead(ldrPin);         
          int rainStatus = digitalRead(D5);
          
//          if(ldrStatus <=300){
//           pfLDR="LOW"; 
//          }
            
//          else if(ldrStatus <= 600){
//           pfLDR="MEDIUM"; 
//          }
//          else if(ldrStatus >600){
//           pfLDR="HIGH";
//          }
          if(rainStatus > 0){
           pfRain = "NO RAIN";}
          else if(rainStatus < 1){
           pfRain = "RAIN";}
           
            StaticJsonBuffer<500> jsonBuffer;
            JsonObject& json = prepareResponse(jsonBuffer);
            writeResponse(client, json);}

            
  else if (req.indexOf("/temphum") != -1){
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connection: close"); 
    client.println("Refresh: 5");  
    client.println();
    client.println("<!DOCTYPE html>");
    client.println("<html xmlns='http://www.w3.org/1999/xhtml'>");
    client.println("<head>\n<meta charset='UTF-8'>");
    client.println("</head>\n<body>");
    client.println("<pre>");
    client.print("Humidity(%): ");
    client.println((float)humi, 2);
    client.print("Temp(°C)  : ");
    client.println((float)temp, 2);
    client.print("Temp(°F)  : ");
    client.println(Fahrenheit(temp), 2);
    client.print("Temp(°K)  : ");
    client.println(Kelvin(temp), 2);
    client.println("</pre>");
    client.print("</body>\n</html>");
    delay(DHT11_RETRY_DELAY); 

  }
   
}

void connectWiFi() {
  byte ledStatus = LOW;
  Serial.println();
  Serial.println("Connecting to: " + String(ssid));
  WiFi.mode(WIFI_STA);

  WiFi.begin(ssid,pass);

  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(PIN1, ledStatus);
    ledStatus = (ledStatus == LOW) ? LOW : HIGH;
    delay(100);
  }
  Serial.println("WiFi Connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void setupMDNS()
{  if (!MDNS.begin("thing")){
    Serial.println("Error setting up MDNS responder!");
    while(1) {
      delay(1000);}} 
    Serial.println("mDNS responder started");}

void writeResponse(WiFiClient& client, JsonObject& json) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: application/json");
  client.println("Connection: close");
  client.println("Refresh: 10");
  client.println();
  json.prettyPrintTo(client);}

void initHardWare()
{ Serial.begin(115200);
  pinMode(PIN1, OUTPUT);
  pinMode(PIN2, OUTPUT);
  pinMode(PID, OUTPUT);
  
}

void maju(void)
{ digitalWrite(D2, 255);
  digitalWrite(D3, 0);
  digitalWrite(D6, 255);
  digitalWrite(D7, 0);
  }
   
void mundur(void)
{ digitalWrite(D2, 0);
  digitalWrite(D3, 255);
  digitalWrite(D6, 0);
  digitalWrite(D7, 255);
  }

void move_maju(void)
{maju();}

void move_mundur(void)
{ mundur();}

void stop_all_now(void)
{ digitalWrite(D2, 0);
  digitalWrite(D3, 0);
  digitalWrite(D6, 0);
  digitalWrite(D7, 0);}

void email1(){
    Gsender *gsender = Gsender::Instance();    
    String subject = "RAIN";
    if (gsender->Subject(subject)->Send("kakaspensa@gmail.com", "Terjadi Hujan!!!")) {
        Serial.println("Message send.");} 
    else {
        Serial.print("Error sending message: ");
        Serial.println(gsender->getError());} }

void email2(){
    Gsender *gsender = Gsender::Instance();    
    String subject = "NO RAIN";
    if (gsender->Subject(subject)->Send("kakaspensa@gmail.com", "Alhamdulillah Reda")) {
        Serial.println("Message send.");} 
    else {
        Serial.print("Error sending message: ");
        Serial.println(gsender->getError());} }
