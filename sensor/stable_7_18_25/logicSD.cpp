#include "logicSD.h"

//dataToCSV variables
File myFile;
bool first_time = 0;

int findMax() {
  File root = SD.open("/");

  int max_fn = 0;
  int pmax_fn = 0;
  if (!root) {
    Serial.println("Failed to open directory");
    return 0;
  }
  if (!root.isDirectory()) {
    Serial.println("Not a directory");
    return 0;
  }
  File file = root.openNextFile();
  while (file) {
    char *pch;
    pch = strstr(file.name(), "sd_data_");
    if (pch != NULL) {
      char *after = pch + strlen("sd_data_");

      char *dotPos = strstr(after, ".");
      if (dotPos != NULL) {
        *dotPos = '\0';
      }
      max_fn = atoi(after);  //converts the number in the string to an int
      if (max_fn > pmax_fn) {
        pmax_fn = max_fn;
      }
    }
    file = root.openNextFile();
  }
  Serial.println("we got the max file number as:");
  Serial.println(pmax_fn);
  return pmax_fn;
}

int findMin(){
  File root = SD.open("/");

  if (!root) {
    Serial.println("Failed to open directory");
    return -1;
  }
  if (!root.isDirectory()) {
    Serial.println("Not a directory");
    return -1;
  } else {
    File file = root.openNextFile();
    int min_fn=700;
    int pmin_fn=700; 
    while (file) {
      char * pch;
      pch = strstr(file.name(),"sd_data_");
      if (pch!=NULL) {
        char* after = pch + strlen("sd_data_");
        Serial.println(after);

        char* dotPos = strstr(after,".");
        if (dotPos != NULL){
          *dotPos = '\0';
        }
        
        min_fn = atoi(after); //converts the number in the string to an int
        Serial.print("MIN_FN IS...");
        Serial.print(min_fn);
        if (min_fn < pmin_fn) {
          pmin_fn = min_fn;        
        }
      }
      file = root.openNextFile();
    }
    Serial.println("we got the min file number as:");
    Serial.println(pmin_fn);
    if (pmin_fn != 700) { 
      return pmin_fn;
    } else {
      return -1;
    }
  }
}


void listDir(fs::FS &fs, const char *dirname, uint8_t levels) {
  //Serial.println(dirname);

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
  //Serial.printf("Creating Dir: %s\n", path);
  if (fs.mkdir(path)) {
    Serial.println("Dir created");
  } else {
    Serial.println("mkdir failed");
  }
}

void removeDir(fs::FS &fs, const char *path) {
  //Serial.printf("Removing Dir: %s\n", path);
  if (fs.rmdir(path)) {
    Serial.println("Dir removed");
  } else {
    Serial.println("rmdir failed");
  }
}

void readFile(fs::FS &fs, const char *path) {
  //Serial.printf("Reading file: %s\n", path);

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
  //Serial.printf("Writing file: %s\n", path);

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
  //Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if (!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if (file.print(message)) {
    // Serial.println("Message appended");
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

void dataToCSV(int cfn_input) {
  // ---- ESP32 NOW setup ----- 
  if((rtc.getYear() < 2000) || (stored_min != rtc.getMinute())){ // if the year is wrong OR it hasn't updated in a MINUTE (could do hour or day)...
    Serial.print("YESSIR WE ARE UPDATING THE TIME WITH NTP STUFF \n");
    if (esp_now_init() != ESP_OK) { // Init ESP-NOW
      Serial.println("Error initializing ESP-NOW");
      return;
      }
      // Register receive callback
    esp_now_register_recv_cb(OnDataRecv);
    int timestamp = millis(); //wait 15 seconds to get time if has already recieved an NTP timestamp
    while(myData.year == NULL || myData.second == myData_copy.second && (millis()-timestamp<15000) && first_time != 0){ // while the data value has not updated //CHECK THE VALUE IN THE STRUCT
      Serial.print("HELP IM STUCK HELP IM TRAPPED \n");
      delay(1000);
    }
    first_time = 1;
    Serial.print("IM FREEEEEEEEEEEEEEEE \n");
    myData_copy = myData;
    stored_min = rtc.getMinute();
     esp_now_deinit();
    canWriteAgain = true;
  } else {
    sen66.readMeasuredValues(
    massConcentrationPm1p0, massConcentrationPm2p5, massConcentrationPm4p0,
    massConcentrationPm10p0, humidity, temperature, vocIndex, noxIndex,
    co2);

    float mea[9] = {massConcentrationPm1p0, massConcentrationPm2p5, massConcentrationPm4p0, 
    massConcentrationPm10p0, humidity, temperature, vocIndex, noxIndex, co2};

    char cfn_buffer[16];

    sprintf(cfn_buffer, "/sd_data_%d.csv", cfn_input);

    myFile = SD.open(cfn_buffer, FILE_READ);
    Serial.printf("This is a test message \n");
    Serial.printf("Used space: %fMB\n", SD.usedBytes() / (1024.0 * 1024.0));
          
    float usage = SD.usedBytes() / (1024.0 * 1024.0);


    if (usage > 25600) {  //Usage in megabytes originally 100 //this checks if the SD card is too full... if it is, delete the oldest file //if (usage > 0.25) { -N-
      Serial.println("We try deleting a file.");
      int del_fn = findMin();
      char delfn_buffer[16];
      sprintf(delfn_buffer, "/sd_data_%d.csv", del_fn);
      Serial.println("the file number we delete is:");
      Serial.println(del_fn);
      deleteFile(SD, delfn_buffer);
    } else if (!myFile || myFile.size() == 0) { //if the file is nonexistant we make it!
      Serial.println("Creating new file with headers...");
      writeFile(SD, cfn_buffer, "PM 1, PM 2.5, PM4, PM10, Humidity, Temperature, VOC, NOx, CO2, Time, DONE\n");
    } else if (!myFile || myFile.size() > 50000000) { //10000 is 10kB! -N-
      Serial.println("The start of a new file...");
      countfn++;
    } else {
      char data_arr[9][32];
      char dateAndTime[64];

      int month = rtc.getMonth() + 1;
      int day = rtc.getDay();
      int year = rtc.getYear();
      int hour = rtc.getHour(true);
      int min = rtc.getMinute();
      int sec = rtc.getSecond();

      for (int i = 0; i < 9; i++) {
        snprintf(data_arr[i], sizeof(data_arr[i]), "%.4f", mea[i]);
        appendFile(SD, cfn_buffer, data_arr[i]);
        appendFile(SD, cfn_buffer, ",");
      }

      sprintf(dateAndTime, "%d/%d/%d %d:%d:%d ", month, day, year, hour, min, sec);
      appendFile(SD, cfn_buffer, dateAndTime);
      appendFile(SD, cfn_buffer, ",");

      appendFile(SD, cfn_buffer, "DONE \n");  
    }
    canWriteAgain = true;
  }    
}
