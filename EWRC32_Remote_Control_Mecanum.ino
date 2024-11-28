/******************* WiFi Robot Remote Control Mode ********************/
#include <WiFi.h>
//#include <ArduinoOTA.h>
#include <ESPAsyncWebServer.h>
#include <Ps3Controller.h>


// connections for drive Motors
int PWM_FR = 12;
int PWM_FL = 13;
int DIR_FR = 14;
int DIR_FL = 15;

int PWM_BR = 16;
int PWM_BL = 17;
int DIR_BR = 18;
int DIR_BL = 19;

const int buzPin = 5;      // set digital pin D5 as buzzer pin (use active buzzer)
const int ledPin = 4;      // set digital pin D8 as LED pin (use super bright LED)
const int wifiLedPin = 2;  // set digital pin D0 as indication, the LED turn on if NodeMCU connected to WiFi as STA mode

bool ledStatus = false;

String command;          // String to store app command state.
int SPEED = 1023;        // 330 - 1023.
int speed_Coeff = 5;

AsyncWebServer server(80);


void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}


unsigned long previousMillis = 0;

//String sta_ssid = "$your_ssid_maximum_32_characters";      // set Wifi networks you want to connect to
//String sta_password = "$your_pswd_maximum_32_characters";  // set password for Wifi networks

String sta_ssid = "";      // set Wifi networks you want to connect to
String sta_password = "";  // set password for Wifi networks


void setup(){
  Serial.begin(115200);    // set up Serial library at 115200 bps
  Serial.println();
  Serial.println("*ESP32 WiFi Robot Remote Control Mode*");
  Serial.println("--------------------------------------");
 
  pinMode(buzPin, OUTPUT);      // sets the buzzer pin as an Output
  pinMode(ledPin, OUTPUT);      // sets the LED pin as an Output
  pinMode(wifiLedPin, OUTPUT);  // sets the Wifi LED pin as an Output
  digitalWrite(buzPin, LOW);
  digitalWrite(ledPin, LOW);
  digitalWrite(wifiLedPin, HIGH);
  digitalWrite(wifiLedPin, HIGH);
    
  // Set all the motor control pins to outputs
  pinMode(PWM_FR, OUTPUT);
  pinMode(PWM_FL, OUTPUT);
  pinMode(DIR_FR, OUTPUT);
  pinMode(DIR_FL, OUTPUT);
  
  pinMode(PWM_BR, OUTPUT);
  pinMode(PWM_BL, OUTPUT);
  pinMode(DIR_BR, OUTPUT);
  pinMode(DIR_BL, OUTPUT);
  
  // Turn off motors - Initial state
  digitalWrite(DIR_FR, LOW);
  digitalWrite(DIR_FL, LOW);
  digitalWrite(DIR_BR, LOW);
  digitalWrite(DIR_BL, LOW);

  Ps3.attach(notify);
  Ps3.attachOnConnect(onConnect);
  Ps3.attachOnDisconnect(onDisConnect);
  Ps3.begin();
  String address = Ps3.getAddress();
  Serial.print("The ESP32's Bluetooth MAC address is: ");
  Serial.println(address);

//  ledcSetup(0, 1000 , 8);
//  ledcAttachPin(PWM_FR, 0);
//  ledcSetup(1, 1000 , 8);
//  ledcAttachPin(PWM_FL, 1);
//  
//  ledcSetup(2, 1000 , 8);
//  ledcAttachPin(PWM_BR, 2);
//  ledcSetup(3, 1000 , 8);
//  ledcAttachPin(PWM_BL, 3);

  // set ESP32 Wifi hostname based on chip mac address
  char chip_id[15];
  snprintf(chip_id, 15, "%04X", (uint16_t)(ESP.getEfuseMac()>>32));
  String hostname = "esp32Car-" + String(chip_id);

  Serial.println();
  Serial.println("Hostname: "+hostname);

  // first, set NodeMCU as STA mode to connect with a Wifi network
  WiFi.mode(WIFI_STA);
  WiFi.begin(sta_ssid.c_str(), sta_password.c_str());
  Serial.println("");
  Serial.print("Connecting to: ");
  Serial.println(sta_ssid);
  Serial.print("Password: ");
  Serial.println(sta_password);

  // try to connect with Wifi network about 10 seconds
  unsigned long currentMillis = millis();
  previousMillis = currentMillis;
  while (WiFi.status() != WL_CONNECTED && currentMillis - previousMillis <= 10000) {
    delay(500);
    Serial.print(".");
    currentMillis = millis();
  }

  // if failed to connect with Wifi network set NodeMCU as AP mode
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("*WiFi-STA-Mode*");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    digitalWrite(wifiLedPin, HIGH);    // Wifi LED on when connected to Wifi as STA mode
    delay(2000);
  } else {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(hostname.c_str());
    IPAddress myIP = WiFi.softAPIP();
    Serial.println("");
    Serial.println("WiFi failed connected to " + sta_ssid);
    Serial.println("");
    Serial.println("*WiFi-AP-Mode*");
    Serial.print("AP IP address: ");
    Serial.println(myIP);
    digitalWrite(wifiLedPin, LOW);   // Wifi LED off when status as AP mode
    delay(2000);
  }
 

  // Send a GET request to <ESP_IP>/?fader=<inputValue>
    server.on("/", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputValue;
    String inputMessage;

    // Get value for Forward/Backward
    if (request->hasParam("State")) {
      inputValue = request->getParam("State")->value();
      
      if (inputValue.equals("F")) Forward();          // check string then call a function or set a value
      else if (inputValue.equals("B")) Backward();
      else if (inputValue.equals("R")) TurnRight();
      else if (inputValue.equals("L")) TurnLeft();
      else if (inputValue.equals("G")) ForwardLeft();
      else if (inputValue.equals("H")) BackwardLeft();
      else if (inputValue.equals("I")) ForwardRight();
      else if (inputValue.equals("J")) BackwardRight();
      else if (inputValue.equals("S")) Stop();
      else if (inputValue.equals("V")) BeepHorn();
      else if (inputValue.equals("W")) TurnLightOn();
      else if (inputValue.equals("w")) TurnLightOff();
      else if (inputValue.equals("0")) SPEED = 330;
      else if (inputValue.equals("1")) SPEED = 400;
      else if (inputValue.equals("2")) SPEED = 470;
      else if (inputValue.equals("3")) SPEED = 540;
      else if (inputValue.equals("4")) SPEED = 610;
      else if (inputValue.equals("5")) SPEED = 680;
      else if (inputValue.equals("6")) SPEED = 750;
      else if (inputValue.equals("7")) SPEED = 820;
      else if (inputValue.equals("8")) SPEED = 890;
      else if (inputValue.equals("9")) SPEED = 960;
      else if (inputValue.equals("q")) SPEED = 1023;
      else inputValue = "No message sent";
    }
     
    Serial.println(inputValue);
    inputValue="";
    request->send(200, "text/text", "");
  });

  server.onNotFound (notFound);    // when a client requests an unknown URI (i.e. something other than "/"), call function "handleNotFound"
  server.begin();  

  
//  ArduinoOTA.begin();                       // enable to receive update/uploade firmware via Wifi OTA
}

void notify()
{
  int lxAxisValue =(Ps3.data.analog.stick.lx);  //Left stick  - x axis - turnright/turnleft car movement
  int lyAxisValue =(Ps3.data.analog.stick.ly);  //Left stick  - y axis - forward/backward car movement
  int rxAxisValue =(Ps3.data.analog.stick.rx);  //Right stick - x axis - left/right car movement

  if (lyAxisValue <= -50){       //Move car Forward
    Forward();
  }
  else if (lyAxisValue >= 50){   //Move car Backward
    Backward();
  }
  else if (lxAxisValue >= 50){   //Move car ForwardRight
    ForwardRight();
  }
  else if (lxAxisValue <= -50){   //Move car ForwardLeft
    ForwardLeft();
  }
  else if (rxAxisValue >= 50){  //Move car Right
    TurnRight();
  }
  else if (rxAxisValue <= -50){   //Move car Left
    TurnLeft();
  }
  else {
    Stop();
  } 

  if(Ps3.event.button_down.cross){
    digitalWrite(buzPin,HIGH);
    delay(10);
  } else if (Ps3.event.button_up.cross){
    digitalWrite(buzPin,LOW);
  }

  if(Ps3.event.button_down.circle){
    ledStatus=!ledStatus;
    digitalWrite(ledPin,ledStatus);
  }
  
}

void onConnect(){
  Serial.println("Connected!.");
}

void onDisConnect(){
  
}

void loop() {
//    ArduinoOTA.handle();          // listen for update OTA request from clients
}


// function to move forward
void Forward(){ 
  digitalWrite(DIR_FR, HIGH);
  digitalWrite(DIR_FL, HIGH);
  digitalWrite(DIR_BR, HIGH);
  digitalWrite(DIR_BL, HIGH);
  digitalWrite(PWM_FR, HIGH);
  digitalWrite(PWM_FL, HIGH);
  digitalWrite(PWM_BR, HIGH);
  digitalWrite(PWM_BL, HIGH);
}

// function to move backward
void Backward(){
  digitalWrite(DIR_FR, LOW);
  digitalWrite(DIR_FL, LOW);
  digitalWrite(DIR_BR, LOW);
  digitalWrite(DIR_BL, LOW);
  digitalWrite(PWM_FR, HIGH);
  digitalWrite(PWM_FL, HIGH);
  digitalWrite(PWM_BR, HIGH);
  digitalWrite(PWM_BL, HIGH);
}

// function to turn right
void TurnRight(){
  digitalWrite(DIR_FR, LOW);
  digitalWrite(DIR_FL, HIGH);
  digitalWrite(DIR_BR, HIGH);
  digitalWrite(DIR_BL, LOW);
  digitalWrite(PWM_FR, HIGH);
  digitalWrite(PWM_FL, HIGH);
  digitalWrite(PWM_BR, HIGH);
  digitalWrite(PWM_BL, HIGH);
}

// function to turn left
void TurnLeft(){
  digitalWrite(DIR_FR, HIGH);
  digitalWrite(DIR_FL, LOW);
  digitalWrite(DIR_BR, LOW);
  digitalWrite(DIR_BL, HIGH);
  digitalWrite(PWM_FR, HIGH);
  digitalWrite(PWM_FL, HIGH);
  digitalWrite(PWM_BR, HIGH);
  digitalWrite(PWM_BL, HIGH);
}

// function to move forward left
void ForwardLeft(){
  digitalWrite(DIR_FR, HIGH);
  digitalWrite(DIR_FL, LOW);
  digitalWrite(DIR_BR, HIGH);
  digitalWrite(DIR_BL, LOW);
  digitalWrite(PWM_FR, HIGH);
  digitalWrite(PWM_FL, HIGH);
  digitalWrite(PWM_BR, HIGH);
  digitalWrite(PWM_BL, HIGH);
}

// function to move backward left
void BackwardLeft(){
  digitalWrite(DIR_FR, LOW);
  digitalWrite(DIR_FL, HIGH);
  digitalWrite(DIR_BR, LOW);
  digitalWrite(DIR_BL, HIGH);
  digitalWrite(PWM_FR, HIGH);
  digitalWrite(PWM_FL, HIGH);
  digitalWrite(PWM_BR, HIGH);
  digitalWrite(PWM_BL, HIGH);
}

// function to move forward right
void ForwardRight(){
  digitalWrite(DIR_FR, LOW);
  digitalWrite(DIR_FL, HIGH);
  digitalWrite(DIR_BR, LOW);
  digitalWrite(DIR_BL, HIGH);
  digitalWrite(PWM_FR, HIGH);
  digitalWrite(PWM_FL, HIGH);
  digitalWrite(PWM_BR, HIGH);
  digitalWrite(PWM_BL, HIGH);
}

// function to move backward left
void BackwardRight(){ 
  digitalWrite(DIR_FR, HIGH);
  digitalWrite(DIR_FL, LOW);
  digitalWrite(DIR_BR, HIGH);
  digitalWrite(DIR_BL, LOW);
  digitalWrite(PWM_FR, HIGH);
  digitalWrite(PWM_FL, HIGH);
  digitalWrite(PWM_BR, HIGH);
  digitalWrite(PWM_BL, HIGH);
}

// function to stop motors
void Stop(){  
  digitalWrite(DIR_FR, LOW);
  digitalWrite(DIR_FL, LOW);
  digitalWrite(DIR_BR, LOW);
  digitalWrite(DIR_BL, LOW);
  digitalWrite(PWM_FR, LOW);
  digitalWrite(PWM_FL, LOW);
  digitalWrite(PWM_BR, LOW);
  digitalWrite(PWM_BL, LOW);
}

// function to beep a buzzer
void BeepHorn(){
  digitalWrite(buzPin, HIGH);
  delay(150);
  digitalWrite(buzPin, LOW);
  delay(80);
  digitalWrite(buzPin, HIGH);
  delay(150);
  digitalWrite(buzPin, LOW);
  delay(80);
}

// function to turn on LED
void TurnLightOn(){
  digitalWrite(ledPin, HIGH);
}

// function to turn off LED
void TurnLightOff(){
  digitalWrite(ledPin, LOW);
}
