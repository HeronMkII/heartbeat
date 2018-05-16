#ifndef HEARTBEAT_H
#define HEARTBEAT_H


/*Heartbeat functions
By Brytni on April 28, 2018
Contains all heartbeat.c functions
*/

//Defines
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

extern uint8_t SELF_state;
extern uint16_t SELF_EEPROM_ADDRESS;
extern uint8_t OBC_state;//2
extern uint8_t EPS_state;//3
extern uint8_t PAY_state;//4
extern uint8_t CAN_MSG_RCV;

extern mob_t rx_mob;
extern mob_t tx_mob;

void rx_callback(uint8_t*, uint8_t);
void tx_callback(uint8_t*, uint8_t*);
void init_eeprom();

uint8_t len_check(uint8_t len);
uint8_t increment_check(uint8_t old_val, uint8_t new_val);
uint8_t same_val_check(uint8_t old_val, uint8_t new_val);
uint8_t in_range_check(uint8_t state, uint8_t min, uint8_t max);


#endif
