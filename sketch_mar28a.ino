#include <Wire.h>
#include <LiquidCrystal.h>

#define RDA5807M_RANDOM_ACCESS_ADDRESS 0x11
// регистры
#define RDA5807M_REG_CONFIG 0x02
#define RDA5807M_REG_TUNING 0x03
#define RDA5807M_REG_VOLUME 0x05
#define RDA5807M_REG_RSSI   0x0B
// флаги
#define RDA5807M_FLG_DHIZ 0x8000
#define RDA5807M_FLG_DMUTE 0x4000
#define RDA5807M_FLG_BASS 0x1000
#define RDA5807M_FLG_ENABLE word(0x0001)
#define RDA5807M_FLG_TUNE word(0x0010)
//маски
#define RDA5807M_CHAN_MASK 0xFFC0
#define RDA5807M_CHAN_SHIFT 6
#define RDA5807M_VOLUME_MASK word(0x000F)
#define RDA5807M_VOLUME_SHIFT 0
#define RDA5807M_RSSI_MASK 0xFE00
#define RDA5807M_RSSI_SHIFT 9

uint8_t volume = 3; // 0..15
uint16_t freq = 870; // 107.3FM
uint16_t reg02h, reg03h, reg05h, reg0Bh;

unsigned long int t;

const int rs = 8, en = 9, d4 = 4, d5 = 5, d6 = 6, d7 = 7;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

void setup() {
  Serial.begin(9600);
  Wire.begin();
  
  lcd.begin(16, 2);
  
  // Регистр 02h - включение, настройки
  reg02h = RDA5807M_FLG_ENABLE | RDA5807M_FLG_DHIZ | RDA5807M_FLG_DMUTE;
  setRegister(RDA5807M_REG_CONFIG, reg02h);
  
  // А потом решили еще усилить басы:
  reg02h |= RDA5807M_FLG_BASS;
  setRegister(RDA5807M_REG_CONFIG, reg02h);
  
  // Регистр 03h - выбор радиостанции
  // После сброса в регистре 03h значение по умолчанию - 0
  // Таким образом BAND = 00 (87..108МГц), STEP = 00 (100кГц). Оставим их как есть
  reg03h = (freq - 870) << RDA5807M_CHAN_SHIFT; // chan = (freq - band) / space
  setRegister(RDA5807M_REG_TUNING, reg03h | RDA5807M_FLG_TUNE);
  
  // Регистр 05h. Установим громкость, остальные биты не изменяем
  reg05h = getRegister(RDA5807M_REG_VOLUME); // Считываем текущее значение
  reg05h &= ~RDA5807M_VOLUME_MASK; // Сбрасываем биты VOLUME
  reg05h |= volume << RDA5807M_VOLUME_SHIFT; // Устанавливаем новую громкость
  setRegister(RDA5807M_REG_VOLUME, reg05h);
  
  lcd.clear();
  
  lcd.setCursor(12, 0);
  lcd.print("RSSI");

  lcd.setCursor(1, 0);
  lcd.print("CHNL");
  lcd.setCursor(1, 1);
  lcd.print(freq);

  lcd.setCursor(7, 0);
  lcd.print("VOL");
  lcd.setCursor(7, 1);
  lcd.print(volume);
  
}

void loop() {
  if (t + 500 >= millis()){
  reg0Bh = getRegister(RDA5807M_REG_RSSI);
  uint8_t RSSI = (reg0Bh & RDA5807M_RSSI_MASK) >> RDA5807M_RSSI_SHIFT;
  Serial.print("RSSI = ");
  Serial.print(RSSI);
  Serial.println(" (0-min, 127-max)");
  
  lcd.setCursor(13, 1);
  lcd.print("   ");
  lcd.setCursor(13, 1);
  lcd.print(RSSI);
  t = millis();
  }
  //53249 32769

  int a = analogRead(A0);
  if(a > 600 && a < 850){
    //mute
  Serial.print(getRegister(RDA5807M_REG_CONFIG));
  if (reg02h == 53249){
    reg02h = 32769;
    lcd.setCursor(7, 1);
    lcd.print("---");
    }
  else {
    reg02h = 53249;
    lcd.setCursor(7, 1);
    lcd.print("   ");
    lcd.setCursor(7, 1);
    lcd.print(volume);
  }
    setRegister(RDA5807M_REG_CONFIG, reg02h);
    
  delay(500);
  }
  if(a > 450 && a < 600){
    //-ch
    freq -= 5;
    reg03h = (freq - 870) << RDA5807M_CHAN_SHIFT;
    setRegister(RDA5807M_REG_TUNING, reg03h | RDA5807M_FLG_TUNE);
    
    lcd.setCursor(1, 1);
    lcd.print("   ");
    lcd.setCursor(1, 1);
    lcd.print(freq);
    delay(500);
  }
  if(a > 250 && a < 450){
    //-vol
    volume -= 1;
    reg05h = getRegister(RDA5807M_REG_VOLUME); // Считываем текущее значение
    reg05h &= ~RDA5807M_VOLUME_MASK; // Сбрасываем биты VOLUME
    reg05h |= volume << RDA5807M_VOLUME_SHIFT; // Устанавливаем новую громкость
    setRegister(RDA5807M_REG_VOLUME, reg05h);
    
    lcd.setCursor(7, 1);
    lcd.print("   ");
    lcd.setCursor(7, 1);
    lcd.print(volume);
    delay(500);
  }
  if(a > 80 && a < 250){
    //+vol

    volume += 1;
    reg05h = getRegister(RDA5807M_REG_VOLUME); // Считываем текущее значение
    reg05h &= ~RDA5807M_VOLUME_MASK; // Сбрасываем биты VOLUME
    reg05h |= volume << RDA5807M_VOLUME_SHIFT; // Устанавливаем новую громкость
    setRegister(RDA5807M_REG_VOLUME, reg05h);
    
    lcd.setCursor(7, 1);
    lcd.print("   ");
    lcd.setCursor(7, 1);
    lcd.print(volume);
    delay(500);
  }
  if(a < 80){
    //+ch

    freq += 5;
    reg03h = (freq - 870) << RDA5807M_CHAN_SHIFT;
    setRegister(RDA5807M_REG_TUNING, reg03h | RDA5807M_FLG_TUNE);
    
    lcd.setCursor(1, 1);
    lcd.print("   ");
    lcd.setCursor(1, 1);
    lcd.print(freq);
    delay(500);
  }
}

void setRegister(uint8_t reg, const uint16_t value) {
  Wire.beginTransmission(0x11);
  Wire.write(reg);
  Wire.write(highByte(value));
  Wire.write(lowByte(value));
  Wire.endTransmission(true);
}

uint16_t getRegister(uint8_t reg) {
  uint16_t result;
  Wire.beginTransmission(RDA5807M_RANDOM_ACCESS_ADDRESS);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom(0x11, 2, true);
  result = (uint16_t)Wire.read() << 8;
  result |= Wire.read();
  return result;
}
