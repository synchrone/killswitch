#include <Keyboard.h>
#include <Bounce2.h>
#include <EEPROM.h>
#include <avr/pgmspace.h>

#define LLED 13
#define PINS 1
static const uint8_t pins[] = {9, 10, 11}; // 3 digital pins. See also A0, A1, A2

#define DEBOUNCE_TIME 50
static Bounce debouncers[PINS];

#define SHCUT_SIZE 10 //Shortcut buffer size. 1 per simultaneous key, 1 for pause
#define ASCII_RS 0x30 //ASCII Record Separator, indicates a 100ms pause
#define ASCII_ETX 0x03 //ASCII End Of Transmission, indicates an end of a shortcut combination

char keyMap[PINS][SHCUT_SIZE];

typedef struct KeymapAssignEvent {
  unsigned int pin;
  char shortcut[SHCUT_SIZE];
} KeymapAssignEvent;
KeymapAssignEvent currentKMAEvent;

#define SIZEOF_CRC sizeof(unsigned long)
int eepromAddr(int pin, int chr) {
  // addresses below SIZEOF_CRC are reserved for CRC
  return SIZEOF_CRC + (pin * SHCUT_SIZE) + chr;
}
unsigned long computeEepromCrc(void) {

  const unsigned long crc_table[16] = { //TODO move to PROGMEM
    0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
    0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
    0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
    0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c
  };

  unsigned long crc = ~0L;

  for (int index = SIZEOF_CRC ; index < EEPROM.length(); ++index) {
    crc = crc_table[(crc ^ EEPROM[index]) & 0x0f] ^ (crc >> 4);
    crc = crc_table[(crc ^ (EEPROM[index] >> 4)) & 0x0f] ^ (crc >> 4);
    crc = ~crc;
  }
  return crc;
}
unsigned long readEepromCrc() {
  long four = EEPROM[0];
  long three = EEPROM[1];
  long two = EEPROM[2];
  long one = EEPROM[3];

  //Return the recomposed long by using bitshift.
  return ((four << 0) & 0xFF) + ((three << 8) & 0xFFFF) + ((two << 16) & 0xFFFFFF) + ((one << 24) & 0xFFFFFFFF);
}
void writeEepromCrc(unsigned long value) {
  Serial.println("# Writing CRC " + String(value));
  
  EEPROM.update(0, value & 0xFF);
  EEPROM.update(1, (value >> 8) & 0xFF);
  EEPROM.update(2, (value >> 16) & 0xFF);
  EEPROM.update(3, (value >> 24) & 0xFF);
}

boolean checkEeprom() {
  unsigned long storedCrc = readEepromCrc();
  unsigned long calculatedCrc = computeEepromCrc();
  Serial.println("#Checking EEPROM. StoredCRC: " + String(storedCrc) + ", CalculatedCRC: " + calculatedCrc);
  return storedCrc  == calculatedCrc;
}
boolean updateEepromKeymap(int pin) {
  if(pin < 0){
    for(int i = 0; i < PINS; i++){
      updateEepromKeymap(i);    
    }
    return true;
  }
  
  Serial.println("# Writing to EEPROM for pin #"+String(pin));
  for (int j = 0; j < SHCUT_SIZE; j++) {
    EEPROM.update(eepromAddr(pin, j), currentKMAEvent.shortcut[j]);
  }
  writeEepromCrc(computeEepromCrc());
  return checkEeprom();
}
void loadEepromKeymap() {
  for (int i = 0; i < PINS; i++) {
    Serial.print("#Read from EEPROM for pin #"+String(i)+": ");
    for (int j = 0; j < SHCUT_SIZE; j++) {
      keyMap[i][j] = (char)EEPROM[eepromAddr(i, j)];
    }
    Serial.println(String(keyMap[i]));
  }
}

const char defaultShortcut1[] PROGMEM = {KEY_LEFT_CTRL, KEY_LEFT_ALT, ASCII_RS, 'l', ASCII_ETX};
const char* const defaultKeyMap[] PROGMEM = {defaultShortcut1};
void loadPrebuiltKeymap(){
  Serial.println("#Loading shortcuts from precompiled defaults");

  for (int i = 0; i < sizeof(defaultKeyMap) / sizeof(*defaultKeyMap); i++) {
    strcpy_P(keyMap[i], (char*)pgm_read_word(&(defaultKeyMap[i])));
  }
}
void updateKeymap() {
  Serial.println("# Updating keymap for pin " + String(currentKMAEvent.pin) + ": " + String(currentKMAEvent.shortcut[currentKMAEvent.pin]));
  
  if(updateEepromKeymap(currentKMAEvent.pin)){
    memcpy(&keyMap[currentKMAEvent.pin], &currentKMAEvent.shortcut, SHCUT_SIZE);
    Serial.println("OK");
  }else{
    Serial.println("FAIL");
  }
}
boolean receiveSerialKeymap() {
  //cleanup buffer
  char buffer[SHCUT_SIZE + 1];
  memset( buffer, '\0', sizeof(char)*sizeof(buffer));

  //cleanup global event struct
  currentKMAEvent.pin = 255;
  memset(&currentKMAEvent.shortcut, 0x00, sizeof(char) * sizeof(currentKMAEvent.shortcut));

  //flash led for serial reading (time-consuming)
  digitalWrite(LLED, HIGH); 
  int bytesRead = Serial.readBytesUntil(ASCII_ETX, buffer, sizeof(buffer)); 
  while (Serial.available()) {
    Serial.read(); //dumping the rest of the transmission
  }
  digitalWrite(LLED, LOW);

  if (bytesRead > 0) {
    Serial.println("#RCV(" + String(bytesRead) + "): " + String(buffer));
    
    currentKMAEvent.pin = (int)buffer[0];
    memcpy(&currentKMAEvent.shortcut[0], &buffer[1], bytesRead - 1);
    currentKMAEvent.shortcut[bytesRead-1] = ASCII_ETX; //mark end of shortcut sequence

    if (currentKMAEvent.pin < PINS) { //basic check against available pins
      return true;
    } else {
      Serial.println("NoSuchPin " + String(currentKMAEvent.pin));
    }
  }

  return false;
}

void setupPins() {
  Serial.println("#Setting " + String(PINS) + " pins to INPUT_PULLUP");
  for (int i = 0; i < PINS; i++) {
    pinMode(pins[i], INPUT_PULLUP);
    debouncers[i] = Bounce();
    debouncers[i].attach(pins[i]);
    debouncers[i].interval(DEBOUNCE_TIME);
  }
}
boolean allPinsDown(){
  for (int i = 0; i < PINS; i++) {
    if(digitalRead(pins[i]) == HIGH){  
      return false;
    }
  }
  return true;
}

void setup() {
  Serial.begin(9600);
  Serial.println("#Killswitch Startup");

  setupPins();
  
  if (allPinsDown() || !checkEeprom()) {
    Serial.println("#EEPROM is corrupt or reset is requested");
    loadPrebuiltKeymap();
    updateEepromKeymap(-1);
  } else {
    Serial.println("#Reading EEPROM");
    loadEepromKeymap();
  }

  Keyboard.begin();
  Serial.println("#Startup done");
}

void loop() {
  if (Serial && receiveSerialKeymap()) {
    updateKeymap();
  }
  
  for (int i = 0; i < PINS; i++) {
    debouncers[i].update();
    
    if (debouncers[i].fell()) {
      
      for (int j = 0; j < SHCUT_SIZE; j++) {
        char keyCode = keyMap[i][j];
        Serial.print("#0x");Serial.print(keyCode, HEX);Serial.print(' ');
        
        if (keyCode == ASCII_RS) { //ascii record separator
          delay(200);
        } else if (keyCode == ASCII_ETX) { //ascii end of transmission
          break;
        } else {
          Keyboard.press(keyCode);
        }
      }
      Serial.print("\n");
      
    }else if(debouncers[i].rose()){
      Keyboard.releaseAll();
      Serial.print("#Released all keys\n"); 
    }
  }
}
