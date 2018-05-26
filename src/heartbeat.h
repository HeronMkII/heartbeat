#ifndef HEARTBEAT_H
#define HEARTBEAT_H

//Assume init_uart() and init_can() have been called
#include <avr/eeprom.h>
#include <stdbool.h>

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
#define INIT_WORD_EEMEM 0x0000
#define OBC_STATUS_EEMEM 0X0004
#define EPS_STATUS_EEMEM 0X0008
#define PAY_STATUS_EEMEM 0X000c

#define DEADBEEF 0Xdeadbeef //4 bytes

extern uint8_t obc_status; //global variables to store SSM status
extern uint8_t eps_status;
extern uint8_t pay_status;

extern uint8_t* self_status;
extern uint8_t* parent_status;
extern uint8_t* child_status;

extern uint8_t ssm_id; //will be changed by each SSM
//obc {0x00} eps {10} pay {01}

extern bool fresh_start;

#endif
