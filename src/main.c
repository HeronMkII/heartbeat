//This version includes the EEMEM Attribute
//Is there any reason to write to a fixed address?
#include <uart/uart.h>
#include <uart/log.h>
#include <can/can.h>
#include <avr/eeprom.h>//added this

#define F_CPU 8
#include <util/delay.h>

#define EEMEM   __attribute__((section(".eeprom")));//added this

void rx_callback(uint8_t*, uint8_t);
void tx_callback(uint8_t*, uint8_t*);

uint8_t EEMEM child_counter = 0; //modified to be stored in EEPROM
uint8_t EEMEM parent_counter = 0; //modified to be stored in EEPROM

#define A_PARENT 0x001c
#define A_CHILD  0x000b
#define B_PARENT 0x001a
#define B_CHILD  0x000c
#define C_PARENT 0x001b
#define C_CHILD  0x000a

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

void tx_callback(uint8_t* data, uint8_t* len) {
  *len = 1;
  data[0] = eeprom_read_byte(&parent_counter);
  //print("Data[0]: %d\n",data[0]);
  eeprom_update_byte(&parent_counter,(parent_counter+1));//parent_counter += 1;
  print("Parent counter incremented\n");

  uint8_t parent_read = eeprom_read_byte(&parent_counter);//replaces old print statements
  print("parent_counter: %d\n", parent_read);
}

void rx_callback(uint8_t* data, uint8_t len) {
  print("TX received!\n");
  if (len != 0) {
        eeprom_update_byte(&child_counter,data[0]);//child_counter = data[0];
        //print("Data[0]: %d\n",data[0]);

        uint8_t child_read = eeprom_read_byte(&child_counter);//replaces old print statement
        print("child_counter: %d\n", child_read);
  } else
  {
    print("No data\n");
  }
}

int main() {
  init_uart();
  init_can();

  init_rx_mob(&rx_mob);

  if (is_paused(&rx_mob)) {
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






















// -----------------------------------------------------------------------------
// //Tasks:
// //Documentation: How to use EEPROM, explain how functions work etc.
// //Implement EEPROM into heartbeat
// //Test EEPROM with heartbeat and document this
// //Two board communication
//
// #include <uart/uart.h>
// #include <uart/log.h>
// #include <can/can.h>
// #include <util/delay.h>
// #include <avr/eeprom.h>
//
// #define EEMEM   __attribute__((section(".eeprom")));
//
// uint8_t EEMEM data_eemem;
//
//
// uint8_t EEMEM block_var[16];
//
// int main(void){
//   init_uart();
//   init_can();
//   _delay_ms(1000);
//
//   //Block Access Testing with EEMEM
//   //block is stored in program space
//   uint8_t block[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
//   //update block_var in EEMEM with same values as block in program space
//   eeprom_update_block((const void*)block, (void*)block_var,16);
//
//   while(1){
//   //reads data from block_var (EEMEM) into block_test (program space) and prints out data from block_test
//     uint8_t block_test[16];
//     eeprom_read_block((void*)block_test, (const void*)block_var, 16);
//     for (int i = 0;i<16;i++){
//       print("Array value: %d at address %x\n",block_test[i],i);
//     }
//   }
//
//   //Block Access Testing without EEMEM
//   //Block Access update
//   uint8_t block_write[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
//   //Takes 16 bytes of data in block_write (in program space) and stores it starting at address 12 in EEPROM
//   eeprom_update_block((const void*)block_write, (void*)12, 16);
//
//   //Block Access reading
//   while(1){
//     uint8_t test_array[16];
//     //takes 16 bytes of data at address 12 in EEPROM, and stores data from EEPROM in test array in program space
//     eeprom_read_block((void*)test_array, (const void*)12, 16);
//     for (int i = 0;i<16;i++){
//       print("Array value: %d at address %x\n",test_array[i],i);
//     }
//   }
//
//
//   //Test EEMEM attribute: Store/retrieve data at arbitrary location
//   //Must define EEMEM var outside of main
//   //update by passing in address of EEMEM var
//   //read by passing in address of var
//   eeprom_update_byte(&var, 0x06);
//   uint8_t read;
//   while(1){
//   read = eeprom_read_byte(&var);
//   print("var: %d\n",read);
//   }
//
//   //Writing data test - delay for print statements
//   //Note: Data is persistent, need to change write_data to observe difference between runs
//   uint8_t write_data;
//   write_data = 0xF9;
//   uint8_t read_data;
//   read_data = eeprom_read_byte((uint8_t*)46);
//
//   _delay_ms(1000);
//   print("Data (Before): %d\n",read_data);
//   eeprom_update_byte((uint8_t*)46, write_data);
//   read_data = eeprom_read_byte((uint8_t*)46);
//   _delay_ms(1000);
//   print("Data (After): %d\n",read_data);
//
//
// //Test for persistence- press reset while loop is running
// //Observed that data stays constant compared to what was previously set
// while (1){
//     read_data = eeprom_read_byte((uint8_t*)46);
//     print("Data: %d\n",read_data);
// }
//
//   //Tests reading data at arbitrary locations
//   //Output is all 255 (0xFF)
//    while(1){
//    uint8_t data;
//    for (int i = 0;i<1000;i++){
//    data = eeprom_read_byte((uint8_t*)(i));
//    print("Data: %d at location %d\n",data,i);
//  }
//  }
//
// }
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
// // -----------------------------------------------------------------------------
// // #include <uart/uart.h>
// // #include <uart/log.h>
// // #include <can/can.h>
// // #include <util/delay.h>
//
// //#include <avr/eeprom.h>
//
// //#define 	eeprom_is_ready()
// //#define 	eeprom_busy_wait()
//
// //#define EEMEM   __attribute__((section(".eeprom")));
//
//
// //#define F_CPU 8
// //The below line needs to be included to use progmem features in AVR-C
// //http://www.nongnu.org/avr-libc/user-manual/group__avr__pgmspace.html#ga75acaba9e781937468d0911423bc0c35
//
// // uint8_t savedEPSChildData = 0; //variable declared to be stored in progmem space
// // //Observe that savedEPSChildData will no longer be zero as the project runs.
// // uint8_t displayEPSChildData;
// //
// // /*variable declared to retrieve data stored in savedEPSChildData in progmem space.
// // Functionality of retrieving data from flash memory has not been completed yet*/
// //
// // void rx_callback(uint8_t*, uint8_t);
// // void tx_callback(uint8_t*, uint8_t*);
// //
// // int child_counter = 0;
// // int parent_counter = 0;
// //
// // #define A_PARENT 0x001c
// // #define A_CHILD  0x000b
// // #define B_PARENT 0x001a
// // #define B_CHILD  0x000c
// // #define C_PARENT 0x001b
// // #define C_CHILD  0x000a
// //
// // /*Suggestion for testing:
// // Include the above ID tags in the the below mobs initialization for each boards
// // under test accordingly
// // */
// //
// // mob_t rx_mob = {
// //     .mob_num = 0,
// //     .mob_type = RX_MOB,
// //     .dlc = 1,
// //     .id_tag = {  }, // ID of this nodes parent
// //     .id_mask = { 0x00f },
// //     .ctrl = default_rx_ctrl,
// //     .rx_cb = rx_callback
// // };
// //
// // mob_t tx_mob = {
// //     .mob_num = 1,
// //     .mob_type = TX_MOB,
// //     .id_tag = { }, // ID of this nodes child
// //     .ctrl = default_tx_ctrl,
// //     .tx_data_cb = tx_callback
// // };
// //
// // void tx_callback(uint8_t* data, uint8_t* len) {
// //     print("TX_CALLBACK TRIGGERED\n"); //indicates that tx_callback function is called
// //     *len = 1;
// //     data[0] = parent_counter; //send parent_counter as tx to another subsystem
// //     //print("test0_tx\n");
// //     parent_counter += 1;
// //     //print("test_tx\n");
// //      //every time tx_callback is called, parent_counter is incremented
// //     print("Tx-EPS set OBC parent counter : %d\n", parent_counter);//TO DO: MOVE
// //     //THIS LINE TO BEFORE parent_counter += 1;
// //     _delay_ms(100); //for testing purposes, delay this func. to try to see if
// //     //could help tx_callback to be called and execute properly. Not 100% if
// //     //this helps yet.
// // }
// //
// // void rx_callback(uint8_t* data, uint8_t len) {
// //     print("RX_CALLBACK TRIGGERED\n");
// //     if (len != 0) {
// //         child_counter = data[0]; //EPS receives parent_counter set by another
// //         //subsystem, and stores it in its child_counter
// //         print("RX-EPS received OBC child_counter: %d\n", child_counter);
// //         //store into memory of this board (add const in front)
// //         uint8_t savedEPSChildData PROGMEM = child_counter; //store child_counter
// //         //in flash memory (or progmem space) as variale savedEPSChildData
// //
// //         uint8_t test_read = savedEPSChildData;
// //         print("Display test: %d\n", test_read);
// //
// //         //unint8_t x = savedEPSChildData;
// //         print("Child Counter: %d\n", child_counter);
// //         print("RX-EPS child_counter saved to PROGMEM: %d\n", savedEPSChildData);
// //         //This saves correctly
// //
// //     } else {
// //         print("No data\n");
// //     }
// //     _delay_ms(100); //delay rx_callback for 100 ms, to see if this helps the
// //     //function being called properly. Not 100% sure.
// // }
//
// //uint8_t const data[1] PROGMEM = { 0x05 };
//
// //--------------------------------------------------------------------------------------
// //uint8_t EEMEM test;
//
// //int main(void) {
//   //  init_uart();
//     //init_can();
//
//
//
//
//
//
//
//
//
//
//     // test = 42;
//     //
//     // while(1){
//     // _delay_ms(100);     //delay for debugging
//     //
//     // print("RESET!!!\n");
//     // //uint8_t EEMEM test;
//     // //test = 'a';
//     //
//     // //Note: may not be able to use EEMEM attribute in functions
//     // //Will have to define address
//     // //Test that read/write is working
//     // uint8_t test_result = 67;
//     // _delay_ms(100);    //delay for debugging purposes
//     // //printf("Test result address(1): %x\n",&test_result);
//     //
//     // //test_result = eeprom_read_byte(&test);
//     // _delay_ms(100); //delay for debug
//     // //print("TEST\n");
//     // print("Test result: %d\n",test_result);
//     //
//     // //printf("Test result address(2): %x\n",&test_result);
//     //
//     // //Test that update is working
//     // eeprom_busy_wait();
//     // eeprom_update_byte (&test , 55) ;
//     // test_result = eeprom_read_byte(&test);
//     // print("Test result (changed): %x\n",test_result);
//     // break;
//
//
//
//
//      //If the board is reset, theoretically, this line should
//     //be printed. However, this has not been confirmed in testing.
//     //_delay_ms(100); //delay rx_callback for 100 ms, to see if this helps the
//     //print("Display data_0: %d\n", displayEPSChildData);
//     //print("Display address_0: %x\n", &displayEPSChildData);
//
//     //uint8_t a = pgm_read_byte(&data[0]);
//
//     //uint8_t savedEPSChildData PROGMEM = 0x04;
//     //uint8_t displayEPSChildData = savedEPSChildData;
//     // print("Display address: %d\n", &savedEPSChildData);
//     //Theoretically, displayEPSChildData should retrieve the data stored in
//     //savedEPSChildData stored in flash memory. THIS FUNCTIONALITY HAS NOT
//     //BEEN DEMONSTRATED or OBSERVED YET.
//     //print("Display data: %d\n", displayEPSChildData);
//
//     //while(1) {
//       //print("Data stored: %d\n", a);
//       //data[0] += 1;
//     //}
//
//
//     // init_rx_mob(&rx_mob);
//     // if (is_paused(&rx_mob)) {
//     //     print("WHAT?? Mob is paused\n");
//     // }
//     // init_tx_mob(&tx_mob);
//     // while (1) {
//     //     resume_mob(&tx_mob);
//     //     while (!is_paused(&tx_mob)) {}
//     //     _delay_ms(100);
//     // };
//     // return 0;
// //}}
