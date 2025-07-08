// Last Updated July 8th, 2025

// ---- Basic Comments ----
  //  https://www.circuitbasics.com/writing-data-to-files-on-an-sd-card-on-arduino/
  //  * For more info see file README.md in this library or on URL:
  //  * https://github.com/espressif/arduino-esp32/tree/master/libraries/SD
  //  */
  //  https://www.circuitbasics.com/writing-data-to-files-on-an-sd-card-on-arduino/
  //  * For more info see file README.md in this library or on URL:
  //  * https://github.com/espressif/arduino-esp32/tree/master/libraries/SD
  //  */

// ------------------------

// --- Includes--------------------
  #include "FS.h"
  #include "SD.h"
  #include "SPI.h"
  #include <time.h>

  // New includes
  #include <Arduino.h>
  #include <SensirionI2cSen66.h>
  #include <time.h>
  #include <Wire.h>
  //#include <TFT_eSPI.h>
  #include <string.h>
  #include <ESP32Time.h>
  ESP32Time rtc(0); // the 0 is the offset. im not sure what it does, exactly. assuming it means offset from GMT but does not effect the code rn
                    // Change later.
                    //https://github.com/fbiego/ESP32Time
// --------------------------------

// --- Definitions/Declarations ---------------------------
  /*
      MODIFICATIONS:
      Declared the pins (22, 27)
  */
  #define I2C_SDA 22
  #define I2C_SCL 27

  // macro definitions
  // make sure that we use the proper definition of NO_ERROR
  #ifdef NO_ERROR
  #undef NO_ERROR
  #endif
  #define NO_ERROR 0
  String serialBuffer = "";
  SensirionI2cSen66 sensor;
// --------------------------------------------------------

// --- Instantiations ---------
  static char errorMessage[64];
  static int16_t error;

  float massConcentrationPm1p0;
  float massConcentrationPm2p5;
  float massConcentrationPm4p0;
  float massConcentrationPm10p0;
  float ambientHumidity;
  float ambientTemperature;
  float vocIndex;
  float noxIndex;
  uint16_t co2;

  int fn;

  //String arc 

  File myFile;
// ----------------------------

/*
Uncomment and set up if you want to use custom pins for the SPI communication
#define REASSIGN_PINS
int sck = -1;
int miso = -1;
int mosi = -1;
int cs = -1;
*/
// --- Methods for SD card ----------------------------------------


  void listDir(fs::FS &fs, const char *dirname, uint8_t levels) {
    Serial.println(dirname);

    File root = fs.open(dirname);
    if (!root) {
      Serial.println("Failed to open directory");
      return;
    }
    if (!root.isDirectory()) {
      Serial.println("Not a directory");
      return;
    }

    File file = root.openNextFile();
    while (file) {
      if (file.isDirectory()) {
        Serial.print("  DIR : ");
        Serial.println(file.name());
        if (levels) {
          listDir(fs, file.path(), levels - 1);
        }
      } else {
        Serial.print("  FILE: ");
        Serial.print(file.name());
        Serial.print("  SIZE: ");
        Serial.println(file.size());
      }
      file = root.openNextFile();
    }
  }

  void createDir(fs::FS &fs, const char *path) {
    Serial.printf("Creating Dir: %s\n", path);
    if (fs.mkdir(path)) {
      Serial.println("Dir created");
    } else {
      Serial.println("mkdir failed");
    }
  }

  void removeDir(fs::FS &fs, const char *path) {
    Serial.printf("Removing Dir: %s\n", path);
    if (fs.rmdir(path)) {
      Serial.println("Dir removed");
    } else {
      Serial.println("rmdir failed");
    }
  }

  void readFile(fs::FS &fs, const char *path) {
    Serial.printf("Reading file: %s\n", path);

    File file = fs.open(path);
    if (!file) {
      Serial.println("Failed to open file for reading");
      return;
    }

    Serial.print("Read from file: ");
    while (file.available()) {
      Serial.write(file.read());
    }
    file.close();
  }

  void writeFile(fs::FS &fs, const char *path, const char *message) {
    Serial.printf("Writing file: %s\n", path);

    File file = fs.open(path, FILE_WRITE);
    if (!file) {
      Serial.println("Failed to open file for writing");
      return;
    }
    if (file.print(message)) {
      Serial.println("File written");
    } else {
      Serial.println("Write failed");
    }
    file.close();
  }

  void appendFile(fs::FS &fs, const char *path, const char *message) {
    Serial.printf("Appending to file: %s\n", path);

    File file = fs.open(path, FILE_APPEND);
    if (!file) {
      Serial.println("Failed to open file for appending");
      return;
    }
    if (file.print(message)) {
      Serial.println("Message appended");
    } else {
      Serial.println("Append failed");
    }
    file.close();
  }

  void renameFile(fs::FS &fs, const char *path1, const char *path2) {
    Serial.printf("Renaming file %s to %s\n", path1, path2);
    if (fs.rename(path1, path2)) {
      Serial.println("File renamed");
    } else {
      Serial.println("Rename failed");
    }
  }

  void deleteFile(fs::FS &fs, const char *path) {
    Serial.printf("Deleting file: %s\n", path);
    if (fs.remove(path)) {
      Serial.println("File deleted");
    } else {
      Serial.println("Delete failed");
    }
  }

  void testFileIO(fs::FS &fs, const char *path) {
    File file = fs.open(path);
    static uint8_t buf[512];
    size_t len = 0;
    uint32_t start = millis();
    uint32_t end = start;
    if (file) {
      len = file.size();
      size_t flen = len;
      start = millis();
      while (len) {
        size_t toRead = len;
        if (toRead > 512) {
          toRead = 512;
        }
        file.read(buf, toRead);
        len -= toRead;
      }
      end = millis() - start;
      Serial.printf("%u bytes read for %lu ms\n", flen, end);
      file.close();
    } else {
      Serial.println("Failed to open file for reading");
    }

    file = fs.open(path, FILE_WRITE);
    if (!file) {
      Serial.println("Failed to open file for writing");
      return;
    }

    size_t i;
    start = millis();
    for (i = 0; i < 2048; i++) {
      file.write(buf, 512);
    }
    end = millis() - start;
    Serial.printf("%u bytes written for %lu ms\n", 2048 * 512, end);
    file.close();
  }
// ----------------------------------------------------------------




void Data2Csv(float a, float b, float c,
              float d, float e, float f, float g,
              float h, uint16_t i)

              // (massConcentrationPm1p0, massConcentrationPm2p5, massConcentrationPm4p0,
              // massConcentrationPm10p0, ambientHumidity, ambientTemperature, vocIndex,
              // noxIndex, co2)
        //potential future arguments below
        // (
        // toggle, massConcentrationPm1p0, massConcentrationPm2p5, massConcentrationPm4p0,
        // massConcentrationPm10p0, ambientHumidity, ambientTemperature, vocIndex,
        // noxIndex, co2)
{
  //first, we need to have this method take in an argument. this argument will 
  //take several points of data
  //there are 9 in total.
  //it would be nice if there was an argument to change if or if not there should be a datetime or other possible changes

  //there must be some criterion to determine...
    // a.) will an old CSV file be deleted?
    // b.) if so, create and name a new CSV file
    // c.) if an old CSV file is selected, how will we know what the CSV file is named so that we can append to it?
  //these 9 points of data must be appended to the csv, with a timestamp 
  //how can we ensure the timestamp remains valid, even when things like daylight savings or leap years occur?

  //for now, i create the method to take in arbitrary data...

  
  // File my_edit_file 
  // my_edit_file =





  Serial.println("This is a test line");

  myFile = SD.open("/sd_data_5.csv", FILE_READ); //-N- 100 LINES is approximately 10kB... setting a limit for 2k lines is good enough for one file? about 200kB
  //myFile = SD.open("csv.txt", FILE_WRITE);  
  if (!myFile || myFile.size() == 0) {
        Serial.println("Creating new file with headers...");
        writeFile(SD, "/sd_data_5.csv", "PM1,PM2.5,PM4,PM10,Temperature,Relative Humiditiy,VOC,NOx,CO2\n"); //
        // listDir(SD)
      }
  else if (!myFile || myFile.size() > 10000) {

        Serial.println("The start of a new file...");
        writeFile(SD, "/sd_data_6.csv", "PM1,PM2.5,PM4,PM10,Temperature,Relative Humiditiy,VOC,NOx,CO2\n");
  }
  else{
    
  char datapull_1[32];    //this could probably use a better naming convention...
  char datapull_2[32];
  char datapull_3[32];
  char datapull_4[32];
  char datapull_5[32];
  char datapull_6[32];
  char datapull_7[32];
  char datapull_8[32];
  char datapull_9[32];

  int Month;
  int Day;
  int Year;
  int Hour;
  int Min;  
  
  char DateTime[64];
  String DT;

  sprintf(datapull_1, "%f",a);  // I use %f instead of %d since these data points are floats, not integers
  //myFile.printf(datapull_1);

  //myFile.printf(",");
  // Serial.printf(datapull_1);
  // Serial.printf(". \n");

  // myFile.printf(datapull_2);

  //myFile.printf(",");                                     //A 2D array, use a for loop
  appendFile(SD, "/sd_data_5.csv", datapull_1); //sd    
  appendFile(SD, "/sd_data_5.csv", ",");
  sprintf(datapull_2, "%f",b);
  appendFile(SD, "/sd_data_5.csv", datapull_2); //sd    
  appendFile(SD, "/sd_data_5.csv", ",");
  sprintf(datapull_3, "%f",c);
  appendFile(SD, "/sd_data_5.csv", datapull_3); //sd    
  appendFile(SD, "/sd_data_5.csv", ",");
  sprintf(datapull_4, "%f",d);
  appendFile(SD, "/sd_data_5.csv", datapull_4); //sd    
  appendFile(SD, "/sd_data_5.csv", ",");
  sprintf(datapull_5, "%f",e);  
  appendFile(SD, "/sd_data_5.csv", datapull_5); //sd    
  appendFile(SD, "/sd_data_5.csv", ","); 
  sprintf(datapull_6, "%f",f);
  appendFile(SD, "/sd_data_5.csv", datapull_6); //sd    
  appendFile(SD, "/sd_data_5.csv", ",");
  sprintf(datapull_7, "%f",g);
  appendFile(SD, "/sd_data_5.csv", datapull_7); //sd    
  appendFile(SD, "/sd_data_5.csv", ",");
  sprintf(datapull_8, "%f",h);
  appendFile(SD, "/sd_data_5.csv", datapull_8); //sd    
  appendFile(SD, "/sd_data_5.csv", ",");
  sprintf(datapull_9, "%d",i);  
  appendFile(SD, "/sd_data_5.csv", datapull_9); //sd    
  appendFile(SD, "/sd_data_5.csv", ",");

  DT = rtc.getDateTime();
  Month = rtc.getMonth();
  Month += 1;
  Day = rtc.getDay();
  Year = rtc.getYear();
  Hour = rtc.getHour(true);
  Min = rtc.getMinute();

  sprintf(DateTime, "%d/%d/%d %d:%d", Month, Day, Year, Hour, Min);
  appendFile(SD, "/sd_data_5.csv", DateTime);
  Serial.println(Month);
  Serial.println(Day);
  Serial.println(Year);

  // sprintf(DateTime, "%s", DT);
  // appendFile(SD, "/sd_data_5.csv", DateTime);
  appendFile(SD, "/sd_data_5.csv", "DONE \n"); //sd 



  //myFile.close(); 
  Serial.printf("Pulled data once... \n");


  //the following routine renames all of the csv files, after a presumed deletion of the original oldest file, sd_data_1
  // Serial.printf("+-+-+- File Capacity Reached. Initiating Oldest File Delete and Directory Rename Routine-+-+-+  \n");
  
  // int csv_count = 4;

  // char base1[16]; //creating a base to fit the shared part of the filename in...
  // char base2[16];

  // for (int i = 1; i < csv_count; i++)
  // {
  //   int f=i+1;
  //   // String file_count_f = "sd_data_" + String(i+1) + ".csv";
  //   // String file_count_b = "sd_data_" + String(i) + ".csv";
  //   sprintf(base1, "/sd_data_%d.csv", f); //sprintf is formatting strings into char. %d is a placeholder for whatever i+1 is
  //   const char* file_count_f = base1;
  //   Serial.printf(file_count_f);
  //   Serial.printf("\n");
  //   sprintf(base2, "/sd_data_%d.csv", i); //sprintf is formatting strings into char. %d is a placeholder for whatever i+1 is
  //   const char* file_count_b = base2;
  //   Serial.printf(file_count_b);
  //   Serial.printf("\n");
  //   renameFile(SD, file_count_f, file_count_b);
  //   //renameFile(SD, "/sd_data_2.csv", "/sd_data_1.csv");
  // }
  
  // Serial.printf("+-+-+- Oldest File Delete and Directory Rename Routine Complete -+-+-+  \n");

  

//what if there are more files then the count expects? what if we want to change csv_count?
  }
};

//USEFUL METHODS...
/*
  writeFile(SD, "/hello.txt", "Hello ");
  appendFile(SD, "/hello.txt", "World!\n");
  readFile(SD, "/hello.txt");
  deleteFile(SD, "/foo.txt");
  renameFile(SD, "/hello.txt", "/foo.txt");
  readFile(SD, "/foo.txt");
  testFileIO(SD, "/test.txt");
*/


void setup() {
  Serial.begin(115200);
  rtc.setTime(30, 30, 16 ,3, 7, 2025); //-N- MANUALLY SETTING REAL TIME CLOCK
  Wire.begin(I2C_SDA, I2C_SCL);
  sensor.begin(Wire, SEN66_I2C_ADDR_6B);
  error = sensor.deviceReset();
  if (error != NO_ERROR) {
      Serial.print("Error trying to execute deviceReset(): ");
      errorToString(error, errorMessage, sizeof errorMessage);
      Serial.println(errorMessage);
      return;
}
delay(1200);
int8_t serialNumber[32] = {0};
error = sensor.getSerialNumber(serialNumber, 32);
if (error != NO_ERROR) {
    Serial.print("Error trying to execute getSerialNumber(): ");
    errorToString(error, errorMessage, sizeof errorMessage);
    Serial.println(errorMessage);
    return;
}
Serial.print("serialNumber: ");
Serial.print((const char*)serialNumber);
Serial.println();
error = sensor.startContinuousMeasurement();
if (error != NO_ERROR) {
    Serial.print("Error trying to execute startContinuousMeasurement(): ");
    errorToString(error, errorMessage, sizeof errorMessage);
    Serial.println(errorMessage);
    return;
}

#ifdef REASSIGN_PINS
  SPI.begin(sck, miso, mosi, cs);
  if (!SD.begin(cs)) {
#else
  if (!SD.begin()) {
#endif
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();

  if (cardType == CARD_NONE) {
    Serial.println("No SD card attached");
    return;
  }

  Serial.print("SD Card Type: ");
  if (cardType == CARD_MMC) {
    Serial.println("MMC");
  } else if (cardType == CARD_SD) {
    Serial.println("SDSC");
  } else if (cardType == CARD_SDHC) {
    Serial.println("SDHC");
  } else {
    Serial.println("UNKNOWN");
  }

  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);
  Serial.printf("Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
  Serial.printf("Used space: %lluMB\n", SD.usedBytes() / (1024 * 1024));        
    
  // else 
  //   {
  //     writeFile(SD, "/sd_data_5.csv", "trying.... ");
  //     myFile = SD.open("/sd_data_5.csv", FILE_WRITE); //-N-
  //     Serial.printf("Writing headers to sd_data_5.csv \n");
  //     //myFile.printf("PM1,PM2.5,PM4,PM10,Temperature,Relative Humiditiy,VOC,NOx,CO2 \n");
  //     appendFile(SD, "/sd_data_5.csv", "PM1,PM2.5,PM4,PM10,Temperature,Relative Humiditiy,VOC,NOx,CO2 \n");
  //     Serial.printf("Headers written \n");
  //     myFile.close(); 
  //   }  
  //Serial.println("Enter w for write, r for read or s for split csv");  



  // sensor.readMeasuredValues(
  //     massConcentrationPm1p0, massConcentrationPm2p5, massConcentrationPm4p0,
  //     massConcentrationPm10p0, ambientHumidity, ambientTemperature, vocIndex,
  //     noxIndex, co2);


  // Data2Csv(massConcentrationPm1p0, massConcentrationPm2p5, massConcentrationPm4p0, massConcentrationPm10p0, ambientHumidity, ambientTemperature, vocIndex, noxIndex, co2);
  //Serial.printf();
  //massConcentrationPm1p0
  //String type_4_dat = typeid(massConcentrationPm1p0).name();

  // void listDir(fs::FS &fs, const char *dirname, uint8_t levels) {
  //   Serial.println(dirname);

  //   File root = fs.open(dirname);
  //   if (!root) {
  //     Serial.println("Failed to open directory");
  //     return;
  //   }
  //   if (!root.isDirectory()) {
  //     Serial.println("Not a directory");
  //     return;
  //   }

  //   File file = root.openNextFile();
  //   while (file) {
  //     if (file.isDirectory()) {
  //       Serial.print("  DIR : ");
  //       Serial.println(file.name());
  //       if (levels) {
  //         listDir(fs, file.path(), levels - 1);
  //       }
  //     } else {
  //       Serial.print("  FILE: ");
  //       Serial.print(file.name());
  //       Serial.print("  SIZE: ");
  //       Serial.println(file.size());
  //     }
  //     file = root.openNextFile();
  //   }
  // }
  File root = SD.open("/");
  if (!root) {
    Serial.println("Failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    Serial.println("Not a directory");
    return;
  }
  File file = root.openNextFile();
  while (file) {
    // ----- working ver
      // char * pch;
      // char f_n[16];
      // sprintf(f_n, "%s",file.name()); 
      // pch = strstr(f_n,"sd_data_");
    // ------

    char * pch;
    pch = strstr(file.name(),"sd_data_");
    if (pch!=NULL) {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("  SIZE: ");
      Serial.println(file.size());
      Serial.println(" ");

      // char* after = pch + strlen("sd_data_")
      // Serial.println(after);
      }
    
    // else {}
    file = root.openNextFile();
  }

}



void loop() 
{
  // sensor.readMeasuredValues(
  //   massConcentrationPm1p0, massConcentrationPm2p5, massConcentrationPm4p0,
  //   massConcentrationPm10p0, ambientHumidity, ambientTemperature, vocIndex,
  //   noxIndex, co2);

  // Data2Csv(massConcentrationPm1p0, massConcentrationPm2p5, massConcentrationPm4p0, massConcentrationPm10p0, ambientHumidity, ambientTemperature, vocIndex, noxIndex, co2);
}




//headers
//rows and columns
//select and change rows and columns
