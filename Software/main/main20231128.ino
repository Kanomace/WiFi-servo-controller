#include <WiFi.h>
#include <PubSubClient.h>
#include <EEPROM.h>
#include <ETH.h> //引用以使用ETH

//以太网口
#define ETH_ADDR        0//PHY地址和板子上对应，默认是0
#define ETH_POWER_PIN  -1
#define ETH_MDC_PIN    23
#define ETH_MDIO_PIN   18
#define LED             2
#define ETH_TYPE       ETH_PHY_LAN8720
#define ETH_CLK_MODE   ETH_CLOCK_GPIO17_OUT
#define ETH_RESET      5//ESP32的io5可控制以太网模块复位，低电平有效，不用时可不用配置
//声明一个定时器
hw_timer_t * timer = NULL;  
//定时功能使能标志
int FLAG_timer = 0;  
//创建定时器中断触发标志 
int FLAG_timIT = 0;  
//实时时间
const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 8 * 3600;
const int daylightOffset_sec = 0;
//声明定时结构体
struct timestr {
    int timehour;
    int timemin;
    int state;
};
timestr settings[10];
//定义继电器接口
int H6 = 25; // 向上
int H7 = 26; // 向下
int H8 = 27; // 停止

// WiFi
const char *ssid = "huangjiacheng"; // Enter your WiFi name
const char *password = "88888888";  // Enter WiFi password

// MQTT Broker
const char *mqtt_broker = "192.168.110.86";
const char *mqtt_username = "admin";
const char *mqtt_password = "public";
const int mqtt_port = 1883;
// MQTT订阅主题
const char* doorControlTopic = "DoorControl";
const char* timerSettingTopic = "TimerSetting";
const char* listingTopic = "Listing";
const char* heartbeatTopic = "Heartbeat";


WiFiClient espClient;
PubSubClient client(espClient);

void  IRAM_ATTR onTimer();
void  connectMQTT();
void  connectWiFi();
void printLocalTime();

//----------------init function---------------
void setup() {
//波特率初始化 :115200;
    Serial.begin(115200);
    
//定时器初始化
    timer = timerBegin(0, 80, true);                
    timerAttachInterrupt(timer, &onTimer, true);    //调用中断函数
    timerAlarmWrite(timer, 1000000, true);        //timerBegin的参数二 80位80MHZ，这里为1000000  意思为1秒
  //timerDetachInterrupt(timer);                //关闭定时器
  
//------connecting to a Ethernet/WIFI --------
    pinMode(LED, OUTPUT);  
    pinMode(ETH_RESET, OUTPUT); 
    digitalWrite(ETH_RESET, HIGH); 
    connectETH();

//------connecting to a mqtt broker-----------
    client.setServer(mqtt_broker, mqtt_port);
    client.setCallback(callback);
    connectMQTT();

// publish and subscribe
    client.subscribe(doorControlTopic);
    client.subscribe(timerSettingTopic);
    client.subscribe(listingTopic);
    client.subscribe(heartbeatTopic);
    client.publish(doorControlTopic, "Hi EMQX I'm ESP32 ^^");
    client.publish(timerSettingTopic, "Hi EMQX I'm ESP32 ^^");
    client.publish(listingTopic, "Hi EMQX I'm ESP32 ^^");
    client.publish(heartbeatTopic, "Hi EMQX I'm ESP32 ^^");

//定时器使能 
    timerAlarmEnable(timer);
//EEPROM初始化                     
    EEPROM.begin(4096);//设置EEPROM的大小为1024个byte
    EEPROM.commit();//开启EEPROM

//初始化继电器
    pinMode(H6, OUTPUT);
    pinMode(H7, OUTPUT);
    pinMode(H8, OUTPUT);
    digitalWrite(H6, LOW); // 初始关闭继电器
    digitalWrite(H7, LOW); // 初始关闭继电器
    digitalWrite(H8, LOW); // 初始关闭继电器

// 从网络时间服务器上获取并设置时间
// 获取成功后芯片会使用RTC时钟保持时间的更新
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    printLocalTime();
}


//----------------loop function---------------
void loop() {
    client.loop();
//    timeread();
    printLocalTime();
    delay(1000);
        if(FLAG_timIT = 1){               //检测定时器中断触发标志
            FLAG_timIT = 0;               //清除定时器中断触发标志
//...此处写定时器中断触发后要执行的任务
            if(ETH.linkUp() == false && WiFi.status() != WL_CONNECTED)
            {
                ESP.restart();
            }                             
  }
}

//-------------callback function--------------
void callback(char *topic, byte *payload, unsigned int length) {
  Serial.print("Received topic: ");
  Serial.println(topic);
  payload[length] = '\0';
  String message = String((char*)payload);
  Serial.print("Received message : ");
  Serial.println(message);

  // 解析主题
  String topicType = String(topic);
  String idType = message.substring(0, message.indexOf("/"));
  String senderType = message.substring(message.indexOf("/") + 1, message.indexOf("/", message.indexOf("/") + 1));
  String messageType = message.substring(message.indexOf("/", message.indexOf("/") + 1)+1);
  Serial.print("idType : ");
  Serial.println(idType);
  Serial.print("senderType : ");
  Serial.println(senderType);
  Serial.print("messageType : ");
  Serial.println(messageType);
  
  // 根据主题类型执行相应操作
  if (topicType == "DoorControl") {
    handleDoorControl(idType, senderType, messageType);
  } else if (topicType == "TimerSetting") {
    handleTimerSetting(idType, senderType, messageType);
  } else if (topicType == "Listing") {
    handleListing(idType, senderType, messageType);
  } else if (topicType == "Heartbeat") {
    handleHeartbeat(idType, senderType, messageType);
  }
}

//-------------------连接以太网口-------------------
void connectETH(){
    int count=0;
    ETH.begin(ETH_ADDR, ETH_POWER_PIN, ETH_MDC_PIN, ETH_MDIO_PIN, ETH_TYPE, ETH_CLK_MODE); //启用ETH
    while(!((uint32_t)ETH.localIP())) //等待获取到IP
    {
        count++;
        delay(100);
        if(count>=50){
            connectWiFi();
            break; 
        }
    }
    Serial.println("Connected");
    Serial.print("IP Address:");
    Serial.println(ETH.localIP());
    digitalWrite(LED, LOW);  
}

//-------------connecting to WIFI-------------------
void connectWiFi(){
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

//--------------  connecting to MQTT ----------------
void connectMQTT(){
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
    EEPROM.write(num*10,   settings[num].timehour);
    EEPROM.write(num*10+1, settings[num].timemin);
    EEPROM.write(num*10+2, settings[num].state);
    EEPROM.commit(); //保存更改的数据
}

//读取定时
void timeread(){
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

char timeString[50];
void printLocalTime()
{
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo))
    {
        Serial.println("Failed to obtain time");
        return;
    }
    
    strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S %A", &timeinfo);
    Serial.println(timeString);
}

// 处理DoorControl主题
void handleDoorControl(const String& id, const String& sender, const String& message) {
  if (sender == "TX") {
    // 解析内容
    int operation = message.toInt();

    // 执行相应操作
    if (operation == 1) {
      digitalWrite(H6, HIGH); // 开门
      delay(1000);
      digitalWrite(H6, LOW); 
      Serial.println("okk");  
    } else if (operation == 0) {
      digitalWrite(H8, HIGH); // 关门
      delay(1000);
      digitalWrite(H8, LOW); 
      Serial.println("okk");  
    } else if (operation == -1) {
      digitalWrite(H7, HIGH); // 停止
      delay(1000);
      digitalWrite(H7, LOW); 
      Serial.println("okk"); 
    }

    // 返回信息
    String responseTopic = "DoorControl/" + id + "/RX/" + message;
    client.publish(doorControlTopic, responseTopic.c_str());
  }
}

// 处理TimerSetting主题
void handleTimerSetting(const String& id, const String& sender, const String& message) {
  if (sender == "TX") {
    // 解析内容
    String operation = message.substring(0, message.indexOf("/"));
    String group = message.substring(message.indexOf("/") + 1, message.indexOf("/", message.indexOf("/") + 1));
    String hour = message.substring(message.indexOf("/", message.indexOf("/") + 1)+1, message.lastIndexOf("/"));
    String minute = message.substring(message.lastIndexOf("/") + 1);
    Serial.print("operation : ");
    Serial.println(operation);
    Serial.print("group : ");
    Serial.println(group);
    Serial.print("hour : ");
    Serial.println(hour);    
    Serial.print("minute : ");
    Serial.println(minute);  
    int group_int = group.toInt();  
    int hour_int = hour.toInt(); 
    int minute_int = minute.toInt();
    // 执行相应操作
    if (operation.toInt() >= 0 && operation.toInt() <= 9) {
      if (hour_int >= 0 && hour_int <= 23 && minute_int >= 0 && minute_int <= 59) {
        if (operation.toInt() == 1) {
          // 执行定时开启操作
          settings[group_int].timehour = hour_int;
          settings[group_int].timemin = minute_int;
          settings[group_int].state = 1;
          timeset(group_int);
        } else if (operation.toInt() == 0) {
          // 执行定时关闭操作
          settings[group_int].timehour = hour_int;
          settings[group_int].timemin = minute_int;
          settings[group_int].state = 0;
          timeset(group_int);
        } else if (operation.toInt() == -1) {
          // 执行关闭定时功能操作
          FLAG_timer=0;
        }

        // 返回信息
        String responseTopic = "TimerSetting/" + id + "/" + "RX" + "/" + hour + "/" + minute;
        client.publish(timerSettingTopic, responseTopic.c_str());
      }
    }
  }
}

// 处理Listing主题
void handleListing(const String& id, const String& sender, const String& message) {
  if (sender == "TX") {
    // 返回定时列表信息
    String responseTopic = "TimerSetting/" + id + "/RX";
    for(int i=0;i<10;i++){
      int timehour=EEPROM.read(i*10);
      int timemin=EEPROM.read(i*10+1);
      int state=EEPROM.read(i*10+2);
      responseTopic = responseTopic  + "/" + state + "/" + i + "/" + timehour + "/" + timemin;
      }
    Serial.println(responseTopic.c_str());  
    client.publish(listingTopic, responseTopic.c_str());
  }
}

// 处理Heartbeat主题
void handleHeartbeat(const String& id, const String& sender, const String& message) {
  if (sender == "TX") {
    // 返回心跳信息
    String responseTopic = "Heartbeat/" +id + "/RX/" + timeString ;
    client.publish(heartbeatTopic, responseTopic.c_str());
  }
}
