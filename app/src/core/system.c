#include "core/system.h"

#include <libopencm3/cm3/systick.h>
#include <libopencm3/cm3/vector.h>
#include <libopencm3/stm32/rcc.h>
// made ticks internal
static volatile uint64_t ticks = 0; // we normally use volatile when we are using mmio register or interrupts 
void sys_tick_handler(void){
  /*32 bit only last 49 days, so we used 64
  but we are using 32 bit mcu, so 2 assembly instuctions needed for handling. what if interrupt occurs in between them?
  therefor when entering this function, we need to disable->mask_correctly interrupts...*/
  ticks++;
}


static void rcc_setup(void){
  //need to setup clock, it takes a constant pointer (does not update it), 84M cycles per second
  //clock cycle running in nano seconds
  rcc_clock_setup_pll(&rcc_hsi_configs[RCC_CLOCK_3V3_84MHZ]);
}

static void systick_setup(void){
  //set frequency of the tick, we want to deal in milliseconds and not nano seconds with our clock does
  // Configure SysTick to generate SYSTICK_FREQ ticks per second.
	// With SYSTICK_FREQ = 1000, each tick is 1 ms.
  systick_set_frequency(SYSTICK_FREQ, CPU_FREQ);
  	// Start the SysTick countdown counter.
  systick_counter_enable();
  //counter runs 1000 times per second and raise interrupt
  // Raise an interrupt every time the counter reaches zero.
  systick_interrupt_enable();

  // but where't the handler, its handler is defines as weak in vector.c so we need to implement our own handler or it will call do nothing function.
}


// below are public functions

uint64_t system_get_ticks(void){
  return ticks;
}


void system_setup(void){
    rcc_setup();
    systick_setup();
};

