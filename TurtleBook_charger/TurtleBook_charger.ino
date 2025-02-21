//#define OLED_ENABLED
#define SERIAL_ENABLED

#include <Wire.h>

#ifdef OLED_ENABLED
#include <U8g2lib.h>
#endif

#include <INA219_WE.h>
#include <Arduino.h>


#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/wdt.h>

#define I2C_ADDRESS 0x40

float loadVoltage_V = 0.0;


#ifdef OLED_ENABLED
U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, /* clock=*/SCL, /* data=*/SDA, /* reset=*/U8X8_PIN_NONE);  // All Boards without Reset of the Display
#endif





INA219_WE ina219 = INA219_WE(I2C_ADDRESS);
// the setup function runs once when you press reset or power the board
void setup() {
#ifdef SERIAL_ENABLED
  Serial.begin(115200);

  while (!Serial) {
    ;  // wait for serial port to connect. Needed for native USB port only
  }
#endif

  delay(1000);
  Wire.begin();

#ifndef SERIAL_ENABLED
  CLKPR = 0x80;
  CLKPR = 0x01;
#endif
  //ADCSRA = 0;
  // prints title with ending line break
#ifdef SERIAL_ENABLED
  Serial.println("started");
  //return;
  delay(1000);
#endif

  //ADCSRA = 0;
  ADCSRA &= ~(1 << ADEN);
  // initialize digital pin LED_BUILTIN as an output.
  //pinMode(LED_BUILTIN, OUTPUT);

  pinMode(5, OUTPUT);
  digitalWrite(5, HIGH);
  if (!ina219.init()) {
#ifdef SERIAL_ENABLED
    Serial.println("INA219 not connected!");
    delay(1000);
#endif

    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_mode();
  }
#ifdef SERIAL_ENABLED
  Serial.println("INA219  connected!");
  delay(1000);
#endif

  float shuntVoltage_mV = 0.0;

  float busVoltage_V = 0.0;



  shuntVoltage_mV = ina219.getShuntVoltage_mV();
  busVoltage_V = ina219.getBusVoltage_V();

  loadVoltage_V = busVoltage_V + (shuntVoltage_mV / 1000);
#ifdef SERIAL_ENABLED
  Serial.println("test");  // print as an ASCII-encoded decimal

  // print it out in many formats:
  Serial.println(busVoltage_V);     // print as an ASCII-encoded decimal
  Serial.println(shuntVoltage_mV);  // print as an ASCII-encoded decimal
  Serial.println(loadVoltage_V);    // print as an ASCII-encoded decimal
  delay(1000);



#endif
  ina219.powerDown();
//ina219.powerUp();
#ifdef OLED_ENABLED
  u8g2.begin();
  //u8g2.setContrast(0x5);
  u8g2.setFlipMode(1);
  u8g2.clearBuffer();

  u8g2.setFont(u8g2_font_logisoso18_tr);  // choose a suitable font
  u8g2.drawStr(0, 25, "V");

  u8g2.setCursor(55, 25);
  u8g2.print(u8x8_u8toa(loadVoltage_V, 3));


  u8g2.sendBuffer();
  delay(500);
#endif
}


// the loop function runs over and over again forever
int t = 0;
void loop() {

  //return;
  /*if(t==HIGH)t=LOW;
  else t=HIGH;
  digitalWrite(LED_BUILTIN, t);  // turn the LED on (HIGH is the voltage level)
*/
  float shuntVoltage_mV = 0.0;

  float busVoltage_V = 0.0;
#ifdef OLED_ENABLED
  ina219.init();
#endif
  ina219.powerUp();

  shuntVoltage_mV = ina219.getShuntVoltage_mV();
  busVoltage_V = ina219.getBusVoltage_V();

  loadVoltage_V = busVoltage_V + (shuntVoltage_mV / 1000);

  /*if(loadVoltage_V<3.7)
  {
     digitalWrite(5, LOW);  // turn the LED on (HIGH is the voltage level)
  }else{
     digitalWrite(5,HIGH);  // turn the LED on (HIGH is the voltage level)
  }*/

  if (loadVoltage_V >= 3.72) {
    digitalWrite(5, HIGH);  // turn the LED on (HIGH is the voltage level)
  }
  if (loadVoltage_V <= 3.69) {
    digitalWrite(5, LOW);  // turn the LED on (HIGH is the voltage level)
  }
  /*if(t==HIGH)t=LOW;
  else t=HIGH;
  digitalWrite(5, t);  // turn the LED on (HIGH is the voltage level)
*/
  ina219.powerDown();
#ifdef OLED_ENABLED

  u8g2.begin();
  u8g2.clearBuffer();

  u8g2.setFont(u8g2_font_logisoso18_tr);  // choose a suitable font
  u8g2.drawStr(0, 25, "V");

  u8g2.setCursor(55, 25);
  u8g2.print(loadVoltage_V, 3);


  u8g2.sendBuffer();
  delay(2000);
#endif

#ifdef SERIAL_ENABLED

  Serial.println("test");  // print as an ASCII-encoded decimal

  // print it out in many formats:
  Serial.println(busVoltage_V);     // print as an ASCII-encoded decimal
  Serial.println(shuntVoltage_mV);  // print as an ASCII-encoded decimal
  Serial.println(loadVoltage_V);    // print as an ASCII-encoded decimal
  delay(1000);
#endif

  wdt_enable(WDTO_8S);    //Задаем интервал сторожевого таймера (2с)
  WDTCSR |= (1 << WDIE);  //Устанавливаем бит WDIE регистра WDTCSR для разрешения прерываний от сторожевого таймера
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_mode();


  //delay(1000);                      // wait for a second
  //digitalWrite(LED_BUILTIN, LOW);   // turn the LED off by making the voltage LOW
  // delay(1000);                      // wait for a second
}

ISR(WDT_vect) {
  wdt_disable();
  //f = !f;
}
