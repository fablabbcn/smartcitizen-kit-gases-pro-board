#include <Arduino.h>
class curr_source
{
  public:

    int16_t set_source_A (int16_t corriente_pedida);
    int16_t set_source_W (int16_t corriente_pedida);
    int16_t get_current_A ();
    int16_t get_current_W ();
    int16_t inc_source_A ();
    int16_t dec_source_A ();
    int16_t inc_source_W ();
    int16_t dec_source_W ();
  private:

};
