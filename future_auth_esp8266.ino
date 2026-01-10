#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <sha1.h>
#include "SSD1306Brzo.h"

#include "DejaVu_Sans_28.h"
SSD1306Brzo display(0x3c, 4, 5);

char key[] = { 0x27, 0xf9, 0xb1, 0x1d, 0x43, 0x20, 0x39, 0xc0, 0x3c, 0x86, 0x5c, 0x86, 0x8a, 0x4f, 0x4d, 0xe3, 0xb1, 0xd6, 0xde, 0x70 };
char ssid[] = "mySSID";
char pass[] = "myWifiPass";
unsigned int localPort = 2390;
unsigned long epoch, millisAtSync, secsPassedSync, secsSince1900, ntpMicrosec;
const unsigned long seventyYears = 2208988800UL;
String stringHour, stringMin, stringSec;

unsigned int timeoutput, timestamp, swapped, NineTeenthByte, cb;
int time1, time2, time3, time4, myOffset, extractedInt, polished, wallTime, wallHour, wallMin, wallSec;

double lostMillis;

IPAddress timeServerIP;
byte packetBuffer[48];

WiFiUDP udp;

unsigned long sendNTPpacket(IPAddress& address)
{
  memset(packetBuffer, 0, 48);
  packetBuffer[0] = 0xE3;
  packetBuffer[1] = 0x00;
  packetBuffer[2] = 0x06;
  packetBuffer[3] = 0xEC;
  packetBuffer[12]  = 0x31;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 0x31;
  packetBuffer[15]  = 0x34;
  udp.beginPacket(address, 123);
  udp.write(packetBuffer, 48);
  udp.endPacket();
}

void setup(){
//Serial.begin(115200);
//Serial.println("Authenticator booting...");

  display.init();
  display.flipScreenVertically();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(DejaVu_Sans_28);

WiFi.mode(WIFI_STA);
WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
  }

  udp.begin(localPort);
  WiFi.hostByName("ptbtime2.ptb.de", timeServerIP); 
  sendNTPpacket(timeServerIP);
  delay(100); // minimum possible delay for esp8266 to receive UDP reply
  cb = udp.parsePacket();

//if (cb == 48) {
    //Serial.println("success...");
//}
    millisAtSync = millis();
    udp.read(packetBuffer, 48);
    secsSince1900 = (unsigned long)packetBuffer[40] << 24;
    secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
    secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
    secsSince1900 |= (unsigned long)packetBuffer[43];
    ntpMicrosec = (unsigned long)packetBuffer[44] << 24;
    ntpMicrosec |= (unsigned long)packetBuffer[45] << 16;
    ntpMicrosec |= (unsigned long)packetBuffer[46] << 8;
    ntpMicrosec |= (unsigned long)packetBuffer[47];
    lostMillis = ((double(ntpMicrosec) / double(4294967296)) + 0.1)*1000; // add 100ms intrinsic delay
    epoch = secsSince1900 - seventyYears;
}

void loop()
{
secsPassedSync = (millis() - millisAtSync + int(lostMillis)) / 1000;
timeoutput = (epoch + secsPassedSync);
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

wallTime = timeoutput + 3600  ; //ADD 1 Hours (For GMT+1)
// print the hour, minute and second:
wallHour=(wallTime  % 86400L) / 3600;
wallMin=(wallTime % 3600) / 60;
wallSec=(wallTime % 60);

stringHour = (wallHour <= 9) ? "0" + String(wallHour) : String(wallHour);
stringMin = (wallMin <= 9) ? "0" + String(wallMin) : String(wallMin);
stringSec = (wallSec <= 9) ? "0" + String(wallSec) : String(wallSec);

  display.clear();
  display.drawString(0, 0, charToken);
  display.drawString(0, 28, stringHour + ":" + stringMin + ":" + stringSec);
  display.display();
  delay(100); 
}
