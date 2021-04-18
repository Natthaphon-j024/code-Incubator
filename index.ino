#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

int sensorPin = A0;
int sensorValue = 0; 
#include "DHT.h" // if this show error is will be from can't get filename form work location.
#define DHTPIN D1 
#define DHTTYPE DHT11 // DHT 11
#define RelayPin1 14 //D5
#define RelayPin2 12 //D6
#define SwitchPin1 4 //D2  
#define SwitchPin2 0 //D3   
#define VPIN_BUTTON_1    V1 
#define VPIN_BUTTON_2    V2
#define VPIN_TIME       V10
int toggleState_1 = 1; //Define integer to remember the toggle state for relay 1
int toggleState_2 = 1; //Define integer to remember the toggle state for relay 2
int wifiFlag = 0;
int pinValue = 0;
int Time01 = 0;
 
#define AUTH "AUTH TOKEN"  

#include <TridentTD_LineNotify.h> // if this show error is will be from can't get filename form work location.
//#define DHTTYPE DHT22 // DHT 22 (AM2302), AM2321
//#define DHTTYPE DHT21 // DHT 21 (AM2301)

#define SSID "{WIFI SSID}" // แก้ ชื่อ ssid ของ wifi 
#define PASSWORD "{WIFI PASSWORD}" // แก้ รหัสผ่าน wifi
#define LINE_TOKEN "{LINETOKEN}" // แก้ Line Token
char auth[] = "{BLYNK TOKEN}";
char ssid[] = SSID;
char pass[] = PASSWORD;
BlynkTimer timer;
DHT dht(DHTPIN, DHTTYPE);
void relayOnOff(int relay){
  switch(relay){
    case 1: 
      if(toggleState_1 == 1){
        digitalWrite(RelayPin1, LOW); // turn on relay 1
        toggleState_1 = 0;
        Serial.println("Device1 ON");
      }
      else{
        digitalWrite(RelayPin1, HIGH); // turn off relay 1
        toggleState_1 = 1;
        Serial.println("Device1 OFF");
      }
      delay(100);
    break;
    case 2: 
      if(toggleState_2 == 1){
        digitalWrite(RelayPin2, LOW); // turn on relay 2
        toggleState_2 = 0;
        Serial.println("Device2 ON");
      }
      else{
        digitalWrite(RelayPin2, HIGH); // turn off relay 2
        toggleState_2 = 1;
        Serial.println("Device2 OFF");
      }
      delay(100);
    break;
    default : break; 
  }
}

void with_internet(){
  //Manual Switch Control
  if (digitalRead(SwitchPin1) == LOW){
    delay(200);
    relayOnOff(1); 
    Blynk.virtualWrite(VPIN_BUTTON_1, toggleState_1);   // Update Button Widget  
  }
  else if (digitalRead(SwitchPin2) == LOW){
    delay(200);
    relayOnOff(2);      
    Blynk.virtualWrite(VPIN_BUTTON_2, toggleState_2);   // Update Button Widget
  }
}
void without_internet(){
  //Manual Switch Control
  if (digitalRead(SwitchPin1) == LOW){
    delay(200);
    relayOnOff(1);      
  }
  else if (digitalRead(SwitchPin2) == LOW){
    delay(200);
    relayOnOff(2);
  }
}

// When App button is pushed - switch the state
BLYNK_WRITE(VPIN_BUTTON_1) {
  toggleState_1 = param.asInt();
  digitalWrite(RelayPin1, toggleState_1);
}
BLYNK_WRITE(VPIN_BUTTON_2) {
  toggleState_2 = param.asInt();
  digitalWrite(RelayPin2, toggleState_2);
}

void checkBlynkStatus() { // called every 3 seconds by SimpleTimer

  bool isconnected = Blynk.connected();
  if (isconnected == false) {
    wifiFlag = 1;
  }
  if (isconnected == true) {
    wifiFlag = 0;
    Blynk.virtualWrite(VPIN_BUTTON_1, toggleState_1);
    Blynk.virtualWrite(VPIN_BUTTON_2, toggleState_2);
    
  }
}


float oldT = 0; 
float oldH = 0; 



void setup() {
  Blynk.begin(auth, ssid, pass);
  Serial.begin(9600);
  dht.begin();
  Serial.println();
  Serial.println(LINE.getVersion());
  WiFi.begin(SSID, PASSWORD);
  Serial.printf("WiFi connecting to %s\n", SSID);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(400);
  }
  pinMode(RelayPin1, OUTPUT);
  pinMode(RelayPin2, OUTPUT);
  pinMode(SwitchPin1, INPUT_PULLUP);
  pinMode(SwitchPin2, INPUT_PULLUP);

  WiFi.begin(SSID, PASSWORD);
  timer.setInterval(3000L, checkBlynkStatus); // check if Blynk server is connected every 3 seconds
  Blynk.config(AUTH);
  delay(1000);

  Blynk.virtualWrite(VPIN_BUTTON_1, toggleState_1);
  Blynk.virtualWrite(VPIN_BUTTON_2, toggleState_2);


  Serial.printf("\nWiFi connected\nIP : ");
  Serial.println(WiFi.localIP());
  LINE.setToken(LINE_TOKEN);
  LINE.notify("เซนเซอร์วัดความชื้นและอุณหภูมิ เริ่มทำงานแล้ว");
}

void loop() {
  dht_line(); //สำหรับแจ้งเตือนผ่าน line 
  blynk();  // blynk butto digital to analong
  blynkinput();  // reed value to blynk
  KY037();  // reed value status line
  mainTime(); //reed  fom blynk status to line

}

void dht_line() {
  float h = dht.readHumidity(); // ความชื้น
  float t = dht.readTemperature(); // อุณหภูมิ
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
  }  
  if (oldT != t ) {
    if ( t >= 39 ) {
      LINE.notify((String)t+"C อุณหภูมิเกินกำหนด");
      Serial.println("อุณหภูมิเกินกำหนด");
    }else if (t <= 37) {
      LINE.notify((String)t+"C อุณหภูมิต่ำกำหนด");
      Serial.println("อุณหภูมิต่ำกำหนด");
    }
    oldT = t ;
  }
  if (oldH != h ) {
    if ( h >= 70 ) {
      LINE.notify((String)t+"% ความชื้นสูงเกิน");
      Serial.println("ความชื้นสูงเกิน");
    }else if ( h <= 50) {
      LINE.notify((String)t+"% ความชื้นต่ำเกิน");
      Serial.println("ความชื้นต่ำเกิน");
    }
    oldH = h ;
  }
}

void blynk(){
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("WiFi Not Connected");
  } else {
    Blynk.run();
  }
  timer.run(); // Initiates SimpleTimer
  if (wifiFlag == 0) {
    with_internet();
  } else {
    without_internet();
  }
}

void blynkinput(){
  float h = dht.readHumidity(); // ความชื้น
  float t = dht.readTemperature(); // อุณหภูมิ 
  Blynk.run();
  delay(100);
  Blynk.virtualWrite(V5, h);
  Blynk.virtualWrite(V6, t);
}
// ใช้สำอ่านค่า KY037
void KY037(){
  sensorValue = analogRead(sensorPin);
  Blynk.virtualWrite(V9, sensorValue);
  if (sensorValue >= 155)
  {
    LINE.notify("มีสมาชิกใหม่  ํ_ ํ "); 
  }
}

void mainTime(){
  if (Time01 == 2) {
    Serial.println((String)"Case Time01 = 2 :: "+pinValue);
    LINE.notify("Timer is stop.");
    Time01 = 0;
  } else if ( Time01 == 1) {
    Serial.println((String)"Case Timer01 = 1 :: "+pinValue);
    LINE.notify("Timer is start.");
    Time01 = 0;
  }
}

BLYNK_WRITE(VPIN_TIME) { //in case of button time is clicked
  pinValue = param.asInt(); // assigning incoming value from pin V1 to a variable
  Serial.println((String)"V10 Slider value is: "+pinValue);
  if (pinValue == 0)
  {
    Time01 = 2 ; 
  } else if (pinValue == 1) {
    Time01 = 1 ;
  }
}
