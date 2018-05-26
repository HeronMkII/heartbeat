#ifndef SYSTEM_TESTING_H
#define SYSTEM_TESTING_H

extern uint8_t SELF_state;
extern uint16_t SELF_EEPROM_ADDRESS;
extern uint8_t OBC_state;//2
extern uint8_t EPS_state;//3
extern uint8_t PAY_state;//4
extern uint8_t CAN_MSG_RCV;

extern mob_t rx_mob;
extern mob_t tx_mob;

void test_extern();

uint8_t test_extern_state(uint8_t option);

#endif
