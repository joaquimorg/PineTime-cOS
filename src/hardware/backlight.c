#include "backlight.h"
#include "sys.h"

void backlight_init(void){

   nrf_gpio_pin_set(LCD_LIGHT_1);
   nrf_gpio_pin_set(LCD_LIGHT_2);
   nrf_gpio_pin_set(LCD_LIGHT_3);

   nrf_gpio_cfg_output(LCD_LIGHT_1);
   nrf_gpio_cfg_output(LCD_LIGHT_2);
   nrf_gpio_cfg_output(LCD_LIGHT_3);
   
}

void set_backlight_level(uint8_t level) {
      
   switch (level) {
      case 0:
         nrf_gpio_pin_set(LCD_LIGHT_1);
         nrf_gpio_pin_set(LCD_LIGHT_2);
         nrf_gpio_pin_set(LCD_LIGHT_3);
      break;
      case 1:
         nrf_gpio_pin_clear(LCD_LIGHT_1);
         nrf_gpio_pin_set(LCD_LIGHT_2);
         nrf_gpio_pin_set(LCD_LIGHT_3);
         pinetimecos.backlightLevel = 1;
      break;
      case 2:
         nrf_gpio_pin_clear(LCD_LIGHT_1);
         nrf_gpio_pin_clear(LCD_LIGHT_2);
         nrf_gpio_pin_set(LCD_LIGHT_3);
         pinetimecos.backlightLevel = 2;
      break;
      case 3:
         nrf_gpio_pin_clear(LCD_LIGHT_1);
         nrf_gpio_pin_clear(LCD_LIGHT_2);
         nrf_gpio_pin_clear(LCD_LIGHT_3);
         pinetimecos.backlightLevel = 3;
      break;
      default:
      break;
   }
   
}


char * get_backlight_icon(uint8_t level) {
   switch (level) {
      case 1:
         return "\xEE\xA4\x85";
      break;
      case 2:
         return "\xEE\xA4\x86";
      break;
      case 3:
         return "\xEE\xA4\x84";
      break;
      default:
         return "\xEE\xA4\x85";
      break;
   }
}