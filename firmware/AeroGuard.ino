#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

// ESP32-S3 pin map: DHT11=GPIO4, potentiometer wiper=GPIO5 (ADC),
// green/yellow/red LEDs=GPIO19/20/21, buzzer=GPIO6, LCD SDA=8 SCL=9.
constexpr uint8_t DHT_PIN = 4;
constexpr uint8_t POT_PIN = 5;
constexpr uint8_t GREEN_LED = 19;
constexpr uint8_t YELLOW_LED = 20;
constexpr uint8_t RED_LED = 21;
constexpr uint8_t BUZZER_PIN = 6;
constexpr uint8_t I2C_SDA = 8;
constexpr uint8_t I2C_SCL = 9;
constexpr uint8_t DHT_TYPE = DHT11;

// Configurable limits: adjust these values for the educational test scenario.
constexpr float TEMP_WARNING_C = 30.0f;
constexpr float TEMP_CRITICAL_C = 40.0f;
constexpr float HUMID_WARNING_PCT = 70.0f;
constexpr float HUMID_CRITICAL_PCT = 85.0f;
constexpr int ANOMALY_WARNING_ADC = 2200;
constexpr int ANOMALY_CRITICAL_ADC = 3200;
constexpr uint32_t SAMPLE_INTERVAL_MS = 2000; // DHT11 minimum sampling interval
constexpr uint32_t TELEMETRY_INTERVAL_MS = 2000;
constexpr uint32_t DISPLAY_INTERVAL_MS = 500;
constexpr uint8_t FILTER_SAMPLES = 5;

DHT dht(DHT_PIN, DHT_TYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);

enum HealthState : uint8_t { NORMAL, WARNING, CRITICAL };
HealthState state = CRITICAL; // startup remains fail-safe until measurements classify it
float filteredTemp = NAN;
float filteredHumidity = NAN;
float filteredAnomaly = 0.0f;
float tempBuffer[FILTER_SAMPLES] = {};
float humidityBuffer[FILTER_SAMPLES] = {};
float anomalyBuffer[FILTER_SAMPLES] = {};
uint8_t filterIndex = 0;
uint8_t filterCount = 0;
uint32_t lastSampleMs = 0;
uint32_t lastTelemetryMs = 0;
uint32_t lastDisplayMs = 0;
bool sensorValid = false;

float mean(const float *values, uint8_t count) {
  float total = 0.0f;
  for (uint8_t i = 0; i < count; ++i) total += values[i];
  return count ? total / count : 0.0f;
}

const char *stateName(HealthState s) {
  switch (s) {
    case NORMAL: return "NORMAL";
    case WARNING: return "WARNING";
    default: return "CRITICAL";
  }
}

HealthState classify(float t, float h, float anomaly) {
  if (anomaly >= ANOMALY_CRITICAL_ADC || t >= TEMP_CRITICAL_C || h >= HUMID_CRITICAL_PCT) return CRITICAL;
  if (anomaly >= ANOMALY_WARNING_ADC || t >= TEMP_WARNING_C || h >= HUMID_WARNING_PCT) return WARNING;
  return NORMAL;
}

void applyOutputs() {
  digitalWrite(GREEN_LED, state == NORMAL ? HIGH : LOW);
  digitalWrite(YELLOW_LED, state == WARNING ? HIGH : LOW);
  digitalWrite(RED_LED, state == CRITICAL ? HIGH : LOW);
  // Tone changes only on state transition; no delay or blocking alert loop.
  if (state == CRITICAL) tone(BUZZER_PIN, 2200);
  else if (state == WARNING) tone(BUZZER_PIN, 1000);
  else noTone(BUZZER_PIN);
}

void setup() {
  Serial.begin(115200);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(POT_PIN, INPUT);
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(RED_LED, HIGH); // fail-safe indication until first valid sample
  tone(BUZZER_PIN, 1000);
  Wire.begin(I2C_SDA, I2C_SCL);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0); lcd.print("HEALTH MONITOR");
  lcd.setCursor(0, 1); lcd.print("Starting...");
  dht.begin();
  lastSampleMs = millis() - SAMPLE_INTERVAL_MS;
}

void loop() {
  const uint32_t now = millis();

  if (now - lastSampleMs >= SAMPLE_INTERVAL_MS) {
    lastSampleMs = now;
    const float t = dht.readTemperature();
    const float h = dht.readHumidity();
    const float a = static_cast<float>(analogRead(POT_PIN));
    if (isfinite(t) && isfinite(h)) {
      tempBuffer[filterIndex] = t;
      humidityBuffer[filterIndex] = h;
      anomalyBuffer[filterIndex] = a;
      filterIndex = (filterIndex + 1) % FILTER_SAMPLES;
      if (filterCount < FILTER_SAMPLES) ++filterCount;
      filteredTemp = mean(tempBuffer, filterCount);
      filteredHumidity = mean(humidityBuffer, filterCount);
      filteredAnomaly = mean(anomalyBuffer, filterCount);
      sensorValid = true;
      const HealthState next = classify(filteredTemp, filteredHumidity, filteredAnomaly);
      if (next != state) {
        state = next;
        applyOutputs();
        Serial.printf("STATE,%s\n", stateName(state));
      }
    } else {
      // A sensor fault is treated as CRITICAL rather than presenting stale healthy data.
      sensorValid = false;
      if (state != CRITICAL) {
        state = CRITICAL;
        applyOutputs();
        Serial.println("STATE,CRITICAL,SENSOR_FAULT");
      }
    }
  }

  if (now - lastTelemetryMs >= TELEMETRY_INTERVAL_MS) {
    lastTelemetryMs = now;
    if (sensorValid) {
      Serial.printf("TELEMETRY,state=%s,temp_c=%.1f,humidity_pct=%.1f,anomaly_adc=%.0f\n",
                    stateName(state), filteredTemp, filteredHumidity, filteredAnomaly);
    } else {
      Serial.printf("TELEMETRY,state=%s,sensor=INVALID\n", stateName(state));
    }
  }

  if (now - lastDisplayMs >= DISPLAY_INTERVAL_MS) {
    lastDisplayMs = now;
    lcd.setCursor(0, 0);
    if (sensorValid) {
      lcd.printf("T:%4.1fC H:%2.0f%% ", filteredTemp, filteredHumidity);
    } else {
      lcd.print("Sensor fault    ");
    }
    lcd.setCursor(0, 1);
    lcd.printf("%-8s A:%4.0f  ", stateName(state), filteredAnomaly);
  }
}