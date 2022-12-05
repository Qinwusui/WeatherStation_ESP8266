# WeatherStation 简易气象站

## 简介

- 以ESP8266(NodeMCU 1.0 ESP-12E)为基础，C++为编程语言构建
- 使用```[ArduinoJson.h]```作为Json解析库
- 使用```[Adafruit_SSD1306.h]```作为屏幕显示库
- 使用```[ESP8266WiFi.h]```作为WIFI连接库
- 使用```[NTPClient.h]```作为网络时间对齐库
- 使用```[ESP8266WebServer.h]```作为本地服务器库
- 使用```[DHT.h]```作为本地温度传感器检测库
- 使用```[FS.h]```作为本地Flash永久存储库，用于存储WIFI配置，天气请求地点信息
- 使用```[PCTOLCD2002]```进行中文点阵绘制

## 使用方法

- 首先购买硬件设备

  - ESP8266(NodeMCU 1.0 ESP-12E)
  - DHT 11 或者DHT 22
  - SSD1306
  - 若干电线

- 接线```[省略步骤]```

- 准备高德天气KEY

  - [高德天气官网](https://lbs.amap.com/api/webservice/guide/api/weatherinfo)
  - 将Key填入```[WeatherStation.ino]```第50行```[TODO]```位置
  - 注意：
    - DHT数据引脚接在ESP8266的D5(也叫GPIO14)口上
  - 将```[Hanz.h]```放在```[C:\Users\[Lover]此处自行更改\AppData\Local\Arduino15\packages\esp8266\hardware\esp8266\3.0.2\libraries\Hanz]```下
  
- 烧录进ESP8266```[省略步骤]```

## 注意事项

    代码内使用了WebSocket相关类库，如不需要可自行删除
    由于ESP8266没有合适的中文显示库，故本人自行实现了部分中文的点阵显示
    为了求简单，ESP8266的WebServer不会自动关闭，且WiFi的Mode将会一直为STA_AP(接入点模式&热点模式)
    由于代码中写死了绘制坐标，如使用其他规格屏幕可能会显示错乱，有需求请自行更改，无需求请使用128x64大小的屏幕(SSD1306)
