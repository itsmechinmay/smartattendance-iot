# Smart Attendance IoT

**IoT-Based Student Attendance Management System Using ESP32-S3 and RFID**

## 1. Project Overview

Smart Attendance IoT is an RFID-based student attendance management system developed using the ESP32-S3 microcontroller. The system automates the process of recording student attendance and uploads attendance information to the Blynk Cloud platform.

A teacher can access the ESP32-hosted web page using a QR code, log in, select the subject, and activate the attendance session. Students can then mark their attendance by scanning their RFID cards.

## 2. Features

- RFID-based student identification.
- QR-code access to the teacher web interface.
- Teacher login and subject selection.
- Attendance activation and deactivation.
- Duplicate attendance prevention.
- OLED display for system status.
- Buzzer feedback during card scanning.
- Attendance data transmission to Blynk Cloud.
- Real-time attendance monitoring through the Blynk dashboard.

## 3. Hardware Components

| Component              | Purpose                            |
| ---------------------- | ---------------------------------- |
| ESP32-S3-N16R8         | Main microcontroller               |
| RC522 RFID Reader      | Reads student RFID cards           |
| RFID Cards             | Student identification             |
| 0.96-inch OLED Display | Displays QR code and system status |
| Buzzer                 | Provides audio feedback            |
| Breadboard             | Circuit assembly                   |
| Jumper Wires           | Hardware connections               |

## 4. Software and Platforms

- Arduino IDE
- ESP32 Board Package
- Blynk IoT Cloud
- MFRC522 Library
- Adafruit SSD1306 Library
- Adafruit GFX Library
- QR Code Library
- ESP32 WebServer and Wi-Fi libraries

## 5. System Working

1. The ESP32-S3 starts and connects to Wi-Fi.
2. The OLED displays a QR code.
3. The teacher scans the QR code to access the ESP32 web interface.
4. The teacher logs in.
5. The teacher selects Theory or Practical and the required subject.
6. The teacher starts the attendance session.
7. The RFID reader becomes active.
8. Students scan their RFID cards.
9. The ESP32 identifies the student using the RFID UID.
10. The system checks whether attendance has already been marked for that session.
11. If the attendance is new, the system records it and sends the information to Blynk Cloud.
12. The OLED and buzzer provide feedback about the scanning result.

## 6. Circuit Connection Diagram
<img width="675" height="436" alt="circuit diagram" src="https://github.com/user-attachments/assets/57b3a784-c940-4856-91ab-68a36ff4b329" />


## 7. Hardware Pin Connections

### RC522 RFID Reader

| RC522 Pin | ESP32-S3 GPIO |
| --------- | ------------: |
| SDA / SS  |             5 |
| RST       |             4 |
| SCK       |            12 |
| MISO      |            13 |
| MOSI      |            11 |

### OLED Display

| OLED Pin | ESP32-S3 GPIO |
| -------- | ------------: |
| SDA      |             8 |
| SCL      |             9 |

### Buzzer

| Buzzer Pin        | ESP32-S3 GPIO |
| ----------------- | ------------: |
| Signal / Positive |            10 |
| GND / Negative    |           GND |

## 8. Cloud Integration

The project uses Blynk IoT Cloud to transmit and monitor attendance information.

The configured virtual pins are:

- V0 – RFID UID
- V1 – Attendance Status
- V2 – Student Name
- V3 – Date
- V4 – Time
- V5 – Category
- V6 – Subject

## 9. Project Structure

```text
smartattendance-iot/
│
├── Arduino_Code/
│   └── SmartAttendance.ino
│
├── Diagrams/
│   ├── Block_Diagram.png
│   └── Flowchart.png
│
├── README.md
└── .gitignore
```

## 10. Setup Instructions

1. Install Arduino IDE.
2. Install the ESP32 board package.
3. Install the required libraries.
4. Configure Wi-Fi credentials and Blynk authentication details.
5. Connect the hardware according to the pin connection tables.
6. Open `SmartAttendance.ino` in Arduino IDE.
7. Select the appropriate ESP32-S3 board and COM port.
8. Upload the program.
9. Configure the Blynk dashboard to monitor attendance.

**Security Note:** Do not upload Wi-Fi passwords, Blynk authentication tokens, or teacher login passwords to GitHub. Store private credentials securely.

## 11. Future Scope

- Development of a compact and portable attendance device.
- Improved attendance analytics and reporting.
- Enhanced teacher dashboard.
- Support for additional identification methods.
- Improved cloud data management.

## 12. Project Status

The project is developed as a working breadboard-based prototype using ESP32-S3, RFID, OLED, a buzzer, a web interface, and Blynk Cloud.

---

**Project Name:** Smart Attendance IoT
**Platform:** ESP32-S3 and Blynk IoT Cloud
