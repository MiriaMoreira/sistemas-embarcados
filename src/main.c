#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF); // Módulo de log

#define LED_NODE DT_ALIAS(led0)
#define BUTTON_NODE DT_ALIAS(sw0)
#define PWM_LED_NODE DT_ALIAS(pwm_led0)

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED_NODE, gpios);
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(BUTTON_NODE, gpios);
static const struct pwm_dt_spec pwm_led = PWM_DT_SPEC_GET(PWM_LED_NODE);

void fade_effect(void) {
    uint32_t period = pwm_led.period;
    LOG_INF("Iniciando efeito de fade (PWM)");
    for (uint32_t pulse = 0; pulse < period; pulse += period / 100) {
        pwm_set_pulse_dt(&pwm_led, pulse);
        k_msleep(10);
    }
    for (uint32_t pulse = period; pulse > 0; pulse -= period / 100) {
        pwm_set_pulse_dt(&pwm_led, pulse);
        k_msleep(10);
    }
    LOG_INF("Efeito de fade concluído");
}

void main(void)
{
    if (!device_is_ready(led.port) || !device_is_ready(button.port) || !device_is_ready(pwm_led.dev)) {
        LOG_ERR("Erro: algum dispositivo não está pronto!");
        return;
    }

    gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&button, GPIO_INPUT);

    LOG_INF("Sistema iniciado. Pressione o botão para alternar entre modos.");

    int mode = 0;
    int led_state = 0;

    while (1) {
        if (gpio_pin_get_dt(&button)) {
            mode = !mode;
            LOG_INF("Botão pressionado -> Modo alterado para: %s", mode ? "PWM (fade)" : "Digital");
            k_msleep(300); 
        }

        if (mode == 0) {
            led_state = !led_state;
            gpio_pin_set_dt(&led, led_state);
            LOG_INF("Modo Digital: LED %s", led_state ? "LIGADO" : "DESLIGADO");
            k_msleep(500);
        } else {
            fade_effect();
        }
    }
}
