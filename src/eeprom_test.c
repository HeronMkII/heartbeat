//Note: This code contains various read/update tests for eeprom
//Note: This code is not intended to be run as one cohesive unit,
// but rather for the tester to extract pieces to confirm functionality of eeprom
//Note: The commented code directly below demonstrates basic functionality of built-in eeprom functions

/*//Read from fixed address
 uint8_t prg_data; //in program space
 uint8_t eeprom_address = 0xFF; //address in eeprom
 prg_data = eeprom_read_byte((uint8_t*)eeprom_address); //reads value from address
*/

/*//Writes data from program space to address in eeprom
 uint8_t eeprom_address = 0xFF;
 uint8_t write_data = 42;
 eeprom_update_byte((uint8_t*)eeprom_address, write_data);
*/

/*//Reads data with eemem attribute from eeprom
 uint8_t prg_data;
 prg_data = eeprom_read_byte(&data_eemem);
*/

/*//Writes data with eemem attribute
 uint8_t new_data = 42;
 eeprom_update_byte(&data_eemem, new_data);
*/

/*//reads data from block_var (EEMEM) into block_test (program space) and prints out data from block_test
  uint8_t EEMEM block_eemem[16];
  uint8_t block_prgm[16];
  eeprom_read_block((void*)block_prgm, (const void*)block_eemem, 16);

  eeprom_read_block((void*)test_array, (const void*)0xFF, 16);

   //updates block with eemem attribute
   uint8_t block_prgm[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
   eeprom_update_block((const void*)block_prgm, (void*)block_eemem,16);
*/

#include <uart/uart.h>
#include <uart/log.h>
#include <can/can.h>
#include <util/delay.h>
#include <avr/eeprom.h>

#define EEMEM   __attribute__((section(".eeprom")));

uint8_t EEMEM data_eemem;

uint8_t EEMEM block_var[16];

uint8_t EEMEM var;

int main(void){
  //Code contained in various loops below can be used to demonstrate functionality
  //of built-in eeprom functions
  init_uart();
  init_can();
  _delay_ms(1000);

  //Block Access Testing with EEMEM
  //block is stored in program space
  uint8_t block[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
  //update block_var in EEMEM with same values as block in program space
  eeprom_update_block((const void*)block, (void*)block_var,16);

  while(1){
  //reads data from block_var (EEMEM) into block_test (program space) and prints out data from block_test
    uint8_t block_test[16];
    eeprom_read_block((void*)block_test, (const void*)block_var, 16);
    for (int i = 0;i<16;i++){
      print("Array value: %d at address %x\n",block_test[i],i);
    }
  }

  //Block Access Testing without EEMEM
  //Block Access update
  uint8_t block_write[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
  //Takes 16 bytes of data in block_write (in program space) and stores it starting at address 12 in EEPROM
  eeprom_update_block((const void*)block_write, (void*)12, 16);

  //Block Access reading
  while(1){
    uint8_t test_array[16];
    //takes 16 bytes of data at address 12 in EEPROM, and stores data from EEPROM in test array in program space
    eeprom_read_block((void*)test_array, (const void*)12, 16);
    for (int i = 0;i<16;i++){
      print("Array value: %d at address %x\n",test_array[i],i);
    }
  }


  //Test EEMEM attribute: Store/retrieve data at arbitrary location
  //Must define EEMEM var outside of main
  //update by passing in address of EEMEM var
  //read by passing in address of var

  eeprom_update_byte(&var, 0x06);
  uint8_t read;
  while(1){
    read = eeprom_read_byte(&var);
    print("var: %d\n",read);
  }

  //Writing data test - delay for print statements
  //Note: Data is persistent, need to change write_data to observe difference between runs
  uint8_t write_data;
  write_data = 0xF9;
  uint8_t read_data;
  read_data = eeprom_read_byte((uint8_t*)46);//46 can be changed to any address

  _delay_ms(1000);
  print("Data (Before): %d\n",read_data);
  eeprom_update_byte((uint8_t*)46, write_data);
  read_data = eeprom_read_byte((uint8_t*)46);//46 can be changed to any address
  _delay_ms(1000);
  print("Data (After): %d\n",read_data);


//Test for persistence- press reset while loop is running
//Observed that data stays constant compared to what was previously set
  while (1){
    read_data = eeprom_read_byte((uint8_t*)46);//46 can be changed to any address
    print("Data: %d\n",read_data);
  }


  //Tests reading data at arbitrary locations
  //Output is all 255 (0xFF) if not previously modified
  while(1){
    uint8_t data;
    for (int i = 0;i<1000;i++){
    data = eeprom_read_byte((uint8_t*)(i));
    print("Data: %d at location %d\n",data,i);
 }
 }

}
