#define ERA_DEBUG
#define ERA_LOCATION_VN
#if defined(BUTTON_PIN)
#include <pthread.h>
#include <ERa\ERaButton.hpp> 
#endif


#define ERA_AUTH_TOKEN "d0f34f440"

#include <Arduino.h>
#include <ERa.hpp>
#include <PZEM004Tv30.h>

#define LED_PIN 2
#define Relay_PIN 25
#define Pir_PIN 35

const char ssid[] = "HCMUTE";
const char pass[] = "";

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

ERA_CONNECTED() {
  ERA_LOG("ERa", "ERa connected!");
}

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

  if ((ERaMillis() - lastMotionTime >= 10000) && (power > power_val)) {

    ERa.virtualWrite(V13, 1);
    ERa.virtualWrite(V15, "Không có chuyển động và công suất tiêu thụ lớn!");
    Serial.print("Cảnh báo: Không có chuyển động và công suất tiêu thụ lớn hơn ");
    Serial.println(power_val);
  } else {
    ERa.virtualWrite(V13, 0);
    ERa.virtualWrite(V15, "---");
  }

  // Kiểm tra nếu đã quá 5 phút không có chuyển động
  if (ERaMillis() - lastMotionTime >= 10000) {
    digitalWrite(Relay_PIN, LOW);
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
