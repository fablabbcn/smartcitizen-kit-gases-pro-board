/* <Brief:>
          library para el manejo del potenciometro digital MCPP4651
          todas las funciones necesitan al menos la direccion, en 7bits y si A0=A1=A2=0 la direccion es 0x28
          el potenciometro tiene 8 bits para el recorrido del wiper y puede llegar a los extremos, para llegar
          a los extremos hay que escribir el siguiente bit (0x100 y 0x000)
          para set_Value el valor transmitido actualmente es de un byte, es decir no llega a los extemos, pero
          se puede llegar con inc o dec
          el valor retornado desde potX_read es tambien un byte
</Brief>
*/

#include <Arduino.h>
#include <Wire.h>

class MCP4651
{
  public:

    void pot0_set_Value (int pot_address, int value);
    void pot1_set_Value (int pot_address, int value);
    void pot0_inc (int pot_address);
    void pot1_inc (int pot_address);
    void pot0_dec (int pot_address);
    void pot1_dec (int pot_address);
    int pot0_read (int pot_address);
    int pot1_read (int pot_address);

  private:

};
