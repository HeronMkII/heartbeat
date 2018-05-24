//Error checking module
//This code should be used to ensure that error checking in heartbeat.c
//is consistent and robust
//uint8_t main(void) {

//}
//How to handle empty states/empty state values?
//Is that an issue/is possible?
//Check state within heatbeat vs state outside of heartbeat
//

/*Heartbeat 2.0 (Yay!!)
By Jack, Bonnie and Brytni April 4, 2018
This iteration of heartbeat design has the following assumptions:
1. This code is written from the perspective of OBC
   (Will generalize this code, to lib-common structure soon)
2. The triangular configuration of heartbeat is set as:
    OBC -> PAY (OBC sends state data to PAY) (PAY is the parent of OBC)
    PAY -> EPS (PAY sends state data to EPS)
    EPS -> OBC (EPS sends state data to OBC)
3. When there is a state update, a SSM needs to first update its own EEPROM,
   and then send the updated state data to its parent via CAN.
4. The state data transmits through CAN, is an array with size of uint8_t, and
     length of 3. state data stored in order in the array as OBC, PAY, EPS.
5. Error checking for state data is implemented (beta).
Other Note: This code is untested (but should work in theory)

1. Goes out of loop if can message is valid
2. Does not go into loop if not fresh fresh_restart
3. test_10_beats to see if it really increments to 10 and stays that way on restart
4. test_heartbeat and test_heartbeat2
*/
/*
#include <uart/uart.h>
#include <uart/log.h>
#include <can/can.h>
#include <avr/eeprom.h>
#define F_CPU 8
#include <util/delay.h>
//#include <heartbeat/heartbeat.h>
#include "heartbeat.h"

uint8_t SELF_state = 0;//0 at fresh restart
uint16_t SELF_EEPROM_ADDRESS = 0x0000;//OBC
uint8_t OBC_state = 0;//0 at fresh restart
uint8_t EPS_state = 0;//0
uint8_t PAY_state = 0;//0

extern uint8_t CAN_MSG_RCV;

mob_t rx_mob = {
  .mob_num = 0,
  .mob_type = RX_MOB,
  .dlc = 1,
  .id_tag = {  }, // ID of this nodes parent
  .id_mask = { 0x00f },
  .ctrl = default_rx_ctrl,
  .rx_cb = rx_callback
};

mob_t tx_mob = {
  .mob_num = 1,
  .mob_type = TX_MOB,
  .id_tag = { }, // ID of this nodes child
  .ctrl = default_tx_ctrl,
  .tx_data_cb = tx_callback
};

//testing
void test_valid_range() {
  SELF_state = -5;
  uint8_t SELF_end_state = SELF_state + 15;

  for (uint8_t i =0;i<20;i++) {
    //wait 200 ms before sending messages
    SELF_state += 1;
    resume_mob(&tx_mob);
    while (!is_paused(&tx_mob)) {}
    print("Current state is %d\n", SELF_state);
    in_range_check(SELF_state,0,2);
    print("\n");
  }
}

void test_increment() {
  SELF_state = 0;
  print("\n\nThese increments should be valid\n");
  for (uint8_t i =0;i<5;i++) {
    print("Increment by one:\n");
    SELF_state += 1;
    increment_check(eeprom_read_byte((uint8_t*)SELF_EEPROM_ADDRESS),SELF_state);
    resume_mob(&tx_mob);
    while (!is_paused(&tx_mob)) {}
    print("\n");
  }
  SELF_state = 0;
  print("\n\nThese increments should not be valid\n");
  for (uint8_t i =0;i<5;i++) {
    print("Increment by negative one:\n");
    SELF_state += -1;
    increment_check(eeprom_read_byte((uint8_t*)SELF_EEPROM_ADDRESS),SELF_state);
    resume_mob(&tx_mob);
    while (!is_paused(&tx_mob)) {}
    print("\n");
  }
  SELF_state = 0;
  print("\n\nThese increments should not be valid\n");
  for (uint8_t i =0;i<5;i++) {
    print("Increment by zero:\n");
    SELF_state += 0;
    increment_check(eeprom_read_byte((uint8_t*)SELF_EEPROM_ADDRESS),SELF_state);
    resume_mob(&tx_mob);
    while (!is_paused(&tx_mob)) {}
    print("\n");
  }
  print("\n");
}

void test_same_val() {
  //Checks to make sure SELF_state global is the same value as in EEPROM
  SELF_state = 0;
  print("\n\nThese increments should not be valid\n");
  for (uint8_t i =0;i<5;i++) {
    print("Increment by one:\n");
    SELF_state += 1;
    same_val_check(eeprom_read_byte((uint8_t*)SELF_EEPROM_ADDRESS),SELF_state);
    resume_mob(&tx_mob);
    while (!is_paused(&tx_mob)) {}
    print("\n");
  }
  SELF_state = 0;
  print("\n\nThese increments should not be valid\n");
  for (uint8_t i =0;i<5;i++) {
    print("Increment by negative one:\n");
    SELF_state += -1;
    same_val_check(eeprom_read_byte((uint8_t*)SELF_EEPROM_ADDRESS),SELF_state);
    resume_mob(&tx_mob);
    while (!is_paused(&tx_mob)) {}
    print("\n");
  }
  SELF_state = 0;
  print("\n\nThese increments should be valid\n");
  print("%d Self_state \n", SELF_state);
  for (uint8_t i =0;i<5;i++) {
    print("Increment by zero:\n");
    SELF_state += 0;
    same_val_check(eeprom_read_byte((uint8_t*)SELF_EEPROM_ADDRESS),SELF_state);
    resume_mob(&tx_mob);
    while (!is_paused(&tx_mob)) {}
    print("\n");
  }
  print("\n");
}
/*
uint8_t increment_check(uint8_t old_val, uint8_t new_val) {
  //Assume that only valid increment is ++ or same
  if (new_val == old_val +1) {
    print("Passed increment_check\n");
    return 1;
  }
  print("Error: Incorrect increment\n");
  return 0;
}

uint8_t in_range_check(uint8_t state, uint8_t min, uint8_t max) {
  //Verify that state is within valid range
  if (state < min || state > max) {
    print("Error: Invalid state\n");
    print("Expected state between %d and %d (inclusive)\n",min,max);
    return 0;
  }
  print("Valid state\n");
  return 1;
}

uint8_t same_val_check(uint8_t old_val, uint8_t new_val) {
  if (new_val == old_val) {
    print("Passed same_val_check\n");
    return 1;
  }
  print("Error: Unexpected value change\n");
  return 0;
}

uint8_t is_empty_check(uint8_t* state) {
  //state must not be empty
  if (state == NULL) {
    print("Error: NULL state\n");
    return 0;
  }
  print("Passed is_empty\n");
  return 1;
}

void init_eeprom() {
  eeprom_update_byte((uint8_t*)OBC_EEPROM_ADDRESS,0);
  eeprom_update_byte((uint8_t*)EPS_EEPROM_ADDRESS,0);
  eeprom_update_byte((uint8_t*)PAY_EEPROM_ADDRESS,0);
  eeprom_update_dword((uint32_t*)INIT_WORD,DEADBEEF);
}*/
/*
uint8_t main() {
  init_uart();
  init_can();

  //Boot Sequence: Retrieve latest state from its own EEPROM. Then assign the state
  //to itself by first going through switch statements, then find the appropriate
  //funtion calls to that specific state and execute it.

  uint8_t fresh_restart;
  //changed this to be at the beginning, would preferably be in init_heartbeat
  if (eeprom_read_dword((uint32_t*)INIT_WORD) != DEADBEEF) {
    print("Deadbeef detected\n");
    init_eeprom();
    fresh_restart = 1;
  }
  else{
    print("First boot sequence\n");
    OBC_state = eeprom_read_byte((uint8_t*)OBC_EEPROM_ADDRESS);
    EPS_state = eeprom_read_byte((uint8_t*)EPS_EEPROM_ADDRESS);
    PAY_state = eeprom_read_byte((uint8_t*)PAY_EEPROM_ADDRESS);
    fresh_restart = 0;
  }

eeprom_update_dword((uint32_t*)INIT_WORD,0xdeadeeee);
  //testing state retrieval
  //checking if deadbeef was entered
  print("initialized first val: %x Expected: beef\n",eeprom_read_dword((uint32_t*)INIT_WORD));
  print("First boot sequence Expected: ~255. Other boot sequence expected __\n");
  print("OBC First boot sequence Expected: ~255, Given: %d\n", OBC_state);
  print("PAY First boot sequence Expected: ~255, Given: %d\n", PAY_state);
  print("EPS First boot sequence Expected: ~255, Given: %d\n\n", EPS_state);

  //eeprom_update_dword((uint32_t*)INIT_WORD,0xddddffff);
  //Needs to be initialized to recieve requests
  init_tx_mob(&tx_mob);
  init_rx_mob(&rx_mob);

  while(1) {
    resume_mob(&tx_mob);
    while (!is_paused(&tx_mob)) {}
    //print("Hello World\n");
  }//do nothing at end

  //testing functions
  test_valid_range();
  test_same_val();
  test_increment();

  while(1);

  return 0;
}
*/
