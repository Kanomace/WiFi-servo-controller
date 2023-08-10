#include <WiFi.h>
#include <PubSubClient.h>
//声明一个定时器
hw_timer_t * timer = NULL;    


// WiFi
const char *ssid = "huangjiacheng"; // Enter your WiFi name
const char *password = "88888888";  // Enter WiFi password

// MQTT Broker
const char *mqtt_broker = "192.168.202.86";
const char *mqtt_username = "admin";
const char *mqtt_password = "public";
const int mqtt_port = 1883;
const char *topic1 = "001/DoorControl";
const char *topic2 = "002/TimerSetting";

WiFiClient espClient;
PubSubClient client(espClient);

void  IRAM_ATTR onTimer();
void  connectMQTT();
void  connectWiFi();

//----------------init function---------------
void setup() {
// Set software serial baud to 115200;
  Serial.begin(115200);

  timer = timerBegin(0, 80, true);                //初始化
  timerAttachInterrupt(timer, &onTimer, true);    //调用中断函数
  timerAlarmWrite(timer, 1000000, true);        //timerBegin的参数二 80位80MHZ，这里为1000000  意思为1秒

  //timerDetachInterrupt(timer);              //关闭定时器
  
////------connecting to a WiFi network --------
 connectWiFi();
//-----connecting to a mqtt broker----------
 client.setServer(mqtt_broker, mqtt_port);
 client.setCallback(callback);
 connectMQTT();

 
 // publish and subscribe
 client.publish(topic1, "Hi EMQX I'm ESP32 ^^");
 client.subscribe(topic1);
 client.publish(topic2, "Hi EMQX I'm ESP32 ^^");
 client.subscribe(topic2);
 
 timerAlarmEnable(timer);                     //定时器使能
}


//----------------loop function---------------
void loop() {

 client.loop();
// delay(100);
}

//-------------callback function--------------
void callback(char *topic, byte *payload, unsigned int length) {
 Serial.print("Message arrived in topic: ");
 Serial.println(topic);
 String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
//--------topic1---------
 if (strcmp(topic, "001/DoorControl") == 0) {
    // 处理 topic1 的逻辑
    Serial.println("Received message");
    Serial.println(message);
    // TODO: 在这里添加 topic1 的逻辑处理代码
  }

 //--------topic2---------
  else if (strcmp(topic, "002/TimerSetting") == 0) {
    // 处理 topic2 的逻辑
    Serial.println("Received message");
    Serial.println(message);
    // TODO: 在这里添加 topic2 的逻辑处理代码
  }
}

//-------------Reconnect-------------------
void connectWiFi(){
//------connecting to a WiFi network --------
 int count=0;
 WiFi.begin(ssid, password);
 while (WiFi.status() != WL_CONNECTED) {
     delay(1000);
     Serial.println("Connecting to WiFi..");
     if(count++ >= 10){
        ESP.restart();
      }
    }
 Serial.println("Connected to the WiFi network");
}

void connectMQTT(){
//------connecting to a MQTT ----------------
 while (!client.connected()) {
     String client_id = "esp32-client";
     client_id += String(WiFi.macAddress());
     Serial.printf("The client %s connects to the public mqtt broker\n", client_id.c_str());
     if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
         Serial.println("Public emqx mqtt broker connected");
     } else {
         Serial.print("failed with state ");
         Serial.print(client.state());
         delay(2000);
     }
  }
}

//中断函数        
void IRAM_ATTR onTimer() {            
  Serial.println('1');
 // 如果 WIFI 连接断开，尝试重连
  if(WiFi.status() != WL_CONNECTED)
 {
     connectWiFi();
 }
// if (WiFi.status() != WL_CONNECTED) {
//    Serial.println("WiFi connection lost. Reconnecting...");
//    connectWiFi();
//  }
//  // 如果 MQTT 连接断开，尝试重连
// if  (!client.connected()){
//    Serial.println("MQTT connection lost. Reconnecting...");
//    client.setServer(mqtt_broker, mqtt_port);
//    client.setCallback(callback);
//    connectMQTT();
//  }
}
