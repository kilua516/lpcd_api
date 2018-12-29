#ifndef ISO_API_H
#define ISO_API_H


void RF_en(uchar enable);
void init_typeA(void);
void getbuf_printf( uchar Regaddr , uint lenth );
void eeprom_read(u16 addr, u16 length);
void eeprom_write(u16 addr, u16 length, uchar wr_data);
void reqA();
void wakeupA();
void anticollsionA();
void selectA();
void haltA();
void ratsA();
void type_set( unsigned char type);
void loadkey(u8 load_type, u8 key_val,u16 key_addr);
void authentication(u8 block_id);
void secret_set(u8 secret_en);
void write_block(u8 block_id, u8 wr_val);
void read_block(u8 block_id);
void typeA_APDU_random(uchar baud_speed,uint cnt);
void M1_operation_test(uchar loadkey,uchar M1_en,uchar key, uchar secret);
void TypeB_APDU_random(uchar baud_speed,uint cnt);


#endif
// endfile

