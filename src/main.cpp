#include <M5Unified.h>
#include <esp_log.h>

#include <FastLED.h> // http://librarymanager/All#FastLED
#include <ESP_Color.h>

const int NUM_LEDS = 60; //120+60;
static CRGB leds[NUM_LEDS];

void setup() {
  M5.begin();
  M5.Power.setExtPower(true);  // If you need external port 5V output.
  M5.Imu.begin();              // If you need IMU
  Serial.begin(115200);        // If you need Serial
  M5.Display.setBrightness(0); // For models with LCD: backlight control (0~255)

  #define FASTLED_PIN 32
  FastLED.addLeds<SK6812, FASTLED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(4);

  delay(1000);
  printf("ax,ay,az,gx,gy,gz\n");
}

#define HISTORY_SIZE 100
float history[HISTORY_SIZE];
int history_index = 0;
float mx = 0.0;

long loop_counter = 0;
void loop() {
  int val = 0;
  //ESP_LOGI("loop", "millis()=%d", millis());
  if(M5.Imu.getType() == m5::imu_mpu6886){
    // ESP_LOGI("loop", "mpu6886!");
    //float ax, ay, az;
    float gx, gy, gz;
    //M5.Imu.getAccel(&ax, &ay, &az);
    M5.Imu.getGyro(&gx, &gy, &gz);
    // ESP_LOGI("loop", "a=%7.3f %7.3f %7.3f", ax, ay, az);
    // ESP_LOGI("loop", "g=%7.3f %7.3f %7.3f", gx, gy, gz);

    float cur = gz; //gx + gy; // + gz;
    history[history_index%HISTORY_SIZE] = cur;
    float sum = 0.0;
    for(int i=0; i<HISTORY_SIZE; i++){
      sum += history[i];
    }
    history_index++;
    float avg = sum / HISTORY_SIZE;
    float delta = cur - avg;
    if(-2 < delta && delta < 2){ delta *= 0.5; }
    if(-5 < delta && delta < 5){ delta *= 0.5; }
    mx += delta*0.01;

    val = mx;
    while(val<0){ val += 360; }
    val = val % 360;

    printf("%10.4f,%10.4f,%10.4f,%10.4f\n", gx, gy, gz, mx);
  }

  if(loop_counter%10 == 0){
    if(M5.Power.getType() == m5::Power_Class::pmic_axp192){
      //M5.Power.getBatteryLevel();
      // ESP_LOGI("loop", "Current=%7.3f %7.3f %7.3f %7.3f",
      //   M5.Power.Axp192.getVBUSCurrent(),
      //   M5.Power.Axp192.getACINCurrent(),
      //   M5.Power.Axp192.getBatteryDischargeCurrent(),
      //   M5.Power.Axp192.getBatteryChargeCurrent()
      // );
    }
  }

  for(int i=0; i<NUM_LEDS; i++){
    leds[i] = CRGB(0,0,0);
  }

  for(int i=0; i<NUM_LEDS; i++){
    //float h = (loop_counter+i) % 360 / 360.0;
    float h = val % 360 / 360.0;
    ESP_Color::Color c = ESP_Color::Color::FromHsl(h, 1.0, 0.5);
    //ESP_LOGI("loop", "millis()=%d hue=%d", millis(), int(hue), );
    leds[NUM_LEDS-i-1] = CRGB(c.R_Byte(), c.G_Byte(), c.B_Byte());
  }
  FastLED.show();
  loop_counter++;

  //delay(1000);
}
