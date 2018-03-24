// //Modified rx,tx callbacks to fit with EEMEM
// #include <uart/uart.h>
// #include <uart/log.h>
// #include <can/can.h>
// #include <util/delay.h>
// #include <avr/eeprom.h>
//
// #define 	eeprom_is_ready()
// #define 	eeprom_busy_wait()
//
// #define EEMEM   __attribute__((section(".eeprom")));
//
//
// #define F_CPU 8
// //The below line needs to be included to use progmem features in AVR-C
// //http://www.nongnu.org/avr-libc/user-manual/group__avr__pgmspace.html#ga75acaba9e781937468d0911423bc0c35
//
// //modified below data to be stored as EEMEM
// uint8_t EEMEM savedEPSChildData = 0; //variable declared to be stored in progmem space
// //Observe that savedEPSChildData will no longer be zero as the project runs.
// uint8_t EEMEM displayEPSChildData; //<- is this used for anything?
//
// /*variable declared to retrieve data stored in savedEPSChildData in progmem space.
// Functionality of retrieving data from flash memory has not been completed yet*/
//
// void rx_callback(uint8_t*, uint8_t);
// void tx_callback(uint8_t*, uint8_t*);
//
// int child_counter = 0;
// int parent_counter = 0;
//
// #define A_PARENT 0x001c
// #define A_CHILD  0x000b
// #define B_PARENT 0x001a
// #define B_CHILD  0x000c
// #define C_PARENT 0x001b
// #define C_CHILD  0x000a
//
// /*Suggestion for testing:
// Include the above ID tags in the the below mobs initialization for each boards
// under test accordingly
// */
//
// mob_t rx_mob = {
//     .mob_num = 0,
//     .mob_type = RX_MOB,
//     .dlc = 1,
//     .id_tag = {  }, // ID of this nodes parent
//     .id_mask = { 0x00f },
//     .ctrl = default_rx_ctrl,
//     .rx_cb = rx_callback
// };
//
// mob_t tx_mob = {
//     .mob_num = 1,
//     .mob_type = TX_MOB,
//     .id_tag = { }, // ID of this nodes child
//     .ctrl = default_tx_ctrl,
//     .tx_data_cb = tx_callback
// };
//
// void tx_callback(uint8_t* data, uint8_t* len) {
//     print("TX_CALLBACK TRIGGERED\n"); //indicates that tx_callback function is called
//     *len = 1;
//     data[0] = parent_counter; //send parent_counter as tx to another subsystem
//     //print("test0_tx\n");
//     parent_counter += 1;
//     //print("test_tx\n");
//      //every time tx_callback is called, parent_counter is incremented
//     print("Tx-EPS set OBC parent counter : %d\n", parent_counter);//TO DO: MOVE
//     //THIS LINE TO BEFORE parent_counter += 1;
//     _delay_ms(100); //for testing purposes, delay this func. to try to see if
//     //could help tx_callback to be called and execute properly. Not 100% if
//     //this helps yet.
// }
//
// void rx_callback(uint8_t* data, uint8_t len) {
//     print("RX_CALLBACK TRIGGERED\n");
//     if (len != 0) {
//         child_counter = data[0]; //EPS receives parent_counter set by another
//         //subsystem, and stores it in its child_counter
//         print("RX-EPS received OBC child_counter: %d\n", child_counter);
//         //store into memory of this board (add const in front)
//
//         //uint8_t savedEPSChildData PROGMEM = child_counter; <- old code //store child_counter //in flash memory (or progmem space) as variale savedEPSChildData
//
//         //stores child counter into EEMEM as savedEPSChildData
//         eeprom_update_byte(&savedEPSChildData, child_counter);
//
//
//         //tests that data is properly stored
//         uint8_t test_read = eeprom_read_byte(&savedEPSChildData);
//         print("Display test: %d\n", test_read);
//
//         print("Child Counter: %d\n", child_counter);
//         print("RX-EPS child_counter saved to EEMEM: %d\n", savedEPSChildData);
//         //This saves correctly
//
//     } else {
//         print("No data\n");
//     }
//     _delay_ms(100); //delay rx_callback for 100 ms, to see if this helps the
//     //function being called properly. Not 100% sure.
// }
//
// //uint8_t const data[1] PROGMEM = { 0x05 }; <-modified below
//
// uint8_t data[1] EEMEM = 0x05; //Note: not completely sure that this can be set here
//
//
// uint8_t EEMEM test;
//
// int main(void) {
//     init_uart();
//     init_can();
//     test = 42;
//
//     while(1){
//     _delay_ms(100);     //delay for debugging
//
//     print("RESET!!!\n");
//     //uint8_t EEMEM test;
//     //test = 'a';
//
//     //Note: may not be able to use EEMEM attribute in functions
//     //Will have to define address
//     //Test that read/write is working
//     uint8_t test_result = 67;
//     _delay_ms(100);    //delay for debugging purposes
//     //printf("Test result address(1): %x\n",&test_result);
//
//     //test_result = eeprom_read_byte(&test);
//     _delay_ms(100); //delay for debug
//     //print("TEST\n");
//     print("Test result: %d\n",test_result);
//
//     //printf("Test result address(2): %x\n",&test_result);
//
//     //Test that update is working
//     eeprom_busy_wait();
//     eeprom_update_byte (&test , 55) ;
//     test_result = eeprom_read_byte(&test);
//     print("Test result (changed): %x\n",test_result);
//
//     //break; <- Don't think we need this here anymore
//
//
//
//
//
//     //If the board is reset, theoretically, this line should
//     //be printed. However, this has not been confirmed in testing.
//     _delay_ms(100); //delay rx_callback for 100 ms, to see if this helps the
//     print("Display data_0: %d\n", displayEPSChildData);
//     print("Display address_0: %x\n", &displayEPSChildData);
//
//     uint8_t a = eeprom_read_byte(&data[0]);
//
//     //uint8_t savedEPSChildData PROGMEM = 0x04; <- this line won't work, not sure how to change it
//
//     uint8_t displayEPSChildData = eeprom_read_byte(&savedEPSChildData);
//     //print("Display address: %d\n", &savedEPSChildData); <- not needed anymore
//   //  Theoretically, displayEPSChildData should retrieve the data stored in
//   //  savedEPSChildData stored in flash memory. THIS FUNCTIONALITY HAS NOT
//   //  BEEN DEMONSTRATED or OBSERVED YET.
//     print("Display data: %d\n", displayEPSChildData);//<- this should be correct
//
//     while(1) {
//       uint8_t temp = data[0]+1;
//       print("Data stored: %d\n", a);
//       eeprom_update_byte(&data[0],temp);//update data
//     }
//
//
//     init_rx_mob(&rx_mob);
//     if (is_paused(&rx_mob)) {
//         print("WHAT?? Mob is paused\n");
//     }
//     init_tx_mob(&tx_mob);
//     while (1) {
//         resume_mob(&tx_mob);
//         while (!is_paused(&tx_mob)) {}
//         _delay_ms(100);
//     };
//     return 0;
// }}
