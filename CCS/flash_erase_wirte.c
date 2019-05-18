#include <msp430.h>

// You can monitor value of memory at here
// view -> Memory Browser

void flash_mem_erase(unsigned long * addr);
void flash_mem_write(unsigned long * addr, unsigned char data);

unsigned long * mem_start_addr  = 0x1800; // start information memory address bit of MSP430F5529
unsigned long * mem_end_addr    = 0x19ff; // end information memory address bit of MSP430F5529
unsigned long * mem_target_addr = 0x1820;
const unsigned char VALUE       = 0x22;

int main(void){
    WDTCTL = WDTPW + WDTHOLD;                 // Stop watchdog timer

    flash_mem_erase((unsigned long *) mem_target_addr);
    flash_mem_write((unsigned long *) mem_target_addr, VALUE);

    while(1){

    }
}

void flash_mem_erase(unsigned long * addr){
    //__disable_interrupt();
    unsigned long * info_flash_area;

    // should work in only information memory area
    if((addr>= (unsigned long *) mem_start_addr) && (addr <= (unsigned long *) mem_end_addr)){
        info_flash_area = (unsigned long *) addr;
    }
    else{
        return;
    }
    //set flash control register
    FCTL1 = FWKEY + ERASE;                    // Set Erase bit
    FCTL3 = FWKEY;                            // Clear Lock bit

    *(unsigned char * )info_flash_area = 0;

    FCTL1 = FWKEY;
    FCTL3 = FWKEY;
    //__enable_interrupt();
}

void flash_mem_write(unsigned long * addr, unsigned char data){
    //__disable_interrupt();
    unsigned long * info_flash_area;

    // should work in only information memory area
    if((addr>= (unsigned long *) mem_start_addr) && (addr <= (unsigned long *) mem_end_addr)){
            info_flash_area = (unsigned long *) addr;
    }
    else{
        return;
    }
    //set flash control register
    FCTL1 = FWKEY + WRT;                      // Set WRT bit for write operation
    FCTL3 = FWKEY;

    *(unsigned char * )info_flash_area = data;

    FCTL1 = FWKEY;
    FCTL3 = FWKEY + LOCK;                     // Set LOCK bit

    //__enable_interrupt();
}