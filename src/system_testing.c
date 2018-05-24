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

Instructions
1. Tests extern increments
2. Does not go into loop if not fresh fresh_restart (test_heartbeat and test_heartbeat2)
3. test_10_beats to see if it really increments to 10 and stays that way on restart

TODO
1. Interrupt loop Bug
2. Masks for mobs
*/

#include <uart/uart.h>
#include <uart/log.h>
#include <can/can.h>
#include <avr/eeprom.h>
#define F_CPU 8
#include <util/delay.h>
//#include <heartbeat/heartbeat.h>
#include "system_testing.h"
#include "heartbeat.h"

uint8_t SELF_state = 0;//0 at fresh restart
uint16_t SELF_EEPROM_ADDRESS = OBC_EEPROM_ADDRESS;//OBC
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
//One board will use this function, the other will use heartbeat.c
//Changes its own state then sends that changed state to the other boards
//Does not test CAN_MSG_RCV for one board
//by removing same_val_check error function
//can also change other boards state then sends that data
/*
void test_single_heartbeat() {
  uint8_t leave = 0;//when to stop sending messages
  while (leave == 0) {
    //wait 2 seconds before sending messages
    _delay_ms(2000);
    OBC_state += 1;
    //send mob
    //state changes in tx_mob
    resume_mob(&tx_mob);
    while (!is_paused(&tx_mob)) {}
    print("Current OBC state is %d, PAY is %d", OBC_state, PAY_state);
  }
}

//Changes state 10 times then stops
void test_10_beats() {
  uint8_t leave = 0;//when to stop sending messages
  uint8_t EPS_end_state = EPS_state + 10;
  SELF_state = EPS_state;
  SELF_EEPROM_ADDRESS = EPS_EEPROM_ADDRESS;
  while (leave <= 10) {
    //wait 2 seconds before sending messages
    leave += 1;
    SELF_state += 1;
    //send mob if state has not increased by 10. Be careful about state overflow
    resume_mob(&tx_mob);
    while (!is_paused(&tx_mob)) {print("paused");}
    _delay_ms(100);
    print("Current EPS state is %d, PAY is %d", SELF_state, PAY_state);
  }
}
*/
//testing
//Both boards can have this function
//Changes its own state then sends that changed state to the other board
//mobs sent may increase exponentially,
//Potential problem in heartbeat.c: tx_mob does not allow it to go out of
//the while loop
//one board has test_heartbeat and the other can have test_heartbeat or
//test_heartbeat2
void test_heartbeat(uint8_t fresh_restart) {

  while(CAN_MSG_RCV == 0 && fresh_restart == 1) {

    print("Entered loop/Fresh restart\n");
    //wait for message from OBC, which can change can_msg_rcv
    //Only enter this with fresh start
    //flag is set to 1 in rx_callback

    //send changed state
    resume_mob(&tx_mob);
    while (!is_paused(&tx_mob)) {}
    print("Tx mob sent\n\n");

    //expected to leave loop because of own state change
    print("Current flag state: %d\n", CAN_MSG_RCV);
  }

  switch(SELF_state) {
    case 0:
      print("SELF is in state 0, PAY is %d\n", PAY_state);
      break;
    case 1:
      print("SELF is in state 1, PAY is %d\n", PAY_state);
      break;
    case 2:
      print("SELF is in state 2, PAY is %d\n", PAY_state);
      break;
    default:
      print("SELF is in ERROR state\n");
      break;
      //set can_msg_rcv to 0?
  }

//  while (1) {
    SELF_state += 1;
    resume_mob(&tx_mob);
    while (!is_paused(&tx_mob)) {}
    print("Tx mob sent\n\n");
    _delay_ms(1000);
  //}
}

//receives heartbeat at the beginning
void test_heartbeat2(uint8_t fresh_restart) {
  while(CAN_MSG_RCV == 0 && fresh_restart == 1) {
    print("Entered loop/Fresh restart\n");
    //wait for message from OBC, which can change can_msg_rcv
    //Only enter this with fresh start
    //flag is set to 1 in rx_callback

    print("Current flag state: %d\n", CAN_MSG_RCV);
  }

  switch(SELF_state) {
    case 0:
      print("SELF is in state 0, PAY is %d\n", PAY_state);
      break;
    case 1:
      print("SELF is in state 1, PAY is %d\n", PAY_state);
      break;
    case 2:
      print("SELF is in state 2, PAY is %d\n", PAY_state);
      break;
    default:
      print("SELF is in ERROR state\n");
      break;
      //set can_msg_rcv to 0?
  }
//  while (1) {
    SELF_state += 1;
    resume_mob(&tx_mob);
    while (!is_paused(&tx_mob)) {}
    print("Tx mob sent\n\n");
//  }
}

//Checks to make sure extern works for SELF_state and SELF_ADDRESS
//Code is for the board that sends the data
//For the board that recieves the data, have it do nothing/just recieve interrupts
uint8_t test_extern_state(uint8_t option) {
  uint8_t BOARD_state;
  uint16_t BOARD_EEPROM_ADDRESS;
  if (option == 1) {//test OBC
    BOARD_state = OBC_state;
    BOARD_EEPROM_ADDRESS = OBC_EEPROM_ADDRESS;
  }
  else if (option == 2) {//test PAY
      BOARD_state = PAY_state;
      BOARD_EEPROM_ADDRESS = PAY_EEPROM_ADDRESS;
  }
  else {//test EPS
      BOARD_state = EPS_state;
      BOARD_EEPROM_ADDRESS = EPS_EEPROM_ADDRESS;
  }

  SELF_state = BOARD_state;//self
  SELF_EEPROM_ADDRESS = BOARD_EEPROM_ADDRESS;
  SELF_state += 1;//increment state
  resume_mob(&tx_mob);//send heartbeat mob
  while (!is_paused(&tx_mob)) {}
  print("Tx mob sent\n\n");
  _delay_ms(100);

  BOARD_state = eeprom_read_byte((uint8_t*)BOARD_EEPROM_ADDRESS);
  print("Self %d, Board %d\n", SELF_state, BOARD_state);
  if (BOARD_state == SELF_state) {//properly externed
    return 1;//pass
  }
  return 0;//fail
}

//tests extern for mobs and states/variables
void test_extern() {
  //Test CAN_MSG_RCV flag (if it changes as a valid heartbeat message is sent)
  print("CAN_MSG_RCV flag before expected: 0, Actual: %d\n", CAN_MSG_RCV);
  resume_mob(&tx_mob);
  while (!is_paused(&tx_mob)) {}//Flag should change after this line
  print("Tx mob sent for flag waiting for response\n");
  while(CAN_MSG_RCV == 0) {_delay_ms(10);}//wait for can message to be recieved
  print("CAN_MSG_RCV flag expected: 1, Actual: %d\n\n", CAN_MSG_RCV);

  //Test states using test_extern_state code
  print("OBC extern states test expected:1, \n");
  print("Recieved: %d\n\n",test_extern_state(1));
  print("EPS extern states test expected:1, \n");
  print("Recieved: %d\n\n",test_extern_state(2));
  print("PAY extern states test expected:1, \n");
  print("Recieved: %d\n\n",test_extern_state(3));
}

uint8_t main() {
  init_uart();
  init_can();

//eeprom_update_dword((uint32_t*)INIT_WORD,0xdeadbbbb);//fresh restart
  //Boot Sequence: Retrieve latest state from its own EEPROM. Then assign the state
  //to itself by first going through switch statements, then find the appropriate
  //funtion calls to that specific state and execute it.
  eeprom_update_dword((uint32_t*)INIT_WORD,0xdeadbeee);
  uint8_t fresh_restart;//fresh reset = 0 if not
  if (eeprom_read_dword((uint32_t*)INIT_WORD) != DEADBEEF) {
    init_eeprom();
    fresh_restart = 1;//fresh restart
    print("fresh \n");
  }
  else{
    OBC_state = eeprom_read_byte((uint8_t*)OBC_EEPROM_ADDRESS);
    EPS_state = eeprom_read_byte((uint8_t*)EPS_EEPROM_ADDRESS);
    PAY_state = eeprom_read_byte((uint8_t*)PAY_EEPROM_ADDRESS);
    fresh_restart = 0;
  }

//  print("initialized first val to: %x\n",eeprom_read_dword((uint32_t*)INIT_WORD));
//  print("First boot sequence Expected: 0. Other boot sequence expected __\n");
//  print("OBC Actual: %d\n", OBC_state);
//  print("PAY Actual: %d\n", PAY_state);
//  print("EPS Actual: %d\n\n", EPS_state);

  //Needs to be initialized to recieve requests
  init_tx_mob(&tx_mob);
  init_rx_mob(&rx_mob);

//  test_extern();

//  test_extern();
  //test_heartbeat2(fresh_restart);
  test_heartbeat(fresh_restart);
  //test_10_beats();

  while(1) {}//do nothing at end

  return 0;
}
