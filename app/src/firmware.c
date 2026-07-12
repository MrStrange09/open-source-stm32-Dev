#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/cm3/scb.h>

#include "core/system.h"
#include "core/uart.h"
#include "timer.h"

#define BOOTLOADER_SIZE (0x8000U)


#define LED_PORT (GPIOA) //setup so we don't have to update all the time
#define LED_PIN (GPIO5)

#define UART_PORT (GPIOA)
#define RX_PIN    (GPIO3)
#define TX_PIN    (GPIO2)

static void vector_setup(void){
  SCB_VTOR = BOOTLOADER_SIZE;
}

static void gpio_setup(void){
  //to send clock to our peripheral port
  rcc_periph_clock_enable(RCC_GPIOA);
  //Pll UP Down is some value that it always keep, overwritable, when we have some floating point
  //set the pin and port as output(passing 0x1 here in place of GPIO_MODE_OUTPUT)
  gpio_mode_setup(LED_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, LED_PIN);
  gpio_set_af(LED_PORT, GPIO_AF1, LED_PIN);

  gpio_mode_setup(UART_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, TX_PIN | RX_PIN);
  gpio_set_af(UART_PORT, GPIO_AF7, TX_PIN | RX_PIN);



}

/*
RCC = clock control/distribution system. It controls which clocks exist and which peripherals receive them.
SysTick = one timer that counts using a clock. It counts down from a reload value to zero. When it reaches zero, it can fire an interrupt.
RCC sets the main clock speed
SysTick uses that speed to count time
we have several clocks, firmware deals with a clock tree, not a single clock.
*/


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
  vector_setup();
  system_setup();
  gpio_setup();
  timer_setup();
  uart_setup();

  uint64_t start_time = system_get_ticks();
  float duty_cycle = 0.0f;
  float direction = 1.0f;

  timer_pwm_set_duty_cycle(duty_cycle);

  while(1){
    if (system_get_ticks() - start_time >= 10) {
		duty_cycle += direction;

		if (duty_cycle >= 100.0f) {
			duty_cycle = 100.0f;
			direction = -1.0f;
		}

		if (duty_cycle <= 0.0f) {
			duty_cycle = 0.0f;
			direction = 1.0f;
		}

		timer_pwm_set_duty_cycle(duty_cycle);
		start_time = system_get_ticks();
    //now we can do useful work
  }
    //delay_cycles(84000000 / 4);

    if(uart_data_available()){
      uint8_t data = uart_read_byte();
      uart_write_byte(data+1);
    }

    // Do useful work;
  }

  //Never return
  return 0;
}


