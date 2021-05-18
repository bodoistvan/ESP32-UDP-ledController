#include "WiFi.h"
#include "AsyncUDP.h"
#include "FastLED.h"

const char* ssid = "myssid"; // replace with your WiFi SSID
const char* password = "mypassword"; // replace with your WiFi password

IPAddress ip(192,168,1,50);   
IPAddress gateway(192,168,1,1);   
IPAddress subnet(255,255,255,0);  
 
AsyncUDP udp;

#define LED_PIN     4
#define NUM_LEDS    200
#define LED_TYPE    WS2812
#define BRIGHTNESS  64
CRGB leds[NUM_LEDS];

unsigned long LASTTIME = 0;
unsigned int LASTPRIORITY = 0;

struct Color{
  int r; int g; int b; 
};

void setup() {
  //
  // start serial
  //
  Serial.begin(115200);

  //
  // connect to WiFi
  //
  WiFi.config(ip, gateway, subnet);
  Serial.print("\nConnecting ");
  WiFi.begin(ssid,password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
    }
  Serial.print("\nConnected with local address ");
  Serial.print(WiFi.localIP());
  Serial.print("\n");
  //
  // setup fastled
  //
  for (int i = 0; i < NUM_LEDS; i++){
    leds[i] = CRGB(100,100,0);
  }
  FastLED.addLeds<LED_TYPE, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness( 0 );
  FastLED.show();
  //
  // start UDP
  //
  udp.listen(4000);
  //
  // UDP packet handler
  //
  udp.onPacket([](AsyncUDPPacket packet) {
    //
    // write to serial
    //
    uint8_t* myData = packet.data();

    uint8_t priorityText[2];
    memcpy( priorityText, myData, 2);
    int priority = StrToHex(priorityText);

    unsigned long nowTime = millis();
    unsigned long timediff = nowTime - LASTTIME;
    
    if (timediff < 5000 && priority < LASTPRIORITY)
      return;

    LASTTIME = nowTime;
    LASTPRIORITY = priority;

    uint8_t brightnessText[2];
    memcpy( brightnessText, &myData[5], 2);
    int brightness = StrToHex(brightnessText);

    uint8_t ledCountText[3];
    memcpy( ledCountText, &myData[2], 3);
    int ledCount = StrToHex(ledCountText);

    struct Color* ledColors = (struct Color*)malloc(ledCount * sizeof (struct Color));

    Serial.println(ledCount);
    uint8_t pixelR[2];
    uint8_t pixelG[2];
    uint8_t pixelB[2];
    
    for (int i = 0; i < ledCount; i++){
      Serial.println(i);
      int j = i * 6;
      memcpy( pixelR, &myData[j + 7], 2);
      memcpy( pixelG, &myData[j + 7 + 2], 2);
      memcpy( pixelB, &myData[j + 7 + 4], 2);
      
      struct Color color;
      int red = hex2intlen((char*)pixelR,2);
      int green = hex2intlen((char*)pixelG,2);
      int blue = hex2intlen((char*)pixelB,2);

      Serial.write("i: ");
      Serial.println(i);
      Serial.println(red);
      Serial.println(green);
      Serial.println(blue);
      
      color.r = red;
      color.g = green;
      color.b = blue;
      ledColors[i] = color;
    }

    for (int i = 0; i< min(NUM_LEDS, ledCount); i++){
      struct Color c = ledColors[i];
      leds[i] = CRGB(c.r,c.g,c.b);
    }
    
    FastLED.setBrightness( brightness );
    FastLED.show();
    free(ledColors);
  
    });
  }

void loop() {
    
  }

int StrToHex(uint8_t str[])
{
  return (int) strtol((char*)str, 0, 16);
}


int hex2intlen(char* str, int len){
  int num = 0;
  for (int i = 0; i < len; i++){
   num += pow(16, len-1-i) * hex2int(str[i]);
  }
  return num;
}
int hex2int(char ch)
{
    if (ch >= '0' && ch <= '9')
        return ch - '0';
    if (ch >= 'A' && ch <= 'F')
        return ch - 'A' + 10;
    if (ch >= 'a' && ch <= 'f')
        return ch - 'a' + 10;
    return -1;
}
