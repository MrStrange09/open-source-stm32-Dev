#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/usart.h>
#include "core/uart.h"
#include "core/ring-buffer.h"


// we have built a  driver that can send bytes (polling/blocking) and receive bytes
// (interrupt-driven). TX is simple and blocking; RX uses an interrupt to push each
// incoming byte into a ring buffer (FIFO) that the main loop drains at its own pace,
// so bursts of bytes are no longer dropped by a single-byte mailbox.


#define BAUD_RATE (115200)
#define RING_BUFFER_SIZE (128) // For maximum of ~10ms of latency

static ring_buffer_t rb = {0U};

static uint8_t data_buffer[RING_BUFFER_SIZE] = {0U}; // backing storage for the RX ring buffer
//static bool data_available = false; // flag: is there an unread byte? //removed due to race condition

/* weak symbol from nvic handler,  this function is automatically wired to the
USART2 interrupt. When a byte arrives (RXNE set) or an overrun happens (ORE set),
it reads the byte via usart_recv() (which also clears the flag) and pushes it into
the RX ring buffer for the main loop to consume later. */
void usart2_isr(void){
    const bool overrun_occurred = usart_get_flag(USART2, USART_FLAG_ORE) == 1;
    const bool received_data = usart_get_flag(USART2, USART_FLAG_RXNE) == 1;

    if(received_data||overrun_occurred){
        if(ring_buffer_write(&rb,(uint8_t)usart_recv(USART2))){
            // Handle failure? Later
        }
    }
}
//setup
/* uart_setup() configures USART2 for the classic 115200 baud, 8-N-1 
(8 data bits, no parity, 1 stop bit), no hardware flow control, in TX+RX mode */

void uart_setup(void){ 
    ring_buffer_setup(&rb, data_buffer, RING_BUFFER_SIZE);

    rcc_periph_clock_enable(RCC_USART2);
    
    usart_set_mode(USART2, USART_MODE_TX_RX);
    usart_set_flow_control(USART2, USART_FLOWCONTROL_NONE);
    usart_set_databits(USART2, 8);
    usart_set_baudrate(USART2, BAUD_RATE);
    usart_set_parity(USART2, 0);
    usart_set_stopbits(USART2, 1);
    
    //fire an interrupt when received a byte
    usart_enable_rx_interrupt(USART2);
    // let the NVIC actually deliver it to the CPU
    nvic_enable_irq(NVIC_USART2_IRQ);
    //turn the peripheral on (done last, after everything is configured)
    usart_enable(USART2);
}

void uart_write(uint8_t* data, const uint32_t length){ 
    for (uint32_t i=0;i<length;i++){
        uart_write_byte(data[i]);
    }
}
// Blocks until the TX register is free, then sends one byte
void uart_write_byte(uint8_t data){ 
    usart_send_blocking(USART2, (uint16_t)data);
}
// Drains up to `length` bytes from the RX ring buffer into `data`;
// returns how many were actually read (0..length, stops early if the buffer empties)
uint32_t uart_read(uint8_t* data, const uint32_t length){ 
   if(length==0){
    return 0;
   }

   for(uint32_t bytes_read=0; bytes_read<length; bytes_read++){
    if(!ring_buffer_read(&rb, &data[bytes_read])){
        return bytes_read;
    }
   }
   return length;
}

uint8_t uart_read_byte(void){ 
        uint8_t byte =0;
        uart_read(&byte,1);
        return byte;
}
// True if the RX ring buffer holds at least one unread byte (non-destructive peek)
bool uart_data_available(void){
    return !ring_buffer_empty(&rb);
}