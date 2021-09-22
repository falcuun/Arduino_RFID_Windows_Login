
#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 10
#define RST_PIN 9
#define Buzzer_Pin 3

MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class
MFRC522::MIFARE_Key key;

// Init array that will store new NUID
byte expectedNUID[4] = {130, 130, 198, 223};
byte password[20];
byte password2[20];

void setup()
{
  Serial.begin(9600);
  pinMode(Buzzer_Pin, OUTPUT);
  SPI.begin();     // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522
}

bool compareNUID(byte *newArr)
{
  for (int i = 0; i < 4; i++)
  {
    if (newArr[i] != expectedNUID[i])
      return false;
  }
  return true;
}

bool is_password_empty(byte *arr, size_t len){
  for(int i = 0; i < len; i++){
    if(arr[i] == 0x00 || arr[i] == 0x20){
      return true;
    }
  }
}

void loop()
{

  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if (!rfid.PICC_IsNewCardPresent())
    return;

  // Verify if the NUID has been readed
  if (!rfid.PICC_ReadCardSerial())
    return;

  if (compareNUID(rfid.uid.uidByte)) {
    Keyboard.press(KEY_ESC);
    delay(10);
    Keyboard.release(KEY_ESC);
    tone(Buzzer_Pin, 500);
    delay(300);
    noTone(Buzzer_Pin);
    delay(500);
    if (is_password_empty(password, 16)|| is_password_empty(password2, 2)) {
      Serial.println("Password not cached");
      Read_From_Block(8, password);
      Read_From_Block(9, password2);
    }
    for (int i = 0; i < 16; i++) {
      Keyboard.print((char)password[i]);
      delay(100);
    }
    delay(100);
    for (int i = 0; i < 2; i++) {
      Keyboard.print((char)password2[i]);
      delay(100);
    }
    Keyboard.press(KEY_ENTER);
    delay(10);
    Keyboard.release(KEY_ENTER);
  }
  // Halt PICC
  //  Write_To_Block(8, buffer, 16);
  //  Write_To_Block(9, buffer2, 16);

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();

}

void Write_To_Block(int block, byte *message, size_t len)
{
  for (byte i = 0; i < 6; i++)
  {
    key.keyByte[i] = 0xFF;
  }
  MFRC522::StatusCode status;
  status = rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(rfid.uid));
  if (status != MFRC522::STATUS_OK)
  {
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(rfid.GetStatusCodeName(status));
    return;
  }

  // Write block
  status = rfid.MIFARE_Write(block, message, len);
  if (status != MFRC522::STATUS_OK)
  {
    Serial.print(F("MIFARE_Write() failed: "));
    Serial.println(rfid.GetStatusCodeName(status));
    return;
  }
  else
    Serial.println(F("MIFARE_Write() success: "));
}

void Read_From_Block(int block, byte* arr)
{
  MFRC522::StatusCode status;
  MFRC522::MIFARE_Key key;
  byte buffer[20];
  byte size = sizeof(buffer);
  for (byte i = 0; i < MFRC522::MF_KEY_SIZE; ++i) {
    key.keyByte[i] = 0xFF;
  }
  status = rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 8, &key, &(rfid.uid));
  if (status == MFRC522::STATUS_OK) {
    status = rfid.MIFARE_Read(block, buffer, &size);
    if (status == MFRC522::STATUS_OK) {
      for (int i = 0; i < 18; i++) {
        arr[i] = buffer[i];
      }
      Serial.println(F("Reading Successfull"));
    } else {
      Serial.println(F("Reading Failed"));
    }
  } else {
    Serial.println(F("Auth Failed"));
  }

}
