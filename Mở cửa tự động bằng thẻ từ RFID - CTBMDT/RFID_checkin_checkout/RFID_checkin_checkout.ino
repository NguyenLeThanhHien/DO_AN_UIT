#include <vector>
#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ESP32Servo.h>
#include <WiFi.h>
#include <WebServer.h>
#include <time.h>

#define RST_PIN 0         // Configurable, see typical pin layout above
#define SS_PIN 5          // Configurable, see typical pin layout above
#define PIN_SG90 14
#define buzzer_PIN 2

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance
Servo sg90;

struct StudentInfo {
    String School;
    String Name;
    String studentID;
    String Time; // Thời gian quét thẻ
};
std::vector<StudentInfo> acceptedStudents; // List of accepted students
std::vector<StudentInfo> unacceptedStudents; // List of unaccepted students

const char* ssid = "QuocBao";
const char* password = "0123456789";
WebServer server(80);

const char targetStudentID[] = "2252";  
String School = "";
String Name = "";
String studentID = "";
volatile bool newCardDetected = false; // Flag to track new card detection

void playTone(int pin, int frequency, int duration) {
    int period = 1000000 / frequency; // Period in microseconds
    int halfPeriod = period / 2;

    unsigned long startTime = millis();
    while (millis() - startTime < duration) {
        digitalWrite(pin, HIGH);
        delayMicroseconds(halfPeriod);
        digitalWrite(pin, LOW);
        delayMicroseconds(halfPeriod);
    }
}

void handleRoot() {
    String html = "<html><head><title>RFID Access System</title>";
    html += "<script>";
    html += "setInterval(() => { ";
    html += "  fetch('/check') ";
    html += "    .then(response => response.json()) ";
    html += "    .then(data => { if (data.newCard) location.reload(); }); ";
    html += "}, 1000);"; // Check every second
    html += "</script>";

    // Thêm CSS vào thẻ <style>
    html += "<style>";
    html += "body { font-family: 'Segoe UI', Consolas, sans-serif; text-align: center; background-color: #f4f4f9; }";
    html += "h1, h2 { color: #333; }";
    html += ".status-box { width: 80%; margin: 20px auto; padding: 20px; border-radius: 10px; color: white; font-size: 24px; font-weight: bold; }";
    html += ".gray { background-color: #9e9e9e; }";
    html += ".green { background-color: #4caf50; }";
    html += ".red { background-color: #f44336; }";
    html += "table { width: 90%; margin: 20px auto; border-collapse: collapse; box-shadow: 0 2px 5px rgba(0, 0, 0, 0.1); font-size: 16px; }";
    html += "th, td { border: 1px solid #ddd; padding: 12px; text-align: center; }";
    html += "th { background-color: #3f51b5; color: white; font-size: 18px; }";
    html += "tr:nth-child(even) { background-color: #f2f2f2; }";
    html += "tr:hover { background-color: #ddd; }";
    html += "a { display: inline-block; margin: 20px; padding: 10px 20px; text-decoration: none; color: white; background-color: #007BFF; border-radius: 5px; font-size: 18px; }";
    html += "a:hover { background-color: #0056b3; }";
    html += "</style>";
    html += "</head><body>";
    html += "<h1>RFID Access Control</h1>";

    html += "<p><b>Current Card Info:</b></p>";
    html += "<p><b>Name:</b>  " + Name + "</p>";
    html += "<p><b>School:</b>  " + School + "</p>";
    html += "<p><b>Student ID:</b> " + studentID + "</p>";
    // Trạng thái của hệ thống khi quét thẻ
    if (studentID.substring(0,4) == "") {
        html += "<div class='status-box gray'>SCAN YOUR CARD HERE</div>";
    } else if (studentID.substring(0,4) == targetStudentID) {
        html += "<div class='status-box green'>ACCESS GRANTED</div>";
        html += "<p><b>Status:</b> DOOR OPENED</p>";
    } else {
        html += "<div class='status-box red'>ACCESS DENIED</div>";
        html += "<p><b>Status:</b> DOOR CLOSED</p>";
    }

    // Bảng Accepted Students
    html += "<h2>Accepted Students</h2>";
    html += "<table><tr><th>School</th><th>Name</th><th>Student ID</th><th>Time</th></tr>";
    for (const auto& student : acceptedStudents) {
        html += "<tr>";
        html += "<td>" + student.School + "</td>";
        html += "<td>" + student.Name + "</td>";
        html += "<td>" + student.studentID + "</td>";
        html += "<td>" + student.Time + "</td>";
        html += "</tr>";
    }
    html += "</table>";

    // Bảng Unaccepted Students
    html += "<h2>Unaccepted Students</h2>";
    html += "<table><tr><th>School</th><th>Name</th><th>Student ID</th><th>Time</th></tr>";
    for (const auto& student : unacceptedStudents) {
        html += "<tr>";
        html += "<td>" + student.School + "</td>";
        html += "<td>" + student.Name + "</td>";
        html += "<td>" + student.studentID + "</td>";
        html += "<td>" + student.Time + "</td>";
        html += "</tr>";
    }
    html += "</table>";

    // Thêm link download CSV
    html += "<a href='/export' download='students_data.csv'>Download Students Data as CSV</a>";

    html += "</body></html>";

    // Gửi nội dung HTML đến trình duyệt
    server.send(200, "text/html", html);
}

void handleExport() {
    String csv = "RFID Access System\n\n"; // Title
    csv += "Category,School,Name,Student ID,Scan Time (GMT+7)\n"; // Header with clear column titles

    // Append data for Accepted Students
    for (const auto& student : acceptedStudents) {
        csv += "Accepted,";  // Category
        csv += student.School + ",";  // School
        csv += student.Name + ",";  // Name
        csv += student.studentID + ",";  // Student ID
        csv += student.Time + "\n";  // Scan Time
    }

    // Append data for Unaccepted Students
    for (const auto& student : unacceptedStudents) {
        csv += "Unaccepted,";  // Category
        csv += student.School + ",";  // School
        csv += student.Name + ",";  // Name
        csv += student.studentID + ",";  // Student ID
        csv += student.Time + "\n";  // Scan Time
    }

    // Send the formatted CSV file to the client
    server.send(200, "text/csv", csv);
}

void setup() {
    Serial.begin(9600);
    SPI.begin();
    mfrc522.PCD_Init();
    Serial.println(F("RFID Access Control System"));

    pinMode(buzzer_PIN, OUTPUT);
    sg90.attach(PIN_SG90,500,2400);
    sg90.setPeriodHertz(50); 
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected.");
    Serial.println(WiFi.localIP());

    server.on("/", handleRoot);
    server.on("/export", handleExport); // Endpoint để xuất dữ liệu dạng CSV
    server.begin();
    Serial.println("HTTP server started.");

    // Thiết lập thời gian thực (NTP server)
    configTime(7*3600, 0, "pool.ntp.org", "time.nist.gov");
    Serial.println("Waiting for NTP time sync...");
    while (!time(nullptr)) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("Time synchronized.");
    server.on("/check", []() {
    String json = "{\"newCard\":" + String(newCardDetected ? "true" : "false") + "}";
    newCardDetected = false; // Reset the flag after sending
    server.send(200, "application/json", json);
});

}

void loop() {
    server.handleClient();

    if (!mfrc522.PICC_IsNewCardPresent()) {
        return;
    }

    if (!mfrc522.PICC_ReadCardSerial()) {
        return;
    }

    // Mark new card detected
    newCardDetected = true;

    // Process RFID data (same as before)
    mfrc522.PICC_DumpDetailsToSerial(&(mfrc522.uid));

    byte buffer1[18], buffer2[18], buffer3[18];
    byte block;
    byte len;
    MFRC522::StatusCode status;
    MFRC522::MIFARE_Key key;
    for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

    // Get First Name
    block = 4;
    len = 18;
    status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
    if (status == MFRC522::STATUS_OK && mfrc522.MIFARE_Read(block, buffer1, &len) == MFRC522::STATUS_OK) {
        School = "";
        for (uint8_t i = 0; i < 16; i++) {
            if (buffer1[i] != 32) School += (char)buffer1[i];
        }
    } else {
        School = "Unknown";
    }

    // Get Last Name
    block = 1;
    status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
    if (status == MFRC522::STATUS_OK && mfrc522.MIFARE_Read(block, buffer2, &len) == MFRC522::STATUS_OK) {
        Name = "";
        for (uint8_t i = 0; i < 16; i++) {
            Name += (char)buffer2[i];
        }
    } else {
        Name = "Unknown";
    }

    // Get Student ID
    block = 8;
    status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
    if (status == MFRC522::STATUS_OK && mfrc522.MIFARE_Read(block, buffer3, &len) == MFRC522::STATUS_OK) {
        studentID = "";
        for (uint8_t i = 0; i < 16; i++) {
            if (buffer3[i] != 32) studentID += (char)buffer3[i];
        }
    } else {
        studentID = "Unknown";
    }

    // Access Control
    // Access Control - Toggle Check-in/Check-out
    
if (studentID.substring(0, 4) == targetStudentID) {
    auto it = std::find_if(acceptedStudents.begin(), acceptedStudents.end(), [&](const StudentInfo& student) {
        return student.studentID == studentID;
    });

    if (it != acceptedStudents.end()) {
        // Student is already checked in, remove them to indicate check-out
        playTone(buzzer_PIN, 987, 200); // Check-out tone
        acceptedStudents.erase(it); // Remove student from the list
        sg90.write(1800); // Open the door for exit
        delay(8000);
        sg90.write(500);  // Close the door
        Serial.println("Checked out: " + studentID);
    } else {
        // Student is not checked in, add them to indicate check-in
        playTone(buzzer_PIN, 1318, 100); // Check-in tone
        sg90.write(1800); // Open the door for entry
        delay(8000);
        sg90.write(500);  // Close the door

        // Get the current time
        time_t now = time(nullptr);
        struct tm* localTime = localtime(&now);
        char timeBuffer[20];
        strftime(timeBuffer, sizeof(timeBuffer), "%Y-%m-%d %H:%M:%S", localTime);

        // Add student to the list
        StudentInfo newStudent = {School, Name, studentID, String(timeBuffer)};
        acceptedStudents.push_back(newStudent);
        Serial.println("Checked in: " + studentID);
    }
} else {
    // Access Denied Logic
    playTone(buzzer_PIN, 400, 1000); // Error tone

    auto it = std::find_if(unacceptedStudents.begin(), unacceptedStudents.end(), [&](const StudentInfo& student) {
        return student.studentID == studentID;
    });

    if (it == unacceptedStudents.end()) {
        // Get the current time
        time_t now = time(nullptr);
        struct tm* localTime = localtime(&now);
        char timeBuffer[20];
        strftime(timeBuffer, sizeof(timeBuffer), "%Y-%m-%d %H:%M:%S", localTime);

        // Add student to the unaccepted list
        StudentInfo newStudent = {School, Name, studentID, String(timeBuffer)};
        unacceptedStudents.push_back(newStudent);
        Serial.println("Access Denied: " + studentID);
    }
}

    delay(1000);
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
}
