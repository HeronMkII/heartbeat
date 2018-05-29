#include "heartbeat.h"

void init_heartbeat();
void assign_heartbeat_status();
void assign_status_message_objects();

void rx_callback(uint8_t*, uint8_t);
void tx_callback(uint8_t*, uint8_t*);

//Pre-initializations
uint8_t obc_status = 0x00; //global variables to store SSM status
uint8_t eps_status = 0x00;
uint8_t pay_status = 0x00;

uint8_t* self_status = 0x00;
uint8_t* parent_status = 0x00;
uint8_t* child_status = 0x00;

uint8_t ssm_id = 0b11; //will be changed by each SSM
//obc {0b00} eps {0b10} pay {0b01}

uint8_t fresh_start = 1; //1 is true. 0 is false

/*Status CAN Message*/
/*Assume 8 bytes: (0)sending SSM (1) receiving SSM (2) message type (2 for
heartbeat) (3) obc status (4) eps status (5) pay status (6) time stamp */
mob_t status_rx_mob = {
    .mob_num = 0,
    .mob_type = RX_MOB,
    .dlc = 8,
    .id_tag = {  }, // ID of this nodes parent
    .id_mask = { 0x00f },
    .ctrl = default_rx_ctrl,
    .rx_cb = rx_callback
};

mob_t status_tx_mob = {
    .mob_num = 1,
    .mob_type = TX_MOB,
    .id_tag = { }, // ID of this nodes child*****
    .ctrl = default_tx_ctrl,
    .tx_data_cb = tx_callback
};

//Functions implementations
void assign_heartbeat_status() {
    switch(ssm_id){
        case 0b00:
            self_status = &obc_status;
            parent_status = &eps_status;
            child_status = &pay_status;
            break;
        case 0b10:
            self_status = &eps_status;
            parent_status = &pay_status;
            child_status = &obc_status;
            break;
        case 0b01:
            self_status = &pay_status;
            parent_status = &obc_status;
            child_status = &eps_status;
            break;
        default:
            print("INVALID SSM ID");
            break;
    }
}

void assign_status_message_objects() {
    switch(ssm_id){
        case 0b00:
            status_rx_mob.mob_num = 1;
            //status_rx_mob.id_tag = {OBC_STATUS_RX_MOB_ID}; //gives error :(
            status_tx_mob.mob_num = 0;
            //status_tx_mob.id_tag = {OBC_STATUS_TX_MOB_ID};
            break;
        case 0b10:
            status_rx_mob.mob_num = 0;
            //status_rx_mob.id_tag = EPS_STATUS_RX_MOB_ID;
            status_tx_mob.mob_num = 1;
            //status_tx_mob.id_tag = EPS_STATUS_TX_MOB_ID;
            break;
        case 0b01:
            status_rx_mob.mob_num = 0;
            //status_rx_mob.id_tag = PAY_STATUS_RX_MOB_ID;
            status_tx_mob.mob_num = 1;
            //status_tx_mob.id_tag = PAY_STATUS_TX_MOB_ID;
            break;
        default:
            print("INVALID SSM ID");
            break;
    }
}

void init_heartbeat() {
    //assign_heartbeat_status();
    assign_heartbeat_status();
    assign_status_message_objects();
    if (eeprom_read_dword((uint32_t*) INIT_WORD_EEMEM) != DEADBEEF){
        print("SSM FRESH START\n");
        eeprom_update_dword((uint32_t*) INIT_WORD_EEMEM, DEADBEEF);
        fresh_start = 0;
    }
    else {
        print("SSM RESTART -> RETRIEVE STATUS\n");
        switch(ssm_id){
            case 0b00:
                *self_status = eeprom_read_byte((uint8_t*) OBC_STATUS_EEMEM);
                *parent_status = eeprom_read_byte((uint8_t*) EPS_STATUS_EEMEM);
                *child_status = eeprom_read_byte((uint8_t*) PAY_STATUS_EEMEM);
                break;
            case 0b10:
                *self_status = eeprom_read_byte((uint8_t*) EPS_STATUS_EEMEM);
                *parent_status = eeprom_read_byte((uint8_t*) PAY_STATUS_EEMEM);
                *child_status = eeprom_read_byte((uint8_t*) OBC_STATUS_EEMEM);
                break;
            case 0b01:
                *self_status = eeprom_read_byte((uint8_t*) PAY_STATUS_EEMEM);
                *parent_status = eeprom_read_byte((uint8_t*) OBC_STATUS_EEMEM);
                *child_status = eeprom_read_byte((uint8_t*) EPS_STATUS_EEMEM);
                break;
            default:
                print("INVALID SSM ID. STATUS NOT RETRIEVED\n");
                break;
        }
        //fresh_start = 0;
    }
}

/*Status CAN Message*/
/*Assume 8 bytes: (0)sending SSM (1) receiving SSM (2) message type (2 for
heartbeat) (3) obc status (4) eps status (5) pay status (6) time stamp */
void tx_callback(uint8_t* data, uint8_t* len) {
    *len = 8;
    data[0] = ssm_id;
    //data[1] = 
    data[1] = 2; //message type: 2 for heartbeat
    data[2] = 0; //field number (follow CAN message format in OBC commnd queue)
    //data[3] = not finished
    //data[4] = not finished
    //data[5] = not finished
    print("Status updated and sent to parent.\n");
}

void rx_callback(uint8_t* data, uint8_t len) {
    print("Received status from child\n");
    if (len != 0) {
        //child_counter = data[0];
        //save updated data into EEPROM
        print("child_counter: \n");
    } else {
        print("No data\n");
    }
}

//This main function simulates the example file in lib-common
int main() {
    ssm_id = 0b00; //obc
    init_heartbeat();
    //uint8_t test = *self_status;
    return 0;

}
