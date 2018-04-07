/*Heartbeat 2.0 (Yay!!)
By Jack and Bonnie April 4, 2018
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
*/

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

//Declare global variables to keep track on state changes in each SSM
//Current mechanism to simulate state changes and for readability purposes
//(subject to change in the next or final iteration)
//At start of program, the initial state of each SSM is zero.
uint8_t OBC_state = 0;//2
uint8_t PAY_state = 0;//3
uint8_t EPS_state = 0;//4

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

void tx_callback(uint8_t* state_data, uint8_t* len) {
  //state_data is an array of length 3 that consists of state data of OBC, PAY,
  //and EPS, in this order.
  *len = 3;
  //Simulate a state change by incrementing the state of OBC
  OBC_state += 1;
  //Upon state change, OBC first updates the state data in its own EEPROM
  //Should only need to update the state data for OBC
  eeprom_update_byte((uint8_t*)OBC_EEPROM_ADDRESS, OBC_state);
  //eeprom_update_byte((uint8_t*)PAY_EEPROM_ADDRESS, PAY_state);
  //eeprom_update_byte((uint8_t*)EPS_EEPROM_ADDRESS, EPS_state);

  //After updating its own EEPROM, OBC sends updated state data to PAY
  //Always send the state_data as an array that consists all states data all 3 SSMs
  //Can also implement this using block access (Bonnie is too lazy to look it up :p )
  state_data[2] = eeprom_read_byte((uint8_t*)OBC_EEPROM_ADDRESS);
  state_data[3] = eeprom_read_byte((uint8_t*)PAY_EEPROM_ADDRESS);
  state_data[4] = eeprom_read_byte((uint8_t*)EPS_EEPROM_ADDRESS);

  //Some print statements for testing and debugging purposes
}

void rx_callback(uint8_t* state_data, uint8_t len) {
  print("Receive updates state data from EPS!\n");
  //Perform preliminary error checking
  //(in place of to-be-implemented error checking module)
  //1 for pass, 0 for failure of any test
  uint8_t pass = 1;
  pass = error_check(state_data,len);//returns 0 if any error
  if (state_data[1] == 2 && pass == 1){//if in heartbeat and passes error checking
  //Update the state data for all 3 SSMs in EEPROM
    CAN_MSG_RCV = 1;
    eeprom_update_byte((uint8_t*)OBC_EEPROM_ADDRESS,state_data[2]);
    eeprom_update_byte((uint8_t*)PAY_EEPROM_ADDRESS,state_data[3]);
    eeprom_update_byte((uint8_t*)EPS_EEPROM_ADDRESS,state_data[4]);
  }
  else{
    print("ERROR OCCURED, DID NOT UPDATE or NOT IN HEARTBEAT\n");
  }
    //Some print statements for testing and debugging purposes
  }

uint8_t error_check(uint8_t* state_data, uint8_t len){
  int pass = 1;
  //Arbitrary min/max values
  int max_state = 2;
  int min_state = 0;
  //Error checking procedure
  if (len_check(len) == 0){
    pass = 0;
    return pass;//return here if empty
  }
  if (len_check(len) == 0){
    pass = 0;
  }
  //If any test fails, return 0
  //old value, new value
  //Check if OBC has same value
  if (same_val_check((uint8_t*)OBC_EEPROM_ADDRESS,state_data[2]) == 0){//PAY
    pass = 0;
  }

  if (increment_check((uint8_t*)PAY_EEPROM_ADDRESS,state_data[3]) == 0 &&
      same_val_check((uint8_t*)PAY_EEPROM_ADDRESS,state_data[3]) == 0){//PAY
    pass = 0;
  }

  if (increment_check((uint8_t*)EPS_EEPROM_ADDRESS,state_data[4]) == 0){//EPS
    pass = 0;
  }

  if (in_range_check(state_data[2],min_state,max_state) == 0){//OBC
    pass = 0;
  }
  if (in_range_check(state_data[3],min_state,max_state) == 0){//PAY
    pass = 0;
  }
  if (in_range_check(state_data[4],min_state,max_state) == 0){//EPS
    pass = 0;
  }
  return pass;
}

uint8_t len_check(uint8_t len){
  //len must be equal to 3
  if (len == 3){
    print("Passed len_check\n");
    return 1;
  }
  print("Error: Incorrect length\n");
  return 0;
}

uint8_t increment_check(uint8_t* old_val, uint8_t new_val){
  //Assume that only valid increment is ++ or same
  if (new_val == *old_val +1){
    print("Passed increment_check\n");
    return 1;
  }
  print("Error: Incorrect increment\n");
  return 0;
}

uint8_t same_val_check(uint8_t* old_val, uint8_t new_val){
  if (new_val == *old_val){
    print("Passed same_val_check\n");
    return 1;
  }
  print("Error: Unexpected value change\n");
  return 0;
}


uint8_t is_empty_check(uint8_t* state){
  //state must not be empty
  if (state == NULL){
    print("Error: NULL state\n");
    return 0;
  }
  print("Passed is_empty\n");
  return 1;
}

uint8_t in_range_check(uint8_t state, uint8_t min, uint8_t max){
  //Verify that state is within valid range
  if (state < min || state > max){
    print("Error: Invalid range\n");
    return 0;
  }
  print("Passed in_range\n");
  return 1;
}

void init_eeprom(){
  eeprom_update_byte((uint8_t*)OBC_EEPROM_ADDRESS,0);
  eeprom_update_byte((uint8_t*)PAY_EEPROM_ADDRESS,0);
  eeprom_update_byte((uint8_t*)EPS_EEPROM_ADDRESS,0);
  eeprom_update_dword((uint32_t*)INIT_WORD,DEADBEEF);
}

uint8_t main() {
  init_uart();
  init_can();

  //Boot Sequence: Retrieve latest state from its own EEPROM. Then assign the state
  //to itself by first going through switch statements, then find the appropriate
  //funtion calls to that specific state and execute it.

  //Change to while loop, wait for message before switch statements
  while(CAN_MSG_RCV == 0){
    //wait for message from OBC, which can change can_msg_rcv
    //Only enter this with fresh start
    //flag is set to 1 in rx_callback
  }

  if (eeprom_read_dword((uint32_t*)INIT_WORD) != DEADBEEF){
    init_eeprom();
  }

  else{
    OBC_state = eeprom_read_byte((uint8_t*)OBC_EEPROM_ADDRESS);
    EPS_state = eeprom_read_byte((uint8_t*)EPS_EEPROM_ADDRESS);
    PAY_state = eeprom_read_byte((uint8_t*)PAY_EEPROM_ADDRESS);
  }

  switch(OBC_state){
    case 0:
      print("OBC is in state 0\n");
      break;
    case 1:
      print("OBC is in state 1\n");
      break;
    case 2:
      print("OBC is in state 2\n");
      break;
    default:
      print("OBC is in ERROR state\n");
      break;
      //set can_msg_rcv to 0?
  }

  init_rx_mob(&rx_mob);
  if (is_paused(&rx_mob)){
    print("WHAT??\n");
  }

  init_tx_mob(&tx_mob);
  while (1) {
    resume_mob(&tx_mob);
    while (!is_paused(&tx_mob)) {}
      _delay_ms(100);
    };
    return 0;
}
