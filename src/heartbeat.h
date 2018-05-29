#ifndef HEARTBEAT_H
#define HEARTBEAT_H

//Assume init_uart() and init_can() have been called
#include <avr/eeprom.h>

//Includes in the user files
#include <uart/uart.h>
#include <can/can.h>
#include <uart/log.h>

//void init_heartbeat();
//void assign_heartbeat_status();

//CAN ids of each SSMs. Follow the convention of can_ids under lib-common
#define OBC_STATUS_RX_MOB_ID {0X001c}
#define OBC_STATUS_TX_MOB_ID {0X000b}
#define EPS_STATUS_RX_MOB_ID {0X001b}
#define EPS_STATUS_TX_MOB_ID {0X000a}
#define PAY_STATUS_RX_MOB_ID {0X001a}
#define PAY_STATUS_TX_MOB_ID {0X000c}

//EEPROM address assignment to store status of each SSM
//Address starts from 0x0000
//Use const uint16_t to decalre the address value and type casted when using
//eeprom functions.
const uint16_t INIT_WORD_EEMEM  = 0x0000; //4 bytes
const uint16_t OBC_STATUS_EEMEM = 0X0004; //1 byte
const uint16_t EPS_STATUS_EEMEM = 0X0005; //1 byte
const uint16_t PAY_STATUS_EEMEM = 0X0006; //1 byte

#define DEADBEEF 0Xdeadbeef //4 bytes

extern uint8_t obc_status; //global variables to store SSM status
extern uint8_t eps_status;
extern uint8_t pay_status;

extern uint8_t* self_status;
extern uint8_t* parent_status;
extern uint8_t* child_status;

extern uint8_t ssm_id; //will be changed by each SSM
//obc {0x00} eps {10} pay {01}

uint8_t fresh_start;

#endif
