// For "Heltec WiFi LoRa 32 (V3)", SF12@125kHz, codingrate 4/8:

// SF12, 250kHz, 30 chars = less than 3.937 seconds, delay(3950)

#include "LoRaWan_APP.h"
#include "Arduino.h"

#define CHARDELAY                                   3950

#define RF_FREQUENCY                                868000000 // 915US or 868EU MHz, 433Asia

#define TX_OUTPUT_POWER                             22        // dBm up to 22

#define LORA_BANDWIDTH                              1         // [0: 125 kHz,
                                                              //  1: 250 kHz,
                                                              //  2: 500 kHz,
                                                              //  3: Reserved]
#define LORA_SPREADING_FACTOR                       12         // [SF7..SF12]
#define LORA_CODINGRATE                             4         // [1: 4/5,
                                                              //  2: 4/6,
                                                              //  3: 4/7,
                                                              //  4: 4/8]
#define LORA_PREAMBLE_LENGTH                        8         // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT                         0         // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false
#define LORA_IQ_INVERSION_ON                        false


#define RX_TIMEOUT_VALUE                            4000

#define CHARS_IN_PACKET                             30

int countchars = 0;
char txpacket[CHARS_IN_PACKET+1];

static RadioEvents_t RadioEvents;

int16_t txNumber;
int16_t rssi,rxSize;

void setup() {
    Serial.begin(9600);
    Mcu.begin(HELTEC_BOARD,SLOW_CLK_TPYE);
    
    txNumber=0;
    rssi=0;
  
    RadioEvents.RxDone = OnRxDone;
    RadioEvents.TxDone = NULL;
    RadioEvents.RxTimeout = OnRxTimeout;
    RadioEvents.TxTimeout = NULL;
    
    Radio.Init( &RadioEvents );
    Radio.SetChannel( RF_FREQUENCY );
    Radio.SetTxConfig( MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                                   LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                                   LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   true, 0, 0, LORA_IQ_INVERSION_ON, RX_TIMEOUT_VALUE );

    Radio.SetRxConfig( MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                               LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                               LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                               0, true, 0, 0, LORA_IQ_INVERSION_ON, true );
    Radio.SetSyncWord((byte) 0xB6);
}


void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr ) {
  char k = 0;
  for (countchars = 0; countchars <= CHARS_IN_PACKET; countchars++) {
    k = payload[countchars];
    if (k == 0) { break; }
    Serial.print(k); // Serial.print('['); Serial.print((int) k); Serial.print(']');
  }
  Serial.flush();
}

void OnRxTimeout(void) {
  Serial.println("TIMEOUT");
}

void Listener() {
        Radio.Rx(RX_TIMEOUT_VALUE);
        Radio.IrqProcess();   
}

void Talker() {
  for (countchars = 0; countchars <= CHARS_IN_PACKET; countchars++) {
    txpacket[countchars] = 0;
  }
  countchars = 0;
  char c = 0;
  while (c != '\n') { // instead of ((c != '\n') && (c != '\r')), as LF ends anyway
        if (Serial.available()) {
          c = Serial.read();
          txpacket[countchars] = c;
          countchars++;
          Serial.print(c);
          delay(1 + CHARDELAY/CHARS_IN_PACKET);
          if (countchars == CHARS_IN_PACKET) {
            Radio.Send( (uint8_t *)txpacket, CHARS_IN_PACKET);
            Radio.IrqProcess();
            // delay(CHARDELAY); // INSTEAD OF THIS, WAIT ON PRINT
            for (countchars = 0; countchars <= CHARS_IN_PACKET; countchars++) {
              txpacket[countchars] = 0;
            }
            countchars = 0;
          }
        }
  }
  Radio.Send( (uint8_t *)txpacket, CHARS_IN_PACKET+1);
  Radio.IrqProcess();
  delay(3*CHARDELAY/4); // assuming "a quarter of max" has been typed, typically.

}


void loop() {
  if (Serial.available()) {
    Talker();
  } else { Listener(); }
}
