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
