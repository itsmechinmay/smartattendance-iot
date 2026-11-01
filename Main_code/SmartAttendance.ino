
  #define BLYNK_TEMPLATE_ID   "YOUR_BLYNK_TEMPLATE_ID"
  #define BLYNK_TEMPLATE_NAME "Student Attendance System"
  #define BLYNK_AUTH_TOKEN    "YOUR_BLYNK_AUTH_TOKEN"

  #include <WiFi.h>
  #include <WebServer.h>
  #include <BlynkSimpleEsp32.h>
  #include <SPI.h>
  #include <MFRC522.h>
  #include <Wire.h>
  #include <SSD1306.h>
  #include <qrcodeoled.h>
  #include <time.h>

  // =================================================
  // 1. WIFI DETAILS
  // =================================================

  char ssid[] = "YOUR_WIFI_NAME";
  char pass[] = "YOUR_WIFI_PASSWORD";

  // =================================================
  // 2. TEACHER LOGIN
  // =================================================

  const char* teacherID = "teacher";
  const char* teacherPassword = "123";

  // =================================================
  // 3. RFID PINS
  // =================================================

  #define RFID_SS    5
  #define RFID_RST   4
  #define RFID_SCK   12
  #define RFID_MISO  13
  #define RFID_MOSI  11

  MFRC522 rfid(RFID_SS, RFID_RST);

  // =================================================
  // 4. OLED PINS
  // =================================================

  #define OLED_SDA 8
  #define OLED_SCL 9

  // One OLED driver for QR and text
  SSD1306 display(0x3C, OLED_SDA, OLED_SCL);
  QRcodeOled qrcode(&display);

  // =================================================
  // 5. BUZZER
  // =================================================

  #define BUZZER_PIN 10

  // =================================================
  // 6. WEB SERVER
  // =================================================

  WebServer server(80);

  String dashboardURL = "";

  // =================================================
  // 7. ATTENDANCE SESSION
  // =================================================

  bool attendanceActive = false;

  String selectedCategory = "Not Selected";
  String selectedSubject = "Not Selected";

  // =================================================
  // 8. STUDENT DATABASE
  // =================================================

  struct Student {
    const char* uid;
    const char* name;
  };

  Student students[] = {
    {"69BC3712", "Chinmay Lokhande"},
    {"A1124166", "Shivam Salve"},
    {"61CD724B", "Chinmay Clg id"},
  };

  const int studentCount =
    sizeof(students) / sizeof(students[0]);

  // =================================================
  // 9. DUPLICATE ATTENDANCE CONTROL
  // =================================================

  // Maximum unique student-subject records in RAM
  #define MAX_ATTENDANCE_RECORDS 100

  struct AttendanceRecord {
    String uid;
    String subject;
  };

  AttendanceRecord attendanceRecords[MAX_ATTENDANCE_RECORDS];

  int attendanceRecordCount = 0;

  // Check if student already marked this subject

  bool isAlreadyMarked(String uid, String subject) {

    for (int i = 0; i < attendanceRecordCount; i++) {

      if (attendanceRecords[i].uid == uid &&
          attendanceRecords[i].subject == subject) {

        return true;
      }
    }

    return false;
  }

  // Save student attendance locally

  bool saveAttendanceRecord(String uid, String subject) {

    if (attendanceRecordCount >= MAX_ATTENDANCE_RECORDS) {

      Serial.println("Attendance record storage full!");

      return false;
    }

    attendanceRecords[attendanceRecordCount].uid = uid;

    attendanceRecords[attendanceRecordCount].subject = subject;

    attendanceRecordCount++;

    return true;
  }

  // =================================================
  // 10. TIME CONFIGURATION
  // =================================================

  const long GMT_OFFSET = 19800;
  const int DAYLIGHT_OFFSET = 0;

  // =================================================
  // 11. SCAN CONTROL
  // =================================================

  unsigned long lastScanTime = 0;

  const unsigned long scanDelay = 2000;

  unsigned long restoreDisplayAt = 0;

  bool restoreDisplayPending = false;

  unsigned long lastBlynkAttempt = 0;
  unsigned long lastRFIDDebugTime = 0;

  // =================================================
  // 12. FIND STUDENT
  // =================================================

  String getStudentName(String uid) {

    for (int i = 0; i < studentCount; i++) {

      if (uid.equalsIgnoreCase(students[i].uid)) {

        return students[i].name;
      }
    }

    return "Unknown";
  }

  // =================================================
  // 13. OLED MESSAGE
  // =================================================

  void showMessage(String line1, String line2) {

    display.clear();

    display.drawString(0, 15, line1);

    display.drawString(0, 35, line2);

    display.display();
  }

  // =================================================
  // 14. SHOW DASHBOARD QR
  // =================================================

  void showQR() {

    qrcode.create(dashboardURL.c_str());

    Serial.println("Dashboard QR displayed");
  }

  // =================================================
  // 15. RESTORE OLED DISPLAY
  // =================================================

  void restoreDisplay() {

    restoreDisplayPending = false;

    if (attendanceActive) {

      showMessage(
        "Attendance Active",
        selectedSubject
      );

    } else {

      showQR();
    }
  }

  // =================================================
  // 16. TEACHER LOGIN CHECK
  // =================================================

  bool checkLogin() {

    if (!server.authenticate(
          teacherID,
          teacherPassword
        )) {

      server.requestAuthentication();

      return false;
    }

    return true;
  }

  // =================================================
  // 17. TEACHER DASHBOARD
  // =================================================

  void handleRoot() {

    if (!checkLogin()) return;

    String statusText;
    String statusColor;

    if (attendanceActive) {

      statusText = "ATTENDANCE ACTIVE";
      statusColor = "green";

    } else {

      statusText = "ATTENDANCE INACTIVE";
      statusColor = "red";
    }

    String page = R"rawliteral(
  <!DOCTYPE html>
  <html>

  <head>

  <meta name="viewport" content="width=device-width, initial-scale=1">

  <title>Student Attendance System</title>

  <style>

  body {
    font-family: Arial;
    background: #eef2f7;
    text-align: center;
    padding: 15px;
  }

  .container {
    background: white;
    max-width: 450px;
    margin: auto;
    padding: 20px;
    border-radius: 12px;
    box-shadow: 0 2px 10px rgba(0,0,0,0.1);
  }

  h2 {
    color: #17365d;
  }

  label {
    display: block;
    text-align: left;
    margin-top: 15px;
    font-weight: bold;
  }

  select {
    width: 100%;
    padding: 12px;
    margin-top: 8px;
    border-radius: 6px;
    border: 1px solid #aaa;
    font-size: 15px;
  }

  button {
    width: 100%;
    padding: 14px;
    margin-top: 15px;
    border: none;
    border-radius: 7px;
    color: white;
    font-size: 16px;
    cursor: pointer;
  }

  .start {
    background: #168b45;
  }

  .stop {
    background: #d93030;
  }

  .status {
    font-weight: bold;
    margin-top: 20px;
    font-size: 18px;
  }

  .info {
    text-align: left;
    margin-top: 18px;
    background: #f2f5f9;
    padding: 12px;
    border-radius: 7px;
  }

  </style>
  </head>

  <body>

  <div class="container">

  <h2>Student Attendance System</h2>

  <p>Teacher Dashboard</p>

  <form action="/start" method="POST">

  <label>Select Subject Category:</label>

  <select name="category" id="category"
  onchange="updateSubjects()" required>

  <option value="">-- Select Category --</option>

  <option value="Theory">Theory</option>

  <option value="Practical">Practical / Lab</option>

  </select>

  <label>Select Subject:</label>

  <select name="subject" id="subject" required>

  <option value="">-- First Select Category --</option>

  </select>

  <button class="start" type="submit">
  START ATTENDANCE
  </button>

  </form>

  <form action="/stop" method="POST">

  <button class="stop" type="submit">
  STOP ATTENDANCE
  </button>

  </form>

  <div class="status" style="color: STATUS_COLOR;">
  STATUS_TEXT
  </div>

  <div class="info">

  <p><b>Selected Category:</b> SELECTED_CATEGORY</p>

  <p><b>Selected Subject:</b> SELECTED_SUBJECT</p>

  </div>

  </div>

  <script>

  function updateSubjects() {

    let category =
      document.getElementById("category").value;

    let subject =
      document.getElementById("subject");

    subject.innerHTML = "";

    let subjects = [];

    if (category === "Theory") {

      subjects = [
        "Analog and Digital Communication (ADCom)",
        "Internet of Things (IoT)",
        "Introduction to VLSI Lifecycle (IVL)",
        "Machine Learning for VLSI (MLVLSI)",
        "Low Power VLSI (LP VLSI)",
        "System Verilog for Design and Verification (SVDV)",
        "MDM 5 (VLSI) - Computer Architecture",
        "VSP - VLSI Signal Processing"
      ];

    }

    else if (category === "Practical") {

      subjects = [
        "Analog and Digital Communication Practical (ADCom Lab)",
        "Internet of Things Practical (IoT Lab)",
        "Programming for Competitive Coding Practical (PCC Lab)",
        "System Verilog for Design and Verification Practical (SVDV Lab)"
      ];

    }

    let firstOption = document.createElement("option");

    firstOption.value = "";

    firstOption.text = "-- Select Subject --";

    subject.add(firstOption);

    subjects.forEach(function(item) {

      let option = document.createElement("option");

      option.value = item;

      option.text = item;

      subject.add(option);

    });

  }

  </script>

  </body>
  </html>
  )rawliteral";

    page.replace("STATUS_COLOR", statusColor);
    page.replace("STATUS_TEXT", statusText);
    page.replace("SELECTED_CATEGORY", selectedCategory);
    page.replace("SELECTED_SUBJECT", selectedSubject);

    server.send(200, "text/html", page);
  }

  // =================================================
  // 18. START ATTENDANCE
  // =================================================

  void handleStart() {

    if (!checkLogin()) return;

    String category = server.arg("category");

    String subject = server.arg("subject");

    if ((category != "Theory" &&
        category != "Practical") ||
        subject.length() == 0) {

      server.send(
        400,
        "text/plain",
        "Please select a valid category and subject."
      );

      return;
    }

    selectedCategory = category;

    selectedSubject = subject;

    attendanceActive = true;

    restoreDisplayPending = false;

    Serial.println();
    Serial.println("ATTENDANCE STARTED");

    Serial.println("Category: " + selectedCategory);

    Serial.println("Subject: " + selectedSubject);

    showMessage(
      "Attendance Active",
      selectedSubject
    );

    if (Blynk.connected()) {

      Blynk.virtualWrite(V5, selectedCategory);

      Blynk.virtualWrite(V6, selectedSubject);

      Blynk.virtualWrite(V1, "Session Active");
    }

    server.sendHeader("Location", "/");

    server.send(303);
  }

  // =================================================
  // 19. STOP ATTENDANCE
  // =================================================

  void handleStop() {

    if (!checkLogin()) return;

    attendanceActive = false;

    restoreDisplayPending = false;

    Serial.println();
    Serial.println("ATTENDANCE STOPPED");

    if (Blynk.connected()) {

      Blynk.virtualWrite(V1, "Session Stopped");
    }

    showQR();

    server.sendHeader("Location", "/");

    server.send(303);
  }

  // =================================================
  // 20. GET DATE AND TIME
  // =================================================

  bool getAttendanceTime(
    String &attendanceDate,
    String &attendanceTime
  ) {

    struct tm timeInfo;

    if (!getLocalTime(&timeInfo, 1000)) {

      return false;
    }

    char dateBuffer[11];

    char timeBuffer[9];

    strftime(
      dateBuffer,
      sizeof(dateBuffer),
      "%Y-%m-%d",
      &timeInfo
    );

    strftime(
      timeBuffer,
      sizeof(timeBuffer),
      "%H:%M:%S",
      &timeInfo
    );

    attendanceDate = String(dateBuffer);

    attendanceTime = String(timeBuffer);

    return true;
  }

  // =================================================
  // 21. SEND ATTENDANCE TO BLYNK
  // =================================================

  bool sendAttendance(
    String uid,
    String studentName,
    String status,
    String date,
    String timeValue
  ) {

    if (!Blynk.connected()) {

      Serial.println("Blynk not connected!");

      return false;
    }

    // Existing Blynk datastreams

    Blynk.virtualWrite(V0, uid);

    Blynk.virtualWrite(V1, status);

    Blynk.virtualWrite(V2, studentName);

    Blynk.virtualWrite(V3, date);

    Blynk.virtualWrite(V4, timeValue);

    // Subject datastreams

    Blynk.virtualWrite(V5, selectedCategory);

    Blynk.virtualWrite(V6, selectedSubject);

    // Attendance event

    if (status == "Present") {

      String eventMessage =
        "Name: " + studentName +
        ", UID: " + uid +
        ", Category: " + selectedCategory +
        ", Subject: " + selectedSubject +
        ", Date: " + date +
        ", Time: " + timeValue;

      Blynk.logEvent(
        "attendance_record",
        eventMessage
      );
    }

    Serial.println("Attendance sent to Blynk!");

    return true;
  }

  // =================================================
  // 22. RFID CARD PROCESSING
  // =================================================

  void processRFID() {

    // Print a heartbeat while attendance is active. This confirms
    // that loop() is reaching the RFID scanning function.
    if (millis() - lastRFIDDebugTime >= 2000) {
      Serial.println("RFID SCANNING ACTIVE - waiting for card...");
      lastRFIDDebugTime = millis();
    }

    // Check for a new RFID card.
    if (!rfid.PICC_IsNewCardPresent()) {
      return;
    }

    Serial.println("CARD DETECTED BY RC522!");

    if (!rfid.PICC_ReadCardSerial()) {
      Serial.println("Card detected, but UID reading failed!");
      return;
    }

    Serial.println("UID read successfully.");

    // Scan delay.
    if (millis() - lastScanTime < scanDelay) {
      Serial.println("Scan ignored because of scan delay.");
      rfid.PICC_HaltA();
      rfid.PCD_StopCrypto1();
      return;
    }

    lastScanTime = millis();

    // Read UID.
    String uid = "";

    for (byte i = 0; i < rfid.uid.size; i++) {
      if (rfid.uid.uidByte[i] < 0x10) {
        uid += "0";
      }
      uid += String(rfid.uid.uidByte[i], HEX);
    }

    uid.toUpperCase();

    Serial.println();
    Serial.println("RFID UID: " + uid);

    // Identify student.
    String studentName = getStudentName(uid);
    String status = (studentName == "Unknown") ? "Unknown" : "Present";

    Serial.println("Student: " + studentName);
    Serial.println("Status: " + status);

    // Duplicate attendance check.
    if (status == "Present" && isAlreadyMarked(uid, selectedSubject)) {
      Serial.println("DUPLICATE ATTENDANCE!");
      showMessage("Already Marked!", "Attendance already marked");
      tone(BUZZER_PIN, 1000, 500);

      rfid.PICC_HaltA();
      rfid.PCD_StopCrypto1();

      restoreDisplayPending = true;
      restoreDisplayAt = millis() + 2500;
      return;
    }

    // Get date and time.
    String attendanceDate;
    String attendanceTime;

    if (!getAttendanceTime(attendanceDate, attendanceTime)) {
      Serial.println("Time synchronization pending");
      showMessage("Time Not Ready", "Please scan again");

      rfid.PICC_HaltA();
      rfid.PCD_StopCrypto1();

      restoreDisplayPending = true;
      restoreDisplayAt = millis() + 2000;
      return;
    }

    // Show student result.
    if (status == "Present") {
      showMessage(studentName, "Attendance Present");
      tone(BUZZER_PIN, 2000, 200);
    } else {
      showMessage("Unknown Card", uid);
      tone(BUZZER_PIN, 1000, 700);
    }

    // Send attendance to Blynk.
    bool uploadSuccess = sendAttendance(
      uid,
      studentName,
      status,
      attendanceDate,
      attendanceTime
    );

    // Save duplicate record locally after Blynk is connected and writes are queued.
    if (uploadSuccess && status == "Present") {
      if (saveAttendanceRecord(uid, selectedSubject)) {
        Serial.println("Attendance successfully marked!");
      } else {
        Serial.println("Could not save attendance record!");
      }
    }

    // Finish RFID communication.
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();

    // Restore OLED display after showing the result.
    restoreDisplayPending = true;
    restoreDisplayAt = millis() + 2000;
  }

  // =================================================
  // 23. SETUP
  // =================================================

  void setup() {

    Serial.begin(115200);

    // Buzzer

    pinMode(BUZZER_PIN, OUTPUT);

    digitalWrite(BUZZER_PIN, LOW);

    // OLED initialization

    display.init();

    display.flipScreenVertically();

    qrcode.init();

    showMessage(
      "Student Attendance",
      "Starting..."
    );

    // RFID initialization

    SPI.begin(
      RFID_SCK,
      RFID_MISO,
      RFID_MOSI,
      RFID_SS
    );

    rfid.PCD_Init();
    delay(100);

    Serial.println("Checking RC522 reader...");
    rfid.PCD_DumpVersionToSerial();
    Serial.println("RC522 initialization check complete.");

    // Wi-Fi

    showMessage(
      "Connecting WiFi",
      "Please wait..."
    );

    WiFi.begin(ssid, pass);

    Serial.print("Connecting to WiFi");

    while (WiFi.status() != WL_CONNECTED) {

      delay(500);

      Serial.print(".");
    }

    Serial.println();

    Serial.println("WiFi Connected");

    // Time configuration

    configTime(
      GMT_OFFSET,
      DAYLIGHT_OFFSET,
      "pool.ntp.org",
      "time.nist.gov"
    );

    // Blynk configuration

    Blynk.config(BLYNK_AUTH_TOKEN);

    Serial.println("Connecting to Blynk...");

    if (Blynk.connect(10000)) {

      Serial.println("Blynk Connected!");

    } else {

      Serial.println("Blynk connection pending.");
    }

    // Web server routes

    server.on("/", HTTP_GET, handleRoot);

    server.on("/start", HTTP_POST, handleStart);

    server.on("/stop", HTTP_POST, handleStop);

    server.begin();

    Serial.println("Web server started");

    // Dashboard URL

    dashboardURL =
      "http://" + WiFi.localIP().toString();

    Serial.print("Dashboard URL: ");

    Serial.println(dashboardURL);

    // Display QR code

    showQR();

    Serial.println("SYSTEM READY");

    Serial.println("Waiting for teacher permission.");
    Serial.println("RFID will scan only after START ATTENDANCE.");

    attendanceActive = false;
  }

  // =================================================
  // 24. MAIN LOOP
  // =================================================

  void loop() {

    // Always service web server

    server.handleClient();

    // Always service Blynk

    Blynk.run();

    // Reconnect Blynk if needed

    if (WiFi.status() == WL_CONNECTED &&
        !Blynk.connected() &&
        millis() - lastBlynkAttempt >= 10000) {

      lastBlynkAttempt = millis();

      Blynk.connect(1000);
    }

    // Restore OLED after scan result

    if (restoreDisplayPending &&
        (long)(millis() - restoreDisplayAt) >= 0) {

      restoreDisplay();
    }

    // RFID permission control

    if (!attendanceActive) {

      return;
    }

    // RFID processing only after START

    processRFID();
  }
