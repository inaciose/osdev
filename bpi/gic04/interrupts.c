#include "uart.h"
#include "timer.h"
#include "gicv2.h"
#include "macro.h"
#include "timer_io.h"

//-------------------------------------------------------------------------

void hexstrings ( unsigned int d )
{
  unsigned int rb;
  unsigned int rc;

  rb=32;
  while(1)
  {
    rb-=4;
    rc=(d>>rb)&0xF;
    if(rc>9) rc+=0x37; else rc+=0x30;
    uart_putc(rc);
    if(rb==0) break;
  }
  uart_putc(0x20);
}

void hexstring ( unsigned int d )
{
  hexstrings(d);
  uart_putc(0x0D);
  uart_putc(0x0A);
}

//-------------------------------------------------------------------------

// global variable
int ledstate = 0;

void greenled_config( void ) {
  // Configure port PH24 (green led) for output
  asm ("\n"
    "mov r0,  #0x00000001 \n"
    "ldr r3, =0x01C20908 \n"
    "str r0, [r3] \n");
}

void greenled_on( void ) {
  
  // Turn led on PH24
  asm (" \n"
    "mov r0,  #0x01000000 \n"
    "ldr r3, =0x01C2090C \n"
    "str r0, [r3]");
}

void greenled_off( void ) {
  // Turn led off PH24
  asm (" \n"
    "mov r0,  #0x00000000 \n"
    "ldr r3, =0x01C2090C \n"
    "str r0, [r3]");
}

//-------------------------------------------------------------------------

void __attribute__((interrupt("UNDEF"))) undefined_instruction_vector(void)
{
  uart_puts("im where UNDEF\n");
  while( 1 ) { }
}

void __attribute__((interrupt("SWI"))) software_interrupt_vector(void)
{
  uart_puts("im where SWI\n");
}

void __attribute__((interrupt("ABORT"))) prefetch_abort_vector(void)
{
  uart_puts("im where ABORT1\n");
  while( 1 ) { }
}

void __attribute__((interrupt("ABORT"))) data_abort_vector(void)
{
  uart_puts("im where ABORT2\n");
  while( 1 ) { }
}

void __attribute__((interrupt("IRQ"))) interrupt_vector(void)
{
  // get & ack the irqno from gic register
  unsigned int intno;
  intno = gic_getack();

  // process the interrupt
  switch(intno) {
    // irqno for uart0 = 33 (0x21)
    case 0x21:
      // echo console
      uart_putc(uart_getc()); 
    break;
    
    // irqno for timer0 = 54 (0x36)
    case 0x36:
      // blink led
      if(ledstate) {
        // led off
        ledstate = 0;
        greenled_off();
      } else {
        // led on
        ledstate = 1;
        greenled_on();
      }

      timer0_clear_irq();
    break;

    // irqno for timer0 = 55 (0x37)
    case 0x37:
      hexstrings(intno);
      hexstrings(mmio_read32(TMR_IRQ_EN));
      hexstrings(mmio_read32(TMR_IRQ_STA));

      hexstrings(mmio_read32(TMR_0_CTRL));
      hexstrings(mmio_read32(TMR_0_INTR_VAL));
      hexstrings(mmio_read32(TMR_0_CUR_VAL));
      
      hexstrings(mmio_read32(TMR_1_CTRL));
      hexstrings(mmio_read32(TMR_1_INTR_VAL));
      hexstring(mmio_read32(TMR_1_CUR_VAL));
      timer1_clear_irq();
    break;
  }
  // clear the irqno from gic register
  gic_eoi(intno);
}

void __attribute__((interrupt("FIQ"))) fast_interrupt_vector(void)
{
  uart_puts("im where FIQ\n");
  while( 1 ) { }
}
