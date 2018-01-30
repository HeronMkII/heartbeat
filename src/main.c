#include <uart/uart.h>
#include <uart/log.h>
#include <can/can.h>

#define F_CPU 8
#include <util/delay.h>

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
    data[0] = parent_counter;
    parent_counter += 1;
    print("Parent counter incremented\n");
}

void rx_callback(uint8_t* data, uint8_t len) {
    print("TX received!\n");
    if (len != 0) {
        child_counter = data[0];
        print("child_counter: %d\n", child_counter);
    } else {
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
