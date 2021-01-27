/* LOAD THIS SKETCH ON CPU SPEED 8mhz
rev6 - for small version only
*Data write pin change to pin 8
*Log data only when the speed is above 3km/h
*Using var usb_configuration; to detect and enable charging automatically when the board is plugged on a computer
*/

#include <AltSoftSerial.h> // Better than old lib
#include <TinyGPS.h>
#include <SD.h> // Standard Arduino SD card library
#include <SPI.h>

File myFile;
const int buttonPin = 17; // Declare PIN 17 for charging control
int buttonState = 0;  // State button int

#define datawrite 8 // Write data from GPS
#define pin16 16  // Signal driving transistor for batt charger

TinyGPS gps;
AltSoftSerial nss; //use default pin for Teensy 2.0 check pjrc.com/teensy
char filename[] = "GPSLOG00.CSV";
boolean logg = true;

extern volatile uint8_t usb_configuration; //Using var usb_configuration; to detect and enable charging automatically when the board is plugged on a computer

void setup() 
{
  //Serial.begin(57600);
  ADCSRA = 0;   // shut off ADC
  nss.begin(4700);
  nss.print("$PMTK220,5000*1F"); //Set NMEA port update rate, 5sec
    
  pinMode(datawrite, OUTPUT);
  pinMode(pin16, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP); // Set input buttonpin & pull-up on analog pin 17
  
 // Initialize SD card
    pinMode(0, OUTPUT);
    if (!SD.begin(0)) 
    {
    logg = false;               //turns off if card not present
}
  if (logg==true)
  {                            // check SD   
    for (uint8_t i = 0; i < 100; i++) {
      filename[6] = i/10 + '0';
      filename[7] = i%10 + '0';
      if (! SD.exists(filename)) {                // open a new file if it doesn't exist        
        myFile = SD.open(filename, FILE_WRITE);  // Create a new file
  // At every power up, we create a new CSV file whith the fields bellow
  if (myFile) 
  {
    myFile.print("Latitude,");
    myFile.print("Longitude,");
    myFile.print("Speed,");
    myFile.print("Altitude,");
    myFile.print("Date,");
    myFile.println("Time");
    myFile.close();
    // Then, close it
        break;
      }
    }
  }
}
}

void loop() 
{
charging();

float maxSpeed = 3; // Default 3km/h

bool newdata = false;               // switch to 'true' argument to debug, you will see "blank" data
unsigned long start = millis();
while (millis() - start < 1000) 
  {
if (feedgps())
newdata = true;                     // switch to 'false' argument to debug, you will see "blank" data
  }
if (newdata) 
  {
      if (gps.f_speed_kmph() > maxSpeed ) // Log data only when the speed is above 3km/h
    {
      maxSpeed = gps.f_speed_kmph();  
gpsdump(gps);
blinkled(); 
  }
}
}

// Feed data as it becomes available 
bool feedgps()
{
  while (nss.available()) 
  {
    if (gps.encode(nss.read()))
      return true;
  }
  return false;
}

//Charger control
void charging()
{
if (usb_configuration == 1)
    {
    digitalWrite(pin16, HIGH); // Enabling charging mode to LTC4054
    } 
    else if     (buttonState = digitalRead(buttonPin)); // Read button state
    if (buttonState == LOW) 
    {
    digitalWrite(pin16, HIGH); // Enabling charging mode to LTC4054
    } 
}

// Get and process GPS data, Add/Remove what you want, see arduiniana.org/libraries/tinygps
void gpsdump(TinyGPS &gps) 
{
  
  float flat, flon;                  // Lat + Long
  float fkmph = gps.f_speed_kmph();  // speed in km/hr
  float falt = gps.f_altitude();    // +/- altitude in meters
  
  unsigned long age;
  int year;
  byte month, day, hour, minute, second;
  gps.crack_datetime(&year, &month, &day, &hour, &minute, &second);
  gps.f_get_position(&flat, &flon, &age);

  // And write it to SD card
  // if the file opened okay, write to it:
  myFile = SD.open(filename, FILE_WRITE);
  if (myFile) 
  {
    myFile.print(flat, 4); // latitude
    myFile.print(",");
    myFile.print(flon, 4); // longitude
    myFile.print(",");
    myFile.print(fkmph, 2); // K/MS
    myFile.print(",");
    myFile.print(falt);  // Altitude
    myFile.print(",");   
    myFile.print(static_cast<int>(day));
    myFile.print(":");
    myFile.print(static_cast<int>(month)); 
    myFile.print(":"); 
    myFile.print(year);
    myFile.print(",");
    myFile.print(static_cast<int>(hour)); //use GMT+2 in my region, adjust as your local/GMT time
    myFile.print(":"); 
    myFile.print(static_cast<int>(minute)); 
    myFile.print(":"); 
    myFile.println(static_cast<int>(second));
    myFile.close();     // close the file:
    }
  }
  
// Blink a LED everytime a sentence has been writed
void blinkled()
{
    digitalWrite(datawrite, HIGH);
    delay(50);
    digitalWrite(datawrite, LOW);
    delay(50);
}
