#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>  
#include <ArduinoJson.h>

const char* ssid = "Anamika";
const char* password = "Radhe-Radhe@2144";

#define BOTtoken "7159480047:AAFqTzYhNNEDa0vyLqkROPxY44am9o-_3q8"  

#define CHAT_ID "792992608"

#ifdef ESP8266
  X509List cert(TELEGRAM_CERTIFICATE_ROOT);
#endif

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

int botRequestDelay = 1000;
unsigned long lastTimeBotRan;

bool isSecondCard = false;
bool isFirstCard = false;

// Handle received messages
void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i=0; i<numNewMessages; i++) {
    // Chat id of the requester
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID){
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }
    
    // Print received message
    String text = bot.messages[i].text;
    Serial.println(text);

    String from_name = bot.messages[i].from_name;

    if (text == "/start") {
      String welcome = "Welcome, " + from_name + ".\n";
      welcome += "Use the following commands:.\n\n";

      welcome += "/getinfo1 to get 1st  child details \n";
       welcome += "/getinfo2 to get 2st  child details \n";
      bot.sendMessage(chat_id, welcome, "");
    }
    
    if (text == "/getinfo1") {
                if (isFirstCard == true) {
                  bot.sendMessage(CHAT_ID,"Child1 Present ", "");
                      sendStudentInfo(0);
                } else{
                   bot.sendMessage(CHAT_ID,"Child Not Present ", "");
                }
    }
     if (text == "/getinfo2") {
                 if (isSecondCard == true) {
                   bot.sendMessage(CHAT_ID,"Child2 Present ", "");
                      sendStudentInfo(1);
                } else{
                   bot.sendMessage(CHAT_ID,"Child Not Present ", "");
                }
    }
  }
}

#define SDA_PIN 21   // SDA pin of RFID reader connected to GPIO 21 (SDA)
#define SCK_PIN 18   // SCK pin of RFID reader connected to GPIO 18 (SCK)
#define MOSI_PIN 23  // MOSI pin of RFID reader connected to GPIO 23 (MOSI)
#define MISO_PIN 19  // MISO pin of RFID reader connected to GPIO 19 (MISO)
#define IRQ_PIN 2    // IRQ pin of RFID reader connected to GPIO 2
#define RST_PIN 5    // RST pin of RFID reader connected to GPIO 5

MFRC522 mfrc522(SDA_PIN, RST_PIN);  // Create MFRC522 instance

struct Student {
  char name[20];
  char rollNo[20];
  char classInfo[10];
};

struct Student studentData[] = {
  { "anss", "11", "Class 4" },
  { "test", "12", "Class 3" },
};

void setup() {
  Serial.begin(115200);                    // Initialize serial communications
  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN);  // Initialize SPI bus with custom pins
  mfrc522.PCD_Init();                      // Initialize RFID reader

  #ifdef ESP8266
   configTime(0, 0, "pool.ntp.org");      // get UTC time via NTP
    client.setTrustAnchors(&cert);         // Add root certificate for api.telegram.org
  #endif

  // Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  #ifdef ESP32
    client.setCACert(TELEGRAM_CERTIFICATE_ROOT);   // Add root certificate for api.telegram.org
  #endif
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
}

void loop() {

  lookForNewMSG();

  // Look for new cards
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    delay(50);
    return;
  }

  // Show some details of the card (for debugging purposes)
  Serial.print(F("Card UID:"));
  dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
  Serial.println();

  // Halt PICC
  mfrc522.PICC_HaltA();

  // Stop encryption on PCD
  mfrc522.PCD_StopCrypto1();
}


void dump_byte_array(byte *buffer, byte bufferSize) {

  char uidString[20];

  sprintf(uidString, "%02X %02X %02X %02X", buffer[0], buffer[1], buffer[2], buffer[3]);
  Serial.print(uidString);
  Serial.println(" ");
  Serial.println(" ");

  if (strcmp(uidString, "03 7A 58 ED") == 0) {
  isFirstCard = true;
   bot.sendMessage(CHAT_ID," ", "");
   bot.sendMessage(CHAT_ID,"Child 1 borded on Bus \n ", "");
 sendStudentInfo(0);
  bot.sendMessage(CHAT_ID," ", "");
  } 
   else if (strcmp(uidString, "03 RA 58 ED") == 0) {
     isSecondCard = true;
      bot.sendMessage(CHAT_ID," ", "");
      bot.sendMessage(CHAT_ID,"Child 2 borded on Bus \n ", "");
 sendStudentInfo(1);
  bot.sendMessage(CHAT_ID," ", "");
  } 
  
  else {
    Serial.print("UnKnown Tag: ");
  }
}

void sendStudentInfo(int index) {
  String message = "Student Name: ";
  message += studentData[index].name;
  message += "\nRoll No: ";
  message += studentData[index].rollNo;
  message += "\nClass: ";
  message += studentData[index].classInfo;

  bot.sendMessage(CHAT_ID, message, "");
}

void lookForNewMSG(){
  if (millis() > lastTimeBotRan + botRequestDelay)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while(numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
}


