#include <uart/uart.h>
#include <uart/log.h>
#include <can/can.h>
#include <avr/eeprom.h>
#define F_CPU 8
#include <util/delay.h>

void rx_callback(uint8_t*, uint8_t);
void tx_callback(uint8_t*, uint8_t*);
void init_eeprom();

uint8_t error_check(uint8_t* state_data, uint8_t len);
uint8_t len_check(uint8_t len);
uint8_t increment_check(uint8_t* old_val, uint8_t new_val);
uint8_t same_val_check(uint8_t* old_val, uint8_t new_val);
uint8_t is_empty_check(uint8_t* state);
uint8_t in_range_check(uint8_t state, uint8_t min, uint8_t max);


//heartbeat ID for each SSM to send CAN messages
#define OBC_PARENT 0x001c
#define OBC_CHILD  0x000b
#define PAY_PARENT 0x001a
#define PAY_CHILD  0x000c
#define EPS_PARENT 0x001b
#define EPS_CHILD  0x000a

//define fixed EEPROM address to store the states of each SSM.
//addresses are chosen arbitrary. Possible future work-> EEPROM organization
#define OBC_EEPROM_ADDRESS 0x00 //0 in base 10
#define PAY_EEPROM_ADDRESS 0x08 //8 in base 10
#define EPS_EEPROM_ADDRESS 0x10 //16 in base 10
#define INIT_WORD 0x18 //24 in base 10, stores init_word

#define DEADBEEF 0xdeadbeef
