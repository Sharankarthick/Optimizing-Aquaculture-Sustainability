# Optimizing Aquaculture Sustainability 🌊🐟

A complete IoT solution aimed at improving water quality and operational efficiency in aquaculture environments through real-time monitoring and smart control systems.

---

## 📌 Overview

This project automates and monitors critical water parameters in aquaculture tanks—ensuring optimal conditions for fish health and sustainability.

It involves:

- Real-time monitoring of **TDS**, **Turbidity**, **pH**, **Temperature**, and **Light Intensity**
- Remote control of **Filtration Unit** and **Oxygen Pump**
- Dual-mode (Manual / Automatic) operation via a mobile app
- Local and Cloud-based data storage using **MySQL** and **Firebase**
- Offline sync: Automatically uploads cached local data to Firebase when internet is restored

---

## 🛠️ Tech Stack

| Component | Tech/Tool |
|----------|-----------|
| Microcontrollers | Arduino UNO, NodeMCU (ESP8266) |
| Sensors | TDS, Turbidity, pH + Temp (via pH board), LDR, Water Level |
| Communication | UART (Serial), MQTT |
| Backend | MySQL (local on Raspberry Pi), Firebase (Cloud) |
| Mobile App | MIT App Inventor |
| Programming | C/C++ (Arduino IDE), Python (Raspberry Pi) |

---

## 📡 System Architecture

mermaid
graph TD
    Sensors --> Arduino
    Arduino -->|UART| NodeMCU
    NodeMCU -->|MQTT| RaspberryPi
    RaspberryPi -->|MySQL| LocalDB
    RaspberryPi -->|Firebase SDK| Firebase
    MITAppInventor -->|Firebase| Firebase
    MITAppInventor -->|Control Signals| Firebase --> RaspberryPi -->|MQTT| NodeMCU --> |UART| Arduino --> Relays


---

## 📲 Features
### 📈 Real-Time Monitoring

    Mobile app shows live graphs of:

        pH

        Temperature

        TDS

        Turbidity

        Light Intensity

### ⚙️ Dual Mode Control

    Manual Mode: Toggle Filtration Unit and Oxygen Pump from app

    Automatic Mode: System controls relays based on water quality parameters

### 🌐 Connectivity Features

    MQTT ensures low-latency communication

    MySQL used as cache in case of internet outage

    Firebase used for real-time cloud sync and app data
---

## 📂 Project Structure

Optimizing-Aquaculture-Sustainability/

├── Arduino_Code/

│   └── sensor_reader.ino

├── NodeMCU_Code/

│   └── mqtt_uploader.ino

├── RaspberryPi_Code/

│   ├── mqtt_subscriber.py

│   └── firebase_uploader.py

├── MIT_App_Inventor/

│   └── aia_file/

└── README.md

---

## 🧪 Sensor Modules Used
| Sensor | Function |
|----------|-----------|
|TDS Sensor|	Measures Total Dissolved Solids|
|Turbidity Sensor|	Measures clarity of water|
|pH Sensor Board|	Measures pH & water temperature|
|LDR|	Detects light intensity (optional usage)|

---

## 📸 Screenshots / GIFs

![Flow Chart](/images/FlowChart.jpg)

![Setup](/images/Setup.jpg)

![](/images/Setup2.jpg)

### App Interface

![App Interface](/images/app1.jpg)
![](/images/app2.jpg)
![](/images/app3.jpg)
![](/images/app4.jpg)
![](/images/app5.jpg)
![](/images/app6.jpg)
![](/images/app7.jpg)
![](/images/app8.jpg)
