#include <DFRobotDFPlayerMini.h>
#include <SoftwareSerial.h>
#include <RTClib.h> // RTC library

// Initialize MP3 Player and RTC module
SoftwareSerial mp3Serial(11, 10); // RX, TX for DFPlayer
DFRobotDFPlayerMini myDFPlayer;
RTC_DS3231 rtc; // RTC object for DS3231

// Staff structure
struct Staff {
    String id;
    String name;
};

// Staff database
Staff staffDatabase[] = {
    {"6438409085337", "Shyam Sundhar N"},
    {"10420018", "Kirubahari"},
    {"214007", "Sasikumar P"},
    {"214044", "Sivaprakash S"}
};

const int maxStaffCount = sizeof(staffDatabase) / sizeof(staffDatabase[0]);
unsigned long startTime;
bool scannedBeforeTimeout = false;
bool isPlayingAudio = false; // Flag to track audio playback state

// Period management
unsigned long periodStartTime;
const unsigned long periodDuration = 3000000; // 50 minutes in milliseconds
const unsigned long checkInterval = 180000; // 3 minutes in milliseconds
int currentPeriod = 0;

void setup() {
    Serial.begin(9600); // Start serial for debugging
    mp3Serial.begin(9600);
    
    // Initialize DFPlayer
    if (!myDFPlayer.begin(mp3Serial)) {
        Serial.println("DFPlayer Mini not found");
        while (true); // Loop indefinitely if DFPlayer is not found
    }
    myDFPlayer.volume(20); // Set volume level

    // Initialize RTC
    if (!rtc.begin()) {
        Serial.println("Couldn't find RTC");
        while (true); // Halt if RTC is not found
    }
    
    if (rtc.lostPower()) {
        Serial.println("RTC lost power, setting the time!");
        rtc.adjust(DateTime(2024, 10, 28, 9, 16, 17)); // Set manually to the current real time once
    }

    // Initialize period management
    periodStartTime = millis();
    Serial.println("System Initialized. Please scan your ID within 3 minutes."); // Initial message
}

void loop() {
    monitorAttendance(); // Continuously monitor attendance
    checkForTimeout();   // Check if 3 minutes have passed
    checkForPeriodChange(); // Check for period change every 50 minutes
}

// Function to monitor attendance
void monitorAttendance() {
    static String scannedID = ""; // Use static variable to maintain state across loop iterations
    while (Serial.available() > 0) {
        char c = Serial.read();
        // Collect characters until a newline or carriage return
        if (c == '\n' || c == '\r') {
            // Process the scanned ID if it has been populated
            if (scannedID.length() > 0) {
                Serial.print("Scanned ID: "); // Print the scanned ID to Serial Monitor
                Serial.println(scannedID); // Output the scanned ID

                // Process the scanned ID
                int staffIndex = getStaffIndex(scannedID); // Get the index of the scanned ID
                if (staffIndex != -1) { // Valid staff member found
                    Serial.print("Matched Staff: "); // Print matched staff name
                    Serial.println(staffDatabase[staffIndex].name);
                    
                    // Attempt to play the logged audio (001.mp3)
                    Serial.println("Playing audio 001.mp3"); // Debug print
                    myDFPlayer.play(2); // Play logged audio for a successful match (001.mp3)
                    waitForAudioToFinish(); // Wait for the audio to complete
                    
                    // Get the current time from RTC
                    String currentTime = getCurrentTimeString();

                    // Output attendance data in the desired format
                    Serial.print(scannedID);
                    Serial.print(", ");
                    Serial.print(staffDatabase[staffIndex].name);
                    Serial.print(", Present, ");
                    Serial.println(currentTime); // Print the time with AM/PM
                } else {
                    Serial.println("Error: ID not matched"); // Print error message for no match
                    myDFPlayer.play(1); // Play error audio (004.mp3)
                    waitForAudioToFinish(); // Wait for error audio to complete
                }
                scannedID = ""; // Reset for next scan
                scannedBeforeTimeout = true; // Mark that we scanned before timeout
            }
        } else {
            // Append character to scannedID
            if (isPrintable(c)) { // Only append printable characters
                scannedID += c;
            }
        }
    }
}

// Function to check if the scan timeout has occurred
void checkForTimeout() {
    unsigned long currentTime = millis();
    
    // Check for timeout only if there has been no scan
    if (!scannedBeforeTimeout && (currentTime - startTime >= checkInterval)) { // 180000 ms = 3 minutes
        Serial.println("No scan detected in 3 minutes. Playing alert audio (002.mp3)");
        myDFPlayer.play(3); // Play alert audio for timeout (002.mp3)
        waitForAudioToFinish(); // Wait for alert audio to complete
        
        // Reset the start time to play the timeout audio every 3 minutes
        startTime = currentTime; // Keep the timer running
    }

    // Reset the scannedBeforeTimeout flag after timeout audio is played
    if (isPlayingAudio && (currentTime - startTime >= 3000)) { // Wait for 3 seconds after playing audio
        scannedBeforeTimeout = false; // Reset flag to allow for future timeouts
    }
}

// Function to check if the period has changed
void checkForPeriodChange() {
    unsigned long currentTime = millis();
    
    if (currentTime - periodStartTime >= periodDuration) { // 50 minutes
        // Play audio indicating the start of the next period
        Serial.println("Starting next period. Playing audio 003.mp3");
        myDFPlayer.play(4); // Play audio for starting next period (003.mp3)
        waitForAudioToFinish(); // Wait for audio to complete

        // Reset period start time and increment period count
        periodStartTime = currentTime;
        currentPeriod++;
        
        // Check for the end of the work day
        if (currentPeriod >= 6) { // 6 periods from 9 AM to 4 PM
            Serial.println("End of the work day.");
            while (true); // Halt the system or reset as needed
        }

        // Reset the scannedBeforeTimeout flag for the new period
        scannedBeforeTimeout = false;
        startTime = currentTime; // Reset start time for the 3-minute check
    }
}

// Function to get index of staff in the database
int getStaffIndex(String scannedID) {
    for (int i = 0; i < maxStaffCount; i++) {
        if (staffDatabase[i].id == scannedID) {
            return i; // Return the index if a match is found
        }
    }
    return -1; // Return -1 if no match is found
}

// Function to wait until audio playback has finished
void waitForAudioToFinish() {
    isPlayingAudio = true; // Set the flag when starting to play audio
    while (myDFPlayer.readState() == 2) { // 2 means it's playing
        delay(10); // Small delay to avoid excessive looping
    }
    isPlayingAudio = false; // Reset flag when audio is done playing
}

// Function to get current time as a formatted string from RTC
String getCurrentTimeString() {  
    DateTime now = rtc.now(); // Get the current time from RTC

    int hour = now.hour() % 12; // Convert to 12-hour format
    if (hour == 0) hour = 12; // Handle midnight and noon
    int minute = now.minute();

    // Determine AM or PM
    String ampm = (now.hour() >= 12) ? "PM" : "AM";

    // Format the time as HH:MM AM/PM
    char timeString[10];
    sprintf(timeString, "%02d:%02d %s", hour, minute, ampm.c_str());

    return String(timeString); // Return the formatted time string
}