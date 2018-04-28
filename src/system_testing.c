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

TODO
1. Goes out of loop if can message is valid
2. Does not go into loop if not fresh fresh_restart
3. test_10_beats to see if it really increments to 10 and stays that way on restart
4. test_heartbeat and test_heartbeat2
*/

#include <uart/uart.h>
#include <uart/log.h>
#include <can/can.h>
#include <avr/eeprom.h>
#define F_CPU 8
#include <util/delay.h>
//#include <heartbeat/heartbeat.h>
#include "heartbeat.h"

uint8_t OBC_state = 0;//2
uint8_t EPS_state = 0;//3
uint8_t PAY_state = 0;//4

uint8_t CAN_MSG_RCV = 0;

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
void test_single_heartbeat(){
  int leave = 0;//when to stop sending messages
  while (leave == 0){
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
void test_10_beats(){
  int leave = 0;//when to stop sending messages
  int OBC_end_state = OBC_state + 10;
  while (leave == 0){
    //wait 2 seconds before sending messages
    _delay_ms(2000);
    OBC_state += 1;
    //send mob if state has not increased by 10. Be careful about state overflow
    if (OBC_state <= OBC_end_state){
      resume_mob(&tx_mob);
      while (!is_paused(&tx_mob)) {}
      print("Current OBC state is %d, PAY is %d", OBC_state, PAY_state);
    }
    else {
      //leave = 1; //leave function
      leave = 0;//stay in function
    }
  }
}

//testing
//Both boards can have this function
//Changes its own state then sends that changed state to the other board
//mobs sent may increase exponentially,
//Potential problem in heartbeat.c: tx_mob does not allow it to go out of
//the while loop
//one board has test_heartbeat and the other can have test_heartbeat or
//test_heartbeat2
void test_heartbeat(int fresh_restart){
  while(CAN_MSG_RCV == 0 && fresh_restart == 1){
    print("Entered loop/Fresh restart\n");
    //wait for message from OBC, which can change can_msg_rcv
    //Only enter this with fresh start
    //flag is set to 1 in rx_callback

    //send changed state
    resume_mob(&tx_mob);
    while (!is_paused(&tx_mob)) {}
    print("Tx mob sent\n\n");
    _delay_ms(100);
    //expected to leave loop because of own state change
    //

    print("Current flag state: %d\n", CAN_MSG_RCV);
  }

  switch(OBC_state){
    case 0:
      print("OBC is in state 0, PAY is %d\n", PAY_state);
      break;
    case 1:
      print("OBC is in state 1, PAY is %d\n", PAY_state);
      break;
    case 2:
      print("OBC is in state 2, PAY is %d\n", PAY_state);
      break;
    default:
      print("OBC is in ERROR state\n");
      break;
      //set can_msg_rcv to 0?
  }

  while (1){
    OBC_state += 1;
    resume_mob(&tx_mob);
    while (!is_paused(&tx_mob)) {}
    print("Tx mob sent\n\n");
    _delay_ms(100);
  }
}

//receives heartbeat at the beginning
void test_heartbeat2(int fresh_restart){
  while(CAN_MSG_RCV == 0 && fresh_restart == 1){
    print("Entered loop/Fresh restart\n");
    //wait for message from OBC, which can change can_msg_rcv
    //Only enter this with fresh start
    //flag is set to 1 in rx_callback

    print("Current flag state: %d\n", CAN_MSG_RCV);
  }

  switch(OBC_state){
    case 0:
      print("OBC is in state 0, PAY is %d\n", PAY_state);
      break;
    case 1:
      print("OBC is in state 1, PAY is %d\n", PAY_state);
      break;
    case 2:
      print("OBC is in state 2, PAY is %d\n", PAY_state);
      break;
    default:
      print("OBC is in ERROR state\n");
      break;
      //set can_msg_rcv to 0?
  }

  while (1){
    PAY_state += 1;
    resume_mob(&tx_mob);
    while (!is_paused(&tx_mob)) {}
    print("Tx mob sent\n\n");
    _delay_ms(100);
  }
}

/*
//test error coding for heartbeat
//only add this code to one board
void test_heartbeat_errors (){
  //State changed by 2/increment_check
  print("State increment check");
  OBC_state += 2;
  resume_mob(&tx_mob);
  while (!is_paused(&tx_mob)) {}
  print("Current OBC state is %d, PAY is %d", OBC_state, PAY_state);

  //Unexpected board changed/same_val_check
  print("Unexpected board value change check");
  PAY_state += 1;
  resume_mob(&tx_mob);
  while (!is_paused(&tx_mob)) {}
  print("Current OBC state is %d, PAY is %d", OBC_state, PAY_state);
}*/

uint8_t main() {
  init_uart();
  init_can();

  //Boot Sequence: Retrieve latest state from its own EEPROM. Then assign the state
  //to itself by first going through switch statements, then find the appropriate
  //funtion calls to that specific state and execute it.

  int fresh_restart;
  //changed this to be at the beginning, would preferably be in init_heartbeat
  if (eeprom_read_dword((uint32_t*)INIT_WORD) != DEADBEEF){
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
  //eeprom_update_dword((uint32_t*)INIT_WORD,0xdededede);

  //testing functions
  test_single_heartbeat();

  while(1);//do nothing at end

  return 0;
}
