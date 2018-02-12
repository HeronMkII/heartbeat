#include <uart/uart.h>
#include <uart/log.h>
#include <can/can.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#define F_CPU 8
//The below line needs to be included to use progmem features in AVR-C
//http://www.nongnu.org/avr-libc/user-manual/group__avr__pgmspace.html#ga75acaba9e781937468d0911423bc0c35
#define PROGMEM __ATTR_PROGMEM__

uint8_t savedEPSChildData = 0; //variable declared to be stored in progmem space
//Observe that savedEPSChildData will no longer be zero as the project runs.
uint8_t displayEPSChildData;
/*variable declared to retrieve data stored in savedEPSChildData in progmem space.
Functionality of retrieving data from flash memory has not been completed yet*/

void rx_callback(uint8_t*, uint8_t);
void tx_callback(uint8_t*, uint8_t*);

int child_counter = 0;
int parent_counter = 0;

#define A_PARENT 0x001c
#define A_CHILD  0x000b
#define B_PARENT 0x001a
#define B_CHILD  0x000c
#define C_PARENT 0x001b
#define C_CHILD  0x000a

/*Suggestion for testing:
Include the above ID tags in the the below mobs initialization for each boards
under test accordingly
*/

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
    print("TX_CALLBACK TRIGGERED\n"); //indicates that tx_callback function is called
    *len = 1;
    data[0] = parent_counter; //send parent_counter as tx to another subsystem
    parent_counter += 1; //every time tx_callback is called, parent_counter is incremented
    print("Tx-EPS set OBC parent counter : %d\n", parent_counter);//TO DO: MOVE
    //THIS LINE TO BEFORE parent_counter += 1;
    _delay_ms(100); //for testing purposes, delay this func. to try to see if
    //could help tx_callback to be called and execute properly. Not 100% if
    //this helps yet.
}

void rx_callback(uint8_t* data, uint8_t len) {
    print("RX_CALLBACK TRIGGERED\n");
    if (len != 0) {
        child_counter = data[0]; //EPS receives parent_counter set by another
        //subsystem, and stores it in its child_counter
        print("RX-EPS received OBC child_counter: %d\n", child_counter);
        //store into memory of this board (add const in front)
        uint8_t savedEPSChildData PROGMEM = child_counter; //store child_counter
        //in flash memory (or progmem space) as variale savedEPSChildData
        print("RX-EPS child_counter saved to flash drive: %d\n", savedEPSChildData);
    } else {
        print("No data\n");
    }
    _delay_ms(100); //delay rx_callback for 100 ms, to see if this helps the
    //function being called properly. Not 100% sure.
}

int main() {
    print("RESET!!!"); //If the board is reset, theoretically, this line should
    //be printed. However, this has not been confirmed in testing.
    init_uart();
    init_can();

    displayEPSChildData = pgm_read_word_near(&savedEPSChildData);
    //Theoretically, displayEPSChildData should retrieve the data stored in
    //savedEPSChildData stored in flash memory. THIS FUNCTIONALITY HAS NOT
    //BEEN DEMONSTRATED or OBSERVED YET.
    print("Display data: %d\n", displayEPSChildData);

    init_rx_mob(&rx_mob);
    if (is_paused(&rx_mob)) {
        print("WHAT?? Mob is paused\n");
    }
    init_tx_mob(&tx_mob);
    while (1) {
        resume_mob(&tx_mob);
        while (!is_paused(&tx_mob)) {}
        _delay_ms(100);
    };
    return 0;
}
