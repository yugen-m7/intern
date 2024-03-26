#include <stdio.h>
#include <driver/ledc.h>

#define ledc_res LEDC_TIMER_10_BIT // resolution of PWM duty
#define ledc_speed LEDC_LOW_SPEED_MODE
#define ledc_channel LEDC_CHANNEL_0


static int ledc_freq = 100;
static int ledc_dc = 512;

void ledc_init(){
  ledc_timer_config_t timer={
    .freq_hz = ledc_freq,
    .duty_resolution = ledc_res,
    .timer_num = LEDC_TIMER_0,
    .speed_mode = ledc_speed,
    .clk_cfg = LEDC_AUTO_CLK,
  };
  ledc_timer_config(&timer);

  ledc_channel_config_t channel={
    .gpio_num = 13,
    .speed_mode = ledc_speed,
    .timer_sel = LEDC_TIMER_0,
    // .duty = ledc_dc,
    .hpoint = 0,
    .channel = LEDC_CHANNEL_0,
  };
  ledc_channel_config(&channel);
}

void app_main(void)
{
  ledc_init();
  ledc_fade_func_install(0);
  while(1){ 
    ledc_set_fade_time_and_start(ledc_speed, ledc_channel, 0, 1000, LEDC_FADE_WAIT_DONE);
    ledc_set_fade_time_and_start(ledc_speed, ledc_channel, 1024, 1000,LEDC_FADE_WAIT_DONE);
  }
}
