// Include the libraries we need
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SD.h>
#include <SPI.h>

File myFile;
int pinCS = 3; // Pin 10 on Arduino Uno

// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS 2
#define TEMPERATURE_PRECISION 11
#define DEVICE_COUNT_MAX 16

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

// arrays to hold device addresses
DeviceAddress address[DEVICE_COUNT_MAX];

// File name
String File_Name = "Log";
int File_No = 1;

// number of sensors
int deviceCount;

// last sample
unsigned long lastSampleTimeUS;

void setup(void)
{
  // start serial port
  Serial.begin(115200);
  pinMode(pinCS, OUTPUT);

  // SD Card Initialization
  if(SD.begin()) {
    Serial.println("SD card is ready to use.");
    while(SD.exists(getFileName(File_Name, File_No))){
      File_No += 1;
    }
    File_Name = getFileName(File_Name, File_No);
    Serial.println("Log File is " + File_Name);
  }else{
    Serial.println("SD card initialization failed");
    File_No = 0;
  }

  Serial.println("Dallas Temperature IC Control Library");

  // Start up the library
  sensors.begin();

  findDevices();
}

void saveStringToFile(String Line)
{
  if(File_No > 0){
    // Create/Open file
    myFile = SD.open(File_Name, FILE_WRITE);
  
    // if the file opened okay, write to it:
    if (myFile) {
      // Write to file
      myFile.println(Line);
      myFile.close(); // close the file
    } else {
      // if the file didn't open, print an error:
      Serial.println("error opening " + File_Name);
    }
  }
}

String getFileName(String Name, int Number)
{
  Name += "_";
  if(Number<100){
    Name += "0";
  }
  if(Number<10){
    Name += "0";
  }
  return Name + String(File_No, DEC) + ".txt";
}

void findDevices(void)
{
  // locate devices on the bus
  Serial.println("Locating devices...");
  
  deviceCount = sensors.getDeviceCount();
  String NewLine = "Found " + String(deviceCount, DEC) + " devices.";
  Serial.println(NewLine);
  saveStringToFile(NewLine);

  // report parasite power requirements
  Serial.print("Parasite power is: ");
  if (sensors.isParasitePowerMode()) Serial.println("ON");
  else Serial.println("OFF");

  // method 1: by index
  uint8_t count = 0;
  while (sensors.getAddress(address[count], count)) {
    // set the resolution to 9 bit per device
    sensors.setResolution(address[count], TEMPERATURE_PRECISION);
    NewLine = "Device: " + String(count);
    NewLine += ",  Address: " + printAddress(address[count]);
    NewLine += ", Resolution: " + printResolution(address[count]);
    Serial.println(NewLine);
    saveStringToFile(NewLine);
    count++;
  }
}

/*
   Main function, calls the temperatures in a loop.
*/
void loop(void)
{
  // call sensors.requestTemperatures() to issue a global temperature
  // request to all devices on the bus

  // Requesting temperatures...
  sensors.requestTemperatures();
  String NewLine = printOutputLine();
  Serial.println(NewLine);
  saveStringToFile(NewLine);
}

// function to print a device address
String printOutputLine(void)
{
  String Output = "";
  //Output += String(micros() - lastSampleTimeUS);
  //Output += ", ";
  lastSampleTimeUS = micros();
  Output += String(lastSampleTimeUS);

  // print the device information
  for (uint8_t i = 0; i < deviceCount; i++)
  {
    Output += ", ";
    Output += printTemperature(address[i]);
  }
  return Output;
}

// function to print a device address
String printAddress(DeviceAddress deviceAddress)
{
  String Output = "";
  for (uint8_t i = 0; i < 8; i++)
  {
    // zero pad the address if necessary
    if (deviceAddress[i] < 16)
    {
      Output = String(Output + "0");
    }
    Output += String(deviceAddress[i], HEX);
  }
  return Output;
}

// function to print the temperature for a device
String printTemperature(DeviceAddress deviceAddress)
{
  float tempC = sensors.getTempC(deviceAddress);
  return String(tempC, 3);
}

// function to print a device's resolution
String printResolution(DeviceAddress deviceAddress)
{
  String Output = "Resolution: ";
  Output += String(sensors.getResolution(deviceAddress));
  return Output;
}

// main function to print information about a device
String printData(DeviceAddress deviceAddress)
{
  String Output = "Device Address: ";
  Output += printAddress(deviceAddress);
  Output += ", ";
  Output += printTemperature(deviceAddress);
  return Output;
}
