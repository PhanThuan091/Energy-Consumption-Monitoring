// Enable debug console
// Set CORE_DEBUG_LEVEL = 3 first
#define ERA_DEBUG
// #define ERA_SERIAL Serial

/* Select ERa host location (VN: Viet Nam, SG: Singapore) */
#define ERA_LOCATION_VN
// #define ERA_LOCATION_SG

#if defined(BUTTON_PIN)
#include <pthread.h>
#include <ERa\ERaButton.hpp> 
#endif

// Cấu hình wifi


#define ERA_AUTH_TOKEN "84af8048-4e85-4479-b4f5-989d0f34f440"
// #define ERA_AUTH_TOKEN "ERA2706"
#include <Arduino.h>
#include <ERa.hpp>
#include <PZEM004Tv30.h>

#define LED_PIN 2
#define Relay_PIN 25
#define Pir_PIN 35

const char ssid[] = "Wi-MESH - HCMUTE";
const char pass[] = "hcmute@2024";
// const char ssid[] = "IoT Lab";
// const char pass[] = "IoT@123456";
// const char ssid[] = "YOUR_SSID";
// const char pass[] = "YOUR_PASSWORD";

unsigned long lastMotionTime;
uint8_t power_value = 100;

PZEM004Tv30 pzem(Serial2, 16, 17);
/* This function is called every time the Virtual Pin 0 state change */
ERA_WRITE(V0) {
  /* Get value from Virtual Pin 0 and write LED */
  uint8_t value = param.getInt();
  digitalWrite(LED_PIN, value ? HIGH : LOW);

  // Send the LED status back
  ERa.virtualWrite(V0, digitalRead(LED_PIN));
}

/* This function will execute each time from the Text Box to your chip */
ERA_WRITE(V1) {
  /* Get value from Virtual Pin 1 */
  ERaString estr = param.getString();

  // If you type "on", turn on LED
  // If you type "off", turn off LED
  if (estr == "on") {
    digitalWrite(LED_PIN, HIGH);
  } else if (estr == "off") {
    digitalWrite(LED_PIN, LOW);
  }

  // Send it back
  ERa.virtualWrite(V1, estr);
  // Send the LED status back
  ERa.virtualWrite(V0, digitalRead(LED_PIN));
}

ERA_WRITE(V10) {
  /* Get value from Virtual Pin 10 and write LED */
  uint8_t value = param.getInt();
  digitalWrite(Relay_PIN, value ? HIGH : LOW);

  // Send the LED status back
  ERa.virtualWrite(V10, digitalRead(Relay_PIN));
}

ERA_WRITE(V14) {
  /* Get value from Virtual Pin 14 */
  power_value = param.getInt();
  Serial.print("Power: ");
  Serial.println(power_value);
  // Send the LED status back
  // ERa.virtualWrite(V14, power_value);
}

/* This function will run every time ERa is connected */
ERA_CONNECTED() {
  ERA_LOG("ERa", "ERa connected!");
}
/* This function will run every time ERa is disconnected */
ERA_DISCONNECTED() {
  ERA_LOG("ERa", "ERa disconnected!");
}


void timerEvent() {
  float voltage = pzem.voltage();
  float current = pzem.current();
  float power = pzem.power();
  float energy = pzem.energy();
  float frequency = pzem.frequency();
  float pf = pzem.pf();
  // string text = "Cảnh báo: Không có chuyển động và công suất >" + (string) power_value;

  // Gửi các giá trị đo lường tới ERa
  ERa.virtualWrite(V3, voltage);
  ERa.virtualWrite(V4, current);
  ERa.virtualWrite(V5, power);
  ERa.virtualWrite(V6, energy);
  ERa.virtualWrite(V7, frequency);
  ERa.virtualWrite(V8, pf);
  
  // Đọc trạng thái cảm biến chuyển động
  int motionState = digitalRead(Pir_PIN);
  int power_val = power_value*10;
  // Gửi giá trị cảm biến chuyển động tới ERa Virtual Pin V11
  ERa.virtualWrite(V11, motionState);
  Serial.print("Motion Sensor State: ");
  Serial.println(motionState);
  Serial.print("power: ");
  Serial.println(power_val);
  // Nếu có chuyển động, cập nhật thời gian
  if (motionState == HIGH) {
    lastMotionTime = ERaMillis();
  }
 
  // Kiểm tra nếu đã quá 5 phút không có chuyển động và công suất > 1000W  300000ms
  if ((ERaMillis() - lastMotionTime >= 10000) && (power > power_val)) {
    // Gửi cảnh báo lên ERa
    // warningMessage = "Cảnh báo: Không có chuyển động và công suất > " ;
    // ERa.virtualWrite(V12, warningMessage);
    ERa.virtualWrite(V13, 1);
    ERa.virtualWrite(V15, "Không có chuyển động và công suất tiêu thụ lớn!");
    Serial.print("Cảnh báo: Không có chuyển động và công suất tiêu thụ lớn hơn ");
    
    Serial.println(power_val);
    // digitalWrite(Relay_PIN, LOW);
    // // Gửi trạng thái relay về
    // ERa.virtualWrite(V10, digitalRead(Relay_PIN));
  } else {
    ERa.virtualWrite(V13, 0);
    ERa.virtualWrite(V15, "---");
  }

  // Kiểm tra nếu đã quá 5 phút không có chuyển động
  if (ERaMillis() - lastMotionTime >= 10000) {
    // Tắt relay
    digitalWrite(Relay_PIN, LOW);
    // Gửi trạng thái relay về
    ERa.virtualWrite(V10, digitalRead(Relay_PIN));
  }
}

void setup() {
  #if defined(ERA_DEBUG)
    Serial.begin(115200);
  #endif

  pinMode(LED_PIN, OUTPUT);
  pinMode(Relay_PIN, OUTPUT);
  pinMode(Pir_PIN, INPUT_PULLUP);

  ERa.setScanWiFi(true);
  ERa.begin(ssid, pass);

  ERa.addInterval(1000L, timerEvent);
  lastMotionTime = ERaMillis();
}

void loop() {
  ERa.run();
}
