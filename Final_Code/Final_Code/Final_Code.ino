//Thu vien can thiet
#define BLYNK_FIRMWARE_VERSION        "0.1.0"
#define BLYNK_PRINT Serial
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
BlynkTimer timer;
#include "DHT.h"

//Khai bao wifi, Autoken va api thingspeak
String apiKey = "FQN6UN7C7PXINURH";
char auth[] = "_vptzflsV0AZyUF_vZV51Xa3Kz6vTJcZ";//Enter your Auth token
char ssid[] = "Tung Pham";//Enter your WIFI name
char pass[] = "tungvukhanh";//Enter your WIFI password
const char *server = "api.thingspeak.com";
char status;
WiFiClient client;

//LCD
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2); //D1 va D2

//Cam bien DHT22
#include <DHT.h>
#define DHTPIN D3
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

//Cam bien do am dat
int moisture = A0;

//cam bien anh sang LDR
#define light D4

//cam bien mua Rain
#define Rain D7

//Khai bao bien toan cuc cho cam bien DHT22 va Moisture
float h; 
float t; 
int value; 
 
//Define the buttons pins
#define button_Automanual 12 //D6
//Define relays pin
#define relay_den 15 //D8
#define relay_bom 16 //D0
#define relay_quat 14 //D5

//tao gia tri lap dieu kien
int mucCanhbao = 0; //Moisture
int mucHoatdong = 0; //DHT22
int relay1_state = 0;
boolean runMode = 0;//Bật/tắt chế độ cảnh báo
boolean canhbaoState=0;
boolean buttonState = HIGH;

BLYNK_CONNECTED() {
  Blynk.syncVirtual(V1); //relay den
  Blynk.syncVirtual(V2); //relay may bom
  Blynk.syncVirtual(V3); //nhiet do
  Blynk.syncVirtual(V4); //do am
  Blynk.syncVirtual(V5); //Moisture
  Blynk.syncVirtual(V6); //LDR
  Blynk.syncVirtual(V7); //Rain
  Blynk.syncVirtual(V8); // led canh bao
  Blynk.syncVirtual(V9); //nut nhan runmode
  Blynk.syncVirtual(V10); //muc canh bao
  Blynk.syncVirtual(V11); //Relay Quat
  Blynk.syncVirtual(V12); // led canh bao cam bien nhiet do do am
  Blynk.syncVirtual(V13); //muc Hoat Dong
}

BLYNK_WRITE(V1) {
  bool value1 = param.asInt();
  // Check these values and turn the relay2 ON and OFF
  if (value1 == 1) {
    digitalWrite(relay_den, LOW);
    Serial.println("Den ON");
  } else {
    digitalWrite(relay_den, HIGH);
    Serial.println("Den OFF");
  }
}

BLYNK_WRITE(V2) {
  bool value2 = param.asInt();
  // Check these values and turn the relay2 ON and OFF
  if (value2 == 1) {
    digitalWrite(relay_bom, LOW);
    Serial.println("Bom ON");
  } else {
    digitalWrite(relay_bom, HIGH);
    Serial.println("Bom OFF");
  }
}

BLYNK_WRITE(V9) {
  runMode = param.asInt();
}

BLYNK_WRITE(V10) {
  mucCanhbao = param.asInt();
}

BLYNK_WRITE(V11) {
  bool value3 = param.asInt();
  // Check these values and turn the relay2 ON and OFF
  if (value3 == 1) {
    digitalWrite(relay_quat, LOW);
    Serial.println("Quat ON");
  } else {
    digitalWrite(relay_quat, HIGH);
    Serial.println("Quat OFF");
  }
}

BLYNK_WRITE(V13) {
  mucHoatdong = param.asInt();
}

void setup() {
  //Hien thi LCD
  lcd.init();
  lcd.backlight(); 
  lcd.setCursor(3, 0);
  lcd.print("IOT Graden");
  lcd.setCursor(4, 1);
  lcd.print("Project");
  delay(2000);
  lcd.clear();

  //Kiem tra Wifi
  Serial.begin(115200);
  delay(10);
  Wire.begin();
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  //Chay Blynk va cau hinh ngat timer ID1
  Blynk.begin(auth, ssid, pass, "blynk.cloud", 80);
  timer.setInterval(1000L,handleTimerID1);
  
  //Chay DHT22
  dht.begin();
  
  //Set the Button la chan INPUT
  pinMode(button_Automanual, INPUT_PULLUP);

  //Set the relay pins la chan OUTPUT 
  pinMode(relay_den, OUTPUT);
  pinMode(relay_bom, OUTPUT);
  pinMode(relay_quat, OUTPUT);

  // Turn OFF the relay
  digitalWrite(relay_den, HIGH);
  digitalWrite(relay_bom, HIGH);
  digitalWrite(relay_quat, HIGH);
  
  //Cho Blynk Tat Relay
  Blynk.virtualWrite(V1, LOW);
  Blynk.virtualWrite(V2, LOW);
  Blynk.virtualWrite(V11, LOW);
  
  //input LDR va Rain sensor
  pinMode(light, INPUT);
  pinMode(Rain , INPUT);
  
  //Call the functions - Goi gia tri cua tung cam bien trong ham phia duoi
  timer.setInterval(100L, DHT22sensor);
  timer.setInterval(100L, MoistureSensor);
  timer.setInterval(100L, LDRsensor);
  timer.setInterval(100L, Rainsensor);
}

void manual_auto() 
{
    if(digitalRead(button_Automanual)==LOW)// thả nút nhấn
    {
      if(buttonState == HIGH)
      {
        buttonState = LOW;
        runMode =!runMode;
        Serial.println("Run mode: " + String(runMode));
        Blynk.virtualWrite(V9,runMode);
        delay(200);
      }
    }
    else
    {
      buttonState = HIGH;
    }
}

void MoistureSensor() 
{
  value = analogRead(moisture);
  value = map(value, 0, 1024, 100, 0);
  Blynk.virtualWrite(V5, value);

  lcd.setCursor(0, 1);
  lcd.print("M:");
  lcd.print(value);
  lcd.print(" ");
}

void DHT22sensor()
{
  h = dht.readHumidity();
  t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  Blynk.virtualWrite(V3, t);
  Blynk.virtualWrite(V4, h);
  if(runMode == 1)
  {
    if(t < mucHoatdong)
    {
      Blynk.logEvent("thong_tin_dht22", String("Cảnh báo! Cảm biến DHT22 có nhiệt độ = " + String(t)+" vượt quá mức cho phép!"));
      digitalWrite(relay_quat,HIGH);
      Blynk.virtualWrite(V12,HIGH);
      Blynk.virtualWrite(V11,LOW);
      Serial.println("Đã bật cảnh báo!");
      lcd.setCursor(0, 0);
      lcd.print("T:");
      lcd.print(t);
    
      lcd.setCursor(8, 0);
      lcd.print("H:");
      lcd.print(h);
    }
    else
    {
      digitalWrite(relay_quat,LOW);
      Blynk.virtualWrite(V12,LOW);
      Blynk.virtualWrite(V11,HIGH);
      Serial.println("Đã tắt cảnh báo!");
      lcd.setCursor(0, 0);
      lcd.print("T:");
      lcd.print(t);
    
      lcd.setCursor(8, 0);
      lcd.print("H:");
      lcd.print(h);
     }
  }
  else
  {
      Blynk.virtualWrite(V12,LOW);
      Serial.println("Đã tắt cảnh báo!");
      lcd.setCursor(0, 0);
      lcd.print("T:");
      lcd.print(t);
    
      lcd.setCursor(8, 0);
      lcd.print("H:");
      lcd.print(h);
  }
}

void Rainsensor() 
{
  bool value3 = digitalRead(Rain);
  if (value3 == 0) 
  {
    WidgetLED LED(V7);
    LED.on();
    lcd.setCursor(10, 1);
    lcd.print("R:");
    lcd.print(value3);
    lcd.print(" ");
    Blynk.logEvent("thong_tin_mua", String("Trời đã mưa! Giá trị cảm biến = " + String(value3)+" "));
  }
  else
  {
    WidgetLED LED(V7);
    LED.off();
    lcd.setCursor(10, 1);
    lcd.print("R:");
    lcd.print(value3);
    lcd.print(" ");
  }
}

void LDRsensor() 
{
  bool value2 = digitalRead(light);
  if(runMode == 1)
  {
    if (value2 == 0) 
    {
      WidgetLED LED(V6);
      LED.on();
      lcd.setCursor(6, 1);
      lcd.print("L:");
      lcd.print(value2);
      lcd.print(" ");
      digitalWrite(relay_den,HIGH);
      Blynk.virtualWrite(V1, LOW); //update button state
    }
    else
    {
      WidgetLED LED(V6);
      LED.off();
      lcd.setCursor(6, 1);
      lcd.print("L:");
      lcd.print(value2);
      lcd.print(" ");
      digitalWrite(relay_den,LOW);
      Blynk.logEvent("thong_tin", String("Trời đã tối! Giá trị cảm biến = " + String(value2)+" nên sẽ bật đèn!"));
      Blynk.virtualWrite(V1, HIGH); //update button state
    }
  }
  else
  {
    if (value2 == 0) 
    {
      WidgetLED LED(V6);
      LED.on();
      lcd.setCursor(6, 1);
      lcd.print("L:");
      lcd.print(value2);
      lcd.print(" ");
    }
    else
    {
      WidgetLED LED(V6);
      LED.off();
      lcd.setCursor(6, 1);
      lcd.print("L:");
      lcd.print(value2);
      lcd.print(" ");
    }
  }
}

void handleTimerID1()
{
  int value = analogRead(moisture);
  value = map(value, 0, 1024, 100, 0);
  
  Blynk.virtualWrite(V5,value);
  if(runMode==1)
  {
    if(value<mucCanhbao)
    {
      if(canhbaoState==0)
      {
        canhbaoState = 1;
        Blynk.logEvent("canhbao", String("Cảnh báo! Cam bien Do Am Dat =" + String(value)+" bé hơn mức cho phép!"));
        timer.setTimeout(60000L,handleTimerID2);
      }
      digitalWrite(relay_bom,LOW);
      Blynk.virtualWrite(V8,HIGH);
      Blynk.virtualWrite(V2,HIGH);
      Serial.println("Đã bật cảnh báo!");
    }
    else
    {
      digitalWrite(relay_bom,HIGH);
      Blynk.virtualWrite(V8,LOW);
      Blynk.virtualWrite(V2,LOW);
      Serial.println("Đã tắt cảnh báo!");
    }
  }
  else
  {
    Blynk.virtualWrite(V8,LOW);
    Serial.println("Đã tắt cảnh báo!");
  }
}

void handleTimerID2(){
  canhbaoState=0;
}

void loop() {
  Blynk.run();//Run the Blynk library
  timer.run();//Run the Blynk timer
  manual_auto();

  DHT22sensor();
  MoistureSensor();
  
  if (client.connect(server, 80)) 
  {
    String postStr = apiKey;
    postStr += "&field1=";
    postStr += String(t);
    postStr += "&field2=";
    postStr += String(h);
    postStr += "&field3=";
    postStr += String(value);
    postStr += "\r\n\r\n\r\n\r\n";

    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: " + apiKey + "\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(postStr.length());
    client.print("\n\n\n\n");
    client.print(postStr);

    Serial.print("Temperature: ");
    Serial.println(t);
    Serial.print("Humidity: ");
    Serial.println(h);
    Serial.print("Rain: ");
    Serial.println(value);

  }
  client.stop();
  delay(500);
}


