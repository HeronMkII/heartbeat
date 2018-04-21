//The purpose of this script is to illustrate how to use EEPROM functions in AVR.
/*The basic functions are listed in the following, while the script illustrate
the functionality using print statements.
(1)Read from fixed address
 uint8_t prg_data; //in program space
 uint8_t eeprom_address = 0xFF; //address in eeprom
 prg_data = eeprom_read_byte((uint8_t*)eeprom_address); //reads value from address

(2) Writes data from program space to address in eeprom
 uint8_t eeprom_address = 0xFF;
 uint8_t write_data = 42;
 eeprom_update_byte((uint8_t*)eeprom_address, write_data);

(3) Reads data with eemem attribute from eeprom
 uint8_t prg_data;
 prg_data = eeprom_read_byte(&data_eemem);

(4) Writes data with eemem attribute
 uint8_t new_data = 42;
 eeprom_update_byte(&data_eemem, new_data);

(5) reads data from block_var (EEMEM) into block_test (program space) and prints out data from block_test
 uint8_t EEMEM block_eemem[16];
 uint8_t block_prgm[16];
 eeprom_read_block((void*)block_prgm, (const void*)block_eemem, 16);
 eeprom_read_block((void*)test_array, (const void*)0xFF, 16);

(6) updates block with eemem attribute
  uint8_t block_prgm[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
  eeprom_update_block((const void*)block_prgm, (void*)block_eemem,16);
*/

#include <uart/uart.h>
#include <uart/log.h>
#include <can/can.h>

#define F_CPU 8
#include <util/delay.h>
#include <avr/eeprom.h>

#define EEMEM   __attribute__((section(".eeprom")));

uint8_t EEMEM eemem_var;
uint8_t EEMEM data_eemem;
uint8_t EEMEM eemem_block[16];

int main(void){

  init_uart();
  init_can();

  //**Writing and reading data in EEPROM**
  //Note: Data is persistent, need to change write_data to observe difference between runs
  uint8_t write_data;
  write_data = 0xF9;
  uint8_t read_data;
  read_data = eeprom_read_byte((uint8_t*)46);//read data at address 46 in EEPROM

  _delay_ms(1000); //delay for readibility on CoolTerm
  print("Data (Before): %d\n",read_data);
  eeprom_update_byte((uint8_t*)46, write_data); //update the value at address 46 in EEPROM to write_data
  read_data = eeprom_read_byte((uint8_t*)46);//read data again from address 46 in EEPROM
  _delay_ms(1000);
  print("Data (After): %d\n",read_data);

  //Test for persistence- press reset while loop is running
  //Observed that data stays constant compared to what was previously set
  /*
  while (1){
    read_data = eeprom_read_byte((uint8_t*)46);//46 can be changed to any address
    print("Data: %d\n",read_data);
    }
*/

  //Tests reading data at arbitrary locations
  //Output is all 255 (0xFF) if not previously modified.
  //Note: The size of EEPROM is 1024 bytes
  uint8_t data;
  uint8_t m = 0;
  for (m; m<1024; m++){
    data = eeprom_read_byte((uint8_t*)(m));
    print("Data: %d at location %d\n", data, m);
  }


  //**Block Access Reading**
  uint8_t test_array[16];
  //takes 16 bytes of data starting at address 12 in EEPROM, and stores data from EEPROM in test array in program space
  eeprom_read_block((void*)test_array, (const void*)12, 16);
  uint8_t k = 0;
  for (k; k<16; k++){
    print("Array value: %d at address %x\n",test_array[k],k);
  }

  //**Block Access Updates and Reads**
  //Block Access update
  uint8_t block_write[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
  eeprom_update_block((const void*)block_write, (void*)12, 16);
  //Takes 16 bytes of data in block_write (in program space) and stores it starting at address 12 in EEPROM
  uint8_t block_read[16];
  eeprom_read_block((void*)block_read, (const void*)12, 16);
  //reads data from address 12 to 27 in EEPROM into block_read (program space)
  uint8_t j = 0;
  for (j; j<16; j++){
    print("Array value in block_read: %d at address %x\n",block_read[j],12+j);
    //output: 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15
  }


  //**Using EEMEM attribute to store/retrieve data at arbitrary address in EEPROM**
  //Must define EEMEM var outside of main (see line 9-13)
  uint8_t update = 6;
  eeprom_update_byte(&eemem_var, update);
  //update eemem_var to the value update. Argument must be type of uint8_t* and uint8_t
  uint8_t pgm_var;
  pgm_var = eeprom_read_byte(&eemem_var);
  //read the value stores in eemem_var in EEPROM and store it in pgm_var in program space
  print("eemem_var: %d\n",eemem_var); //output: 6
  print("pgm_var: %d\n",pgm_var); //output: 6


  //**Using Block Access with EEMEM**
  uint8_t block[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};//block is stored in program space
  eeprom_update_block((const void*)block, (void*)eemem_block,16);
  //update eemem_block in EEMEM with same values as block in program space.The last argument is the size of array
  uint8_t block_test[16];
  eeprom_read_block((void*)block_test, (const void*)eemem_block, 16);
  //reads data from eemem_block into block_test (program space) and prints out data from block_test
  uint8_t i = 0;
  for (i;i<16; i++){
    print("Array value in block_test: %d at address %x\n",block_test[i],i);
    //output: 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15
    }
}
