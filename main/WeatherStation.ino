/**
 * @file WeatherStation.ino
 * @author Qinwusui
 * @brief 
 * @version 0.1
 * @date 2022-11-27
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <Arduino.h>
#include <ArduinoJson.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
ESP8266WiFiMulti WiFiMulti;
#include <ESP8266HTTPClient.h>

#include <WiFiClient.h>
#include <WebSocketsClient.h>
WebSocketsClient client;
#include <DHT.h>
DHT dht(14, DHT11);
#include <ESP8266WebServer.h>
ESP8266WebServer server(80);
//定时器
// #include <Ticker.h>
//获取网络时间
#include <NTPClient.h>
#include <WiFiUdp.h>
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "ntp1.aliyun.com", 60 * 60 * 8, 1000);

#include <FS.h>

Adafruit_SSD1306 oled(128, 64, &Wire, -1);
String msg = "";
String ssid = "";
String pwd = "";
String WeatherKey = "TODO";
String loc = "";
String locId = "";
typedef struct WeatherData
{
    String tmp;      //温度
    String text;     //天气描述
    String humidity; //相对湿度
    String pressure; //压强
    String precip;   //当前小时累计降水量
} weather;
weather wa;
#include <Hanz.h>
scu StartLogo[] = {
    0x00, 0x00, 0xC0, 0x00, 0x80, 0x0D, 0xC0, 0x0F, 0xE0, 0x0F, 0xF0, 0x1F, 0xF8, 0x1F, 0xFC, 0x13,
    0x34, 0x10, 0x30, 0x10, 0x60, 0x10, 0x74, 0x08, 0x78, 0x1E, 0xF0, 0x15, 0xC0, 0x1F, 0xC0, 0x0F};


void doSth(uint8_t *s)
{
    String m = (char *)s;
    // displayLogo(m);
}

void webSocketEvent(WStype_t type, uint8_t *payload, size_t length)
{

    switch (type)
    {
    case WStype_DISCONNECTED:
        break;
    case WStype_CONNECTED:
        break;
    case WStype_TEXT:
        doSth(payload);
        break;
    default:
        break;
    }
}

//执行一次，初始化操作
String wifiFile = "/wifi/wifiConfig.ini";
String locFile = "/loc/location.ini";
void setup()
{
    Serial.begin(115200);
    dht.begin();
    initOled();
    showWellcome();
    initServer();
    initSPIFFS();
    // startWebSocket();
    timeClient.begin();
    // tickerGetTemp.start();
}
void drawBitMap(const uint8_t **bitmap, int size, int x, int y, int width, int height, bool needDot)
{
    oled.clearDisplay();
    int aX = x;
    for (int i = 0; i < size; i++)
    {
        oled.drawXBitmap(aX, y, bitmap[i], width, height, 1);
        aX += 12;
    }
    if (needDot)
    {
        oled.setCursor(aX, y);
        oled.print("...");
    }
    oled.drawFastHLine(0, 50, 128, 1);            //横线
    oled.drawXBitmap(2, 2, StartLogo, 16, 16, 1); // Logo
    oled.display();
}
void initSPIFFS()
{
    if (SPIFFS.begin())
    {
        //"SPIFFS Loading..."
        drawBitMap(初始化存储器, 6, 0, 52, 12, 12, true); // 12*6+3*6
        if (SPIFFS.exists(wifiFile))
        {
            initWifi();
        }

        if (SPIFFS.exists(locFile))
        {
            initLoc();
        }
        drawBitMap(定位中, 3, 0, 52, 12, 12, true);
    }
    else
    {
        drawBitMap(存储器初始化失败, 7, 0, 52, 12, 12, true);
    }
}
String getWifiConfig()
{
    String wifi("");
    File f = SPIFFS.open(wifiFile, "r");
    for (int i = 0; i < f.size(); i++)
    {
        wifi += f.readString();
    }
    f.close();
    return wifi;
}
String readPwd()
{
    String wifi = getWifiConfig();
    int index = wifi.indexOf("||");
    String s2 = wifi.substring(index + 2, wifi.length());
    return s2;
}
String readSSID()
{
    String wifi = getWifiConfig();
    int index = wifi.indexOf("||");
    String s1 = wifi.substring(0, index).c_str();

    return s1;
}
void showWellcome()
{
    oled.clearDisplay();
    oled.drawXBitmap(2, 2, StartLogo, 16, 16, 1);
    oled.setTextSize(1);
    oled.setCursor(37, 20);
    oled.print("Wellcome!");
    oled.display();
    oled.setTextSize(1);
    delay(2000);
    oled.clearDisplay();
}

//通过Web进行配网
void initServer()
{
    drawBitMap(创建热点中, 5, 0, 52, 12, 12, true);
    bool runOk = WiFi.softAP("NodeMCU-12E", "qinsansui233", 10, 0, 8);
    if (!runOk)
    {
        Serial.println("热点创建失败！");
        return;
    }
    server.on("/", handleRoot);
    server.on("/put", handleConfigWifi);
    server.on("/loc", handleLocation);
    server.on("/location", handleConfigLoc);
    server.begin();
}
void handleConfigWifi()
{
    ssid = server.arg("ssid");
    pwd = server.arg("pwd");
    if (ssid == "" || pwd.length() < 8)
    {
        String htm = R"delimiter(
        <html lang="en">

    <head>
    <meta charset="UTF-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Wrong</title>
    </head>

    <body>
    <span style="text-align: center;">WiFi配置不正确，请点击按钮返回上一页重新填写！</span><br>
    <button id="back">返回</button>
    </body>
    <script>
    var button = document.getElementById("back")
    button.addEventListener("click", function () {
        window.history.back()
    })
    </script>

    </html>
        )delimiter";
        server.send(200, "text/html", htm);
    }
    else
    {
        String wifi = ssid + "||" + pwd;
        File f = SPIFFS.open(wifiFile, "w");
        f.print(wifi);
        f.close();
        String html = R"delimiter(
        <html lang="en">
    <head>
    <meta charset="UTF-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Finished</title>
    </head>
    <body>
    <span style="text-align: center;">已上传WiFi配置，正在配网中，您可以关闭该网页！</span>
    </body>
    </html>
    )delimiter";
        server.send(200, "text/html", html);
        initWifi();
    }
}
void handleRoot()
{
    String s = R"delimiter(
        <html lang="en">
    <head>
    <meta charset="UTF-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Esp8266配网</title>
    </head>
    <body>
    <form action="put">
        <span>输入WIFI SSID：</span><input type="text" name="ssid" id="ssid"><br />
        <span>输入PassWord：</span><input type="text" name="pwd" id="pwd">
        <button type="submit">提交</button>
    </form>
    <span style="text-align: center;"><a href="loc">打开位置配置</a></span>
    </body>
    </html>
    )delimiter";
    server.send(200, "text/html", s);
}
void handleLocation()
{
    String html = R"delimiter(
        <html lang="en">
    <head>
    <meta charset="UTF-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>地区填写</title>
    </head>
    <body>
    <span>请填写你所在的地区</span><br>
    <form action="location">
        <input type="text" name="location"  id="location">
        <input type="submit" value="提交">
    </form>
    </body>
    </html>
    )delimiter";
    server.send(200, "text/html", html);
}
void handleConfigLoc()
{
    loc = server.arg("location");
    File f = SPIFFS.open(locFile, "w");
    f.print(loc);
    f.close();
    String html = R"delimiter(
        <html lang="en">
    <head>
    <meta charset="UTF-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Finished</title>
    </head>
    <body>
    <span style="text-align: center;">填写成功，现在可以返回单片机继续操作了捏</span>
    </body>
    </html>
    )delimiter";
    server.send(200, "text/html", html);
    initLoc();
}
//配置显示驱动
void initOled()
{
    oled.begin(SSD1306_SWITCHCAPVCC, 0x3c);
    oled.setTextColor(WHITE);
    oled.clearDisplay();
}
//初始化Wifi
void initWifi()
{
    ssid = readSSID();
    ssid.replace("\n", "");
    ssid.trim();
    pwd = readPwd();
    pwd.replace("\n", "");
    pwd.trim();
    if (ssid == "" || pwd == "")
    {
        return;
    }
    WiFi.mode(WIFI_AP_STA);
    WiFi.begin(ssid, pwd, 1, NULL, true);
    drawBitMap(初始化无线网络中, 8, 0, 52, 12, 12, true);
    int time = 0;
    while (WiFi.status() != WL_CONNECTED)
    {
        time += 1;
        // displayLogo(); //"Connecting Wifi..."
        if (time == 10)
        {
            // 10秒，没连上WiFi
            break;
        }
        delay(1000);
    }
    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("连接成功\nIP:" + WiFi.localIP().toString());
        drawBitMap(无线网络初始化成功, 9, 0, 52, 12, 12, false);
        // displayLogo(); //"WiFi:" + ssid + "\n" + "IP:" + WiFi.localIP().toString()
    }
    else
    {
        drawBitMap(无线网络初始化失败, 8, 0, 52, 12, 12, false);
        // displayLogo(); //"Not Connect WiFi" + ssid
    }
}

void startWebSocket()
{
    // displayLogo(); //"WebSocket initing..."
    client.begin("172.16.15.125", 54322, "/iot/msg");
    client.onEvent(webSocketEvent);
    // client.setAuthorization()
    client.setReconnectInterval(5000);
    client.enableHeartbeat(15000, 3000, 2);
}
void loop()
{
    server.handleClient();
    DisPlay_Time(1000);
    client.loop();
}
unsigned long t = 0;
void DisPlay_Time(uint32_t p)
{
    unsigned long time = millis();
    if (time - t >= p)
    {
        t = time;
        timeClient.update();
        getLocalTemp();
    }
}
void getLocalTemp()
{

    oled.clearDisplay();
    oled.drawXBitmap(2, 2, StartLogo, 16, 16, 1);

    oled.drawFastHLine(0, 36, 128, 1);

    oled.setTextSize(1);
    // int h = dht.readHumidity();    //湿度
    int t = dht.readTemperature(); //温度°C
    // oled.setCursor(2, 48);
    // oled.printf("Humi:%d", h);
    oled.drawXBitmap(0, 50, 室温[0], 12, 12, 1);
    oled.drawXBitmap(12, 50, 室温[1], 12, 12, 1);
    oled.setCursor(26, 53);
    oled.printf("%d", t);
    oled.drawXBitmap(40, 50, TempC, 12, 12, 1);
    oled.setCursor(46, 53);
    oled.print("C");
    oled.setCursor(34, 12);
    oled.setTextSize(2);
    int hours = timeClient.getHours();
    int min = timeClient.getMinutes();
    // Serial.println(timeClient.getDay());
    oled.printf("%s:%s", hours < 10 ? "0" + String(hours) : String(hours),
                min < 10 ? "0" + String(min) : String(min));
    oled.setTextSize(1);
    if (&wa != NULL)
    {
        oled.drawXBitmap(54, 50, 外温[0], 12, 12, 1);
        oled.drawXBitmap(66, 50, 外温[1], 12, 12, 1);
        oled.setCursor(78, 53);
        oled.printf("%s", wa.tmp);
        oled.drawXBitmap(90, 50, TempC, 12, 12, 1);
        oled.setCursor(96, 53);
        oled.print("C");
    }
    int day = timeClient.getDay();
    // oled.setCursor(48, 48);
    oled.drawXBitmap(104, 50, WeekDay[day][0], 12, 12, 1); //周
    oled.drawXBitmap(116, 50, WeekDay[day][1], 12, 12, 1); //一二三四五六日
    oled.display();
}

void initLoc()
{
    loc = readLoc();
    if (loc == "")
    {
        return;
    }
解决Gzip压缩的问题:
    WiFiClient client;
    String url = "192.168.1.125";
    wa = WeatherData();
    String content = (String)("GET ") + "/iot/weather?location=" + loc + "&key=" + WeatherKey +
                     " HTTP/1.1\r\n " +
                     "Content-Type: text/html;charset=utf-8\r\n" +
                     "Connection: Keep Alive\r\n\r\n";
    if (!client.connect(url, 54322))
    {
        Serial.println("不能连接到中转天气服务器");
        return;
    }
    client.print(content);
    String line = client.readStringUntil('\n');
    while (client.available())
    {
        line = client.readStringUntil('\n');
        if (line.startsWith("{"))
        {
            break;
        }
    }
    Serial.print(line);
    line.trim();
    DynamicJsonDocument json(line.length() * 2);
    deserializeJson(json, line);
    String temp = json["now"]["temp"].as<String>();
    String text = json["now"]["text"].as<String>();
    String humidity = json["now"]["humidity"].as<String>();
    String precip = json["now"]["precip"].as<String>();
    String pressure = json["now"]["pressure"].as<String>();
    wa.humidity = humidity;
    wa.precip = precip;
    wa.pressure = pressure;
    wa.text = text;
    wa.tmp = temp;
    client.stop();
}

String readLoc()

{
    String s = "";
    File f = SPIFFS.open(locFile, "r");
    for (int i = 0; i < f.size(); i++)
    {
        char j = f.read();
        if (j == '\n')
        {
            continue;
        }
        s += (char)j;
    }
    f.close();
    return s;
}
