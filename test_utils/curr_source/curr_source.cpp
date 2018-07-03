/* <Brief:>
    int16_t curr_source::source_X (int16_t corriente_pedida): ajusta la salida A de la fuente de corriente
                                                              al valor pedido en nA (+/-1400), devuelve el valor leido
    int16_t curr_source::get_current_source_X (): devuelve el valor leido en nA
    int16_t curr_source::inc_source_X(): incrementa en 1nA la corriente de la fuente especificada y retorna
                                          el valor actual de la corriente en nA
    int16_t curr_source::dec_source_X(): decrementa en 1nA la corriente de la fuente especificada y retorna
                                          el valor actual de la corriente en nA
</Brief>
*/

#include "Arduino.h"
#include "curr_source.h"
#include "Wire.h"
#include <MCP342X.h>
#include <MCP4651.h>
//#include "board_1-V-0_3.h"   // board_1-V-0_3.h es la tabla que contiene la equivalencia corriente -> valores de los potenciometros
//#include "board_2-V-0_3.h"   // elegir aqui la tabla correspondiente a la placa en uso
#include "board_3-V-0_3.h"

MCP4651 _pot;

#define _potW 0x28
#define _potA 0x2A

int16_t curr_source::set_source_A (int16_t corriente_pedida){
  int16_t corr_offset = corriente_pedida + 1400;
  _pot.pot0_set_Value(_potA, current_source_A [corr_offset] [0]);
  _pot.pot1_set_Value(_potA, current_source_A [corr_offset] [1]);

  // TODO: hay que introducir un delay aqui que este relacionado con
  // la corriente anterior, el integrador que ajusta la corriente tarda
  // un tiempo en llegar hasta el valor pedido, aproximadamente la diferencia
  // desde el valor anterior hasta el valor pedido (nA = mS)

  return get_current_A();
}

int16_t curr_source::set_source_W (int16_t corriente_pedida){
  int16_t corr_offset = corriente_pedida + 1400;
  _pot.pot0_set_Value(_potW, current_source_W [corr_offset] [0]);
  _pot.pot1_set_Value(_potW, current_source_W [corr_offset] [1]);

  return get_current_W();
}

int16_t curr_source::get_current_A (){
  uint8_t esto0 = _pot.pot0_read(_potA);
  uint8_t esto1 = _pot.pot1_read(_potA);
  uint16_t i = 0, size = sizeof current_source_A;
  while((i<size) && (current_source_A [i] [0]!= esto0)) i++;
  while((i<size) && (current_source_A [i] [1]!= esto1)) i++;

  return i - 1400;
}

int16_t curr_source::get_current_W (){
  uint8_t esto0 = _pot.pot0_read(_potW);
  uint8_t esto1 = _pot.pot1_read(_potW);
  uint16_t i = 0, size = sizeof current_source_W;
  while((i<size) && (current_source_W [i] [0]!= esto0)) i++;
  while((i<size) && (current_source_W [i] [1]!= esto1)) i++;

  return i - 1400;
}

int16_t curr_source::inc_source_A(){
  return set_source_A(get_current_A() + 1);
}

int16_t curr_source::dec_source_A(){
  return set_source_A(get_current_A() - 1);
}
// FIXME: limitar el desplazamiento maximo a +-1400, sino se piden absurdos al array!

int16_t curr_source::inc_source_W(){
  return set_source_W(get_current_W() + 1);
}

int16_t curr_source::dec_source_W(){
  return set_source_W(get_current_W() - 1);
}
