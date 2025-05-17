#include "mbed.h"


// SETUP: Pins and Hardware


DigitalOut latchPin(D4);      // Latch for 7-segment
DigitalOut clkPin(D7);        // Clock for 7-segment
DigitalOut dataPin(D8);       // Data for 7-segment

DigitalIn resetBtn(A1);       // Button to reset time
DigitalIn modeBtn(A3);        // Button to change display

AnalogIn sensor(A0);          // Potentiometer (analog input)

Timer myTimer;
Ticker screenUpdater;


// GLOBAL VALUES


// Segment codes (common anode - inverted)
const char numbers[10] = {
    0xC0, 0xF9, 0xA4, 0xB0,
    0x99, 0x92, 0x82, 0xF8,
    0x80, 0x90
};

// Which digit to turn on
const char digitPins[4] = { 0x08, 0x04, 0x02, 0x01 };

int mins = 0;
int secs = 0;
int digitNow = 0;


// SEND DATA TO DISPLAY


void sendByte(char val) {
    for (int i = 7; i >= 0; i--) {
        clkPin = 0;
        dataPin = (val >> i) & 1;
        clkPin = 1;
    }
}


// DISPLAY FUNCTION


void refreshDisplay() {
    if (modeBtn.read() == 0) {
        // Show voltage
        float volts = sensor.read() * 3.3f;

        int v[4] = {
            (int)(volts * 10) % 10,
            (int)(volts * 100) / 10 % 10,
            (int)(volts) % 10,
            (int)(volts * 10) / 10
        };

        latchPin = 0;
        sendByte(numbers[v[digitNow]]);
        sendByte(digitPins[digitNow]);
        latchPin = 1;
    } else {
        // Show time
        auto timePassed = myTimer.elapsed_time();
        int totalSecs = chrono::duration_cast<chrono::seconds>(timePassed).count();
        mins = totalSecs / 60;
        secs = totalSecs % 60;

        int t[4] = {
            secs % 10,
            secs / 10,
            mins % 10,
            mins / 10
        };

        latchPin = 0;
        sendByte(numbers[t[digitNow]]);
        sendByte(digitPins[digitNow]);
        latchPin = 1;
    }

    digitNow = (digitNow + 1) % 4;
}


// MAIN PROGRAM


int main() {
    resetBtn.mode(PullUp);
    modeBtn.mode(PullUp);

    myTimer.start();
    screenUpdater.attach(&refreshDisplay, 3ms);

    while (true) {
        if (resetBtn.read() == 0) {
            myTimer.reset();
            mins = 0;
            secs = 0;
            ThisThread::sleep_for(500ms);
        }
    }
}
