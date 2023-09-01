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
    int ID=1;
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
const char *mqtt_broker = "192.168.124.17";
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
    client.publish(topic1, "Hi EMQX I'm ESP32 ^^");
    client.subscribe(topic1);
    client.publish(topic2, "Hi EMQX I'm ESP32 ^^");
    client.subscribe(topic2);
    client.publish(topic3, "Hi EMQX I'm ESP32 ^^");
    client.subscribe(topic3);
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
    timeread();
    delay(1000);
        if(FLAG_timIT = 1){               //检测定时器中断触发标志
            FLAG_timIT = 0;               //清除定时器中断触发标志
//...此处写定时器中断触发后要执行的任务
            printLocalTime();
            if(ETH.linkUp() == false && WiFi.status() != WL_CONNECTED)
            {
                ESP.restart();
            }
// 如果 WIFI 连接断开，尝试重连
//  if(WiFi.status() != WL_CONNECTED)
// {
//     connectWiFi();
// }
 
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
        if(strcmp(message.c_str(), "open") == 0){
            digitalWrite(H6, HIGH); // 打开继电器
            delay(1000);
            digitalWrite(H6, LOW); // 打开继电器
            Serial.println("okk");  
        }
        if(strcmp(message.c_str(), "stop") == 0){
            digitalWrite(H7, HIGH); // 打开继电器
            delay(1000);
            digitalWrite(H7, LOW); // 打开继电器
            Serial.println("okk");  
        }  
        if(strcmp(message.c_str(), "close") == 0){
            digitalWrite(H8, HIGH); // 打开继电器
            delay(1000);
            digitalWrite(H8, LOW); // 打开继电器
            Serial.println("okk");  
        }  
    }
//--------topic2---------
    else if (strcmp(topic, "001/TimerSetting") == 0) {
// 处理 topic2 的逻辑
        Serial.println("Received message");
        Serial.println(message);
// TODO: 在这里添加 topic2 的逻辑处理代码
      //message.trim(); // 移除字符串开头和结尾的空白字符
// 分离字符串并转为整型
        int num=0;
        num= message.substring(0, 2).toInt();
// 全部清空（-1/10/30/0）
        if(num==00){
            for(int i =0;i<=9;i++){
                settings[i].timehour = 0;
                settings[i].timemin = 0;
                settings[i].state = 0;
                timeset(i);
            }
        }
// 全部打印（-2/10/30/0）
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
// 打开定时功能（-3/10/30/0）
        if(num==-3){    
            FLAG_timer=1;          
        }
// 关闭定时功能（-4/10/30/0）
        if(num==-4){    
            FLAG_timer=0;          
        }
        else if(num==1||num==2||num==3||num==4||num==5||num==6||num==7||num==8||num==9){
            settings[num].timehour = message.substring(3, 5).toInt();
            settings[num].timemin = message.substring(6, 8).toInt();
            settings[num].state = message.substring(9, 11).toInt();
            timeset(num);
        }
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

void printLocalTime()
{
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo))
    {
        Serial.println("Failed to obtain time");
        return;
    }
    Serial.println(&timeinfo, "%F %T %A"); // 格式化输出
}
