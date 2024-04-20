#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <time.h>
#include "timezone.h"
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>   // Universal Telegram Bot Library written by Brian Lough: https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot
#include <ArduinoJson.h>

uint32_t terima = 0;
const char *WIFI_NETWORK_NAME = "Redmi Note 11"; // Change to your wifi network name
const char *WIFI_PASSWORD = "akusayangkamu";      // Change to your wifi password

const char *TIME_SERVER = "pool.ntp.org";
int myTimeZone = UTC; // change this to your time zone (see in timezone.h)
const long gmtOffset_sec = 7 * 3600;

time_t now;

int calVal = 0.5 / 183;
float luas = 0.005026;
// Node 1 = 183 (0.543621)
// Node 2 = 210 (0.473727)
// 500 ml, 0.5L

int stateLast = 0, drop = 0, lastHour;

// Initialize Telegram BOT
#define BOTtoken "6763752845:AAGDtD83KAbBec20LVQeW8KiC31mODxOntc"  // your Bot Token (Get from Botfather)

// Use @myidbot to find out the chat ID of an individual or a group
// Also note that you need to click "start" on a bot before it can
// message you
#define CHAT_ID "6771204483"

X509List cert(TELEGRAM_CERTIFICATE_ROOT);
WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOTtoken, secured_client);

int lastMinute;

// Handle what happens when you receive new messages
void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i=0; i<numNewMessages; i++) {
    // Chat id of the requester
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID){
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }
    
    // Print the received message
    String text = bot.messages[i].text;
    Serial.println(text);

    String from_name = bot.messages[i].from_name;
    if (text == "/reset") {
      bot.sendMessage(CHAT_ID, "Mereset data hari ini ke 0", "");
      drop = 0;
    }
  }
}

void setup()
{
  Serial.begin(115200);

  WiFi.begin(WIFI_NETWORK_NAME, WIFI_PASSWORD);

  secured_client.setTrustAnchors(&cert); // Add root certificate for api.telegram.org

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  WiFi.setOutputPower(20.5);

  configTime(gmtOffset_sec, 0, TIME_SERVER);

  while (now < EPOCH_1_1_2019)
  {
    now = time(nullptr);
    delay(500);
    Serial.print("*");
  }

  pinMode(12, INPUT);
}

// toStringAddZero()
// this function adds a zero to a date string if its a single digit
// for example if data = 1, then the function will return 01
// this makes the date and time string's consistant size for display
String toStringAddZero(int data)
{
  String st = "";
  if (data < 10)
  {
    st = "0" + String(data);
  }
  else
  {
    st = String(data);
  }
  return st;
}

void loop()
{
  int stateNow = digitalRead(12);
  if (stateNow == stateLast) {
    //nothing;
  }
  else {
    drop++;
    Serial.println(drop);
  }

  struct tm *timeinfo;

  time(&now);
  timeinfo = localtime(&now);

  int year = timeinfo->tm_year + 1900;
  int month = timeinfo->tm_mon;
  month++;
  int day = timeinfo->tm_mday;
  int hour = timeinfo->tm_hour;
  int mins = timeinfo->tm_min;
  int sec = timeinfo->tm_sec;
  int day_of_week = timeinfo->tm_wday;

  /*
    Serial.print("Date = " + toStringAddZero(day) + "/" + toStringAddZero(month) + "/" + String(year));
    Serial.println(" Time = " + toStringAddZero(hour) + ":" + toStringAddZero(mins) + ":" + toStringAddZero(sec));
    Serial.print("Day is " + String(DAYS_OF_WEEK[day_of_week]));
    Serial.println(" or " + String(DAYS_OF_WEEK_3[day_of_week]));
    Serial.println(drop);
  */

  String currentDate = toStringAddZero(day) + "/" + toStringAddZero(month) + "/" + String(year);
  String formattedTime = toStringAddZero(hour) + ":" + toStringAddZero(mins) + ":" + toStringAddZero(sec);
  float curahHujan = drop * (0.543621);

  String dataWa = "Node : 1 \nTanggal : " + currentDate + ", Waktu : " + formattedTime + "\n" + "Jumlah drop bucket : " + String(drop) + "\n" + "Curah hujan (L/m²) : " + String(curahHujan);

  if (mins == lastMinute) {
    //nothing
  } else {
    //float curahHujan = drop * 12;
    //String dataWa = "Tanggal : " + currentDate + ", Waktu : " + formattedTime + "\n" + "Jumlah drop bucket : " + String(drop) + "\n" + "Curah hujan (L/m²) : " + String(curahHujan);
    Serial.println(dataWa);
    bot.sendMessage(CHAT_ID, dataWa, "");
    drop = 0;
  }
  /*
  if (millis() - terima > 1000) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    terima = millis();
  }
  */
  stateLast = stateNow;
  lastMinute = mins;

  delay(100);
}
