#include <Arduino.h>
#include <Servo.h>                // 리니어 액추에이터
#include <IO7F8266.h>
#define PIN_SERVO 12


Servo myServo;

int Dir1Pin_A = 12;      // 제어신호 1핀
int Dir2Pin_A = 13;      // 제어신호 2핀
int d = 1;
int delayMS = 50;

String user_html = "";

char* ssid_pfix = (char*)"Donghyun_actu";
unsigned long lastPublishMillis = -pubInterval;
int Actuator_Status = 0;

// put function declarations here:
void SetStrokePerc(float);
void SetStrokeMM(int,int);
void Actuator_UP();
void Acutator_DOWN();
void publishData();
void handleUserCommand(char*, JsonDocument*);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  initDevice();
  JsonObject meta = cfg["meta"];
  pubInterval = meta.containsKey("pubInterval") ? meta["pubInterval"] : 0;
  lastPublishMillis = -pubInterval;

  WiFi.mode(WIFI_STA);
  WiFi.begin((const char*)cfg["ssid"], (const char*)cfg["w_pw"]);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }

  Serial.printf("\nIP address : ");
  Serial.println(WiFi.localIP());

  userCommand = handleUserCommand;
  set_iot_server();
  iot_connect();

  ///////////////////////////////////////////
  //               액추에이터                 //
  ///////////////////////////////////////////  
  myServo.attach(PIN_SERVO);
}

void loop() {
  //Servo myServo; 
  if (!client.connected()) {
        iot_connect();
    }

    client.loop();
    if ((pubInterval != 0) && (millis() - lastPublishMillis > pubInterval)) {
        publishData();
        lastPublishMillis = millis();
    }

  // Actuator_UP();
  // Acutator_DOWN();
  // delay(1000);
}

void Actuator_UP(){
  Actuator_Status = 1;
  for ( int i = 1; i <90; i += d ){ 
    SetStrokePerc(i);
    Serial.println(i);
    delay(delayMS);
  }
}

void Acutator_DOWN(){
  Actuator_Status = 0;
  for ( int i = 90; i > 1;  i -= d ){
    SetStrokePerc(i);
    Serial.println(i);
    delay(delayMS);
  }
}


void SetStrokePerc(float strokePercentage)
{
  if ( strokePercentage >= 1.0 && strokePercentage <= 99.0 )
  {
    int usec = 1000 + strokePercentage * ( 2000 - 1000 ) / 100.0 ;
    myServo.writeMicroseconds( usec );
  }
}

void SetStrokeMM(int strokeReq,int strokeMax)
{
  SetStrokePerc( ((float)strokeReq) / strokeMax );
} 

void publishData() {
    StaticJsonDocument<512> root;
    JsonObject data = root.createNestedObject("d");

    data["actuator"] = Actuator_Status == 1 ? "up" : "down";

    serializeJson(root, msgBuffer);
    client.publish(evtTopic, msgBuffer);
}

void handleUserCommand(char* topic, JsonDocument* root) {
    JsonObject d = (*root)["d"];

    if (d.containsKey("actuator")) {
        if (strstr(d["actuator"], "up")) {
            Actuator_UP();
        } else {
            Acutator_DOWN();
        }
        lastPublishMillis = -pubInterval;
    }
}