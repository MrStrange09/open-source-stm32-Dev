#include "timer.h"
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/rcc.h>

// 84_000_000
// pwm_freq = timer_clock / (prescaler * arr)
#define PRESCALER (84)
#define ARR_VALUE (1000)


void timer_setup(void){
    rcc_periph_clock_enable(RCC_TIM2);

    //set time mode, tim2, divider, on edge, and up counter
    // high level timer configuration
    timer_set_mode(TIM2, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);

    //set pwn so set output compare
    //setup pwm mode
    timer_set_oc_mode(TIM2, TIM_OC1, TIM_OCM_PWM1);

    //Enable pwm output

    //next enable the counter unless it won't move up
    timer_enable_counter(TIM2);
    //setup output compare
    timer_enable_oc_output(TIM2, TIM_OC1);

    //setup frequency and resolution
    timer_set_prescaler(TIM2, PRESCALER-1);
    timer_set_period(TIM2, ARR_VALUE-1);

}

void timer_pwm_set_duty_cycle(float duty_cycle){
    // duty cycle = (ccr/arr) * 100
    // ccr = arr * (duty cycle / 100)

    const float raw_value = (float)ARR_VALUE * (duty_cycle / 100.0f);
    timer_set_oc_value(TIM2, TIM_OC1, (uint32_t)raw_value);
}
