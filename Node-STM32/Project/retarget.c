#include "usart.h"
#include "common.h"

#ifdef __GNUC__
/* Retargeting functions for gcc-arm-embedded */

int _write (int fd, char *ptr, int len)
{
  /* Write "len" of char from "ptr" to file id "fd"
   * Return number of char written.
   * Need implementing with UART here. */
  int i;
  for (i = 0; i < len; ++i)
  {
    _ttywrch(ptr[i]);
  }
  return len;
}

int _read (int fd, char *ptr, int len)
{
  /* Read "len" of char to "ptr" from file id "fd"
   * Return number of char read.
   * Need implementing with UART here. */
  return len;
}

void _ttywrch(int ch) {
  /* Write one char "ch" to the default console
   * Need implementing with UART here. */
  USART_putchar(USART_DBG, ch);
}
#else

int fputc(int ch, FILE *f) {
  USART_putchar(USART_DBG, ch);
  return ch;
}
#endif
