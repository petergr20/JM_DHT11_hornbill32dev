
// #include "Boutons.h"
#include "Free_Fonts.h" // Include the header file attached to this sketch
#include "TFT_eSPI.h"
// #include "cumuls.h"
// #include "logo.h"
#include "time.h"

#include <SPI.h>
#include <WiFi.h>
#include <Ticker.h>
#include "DHTesp.h"
/** Initialize DHT sensor 1 */
DHTesp dhtSensor1;

TaskHandle_t tempTaskHandle = NULL;
/** Pin number for DHT11 1 data pin */
int dhtPin1 = 21;
Ticker tempTicker;
bool gotNewTemperature = false;
/** Data from sensor 1 */
TempAndHumidity sensor1Data;
/* Flag if main loop is running */
bool tasksEnabled = false;

// Activation ecran
TFT_eSPI lcd = TFT_eSPI();

#define LOOP_PERIOD 3500 // Display updates every 35 ms

void Affiche();
void dht11();
void tempTask(void *pvParameters);
void triggerGetTemp();

/*
TFT_eSprite sprite = TFT_eSprite(&lcd);  // Tout l'écran
TFT_eSprite voyant = TFT_eSprite(&lcd);
TFT_eSprite fond = TFT_eSprite(&lcd);
*/

void setup(void)
{
  // Initialisation port série et attente ouverture
  Serial.begin(115200);

  Serial.begin(115200);
  Serial.println("Example for 3 DHT11/22 sensors");

  // Initialize temperature sensor 1
  dhtSensor1.setup(dhtPin1, DHTesp::DHT11);
  // Start task to get temperature
  xTaskCreatePinnedToCore(
      tempTask,        /* Function to implement the task */
      "tempTask ",     /* Name of the task */
      4000,            /* Stack size in words */
      NULL,            /* Task input parameter */
      5,               /* Priority of the task */
      &tempTaskHandle, /* Task handle. */
      1);              /* Core where the task should run */

  if (tempTaskHandle == NULL)
  {
    Serial.println("[ERROR] Failed to start task for temperature update");
  }
  else
  {
    // Start update of environment data every 30 seconds
    tempTicker.attach(30, triggerGetTemp);
  }

  // Signal end of setup() to tasks
  tasksEnabled = true;

  // Initialisation ecran
  lcd.init();
  lcd.setRotation(3);
  lcd.fillScreen(TFT_BLACK);
}

void loop()
{
  if (gotNewTemperature)
  {
    Serial.println("Sensor 1 data:");
    Serial.println("Temp: " +
                   String(sensor1Data.temperature, 2) +
                   "'C Humidity: " + String(sensor1Data.humidity, 1) +
                   "%");
    Affiche();
    delay(LOOP_PERIOD);
  }
}
// gestion des objets dans l'ecran
void Affiche()
{
  // methode affichage refresh obligatoire
  lcd.fillScreen(TFT_BLACK);

  // affichage des donnees du capteur dht11
  lcd.drawRoundRect(0, 0, 310, 235, 3, TFT_GREEN);
  lcd.setFreeFont(FSS24);
  lcd.setTextColor(TFT_BLUE);
  lcd.drawString("Temperature ", 10, 10);
  lcd.setTextColor(TFT_ORANGE);
  lcd.drawString(String(sensor1Data.temperature, 2), 40, 60);
  lcd.drawLine(0, 110, 310, 110, TFT_GREEN);
  lcd.setTextColor(TFT_BLUE);
  lcd.drawString("Humidite ", 10, 120);
  lcd.setTextColor(TFT_ORANGE);
  lcd.drawString(String(sensor1Data.humidity, 1), 40, 180);
}

/*
 * Task to reads temperature from DHT11 sensor
 * @param pvParameters
 *		pointer to task parameters
 */
void tempTask(void *pvParameters)
{
  Serial.println("tempTask loop started");
  while (1) // tempTask loop
  {
    if (tasksEnabled && !gotNewTemperature)
    { // Read temperature only if old data was processed already
      // Reading temperature for humidity takes about 250 milliseconds!
      // Sensor readings may also be up to 2 seconds 'old' (it's a very slow sensor)
      sensor1Data = dhtSensor1.getTempAndHumidity(); // Read values from sensor 1
      gotNewTemperature = true;
    }
    vTaskSuspend(NULL);
  }
}

/**
 * triggerGetTemp
 * Sets flag dhtUpdated to true for handling in loop()
 * called by Ticker tempTicker
 */
void triggerGetTemp()
{
  if (tempTaskHandle != NULL)
  {
    xTaskResumeFromISR(tempTaskHandle);
  }
}
