/*Heartbeat 2.0.1 (Yay!!)
By Jack and Bonnie April 14, 2018 (v. 2.0 04/04/18)
This iteration of heartbeat design has the following assumptions:
1. This code is written from the perspective of OBC
   (Will generalize this code, to lib-common structure soon)
2. The triangular configuration of heartbeat is set as:
    OBC -> EPS (OBC sends state data to PAY) (PAY is the parent of OBC)
    EPS -> PAY (PAY sends state data to EPS)
    PAY -> OBC (EPS sends state data to OBC)
3. When there is a state update, a SSM needs to first update its own EEPROM,
   and then send the updated state data to its parent via CAN.
4. The state data transmits through CAN, is an array with size of uint8_t, and
     length of 5, with the following array structure:
    |     [0]   |   [1]   |   [2]   |   [3]   |   [4]   |
    |Who's from | packet  |OBC state|EPS state|PAY state|
     *The packet of heartbeat is 2.
5. Error checking for state data is implemented (beta).
*/


#include <uart/uart.h>
#include <uart/log.h>
#include <can/can.h>
#include <avr/eeprom.h>
#define F_CPU 8
#include <util/delay.h>
//#include <heartbeat/heartbeat.h>
#include "heartbeat.h"
//#include "heartbeat_defines.h"
#include <util/atomic.h>

extern uint8_t SELF_state;
extern uint16_t SELF_EEPROM_ADDRESS;
extern uint8_t OBC_state;//2
extern uint8_t EPS_state;//3
extern uint8_t PAY_state;//4
uint8_t CAN_MSG_RCV = 0;

extern mob_t rx_mob;
extern mob_t tx_mob;


//Declare global variables to keep track on state changes in each SSM
//Current mechanism to simulate state changes and for readability purposes
//(subject to change in the next or final iteration)
//At start of program, the initial state of each SSM is zero.


void tx_callback(uint8_t* state_data, uint8_t* len) {
  //state_data is an array of length 3 that consists of state data of OBC, PAY,
  //and EPS, in this order.
  *len = 5;
  //Simulate a state change by incrementing the state of OBC
  //Upon state change, OBC first updates the state data in its own EEPROM
  //Should only need to update the state data for OBC
  eeprom_update_byte((uint8_t*)SELF_EEPROM_ADDRESS, SELF_state);
  //eeprom_update_byte((uint8_t*)PAY_EEPROM_ADDRESS, PAY_state);
  //eeprom_update_byte((uint8_t*)EPS_EEPROM_ADDRESS, EPS_state);

  //After updating its own EEPROM, OBC sends updated state data to PAY
  //Always send the state_data as an array that consists all states data all 3 SSMs

  //Verify that all state data elements are in valid range
  //OBC range check (Assume state range is 0-10)
  state_data[1] = 2;
  if (in_range_check(eeprom_read_byte((uint8_t*)OBC_EEPROM_ADDRESS), 0, 10)==1) {
      state_data[2] = eeprom_read_byte((uint8_t*)OBC_EEPROM_ADDRESS);
  }
  else {
    print ("OBC has invalid state\n");//Address by getting state data from other SSMs
  }
  //ESP range check (Assume state range is 0-10)
  if (in_range_check(eeprom_read_byte((uint8_t*)EPS_EEPROM_ADDRESS), 0, 10)==1) {
      state_data[3] = eeprom_read_byte((uint8_t*)EPS_EEPROM_ADDRESS);
  }
  else {
    print ("EPS has invalid state\n");//Address by getting state data from other SSMs
  }
  //PAY range check (Assume state range is 0-10)
  if (in_range_check(eeprom_read_byte((uint8_t*)PAY_EEPROM_ADDRESS), 0, 10)==1) {
      state_data[4] = eeprom_read_byte((uint8_t*)PAY_EEPROM_ADDRESS);
  }
  else {
    print ("PAY has invalid state\n");//Address by getting state data from other SSMs
  }
}

void rx_callback(uint8_t* state_data, uint8_t len) {
  print("Receive updates state data from PAY!\n");
  //Verifies that len is equal to 5
  len_check(len);//Should be handled by asking for re-send and subsequent reboot if unsuccesfully re-sent

  if (state_data[1] == 2) {//if in heartbeat
  //Update the state data for all 3 SSMs in EEPROM
    CAN_MSG_RCV = 1;
    //Assuming that self is OBC, parent is EPS and child is PAY
    //Verify that increments are correct
    if (same_val_check(eeprom_read_byte((uint8_t*)OBC_EEPROM_ADDRESS), state_data[2])==1) {
      eeprom_update_byte((uint8_t*)OBC_EEPROM_ADDRESS,state_data[2]);
    }
    else{
      print("Unexpected state update (self)\n");//Do not update (Potential SSM reset?)
    }

    if (increment_check(eeprom_read_byte((uint8_t*)EPS_EEPROM_ADDRESS),state_data[3])==1) {
      eeprom_update_byte((uint8_t*)EPS_EEPROM_ADDRESS,state_data[3]);
    }
    else{
      print("Unexpected state update (parent)\n");
    }

    if (increment_check(eeprom_read_byte((uint8_t*)PAY_EEPROM_ADDRESS),state_data[4])==1 || same_val_check(eeprom_read_byte((uint8_t*)PAY_EEPROM_ADDRESS),state_data[4])==1) {
      eeprom_update_byte((uint8_t*)PAY_EEPROM_ADDRESS,state_data[4]);
    }
    else{
      print("Unexpected state update (child)\n");
    }
  }
  else{
    print("ERROR OCCURED, DID NOT UPDATE or NOT HEARTBEAT PACKET\n");
  }

  print("OBC: %d\n", OBC_state);
  print("PAY: %d\n", PAY_state);
  print("EPS %d\n\n", EPS_state);
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

uint8_t in_range_check(uint8_t state, uint8_t min, uint8_t max) {
  //Verify that state is within valid range
  if (state < min || state > max) {
    print("Error: Invalid range\n");
    return 0;
  }
  print("Passed in_range\n");
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

uint8_t increment_check(uint8_t old_val, uint8_t new_val) {
  //Assume that only valid increment is ++ or same
  if (new_val == old_val +1) {
    print("Passed increment_check\n");
    return 1;
  }
  print("Error: Incorrect increment\n");
  return 0;
}

uint8_t len_check(uint8_t len) {
  //len must be equal to 5
  if (len == 5) {
    print("Passed len_check\n");
    return 1;
  }
  print("Error: Incorrect length\n");
  return 0;
}

void init_eeprom() {
  eeprom_update_byte((uint8_t*)OBC_EEPROM_ADDRESS,0);
  eeprom_update_byte((uint8_t*)EPS_EEPROM_ADDRESS,0);
  eeprom_update_byte((uint8_t*)PAY_EEPROM_ADDRESS,0);
  eeprom_update_dword((uint32_t*)INIT_WORD,DEADBEEF);
}
