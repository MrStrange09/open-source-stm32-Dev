#include "common-defines.h"
#include <libopencm3/stm32/memorymap.h>
#include <libopencm3/cm3/vector.h>

#define BOOTLOADER_SIZE (0X8000U) //set linked rom size to 32B so we don't exceed our bootloader code than this size
//therefore we have set the max size limit of bootloader, but now need to add padding so its always fixed size
#define MAIN_APP_START_ADDRESS (FLASH_BASE + BOOTLOADER_SIZE)

static void jump_to_main(void){
  typedef void (*void_fn)(void);

  vector_table_t* main_vector_table = (vector_table_t*)(MAIN_APP_START_ADDRESS);
//In C, you can always treat pointers as if they were arrays
//  uint32_t* reset_vector = (uint32_t*)(main_vector_table[1]);
// void_fn jump_fn = (void_fn)main_vector_table[1];
//  jump_fn();

main_vector_table->reset(); 


}

int main(void){
  jump_to_main();
  // Never return
  return 0;
}
