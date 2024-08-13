#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <sha1.h>
#include <U8g2lib.h>
#define OLED_SDA  2
#define OLED_SCL 14
#define OLED_RST  4

U8G2_SSD1306_128X32_UNIVISION_F_SW_I2C u8g2(U8G2_R0, OLED_SCL, OLED_SDA , OLED_RST);

char key[] = { 0x27, 0xf9, 0xb1, 0x1d, 0x43, 0x20, 0x39, 0xc0, 0x3c, 0x86, 0x5c, 0x86, 0x8a, 0x4f, 0x4d, 0xe3, 0xb1, 0xd6, 0xde, 0x70 };
char ssid[] = "mySSID";
char pass[] = "myWifiPass";
unsigned int localPort = 2390;
unsigned long epoch, millisAtSync, secsPassedSync, highWord, lowWord, secsSince1900;
const unsigned long seventyYears = 2208988800UL;
String oldToken = "NEATO";

unsigned int timeoutput, timestamp, swapped, NineTeenthByte, cb;
int time1, time2, time3, time4, myOffset, extractedInt, polished;

IPAddress timeServerIP;
byte packetBuffer[48];

WiFiUDP udp;

unsigned long sendNTPpacket(IPAddress& address)
{
  memset(packetBuffer, 0, 48);
  packetBuffer[0] = 0b11100011;
  packetBuffer[1] = 0;
  packetBuffer[2] = 6;
  packetBuffer[3] = 0xEC;
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;
  udp.beginPacket(address, 123);
  udp.write(packetBuffer, 48);
  udp.endPacket();
}

void setup(){
// REMOVE BELOW
Serial.begin(115200);
Serial.println("Authenticator booting...");
// REMOVE ABOVE
u8g2.begin();
WiFi.mode(WIFI_STA);
WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
udp.begin(localPort);
  delay(1000);


  WiFi.hostByName("ntp.nict.jp", timeServerIP); 
  sendNTPpacket(timeServerIP);
  delay(1000);

  cb = udp.parsePacket();
  if (!cb) {
delay(500);
  }
  else {
    udp.read(packetBuffer, 48);
    highWord = word(packetBuffer[40], packetBuffer[41]);
    lowWord = word(packetBuffer[42], packetBuffer[43]);
    secsSince1900 = highWord << 16 | lowWord;
    epoch = secsSince1900 - seventyYears;
    millisAtSync = millis();
  }
}

void loop()
{
secsPassedSync = (millis() - millisAtSync) / 1000;
timeoutput = 1514381997;
timeoutput = (epoch + secsPassedSync + 1);
timestamp = timeoutput / 30;
time1 = (timestamp >> (3 << 3)) & 0xff;
time2 = (timestamp >> (2 << 3)) & 0xff;
time3 = (timestamp >> (1 << 3)) & 0xff;
time4 = (timestamp >> (0 << 3)) & 0xff;

uint8_t basestring[] = { 0, 0, 0, 0, time1, time2, time3, time4 };

Sha1.initHmac((uint8_t*)key, sizeof(key));

for (int i=0; i<sizeof(basestring); i++) {
Sha1.print((char) basestring[i]);
}
uint8_t *hash;
hash = Sha1.resultHmac();

NineTeenthByte = hash[19];
myOffset = NineTeenthByte & 0xF;
uint8_t extractedFourBytes[] = { hash[myOffset], hash[myOffset+1], hash[myOffset+2], hash[myOffset+3] };

const String authChars = "23456789BCDFGHJKMNPQRTVWXY";
String authToken = "";

extractedInt;
memcpy(&extractedInt, extractedFourBytes, 4);

swapped = ((extractedInt>>24)&0xff) |
              ((extractedInt<<8)&0xff0000) |
              ((extractedInt>>8)&0xff00) |
              ((extractedInt<<24)&0xff000000);

polished = swapped & 0x7FFFFFFF;
        
for (int i=0; i <= 4; i++){
authToken += authChars[polished % 26];
polished /= 26;
}

const char* charToken = authToken.c_str();

if (authToken != oldToken)
{
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_logisoso32_tr);
  u8g2.drawStr(0,32,charToken);
  u8g2.sendBuffer();
}

oldToken = charToken;
  delay(1000); 
}
