#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/cm3/vector.h>

#define LED_PORT (GPIOA) //setup so we don't have to update all the time
#define LED_PIN (GPIO5)

#define CPU_FREQ (84000000)
#define SYSTICK_FREQ (1000)

volatile uint64_t ticks = 0; // we normally use volatile when we are using mmio register or interrupts 
void sys_tick_handler(void){
  /*32 bit only last 49 days, so we used 64
  but we are using 32 bit mcu, so 2 assembly instuctions needed for handling. what if interrupt occurs in between them?
  therefor when entering this function, we need to disable->mask_correctly interrupts...*/
  ticks++;
}

static uint64_t get_ticks(void){
  return ticks;
}

static void rcc_setup(void){
  //need to setup clock, it takes a constant pointer (does not update it), 84M cycles per second
  //clock cycle running in nano seconds
  rcc_clock_setup_pll(&rcc_hsi_configs[RCC_CLOCK_3V3_84MHZ]);
}

static void gpio_setup(void){
  //to send clock to our peripheral port
  rcc_periph_clock_enable(RCC_GPIOA);
  //Pll UP Down is some value that it always keep, overwritable, when we have some floating point
  //set the pin and port as output(passing 0x1 here in place of GPIO_MODE_OUTPUT)
  gpio_mode_setup(LED_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LED_PIN);
}

/*
RCC = clock control/distribution system. It controls which clocks exist and which peripherals receive them.
SysTick = one timer that counts using a clock. It counts down from a reload value to zero. When it reaches zero, it can fire an interrupt.
RCC sets the main clock speed
SysTick uses that speed to count time
we have several clocks, firmware deals with a clock tree, not a single clock.
*/
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

/* Bad implementation
static void delay_cycles(uint32_t cycles){
  //created some delay, then added inline assembly so compiler don't remove this loop
  for (uint32_t i=0; i<cycles;i++){
    __asm__("nop");
  }
}
*/

int main(void){
  //setup functions
  rcc_setup();
  gpio_setup();
  systick_setup();

  uint64_t start_time = get_ticks();


  while(1){
    if (get_ticks()-start_time >= 1000) {
      gpio_toggle(LED_PORT,LED_PIN);
      start_time = get_ticks();
    }
    //now we can do useful work

    //delay_cycles(84000000 / 4);
  }
  //Never return
  return 0;
}


