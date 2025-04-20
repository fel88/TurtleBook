/*
   Print size, modify date/time, and name for all files in root.
*/
#include "SdFat.h"
#include <SPI.h>
int x;
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>


#ifndef APSSID
#define APSSID "ESPap"
#define APPSK "thereisnospoon"
#endif
/* Set these to your desired credentials. */
const char* ssid = APSSID;
const char* password = APPSK;

ESP8266WebServer server(80);

const bool DecryptZCB = false;

//File root;
/**
   data
**/
#define UBYTE uint8_t
#define UWORD uint16_t
#define UDOUBLE uint32_t

/**
   SPI RAM
**/
#define EPD_CS D1
#define EPD_RST D1
//#define SD_CS D4
#define SPIRAM_CS D3


#define SPIRAM_CS_0 digitalWrite(SPIRAM_CS, LOW)
#define SPIRAM_CS_1 digitalWrite(SPIRAM_CS, HIGH)

/**
   SD Card
**/

#define SD_CS_0 digitalWrite(SD_CS, LOW)
#define SD_CS_1 digitalWrite(SD_CS, HIGH)
/**
   e-Paper GPIO
**/

#define EPD_CS_0 digitalWrite(EPD_CS, LOW)
#define EPD_CS_1 digitalWrite(EPD_CS, HIGH)

//#define EPD_DC 9



#define EPD_RST_0 digitalWrite(EPD_RST, LOW)
#define EPD_RST_1 digitalWrite(EPD_RST, HIGH)




#define EPD_RST_PIN EPD_RST
#define EPD_DC_PIN EPD_DC
#define EPD_CS_PIN EPD_CS
#define EPD_BUSY_PIN EPD_BUSY

/**
   GPIO read and write
**/
#define DEV_Digital_Write(_pin, _value) digitalWrite(_pin, _value == 0 ? LOW : HIGH)
#define DEV_Digital_Read(_pin) digitalRead(_pin)

/**
   SPI
**/

// SD_FAT_TYPE = 0 for SdFat/File as defined in SdFatConfig.h,
// 1 for FAT16/FAT32, 2 for exFAT, 3 for FAT16/FAT32 and exFAT.
#define SD_FAT_TYPE 3
/*
  Change the value of SD_CS_PIN if you are using SPI and
  your hardware does not use the default value, SS.
  Common values are:
  Arduino Ethernet shield: pin 4
  Sparkfun SD shield: pin 8
  Adafruit SD shields and modules: pin 10
*/

// SDCARD_SS_PIN is defined for the built-in SD on some boards.
#ifndef SDCARD_SS_PIN
const uint8_t SD_CS_PIN = D2;
#else   // SDCARD_SS_PIN
// Assume built-in SD is used.
const uint8_t SD_CS_PIN = SDCARD_SS_PIN;
#endif  // SDCARD_SS_PIN

// Try max SPI clock for an SD. Reduce SPI_CLOCK if errors occur.
#define SPI_CLOCK SD_SCK_MHZ(50)

// Try to select the best SD card configuration.
//#if HAS_SDIO_CLASS
//#define SD_CONFIG SdioConfig(FIFO_SDIO)
//#elif  ENABLE_DEDICATED_SPI
//#define SD_CONFIG SdSpiConfig(SD_CS_PIN, DEDICATED_SPI, SPI_CLOCK)
//#else  // HAS_SDIO_CLASS
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, SHARED_SPI, SPI_CLOCK)
//#endif  // HAS_SDIO_CLASS

UBYTE my_DEV_Module_Init(void) {
  //return 0;
  //set pin
  //pinMode(SPIRAM_CS, OUTPUT);
  //pinMode(SD_CS, OUTPUT);

  //pinMode(EPD_CS, OUTPUT);
  //pinMode(EPD_DC, OUTPUT);
  pinMode(EPD_RST, OUTPUT);

  EPD_RST_1;

  //EPD_CS_1;
  // SD_CS_1;
  //SPIRAM_CS_1;

  //set Serial
  //Serial.begin(115200);

  //set OLED SPI
  // SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE0));
  /*   SPI.setDataMode(SPI_MODE0);
    SPI.setBitOrder(MSBFIRST);
    SPI.setClockDivider(SPI_CLOCK_DIV4);
    SPI.begin();
  */
  return 0;
  DEV_Digital_Write(EPD_RST_PIN, 1);
  delay(200);
  DEV_Digital_Write(EPD_RST_PIN, 0);
  delay(10);
  DEV_Digital_Write(EPD_RST_PIN, 1);
  delay(200);
  return 0;
}
#if SD_FAT_TYPE == 0
SdFat sd;
File dir;
File file;
#elif SD_FAT_TYPE == 1
SdFat32 sd;
File32 dir;
File32 file;
#elif SD_FAT_TYPE == 2
SdExFat sd;
ExFile dir;
ExFile file;
#elif SD_FAT_TYPE == 3
SdFs sd;
FsFile dir;
FsFile file;
#else  // SD_FAT_TYPE
#error invalid SD_FAT_TYPE
#endif  // SD_FAT_TYPE
//------------------------------------------------------------------------------
// Store error strings in flash to save RAM.
#define error(s) sd.errorHalt(&Serial, F(s))
//------------------------------------------------------------------------------


const String postForms = "<html>\
  <head>\
    <title>BOOK Web Server POST handling</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
    <script src=\"jszip.min.js\" ></script>\
    <script src=\"script.js\" ></script>\
    <script src=\"huff.js\" ></script>\
    <link href=\"styles.css\" rel=\"stylesheet\" />\
  </head>\
  <body>\
   <h1>BOOK</h1><br>\
    <h1>POST plain text to /postplain/</h1><br>\
    <form method=\"post\" enctype=\"text/plain\" action=\"/postplain/\">\
      <input type=\"text\" name=\'{\"hello\": \"world\", \"trash\": \"\' value=\'\"}\'><br>\
      <input type=\"submit\" value=\"Submit\">\
    </form>\
    <h1>POST form data to /postform/</h1><br>\
    <form method=\"post\" enctype=\"application/x-www-form-urlencoded\" action=\"/postform/\">\
      <input type=\"text\" name=\"hello\" value=\"world\"><br>\
      <input type=\"submit\" value=\"Submit\">\
    </form>\
     <h1>list  files   /postform/</h1><br>\
    <form method=\"post\" enctype=\"application/x-www-form-urlencoded\" action=\"/list/\">\
      <input type=\"text\" name=\"hello\" value=\"world\"><br>\
      <input type=\"submit\" value=\"Submit\">\
    </form>\
     <h1>POST form file to /postform/</h1><br>\
    <form method=\"post\" enctype=\"multipart/form-data\" action=\"/upload\">\
  <div>\
    <label>Select file to upload</label>\
    <input name='dummy' type=\"file\">\
  </div>\
  <div>\
  <label>Upload path</label>\
  <input name=\"uploadPathInput\" type=\"text\"  />\
  </div>\
  <button id=\"sub1\" >Send</button>\
  </form>\
     <h1>Download file  /download/</h1><br>\
    <form method=\"get\" enctype=\"multipart/form-data\" action=\"/download/\">\ 
  <div>\
  <label>Download path</label>\
  <input name=\"downloadPathInput\" type=\"text\"  />\
  </div>\
  <button id=\"sub1\" >Send</button>\
  </form>\
";

String middle = "";
const String postFormsEnd = "  </body>\
</html>";

void handleRoot() {
  //digitalWrite(led, 1);
  Serial.println("root request");
  Serial.println(middle);
  server.send(200, "text/html", postForms + middle + postFormsEnd);
  //digitalWrite(led, 0);
  Serial.flush();
}

void handlePlain() {
  if (server.method() != HTTP_POST) {
    //digitalWrite(led, 1);
    server.send(405, "text/plain", "Method Not Allowed");
    // digitalWrite(led, 0);
  } else {
    // digitalWrite(led, 1);
    server.send(200, "text/plain", "POST body was:\n" + server.arg("plain"));
    // digitalWrite(led, 0);
  }
  Serial.flush();
}



/*struct decodeHuffNode {
  // unsigned int  prob;
  unsigned char byte;
  bool isByteAssigned;
  unsigned short left;
  unsigned short right;
  //unsigned short parent;
};*/

short huff[2048];  // 2n, 2n+1;  -1 not setted but has childs, -2 has not childs, >0 terminate symbol node

#define MFILE_WRITE (O_RDWR | O_CREAT | O_AT_END)
#define MFILE_READ (O_RDONLY)
#define MFILE_OWR (O_WRONLY)

union longUnion {
  long data;
  unsigned char b[8];
} lu;
//////////////////huff decoder
void decodeZCB(String filename) {
  String filenameToSave = filename.substring(0, filename.length() - 4);
  Serial.print("decode filename:");
  Serial.println(filename);
  Serial.print("filenameToSave:");
  Serial.println(filenameToSave);

  auto file = sd.open(filename, MFILE_READ);
  auto sfile = sd.open(filenameToSave, MFILE_WRITE);
  int data;

  unsigned long flen = 0;
  file.seek(3);
  for (int i = 0; i < 8; i++) {
    lu.b[i] = (unsigned char)file.read();
    //flen <<= 8;
    //flen |= file.read();
  }
  flen = lu.data;
  Serial.print("file length detected:");
  Serial.println(flen);

  file.seek(3 + 8);
  for (int i = 0; i < 2048; i++) {
    huff[i] = -2;
  }
  int p = 0;
  //restore tree
  for (int i = 0; i < 256; i++) {
    int val = file.read();
    Serial.print("dic val: ");
    Serial.println(val);
    int len = file.read();
    Serial.print("dic len: ");
    Serial.println(len);
    p = 0;  //binary tree index
    for (int j = 0; j < len; j++) {
      int pp = file.read();
      huff[p] = -1;
      if (pp == 1) {
        p = p * 2 + 2;
        Serial.println("right");

      } else {
        p = p * 2 + 1;
        Serial.println("left");
      }
    }
    huff[p] = (short)val;
    Serial.print("huff #");
    Serial.print(p);
    Serial.print(": ");
    Serial.println(huff[p]);
  }

  for (int i = 0; i < 64; i++) {
    Serial.print("huff #");
    Serial.print(i);
    Serial.print(": ");
    Serial.println(huff[i]);
  }
  //decode
  p = 0;
  long totalBytes = 0;
  while ((data = file.read()) >= 0)
  //for (int i = 0; i < 32; i++)
  {
    //data = file.read();
    // Serial.print("data read:");
    // Serial.println(data);
    // Serial.print("bit read ");
    for (int j = 0; j < 8; j++) {
      //Serial.println(bitRead(data, j));
    }
    for (int j = 0; j < 8; j++) {
      // Serial.print("p:");
      //Serial.println(p);
      //Serial.print("huff[p]:");
      // Serial.println(huff[p]);
      if (huff[p] >= 0) {
        sfile.write((unsigned char)huff[p]);
        totalBytes++;
        if ((totalBytes % 100) == 0) {
          Serial.print("totalBytes: ");
          Serial.println(totalBytes);
        }
        p = 0;
      }
      if (bitRead(data, j) > 0) {  //1
                                   //  Serial.println("right");


        p = p * 2 + 2;
      } else {  //0
        //Serial.println("left");
        p = p * 2 + 1;
      }
    }
    if (totalBytes >= flen) {
      Serial.println("flen reached");
      break;
    }
  }
  //while ((data = file.read()) >= 0) {
  //Serial.write(data);
  //}

  file.close();
  sfile.close();
}
////////////////////



const int max_characters = 80;
char f_name[max_characters];

void huffHandle() {
  if (SPIFFS.exists("/huff.js")) {
    Serial.println(F("huff.js founded on   SPIFFS"));  //ok

    File file = SPIFFS.open("/huff.js", "r");  //ok

    server.send(200, "text/javascript", file);

    //request->send(SPIFFS, "/jszip.min.js", "text/javascript");
  }
}

void scriptHandle() {
  if (SPIFFS.exists("/script.js")) {
    Serial.println(F("script.js founded on   SPIFFS"));  //ok

    File file = SPIFFS.open("/script.js", "r");  //ok

    server.send(200, "text/javascript", file);

    //request->send(SPIFFS, "/jszip.min.js", "text/javascript");
  }
}
void stylesHandle() {
  if (SPIFFS.exists("/styles.css")) {
    Serial.println(F("styles.css founded on   SPIFFS"));  //ok

    File file = SPIFFS.open("/styles.css", "r");  //ok

    server.send(200, "text/css", file);

    //request->send(SPIFFS, "/jszip.min.js", "text/javascript");
  }
}
void jszip() {
  if (SPIFFS.exists("/jszip.min.js")) {
    Serial.println(F("jszip.min.js founded on   SPIFFS"));  //ok

    File file = SPIFFS.open("/jszip.min.js", "r");  //ok

    server.send(200, "text/javascript", file);

    //request->send(SPIFFS, "/jszip.min.js", "text/javascript");
  }
}

void handleList() {
  if (server.method() != HTTP_POST) {
    //digitalWrite(led, 1);
    server.send(405, "text/plain", "Method Not Allowed");
    // digitalWrite(led, 0);
  } else {
    // digitalWrite(led, 1);
    // digitalWrite(led, 0);
    if (!dir.open("/")) {
      error("dir.open failed");
    }
    String message = "POST form was:\n";


    // Open next file in root.
    // Warning, openNext starts at the current position of dir so a
    // rewind may be necessary in your application.
    while (file.openNext(&dir, O_RDONLY)) {

      file.getName(f_name, max_characters);
      String filename = String(f_name);

      file.printFileSize(&Serial);
      Serial.write(' ');
      file.printModifyDateTime(&Serial);
      Serial.write(' ');
      file.printName(&Serial);
      message += filename;
      if (file.isDir()) {
        // Indicate a directory.
        Serial.write('/');
        message += "/";
      }
      message += "<br>";
      Serial.println();
      file.close();
    }
    if (dir.getError()) {
      Serial.println("openNext failed");
    } else {
      Serial.println("Done!");
    }

    server.send(200, "text/plain", "POST body was:\n" + message);
  }
  Serial.flush();
}

void handleForm() {
  if (server.method() != HTTP_POST) {
    // digitalWrite(led, 1);
    server.send(405, "text/plain", "Method Not Allowed");
    // digitalWrite(led, 0);
  } else {
    //digitalWrite(led, 1);
    String message = "POST form was:\n";

    for (uint8_t i = 0; i < server.args(); i++) {
      message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
      Serial.println(" " + server.argName(i) + ": " + server.arg(i) + "\n");
      Serial.flush();
      //sendStringPacket(" " + server.argName(i) + ": " + server.arg(i) );
    }
    server.send(200, "text/plain", message);
    // digitalWrite(led, 0);
  }
  Serial.flush();
}

//void onUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
void onUpload() {
  String filename;
  if (server.uri() != "/update") return;
  HTTPUpload& upload = server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    filename = upload.filename;
    Serial.print("Upload Name: ");
    Serial.println(filename);
    //sendStringPacket("FILE="+String(filename));
    //UploadFile = SPIFFS.open("/data/" + filename, "w");
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    //if (UploadFile)
    //  UploadFile.write(upload.buf, upload.currentSize);
  } else if (upload.status == UPLOAD_FILE_END) {
    //if (UploadFile)
    //UploadFile.close();
    //  FileRead();  // After file downloads, read it
  }
  Serial.println("onUpload called: ");
}


void handleNotFound() {
  //digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  //digitalWrite(led, 0);
  Serial.flush();
}


long totalbts = 0;
int packetAfterDelay = 5;
int packetsSended = 0;

String lastUploadedFilePath;
void handleFileUpload() {  // upload a new file to the SPIFFS

  HTTPUpload& upload = server.upload();

  if (upload.status == UPLOAD_FILE_START) {
    totalbts = 0;
    packetsSended = 0;
    String filename = upload.filename;
    if (!filename.startsWith("/")) filename = "/" + filename;
    Serial.print("handleFileUpload Name: ");
    Serial.println(filename);
    //sendStringPacket("FILE="+String(filename));

    String upPath = server.arg("uploadPathInput");
    filename = upPath + filename;

    Serial.print("upload path:");
    Serial.println(upPath);

    Serial.print("filename store:");
    Serial.println(filename);

    file = sd.open(filename, MFILE_WRITE);
    lastUploadedFilePath = filename;

    Serial.println("file opened: ");
    Serial.print(filename);
    Serial.println();

    delay(packetAfterDelay);
    packetsSended++;
    //fsUploadFile = SPIFFS.open(filename, "w");            // Open the file for writing in SPIFFS (create if it doesn't exist)
    filename = String();
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    totalbts += upload.currentSize;
    //if(fsUploadFile)
    if ((totalbts / 1000000) % 4) {
      //Serial.print("current size: "); Serial.println(upload.currentSize);
      Serial.print("current size: ");
      Serial.println(totalbts);
    }
    //sendStringPacket("FILE="+String(filename));
    //sendPacket(18);

    //sendPacket(upload.buf,upload.currentSize);
    /*int fullP=upload.currentSize/256;


      for(int i=0;i<fullP;i++){
      sendPacket(upload.buf,i*256,256);packetsSended++;
      delay(packetAfterDelay);}
      int remains=upload.currentSize%256;
      if(remains>0){
      sendPacket(upload.buf,fullP*256,remains);packetsSended++;
      delay(packetAfterDelay);}*/
    for (int i = 0; i < upload.currentSize; i++) {
      file.write(upload.buf[i]);
    }

    //fsUploadFile.write(upload.buf, upload.currentSize); // Write the received bytes to the file
  } else if (upload.status == UPLOAD_FILE_END) {
    file.close();
    Serial.println("file closed");
    if (DecryptZCB) {
      decodeZCB(lastUploadedFilePath);
    }
    // if(fsUploadFile) {                                    // If the file was successfully created
    // fsUploadFile.close();                               // Close the file again
    // sendStringPacket("END");delay(packetAfterDelay);
    packetsSended++;
    //Serial.print("packetsSended: "); Serial.println(packetsSended);
    Serial.print("handleFileUpload Size: ");
    Serial.println(upload.totalSize);
    Serial.print("totalbts Size: ");
    Serial.println(totalbts);
    //  server.sendHeader("Location","/success.html");      // Redirect the client to the success page
    server.send(303);
  } else {
    server.send(500, "text/plain", "500: couldn't create file");
  }
}
//byte data[2048];
void sleepNow() {
  //Serial.println("going to light sleep...");
  //wifi_fpm_set_wakeup_cb(wakeupFromMotion); //wakeup callback
  wifi_fpm_do_sleep(1000000 * 1);
}
int getInternalBootCode() {
  uint32_t result;
  ESP.rtcUserMemoryRead(0, &result, 1);
  return (int)result;
}

void setInternalBootCode(int value) {
  uint32_t storeValue = value;
  ESP.rtcUserMemoryWrite(0, &storeValue, 1);
}


void p2p_enumDir(FsFile* tdir, String path) {
  FsFile file;

  tdir->getName(f_name, max_characters);
  String dname = String(f_name);


  while (file.openNext(tdir, O_RDONLY)) {



    file.getName(f_name, max_characters);
    //filename = String(f_name);
    String filename = String(f_name);
    Serial.print(filename);
    Serial.print("\r");

    Serial.flush();
    Serial.readStringUntil('\r');

    if (file.isDir()) {

      //recDir(&file, path + '/' + dname);
      //middle += "<p><a href=\"#\" onclick=\"alert('" + filename + "');\">" + filename + "</a></p>";

      // Indicate a directory.
      // Serial.write('/');
    } else {
    }
    //Serial.println();
    file.close();
  }
}
void recDir(FsFile* tdir, String path) {
  FsFile file;

  tdir->getName(f_name, max_characters);
  String dname = String(f_name);
  middle += "<li><span class=\"caret\">" + dname + "</span>";
  middle += "<div style=\"display:none;\">" + path + "</div>";
  middle += "<button  class=\"set-button\" >set path</button>";
  middle += "<ul class=\"nested\">";

  while (file.openNext(tdir, O_RDONLY)) {
    file.printFileSize(&Serial);

    Serial.write(' ');
    file.printModifyDateTime(&Serial);
    Serial.write(' ');
    file.printName(&Serial);

    file.getName(f_name, max_characters);
    //filename = String(f_name);
    String filename = String(f_name);
    if (file.isDir()) {

      recDir(&file, path + '/' + dname);
      //middle += "<p><a href=\"#\" onclick=\"alert('" + filename + "');\">" + filename + "</a></p>";

      // Indicate a directory.
      Serial.write('/');
    } else {
      middle += "<li>" + filename + "</li>";
      middle += "<button class=\"del-button\">delete </button>";
    }
    Serial.println();
    file.close();
  }
  middle += "</ul>";
  middle += "</li>";
}

void File_Download() {  // This gets called twice, the first pass selects the input, the second pass then processes the command line arguments
  Serial.println("download request");
  String dPath = server.arg("downloadPathInput");

  if (!dPath.startsWith("/"))
    dPath = "/" + dPath;

  Serial.print("dpath : ");
  Serial.println(dPath);

  file = sd.open(dPath, MFILE_READ);
  server.sendHeader("Content-Type", "application/octet-stream");
  server.sendHeader("Content-Disposition", "attachment; filename=\"" + dPath + "\"");
  server.sendHeader("Connection", "close");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  char data[128];


  server.send(200, "application/octet-stream", "");

  int size = 0;
  for (long i = 0; i < file.size(); i++) {

    data[size] = file.read();
    size++;
    if (size == 128) {
      server.sendContent(data, size);
      size = 0;
    }
  }
  if (size > 0) {
    server.sendContent(data, size);
  }
  //server.streamFile(file, "application/octet-stream");
  file.close();

  //else SelectInput("Enter filename to download","download","download");
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

const char* p2p_ssid = "book";
const char* p2p_password = "12345678";

WiFiServer p2p_server(888);

bool webMode = true;

IPAddress ip(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

void p2pRecieverMode() {
  Serial.print("cp1");
  Serial.print('\r');
  WiFi.mode(WIFI_AP);
  WiFi.setOutputPower(0);
  WiFi.softAPConfig(ip, gateway, subnet);
  WiFi.softAP(p2p_ssid, p2p_password);
  //WiFi.config(ip, gateway, subnet);
  //WiFi.hostname("Magic");  // Name that appears in your network

  /*
  while (WiFi.status() != WL_CONNECTED) {


    delay(100);
  }*/

  p2p_server.begin();
  Serial.print("cp2");
  Serial.print('\r');
  Serial.flush();
  WiFiClient client = p2p_server.available();  // Проверка подключения клиента
  while (!client) {
    delay(100);
    client = p2p_server.available();
  }
  Serial.print("cp22");
  Serial.print('\r');
  Serial.flush();
  while (!client.available()) {  // Ожидание запроса клиента
    delay(10);
  }
  Serial.print("cp3");
  Serial.print('\r');
  Serial.flush();

  client.setTimeout(100000);
  String filename = client.readStringUntil('\r');  // filename
  filename.trim();
  if (!filename.startsWith("/"))
    filename = "/" + filename;
  Serial.print("cp4");
  Serial.print('\r');
  Serial.flush();

  Serial.print(filename);
  Serial.print('\r');
  Serial.flush();

  String request2 = client.readStringUntil('\r');  // size
  request2.trim();
  auto size = request2.toInt();
  Serial.print(size);
  Serial.print('\r');
  Serial.flush();

  //client.flush();
  // Initialize the SD.
  my_DEV_Module_Init();
  bool goodInited = true;
  if (!sd.begin(SD_CONFIG)) {
    sd.initErrorHalt(&Serial);
    goodInited = false;
  }
  Serial.print("cp5");
  Serial.print('\r');
  Serial.flush();

  file = sd.open(filename, MFILE_WRITE);
  Serial.print("cp6");
  Serial.print('\r');
  Serial.flush();

  for (long ii = 0; ii < size; ii++) {
    while (!client.available())
      delay(1);
    file.write(client.read());
    if (ii % 10000 == 0) {
      float vv = ii / (float)size;
      Serial.print((int)(100 * vv));
      Serial.print('\r');
      Serial.flush();
    }
  }

  file.close();
  Serial.print("end");
  Serial.print('\r');
  WiFi.mode(WIFI_OFF);  // TURN OFF WIFI
  WiFi.forceSleepBegin();
  ESP.deepSleep(0);
}

void p2pSenderMode() {
  webMode = false;
  Serial.print("files");
  Serial.print("\r");

  Serial.flush();
  my_DEV_Module_Init();
  // Initialize the SD.
  bool goodInited = true;
  if (!sd.begin(SD_CONFIG)) {
    sd.initErrorHalt(&Serial);
    goodInited = false;
  }

  if (!dir.open("/")) {
    error("dir.open failed");
  }
  p2p_enumDir(&dir, "/");


  Serial.print("<finish>");
  Serial.print("\r");

  Serial.flush();
  Serial.readStringUntil('\r');
  String dPath = "/1.cb";
  Serial.setTimeout(100000);
  while (true) {
    while (!Serial) {
      yield();
    }

    String teststr = Serial.readStringUntil('\r');  //read until timeout

    Serial.print("ack");
    Serial.print("\r");
    Serial.flush();
    teststr.trim();  // remove any \r \n whitespace at the end of the String
    if (teststr == "<select>") {

      dPath = Serial.readStringUntil('\r');  //read until timeout
      Serial.print("ack");
      Serial.print("\r");
      Serial.flush();
      dPath.trim();
      break;
    } else {
    }
  }
  delay(500);
  Serial.print("cp1");
  Serial.print("\r");
  Serial.flush();
  delay(500);
  WiFi.setOutputPower(0);
  WiFi.mode(WIFI_STA);
  WiFi.begin(p2p_ssid, p2p_password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  Serial.print("cp2");
  Serial.print("\r");
  Serial.flush();
  delay(500);
  WiFiClient client;
  if (!client.connect(ip, 888)) {
    //return client;sleep?
    WiFi.mode(WIFI_OFF);  // TURN OFF WIFI
    WiFi.forceSleepBegin();
    ESP.deepSleep(0);
    return;
  }
  //send file name
  //send data
  Serial.print("cp3");
  Serial.print("\r");
  Serial.flush();
  delay(500);


  if (!dPath.startsWith("/"))
    dPath = "/" + dPath;

  client.print(dPath);
  client.print('\r');
  client.flush();

  Serial.print(dPath);
  Serial.print("\r");
  Serial.flush();
  delay(500);
  file = sd.open(dPath, MFILE_READ);
  //Serial.println("max");

  auto sz = file.size();
  client.print(sz);
  client.print('\r');
  client.flush();
  for (long i = 0; i < sz; i++) {

    client.write(file.read());
    if (i % 10000 == 0) {
      float vv = i / (float)sz;
      Serial.print((int)(100 * vv));
      Serial.print('\r');
      Serial.flush();
    }
  }
  file.close();


  WiFi.mode(WIFI_OFF);  // TURN OFF WIFI
  Serial.print("end");
  Serial.print('\r');
  Serial.flush();
  WiFi.forceSleepBegin();
  ESP.deepSleep(0);
}

void setup() {
  /* int rr=getInternalBootCode();

    if(rr!=0 && rr!=1)
    {
    setInternalBootCode(0);rr=getInternalBootCode();
    }
    setInternalBootCode(rr+1);*/
  //Serial.begin(115200);

  WiFi.mode(WIFI_OFF);  // TURN OFF WIFI
  WiFi.forceSleepBegin();
  delay(1);
  //delay(2000);
  //while (!Serial) {
  //yield();
  //}
  //Serial.println("deep");
  //while (Serial.available() == 0) {}
  ////String teststr = Serial.readString();  //read until timeout
  //teststr.trim();                        // remove any \r \n whitespace at the end of the String
  //if (teststr == "run") {
  //ESP.deepSleep(0);

  //}  //else {
  // Serial.println("deep");

  //  ESP.deepSleep(0);

  //}
  // Initialize SPIFFS
  if (!SPIFFS.begin()) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  WiFi.setOutputPower(0);  // this sets wifi to lowest power

  // Serial.println("start");
  /*pinMode(D8, INPUT_PULLUP);      // устанавливает режим работы - выход
    pinMode(3, FUNCTION_3);      // устанавливает режим работы - выход
    pinMode(3, INPUT_PULLUP);      // устанавливает режим работы - выход
    delay(10);
    int rr= digitalRead(D8);
    delay(10);

    int rr2= digitalRead(3);
    Serial.println(rr);

    Serial.println(rr2);
  */
  Serial.begin(115200);
  my_DEV_Module_Init();
  while (true) {
    while (!Serial) {
      yield();
    }

    String teststr = Serial.readStringUntil('\r');  //read until timeout
    teststr.trim();                                 // remove any \r \n whitespace at the end of the String
    if (teststr == "web") {
      break;
    }
    if (teststr.startsWith("p2p.se")) {
      p2pSenderMode();
    } else if (teststr.startsWith("p2p.re")) {
      p2pRecieverMode();
    }
  }
  //else {
  // Serial.println("deep");
  //  Serial.println("start2");

  /*
    pinMode(D3, OUTPUT);      // устанавливает режим работы - выход
    pinMode(D4, OUTPUT);
    pinMode(D2, OUTPUT);  digitalWrite(D2, HIGH);
    digitalWrite(D3, HIGH);   // включает светодиод
    digitalWrite(D4, HIGH);   // включает светодиод
  */


  // Wait for USB Serial
  /* while (!Serial) {
    yield();
    }*/

  /*Serial.println("Type any character to start");
    while (!Serial.available()) {
    yield();
    }*/
  delay(1000);

  WiFi.softAP(ssid, password);
  delay(2000);
  IPAddress myIP = WiFi.softAPIP();
  Serial.println("");

  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.flush();
  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);

  server.on(
    "/upload", HTTP_POST,  // if the client posts to the upload page
    []() {
      server.send(200);
    },                // Send status 200 (OK) to tell the client we are ready to receive
    handleFileUpload  // Receive and save the file
  );

  server.on("/postplain/", handlePlain);
  server.on("/download/", File_Download);
  server.on("/postform/", handleForm);

  server.on("/list/", handleList);

  // Route to load style.css file
  server.on("/jszip.min.js", jszip);

  server.on("/script.js", scriptHandle);
  server.on("/huff.js", huffHandle);
  server.on("/styles.css", stylesHandle);

  server.onNotFound(handleNotFound);
  //server.onFileUpload(onUpload);

  server.begin();
  Serial.println("HTTP server started");
  Serial.flush();
  Serial.println("started");
  Serial.flush();

  // Add service to MDNS-SD
  MDNS.addService("http", "tcp", 80);
  //return;
  // Initialize the SD.
  bool goodInited = true;
  if (!sd.begin(SD_CONFIG)) {
    sd.initErrorHalt(&Serial);
    goodInited = false;
  }

  if (goodInited) {

    // Open root directory
    if (!dir.open("/")) {
      error("dir.open failed");
    }

    // Open next file in root.
    // Warning, openNext starts at the current position of dir so a
    // rewind may be necessary in your application.
    middle += "<h2> folders</h2>";
    //middle += "<div>";
    middle += "<ul id=\"myUL\">";
    recDir(&dir, "/");
    dir.close();
    middle += "</div>";
    if (dir.getError()) {
      Serial.println("openNext failed");
    } else {
      Serial.println("Done!");
    }
  }
  Serial.println("middle:");
  Serial.println(middle);
}

//------------------------------------------------------------------------------
void loop() {
  if (webMode)
    server.handleClient();
}
