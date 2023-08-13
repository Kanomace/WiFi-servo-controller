#include <WiFi.h>
#include <PubSubClient.h>
#include <EEPROM.h>
//声明一个定时器
hw_timer_t * timer = NULL;  
//创建定时器中断触发标志 
int FLAG_timIT = 0;  
//声明定时结构体
struct timestr {
  int ID=1;
  int timehour;
  int timemin;
  int state;
};
timestr settings[10];

// WiFi
const char *ssid = "huangjiacheng"; // Enter your WiFi name
const char *password = "88888888";  // Enter WiFi password

// MQTT Broker
const char *mqtt_broker = "192.168.202.86";
const char *mqtt_username = "admin";
const char *mqtt_password = "public";
const int mqtt_port = 1883;
const char *topic1 = "001/DoorControl";
const char *topic2 = "001/TimerSetting";
const char *topic3 = "101/TimerSetting";

WiFiClient espClient;
PubSubClient client(espClient);

void  IRAM_ATTR onTimer();
void  connectMQTT();
void  connectWiFi();

//----------------init function---------------
void setup() {
//波特率初始化 :115200;
  Serial.begin(115200);
//定时器初始化
  timer = timerBegin(0, 80, true);                
  timerAttachInterrupt(timer, &onTimer, true);    //调用中断函数
  timerAlarmWrite(timer, 1000000, true);        //timerBegin的参数二 80位80MHZ，这里为1000000  意思为1秒

//timerDetachInterrupt(timer);                //关闭定时器
  
//------connecting to a WiFi network --------
  connectWiFi();
//------connecting to a mqtt broker----------
 client.setServer(mqtt_broker, mqtt_port);
 client.setCallback(callback);
 connectMQTT();

// publish and subscribe
 client.publish(topic1, "Hi EMQX I'm ESP32 ^^");
 client.subscribe(topic1);
 client.publish(topic2, "Hi EMQX I'm ESP32 ^^");
 client.subscribe(topic2);
 client.publish(topic3, "Hi EMQX I'm ESP32 ^^");
 client.subscribe(topic3);
 
 timerAlarmEnable(timer);                     //定时器使能
 EEPROM.begin(4096);//设置EEPROM的大小为1024个byte
 EEPROM.commit();//开启EEPROM
}


//----------------loop function---------------
void loop() {
   client.loop();
   timeread();
   delay(1000);
      
  if(FLAG_timIT = 1){             //检测定时器中断触发标志
    FLAG_timIT = 0;               //清除定时器中断触发标志
//...此处写定时器中断触发后要执行的任务
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
  else if (strcmp(topic, "001/TimerSetting") == 0) {
    // 处理 topic2 的逻辑
    Serial.println("Received message");
    Serial.println(message);
    // TODO: 在这里添加 topic2 的逻辑处理代码
//     message.trim(); // 移除字符串开头和结尾的空白字符

    // 分离字符串并转为整型
    int num=0;
    num= message.substring(0, 2).toInt();
    if(num==-1){
    }
    else if(num==-2){
      int h;
      int m;
      int s;
      for(int i=0;i<10;i++){
        int timehour=EEPROM.read(i*10);
        int timemin=EEPROM.read(i*10+1);
        int state=EEPROM.read(i*10+2);
        String msg = String(i); 
        msg += "/ ";
        msg += String(timehour);
        msg += "/ ";
        msg += String(timemin);
        msg += "/ ";
        msg += String(state); 
        client.publish(topic3, msg.c_str());
        delay(50);
       }
    }
    else if(num==0||num==1||num==2||num==3||num==4||num==5||num==6||num==7||num==8||num==9){
        settings[num].timehour = message.substring(3, 5).toInt();
        settings[num].timemin = message.substring(6, 8).toInt();
        settings[num].state = message.substring(9, 11).toInt();
        timeset(num);
    }
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
  FLAG_timIT = 1;            
}

void timeset(int num){
    EEPROM.begin(4096); //申请操作到地址4095（比如你只需要读写地址为100上的一个字节，该处也需输入参数101）
//  for(int addr = 0; addr<4096; addr++)
//  {
//    int data = addr%256; //在该代码中等同于int data = addr;因为下面write方法是以字节为存储单位的
//    EEPROM.write(addr, data); //写数据
//  }
    EEPROM.write(num*10,   settings[num].timehour);
    EEPROM.write(num*10+1, settings[num].timemin);
    EEPROM.write(num*10+2, settings[num].state);
    EEPROM.commit(); //保存更改的数据
}

void timeread(){
//for(int addr = 0; addr<4096; addr++)
//  {
//    int data = EEPROM.read(addr); //读数据
//    Serial.print(data);
//    Serial.print(" ");
//    delay(2);
//    if((addr+1)%256 == 0) //每读取256字节数据换行
//    {
//      Serial.println("");
//    }
//  }
   int state;
   int timehour;
   int timemin;
   for(int i=0;i<10;i++){    
     timehour=EEPROM.read(i*10);
     timemin=EEPROM.read(i*10+1);
     state=EEPROM.read(i*10+2);
     Serial.print("NO.");
     Serial.println(i);
     Serial.print(" time is:");
     Serial.print(timehour);     
     Serial.print(":");
     Serial.println(timemin);
     Serial.print(" state is:");
     Serial.println(state);
     delay(50);
   }
}
