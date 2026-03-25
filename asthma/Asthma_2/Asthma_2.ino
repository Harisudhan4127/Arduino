#include <Wire.h>
#include "MAX30105.h"
#include "spo2_algorithm.h"

#include <MAX3010x.h>
#include "filters.h"

// Sensor (adjust to your sensor type)
MAX30105 sensor;
const auto kSamplingRate = sensor.SAMPLING_RATE_400SPS;
const float kSamplingFrequency = 400.0;

// Finger Detection Threshold and Cooldown
const unsigned long kFingerThreshold = 10000;
const unsigned int kFingerCooldownMs = 500;

// Edge Detection Threshold (decrease for MAX30100)
const float kEdgeThreshold = -2000.0;

// Filters
const float kLowPassCutoff = 5.0;
const float kHighPassCutoff = 0.5;

// Averaging
const bool kEnableAveraging = true;
const int kAveragingSamples = 50;
const int kSampleThreshold = 5;

MAX30105 particleSensor;

#define BUFFER_SIZE 100
uint32_t irBuffer[BUFFER_SIZE];
uint32_t redBuffer[BUFFER_SIZE];

int32_t heartRate;
int8_t validHeartRate;

// Averaging
#define HR_AVG_COUNT 10
int hrHistory[HR_AVG_COUNT];
int hrIndex = 0;
bool hrFilled = false;

int getAverageHR(int newHR) {
  hrHistory[hrIndex] = newHR;
  hrIndex = (hrIndex + 1) % HR_AVG_COUNT;
  if (hrIndex == 0) hrFilled = true;

  int count = hrFilled ? HR_AVG_COUNT : hrIndex;
  long sum = 0;
  for (int i = 0; i < count; i++) sum += hrHistory[i];
  return sum / count;
}

void setup() {
  Serial.begin(115200);

  if (particleSensor.begin(Wire, I2C_SPEED_STANDARD)) {
    particleSensor.setup();
    particleSensor.setPulseAmplitudeRed(0x0A);
    particleSensor.setPulseAmplitudeGreen(0);
    Serial.println("MAX30102 initialized.");
  } else {
    Serial.println("MAX30102 not found!");
    while (1);
  }
}

void loop() {
  // Collect samples
  for (int i = 0; i < BUFFER_SIZE; i++) {
    while (!particleSensor.available());
    redBuffer[i] = particleSensor.getRed();
    irBuffer[i] = particleSensor.getIR();
    particleSensor.nextSample();
  }

  int32_t spo2;  // unused
  int8_t validSPO2;

  // Run algorithm (HR only, ignore SpO2)
  maxim_heart_rate_and_oxygen_saturation(
    irBuffer, BUFFER_SIZE,
    redBuffer,
    &spo2, &validSPO2,
    &heartRate, &validHeartRate
  );

  if (validHeartRate) {
    int avgHR = getAverageHR(heartRate);
    Serial.print("Heart Rate: ");
    Serial.print(heartRate);
    Serial.print(" bpm | Avg: ");
    Serial.print(avgHR);
    Serial.println(" bpm");
  } else {
    Serial.println("Place finger properly on sensor...");
  }

  delay(1000);
}
