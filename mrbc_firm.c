/* ************************************************************************** */
/** mrbc firm

  @Company
    ShimaneJohoshoriCenter.inc

  @File Name
    mrbc_firm.c

  @Summary
    mruby/c IDE support

  @Description
    mruby/c IDE support & Flash writing
 */
/* ************************************************************************** */

#include "mrbc_firm.h"

static uint8_t flashBuffer[MAX_SIZE];
uint8_t t_count = 0;

void flash_del(void* address){
    NVMADDR = ((unsigned int) address & 0x1FFFFFFF);
    NVMCONCLR = 0xF;
    NVMCON = 0x4004;
    NVMKEY = 0xAA996655;
    NVMKEY = 0x556699AA;
    NVMCONSET = 0x8000;
    while (NVMCON & 0x8000);
    NVMCONCLR = 0x4000;
}

void flash_write(void* address, void* data){
    NVMADDR = ((unsigned int) address & 0x1FFFFFFF);
    NVMSRCADDR = ((unsigned int) data & 0x1FFFFFFF);
    NVMCONCLR = 0xF;
    NVMCON = 0x4003;
    NVMKEY = 0xAA996655;
    NVMKEY = 0x556699AA;
    NVMCONSET = 0x8000;
    while (NVMCON & 0x8000);
    NVMCONCLR = 0x4000;
}

static void saveFlush(const uint8_t* writeData, uint16_t size) {
    static int fl_addr = FLASH_SAVE_ADDR;
    if(size > sizeof(flashBuffer)) {
        return -1;
    }

    memset(flashBuffer, 0, sizeof(flashBuffer));
    memcpy(flashBuffer, writeData, size);

   int rowCount = (size % ROW_SIZE == 0) ? size / ROW_SIZE : size / ROW_SIZE + 1;
   int i = 0;
   for(i = 0;i < rowCount; i++) {
        flash_write((void *)(fl_addr + i * ROW_SIZE), (void *)&flashBuffer[i * ROW_SIZE]);
   }
   fl_addr = fl_addr + ROW_SIZE*rowCount;
}

void add_code(){
    char txt[50];
    char *txt_addr;
    int txt_len;
    txt_addr = txt;
    int i = 0;
    for(i = 0;i < 60;i++) {
        flash_del((void *)(FLASH_SAVE_ADDR + i * PAGE_SIZE));
    }
    // [crlf] read waite
    txt_len = u_read(txt_addr);
    if(!txt_len){
        return;
    }
    u_puts("+OK mruby/c\r\n",0);
    memset(txt, 0, sizeof(txt));
    while(1){
        txt_len = u_read(txt_addr);
        if(strncmp(txt_addr,"version",7)==0){
            u_puts("+OK mruby/c PSoC_5LP v1.00 (2018/09/04)\r\n",0);
            memset(txt, 0, sizeof(txt));
        }else if(strncmp(txt_addr,"execute",7)==0){
            u_puts("+OK Execute mruby/c.\r\n",0);
            memset(txt, 0, sizeof(txt));
            return;
        }else if(strncmp(txt_addr,"write",5)==0){
            // bytecode length receive
            u_puts("+OK Write bytecode\r\n",0);
            int size = 0;
            txt_len -= 2;
            int j = 0;
            while(txt_len>5){
                size += (txt[txt_len] - 0x30) * pow(10,j);
                j++;
                txt_len--;
            }

            unsigned int return_size = size;
            uint8_t data[size];

            // mruby/c code write
            int i = 0;
            while (size > 0) {
                IFS1bits.U1RXIF = 0;
                while(!U1STAbits.URXDA);
                data[i] = U1RXREG;
                size--;
                i++;
            }
            saveFlush(data, sizeof(data));
            u_puts("+DONE\r\n",0);
            memset(txt, 0, sizeof(txt));
        }
    }
}