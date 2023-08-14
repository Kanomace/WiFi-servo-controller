# WiFi-servo-controller
 A WiFi communication expansion board for servo motor controllers.

# 概述

本文将介绍如何使用ESP32通讯板对伺服电机控制器进行控制，并使用MQTT向本地服务器EMQX进行通讯。
在开始前，请先参阅[乐鑫科技官网](https://www.espressif.com.cn/en/products/sdks/esp-idf)中的开发引导了解ESP32模组开发相关概念和乐鑫物联网开发框架。
项目工程包含以下部分：
├── Cloud Server
├── Hardware
│   ├── BOM_V1_2023-08-09              **物料清单**
│   ├── Gerber_V1_2023-08-09           **PCB制版文件**
│   ├── PCB_V1_2023-08-09              **PCB文件**
│   └── SCH_Schematic_V1_2023-08-09    **原理图文件**
├── Software
│   ├── Example
│   │   ├── MQTT_connect.ino           **MQTT连接例程** 
│   │   └── MQTT_control.ino           **MQTT控制例程** 
│   └── main
│       └── main20230813.ino           **通讯板烧录代码** 
├── System architecture
│   └── 架构图                          **系统整体架构** 
├── .git                               **git仓库文件** 
├── .gitattributes                     **git仓库文件** 
└── README.md                
# 搭建开发环境

## 软件环境
`arduino`开发环境下的ESP32-SDK[Arduino-SDK](https://github.com/espressif/arduino-esp32)

## MQTT本地服务器环境

# 用户使用说明

## 硬件说明
本项目PCB由立创EDA绘制，包括以下部分：
├── ESP32主控及其下载电路
├── 电源管理电路
├── 继电器通断电路
├── RGB灯珠
└── 复位及网络重启按键

## 软件说明
以下针对用户需要关心的**main文件**做出相关说明

# 踩坑日记

## 看门狗饱和导致ESP32重启，注意esp32-timer-hal.c中定时器和中断函数是不可被阻塞的

解决方法：加个定时器标志位，在`main`中执行中断函数
参考博客：
https://blog.csdn.net/Beihai_Van/article/details/125793806?ops_request_misc=&request_id=&biz_id=102&utm_term=sGuru%20Meditation%20Error:%20Core%20%20&utm_medium=distribute.pc_search_result.none-task-blog-2~all~sobaiduweb~default-1-125793806.142^v92^insert_down28v1&spm=1018.2226.3001.4187

## 未知原因导致每开机两次会有一次连接不上网络
解决方法：加个`ESP.restore` 在连接不上WiFi时,尝试重启ESP32

# 参考资料
- 图文手把手教程--ESP32 MQTT对接EMQX本地服务器(VSCODE+ESP-IDF) 
(https://blog.csdn.net/felix_tao/article/details/125882339?spm=1001.2014.3001.5506)
- MQTT服务器搭建和ESP32实现MQTT代码 
(https://blog.csdn.net/wcc243588569/article/details/123557400?spm=1001.2014.3001.5506)
- ESP32 连接到免费的公共 MQTT 服务器
(https://www.emqx.com/zh/blog/esp32-connects-to-the-free-public-mqtt-broker?utm_source=mqttx&utm_medium=referral&utm_campaign=mqttx-help-to-blog)
- ESP8266 + MQTT ：如何实现 LED 灯的远程控制 
(https://www.emqx.com/zh/blog/esp8266_mqtt_led)