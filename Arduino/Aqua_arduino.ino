#include <SoftwareSerial.h>

//SoftwareSerial phSerial(9, 10);  // RX (receive) pin, TX (transmit) pin
SoftwareSerial nodeSerial(4, 5); // RX (receive) pin, TX (transmit) pin
#define TDS_SENSOR A0      // Define the TDS sensor input pin
#define TURB_SENSOR A1     // Define the turbidity sensor input pin
#define DO_SENSOR A2       // Define the dissolved oxygen sensor input pin
#define RELAY_PIN_1 6      // Relay pin for actuator 1
#define RELAY_PIN_2 7      // Relay pin for actuator 2
#define VREF 5.0           // Analog reference voltage(Volt) of the ADC
#define SCOUNT 30          // Sum of sample points

int analogBuffer[SCOUNT];        // Store the analog value in the array, read from ADC
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0;
int copyIndex = 0;

float pHValue = 0;
float tdsValue = 0;
float turbidityValue = 0;
float doValue = 0;
float temperature = 0.0;
float doPercentage = 0.0;
float FiltrationUnit = 0.0;
float OxygenPump = 1.0;
bool manualControl = false;
float pHThreshold = 6.5;
float turbidityThreshold = 2.6;
float tdsThreshold = 2000.0;
float doThreshold = 40.0;


float temperatureHistory[4] = {0, 0, 0, 0}; // Store the last 4 temperature readings

void setup() {
  //phSerial.begin(9600);
  Serial.begin(9600);
  nodeSerial.begin(9600); // Initialize serial communication
  pinMode(TDS_SENSOR, INPUT);
  pinMode(TURB_SENSOR, INPUT);
  pinMode(DO_SENSOR, INPUT);
  pinMode(RELAY_PIN_1, OUTPUT);
  pinMode(RELAY_PIN_2, OUTPUT);
  digitalWrite(RELAY_PIN_1, LOW);
  digitalWrite(RELAY_PIN_2, LOW);

  Serial.println("Arduino is ready");
}

void loop() {
  // Read and display sensor data
  temperature = readAndDisplayPH();       // Read and display pH value
  readAndDisplayTDS();                    // Read and display TDS value
  readAndDisplayTurbidity();              // Read and display turbidity value
  readAndDisplayDO();                     // Read and display dissolved oxygen value

  Serial.println("");                     // Print a new line for better readability
  delay(1000);                            // Adjust the delay to control the pace

// Handle MQTT messages for relay control
  if (nodeSerial.available()) {
    String receivedData = nodeSerial.readStringUntil('\n');
    Serial.print("Received data: ");
    Serial.println(receivedData);

    // Check for valid message format
    if (receivedData.indexOf("Filter:") != -1 && receivedData.indexOf("Oxygen:") != -1 && receivedData.indexOf("manualControl:") != -1) {
      float filterValue = getValue(receivedData, "Filter");
      float oxygenValue = getValue(receivedData, "Oxygen");
      bool manualControlValue = getBoolValue(receivedData, "manualControl");

      // Debugging statements
      Serial.print("Filter value: ");
      Serial.println(filterValue);
      Serial.print("Oxygen value: ");
      Serial.println(oxygenValue);
      Serial.print("Manual control: ");
      Serial.println(manualControlValue);

      // Control the relays based on the received values
      controlRelays(filterValue, oxygenValue);

      // Update manual control state
      manualControl = manualControlValue;
    } else {
      Serial.println("Invalid MQTT message format");
    }
  }

  if (!manualControl) { // Only check thresholds if manual control is not active
    if (pHValue < pHThreshold || turbidityValue > turbidityThreshold || tdsValue > tdsThreshold) {
      // Activate Filtration Unit
      FiltrationUnit = 1.0;
      digitalWrite(RELAY_PIN_1, HIGH);  // Turn on actuator 1
      Serial.println("Filtration Unit activated");
    } else {
      // Deactivate Filtration Unit
      FiltrationUnit = 0.0;
      digitalWrite(RELAY_PIN_1, LOW);   // Turn off actuator 1
      Serial.println("Filtration Unit deactivated");
    }

    if (doPercentage < doThreshold) {
      // Activate Oxygen Pump
      OxygenPump = 1.0;
      digitalWrite(RELAY_PIN_2, HIGH);  // Turn on actuator 2
      Serial.println("Oxygen Pump activated");
    } else {
      // Deactivate Oxygen Pump
      OxygenPump = 0.0;
      digitalWrite(RELAY_PIN_2, LOW);   // Turn off actuator 2
      Serial.println("Oxygen Pump deactivated");
    }
  }
}

float readAndDisplayPH() {
  // Initialize with default values
  float pHValue = 0;
  float temperatureValue = 0, tempC = 0;

  if (Serial.available()) {
    String data = Serial.readStringUntil('\n'); // Read a line of data
    //Serial.println(data);

    // Check if the data contains "PH:" and "T:"
    if (data.indexOf("PH:") != -1 && data.indexOf("T:") != -1) {
      pHValue = extractValue(data, "PH:");
      temperatureValue = extractValue(data, "T:");
      tempC = (((temperatureValue - 32) * 5) / 9);
    }
  }

  nodeSerial.print("pH: ");
  nodeSerial.print(pHValue, 2); // Print with 2 decimal places
  nodeSerial.print(" Temperature: ");
  nodeSerial.print(tempC, 2);

  return temperatureValue;
}

void readAndDisplayDO() {
  int doValue = analogRead(DO_SENSOR);
  doPercentage = map(doValue, 0, 1023, 0, 100);

  nodeSerial.print("  Dissolved_Oxygen: ");
  nodeSerial.print(doPercentage, 2); // Print with 2 decimal places
}

void readAndDisplayTDS() {
  static unsigned long analogSampleTimepoint = millis();
  if (millis() - analogSampleTimepoint > 800U) {
    analogSampleTimepoint = millis();
    for (copyIndex = 0; copyIndex < SCOUNT; copyIndex++) {
      analogBufferTemp[copyIndex] = analogRead(TDS_SENSOR);
    }

    float averageVoltage = getMedianNum(analogBufferTemp, SCOUNT) * (float)VREF / 1024.0;

    float compensationCoefficient = 1.0 + 0.02 * (temperature - 25.0);
    float compensationVoltage = averageVoltage / compensationCoefficient;

    tdsValue = (133.42 * compensationVoltage * compensationVoltage * compensationVoltage - 255.86 * compensationVoltage * compensationVoltage + 857.39 * compensationVoltage) * 0.5;

    nodeSerial.print("  TDS: ");
    nodeSerial.print(tdsValue, 0); // Print without decimal places
    nodeSerial.print(" ppm");
  }
}

void readAndDisplayTurbidity() {
  int turbValue = analogRead(TURB_SENSOR);
  float voltageturb = turbValue * (5.0 / 1024.0);

  nodeSerial.print("  Turbidity: ");
  nodeSerial.print(voltageturb, 2); // Print with 2 decimal places
}

void controlRelays(float filterValue, float oxygenValue) {
  // Control the relays based on the received values
  if (filterValue == 1.0) {
    digitalWrite(RELAY_PIN_1, HIGH);  // Turn on actuator 1
    Serial.println("Filter actuator turned ON");
  } else {
    digitalWrite(RELAY_PIN_1, LOW);   // Turn off actuator 1
    Serial.println("Filter actuator turned OFF");
  }

  if (oxygenValue == 1.0) {
    digitalWrite(RELAY_PIN_2, HIGH);  // Turn on actuator 2
    Serial.println("Oxygen actuator turned ON");
  } else {
    digitalWrite(RELAY_PIN_2, LOW);   // Turn off actuator 2
    Serial.println("Oxygen actuator turned OFF");
  }
}

float getValue(String data, String identifier) {
  int start = data.indexOf(identifier);

  if (start != -1) {
    // Extract the value following the identifier
    String valueString = data.substring(start + identifier.length() + 1); // Skip the space after the identifier
    return valueString.toFloat(); // Convert the value to a float
  } else {
    return 0.0; // Default value if the identifier is not found
  }
}

float extractValue(String data, String identifier) {
  int start = data.indexOf(identifier);

  if (start != -1) {
    // Extract the value following the identifier
    String valueString = data.substring(start + identifier.length());
    return valueString.toFloat(); // Convert the value to a float
  } else {
    return 0.0; // Default value if the identifier is not found
  }
}

int getMedianNum(int bArray[], int iFilterLen) {
  int bTab[iFilterLen];
  for (int i = 0; i < iFilterLen; i++)
    bTab[i] = bArray[i];

  int i, j, bTemp;

  for (j = 0; j < iFilterLen - 1; j++) {
    for (i = 0; i < iFilterLen - j - 1; i++) {
      if (bTab[i] > bTab[i + 1]) {
        bTemp = bTab[i];
        bTab[i] = bTab[i + 1];
        bTab[i + 1] = bTemp;
      }
    }
  }

  if (iFilterLen & 1) {
    bTemp = bTab[(iFilterLen - 1) / 2];
  } else {
    bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
  }

  return bTemp;
}

bool getBoolValue(String data, String identifier) {
    int start = data.indexOf(identifier);

    if (start != -1) {
        // Extract the value following the identifier
        String valueString = data.substring(start + identifier.length() + 1); // Skip the space after the identifier
        valueString.trim(); // Trim leading and trailing spaces
        return valueString.equalsIgnoreCase("true");
    } else {
        return false; // Default value if the identifier is not found
    }
}


