/*
For use with:
https://www.skpang.co.uk/collections/teensy/products/teensy-4-0-classic-can-can-fd-board-with-800x480-5-0-touch-lcd

Requires LVGL V9.1.0

*/


#include "SSD1963.h"
#include "skp_lvgl.h"
#include <lvgl.h>
#include "ui.h"
#include "ui_helpers.h"


#include <FlexCAN_T4.h>

FlexCAN_T4FD<CAN3, RX_SIZE_256, TX_SIZE_16> FD;

#define RS  26
#define WR  27
#define CS  25
#define RST 28
#define BL_CTL  21
bool set_can_start = 1;
bool set_can_start_old = 0;

uint16_t frame_count = 0;

uint8_t d=0;
bool stopfd = 0;
int led = 13;

SSD1963 lcd(RS, WR, CS, RST);  // Remember to change the model parameter to suit your display module!
elapsedMillis task1;

void sendframe()
{
  CAN_message_t msg2;
  CANFD_message_t msg;
  msg.len = 64;
  msg.id = 0x321;
  msg.seq = 1;
  msg.buf[0] = 0xff & frame_count; msg.buf[1] =(0xff00 & frame_count) >> 8; ; msg.buf[2] = 3; msg.buf[3] = 4;
  msg.buf[4] = 5; msg.buf[5] = 6; msg.buf[6] = 9; msg.buf[7] = 9;
  FD.write(msg);
  

  String frame_count_str;
  frame_count_str = String(frame_count);
  lv_label_set_text(ui_framecount,frame_count_str.c_str());
  
  frame_count++;
}

void canSniff(const CANFD_message_t &msg) {

  Serial.print("ISR - MB "); Serial.print(msg.mb);
  Serial.print("  OVERRUN: "); Serial.print(msg.flags.overrun);
  Serial.print("  LEN: "); Serial.print(msg.len);
  Serial.print(" EXT: "); Serial.print(msg.flags.extended);
  Serial.print(" TS: "); Serial.print(msg.timestamp);
  Serial.print(" ID: "); Serial.print(msg.id, HEX);
  Serial.print(" Buffer: ");
  for ( uint8_t i = 0; i < msg.len; i++ ) {
    Serial.print(msg.buf[i], HEX); Serial.print(" ");
  } Serial.println();
}

void setup()
{
  pinMode(led, OUTPUT);
  pinMode(BL_CTL, OUTPUT);
  digitalWrite(led, HIGH);   
  digitalWrite(BL_CTL, HIGH);   
  delay(500);              
  digitalWriteFast(led, LOW);    
  
  FD.begin();
  CANFD_timings_t config;
  config.clock = CLK_24MHz;
  config.baudrate =   500000;     //500kbps
  config.baudrateFD = 2000000;    //2000kbps
  config.propdelay = 190;
  config.bus_length = 1;
  config.sample = 75;
  FD.setRegions(64);
  FD.setBaudRate(config);
  //FD.onReceive(canSniff);

  FD.setMBFilter(ACCEPT_ALL);

  FD.distribute();
  FD.mailboxStatus();

  lcd.InitLCD();
  // r g b
  lcd.setColor(0, 20,80);
  lcd.fillRect(0, 0, 800, 480);
  
  lcd.setColor(0, 0,255);
  lcd.drawLine(0,0,800,480);

  lcd.setColor(0, 255,0);
  lcd.drawLine(800,0,0,480);

  lcd.setColor(255, 0,0);
  lcd.drawLine(800,240,0,240);
  

  Serial.println("800x480 SSD1963 test skpang.co.uk 2023");
  skp_lvgl_init();

}

void loop()
{
CANFD_message_t msg;
  lv_timer_handler(); /* let the GUI do its work */

  if(set_can_start != set_can_start_old)
  {
    Serial.print("can start ");
    Serial.println(set_can_start);
    set_can_start_old = set_can_start;
  }
  if (task1 >= 200)
  { 
    task1 = task1 - 200;
    if(set_can_start == 1) sendframe();
  }

  FD.events();
  if(FD.readMB(msg)) {

      Serial.print("MB: "); Serial.print(msg.mb);
      Serial.print("  OVERRUN: "); Serial.print(msg.flags.overrun);
      Serial.print("  ID: 0x"); Serial.print(msg.id, HEX );
      Serial.print("  EXT: "); Serial.print(msg.flags.extended );
      Serial.print("  LEN: "); Serial.print(msg.len);
      Serial.print("  BRS: "); Serial.print(msg.brs);
      Serial.print(" DATA: ");
      for ( uint8_t i = 0; i <msg.len ; i++ ) {
        Serial.print(msg.buf[i]); Serial.print(" ");
      }
      Serial.print("  TS: "); Serial.println(msg.timestamp);
    
  }

  //delay(5);
}
