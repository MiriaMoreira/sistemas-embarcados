#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(hello_timer, LOG_LEVEL_DBG);

#define HELLO_INTERVAL K_MSEC(CONFIG_HELLO_INTERVAL)
static struct k_timer hello_timer;

void timer_handler(struct k_timer *timer_id)
{
    static int counter = 0;

    LOG_INF("Hello World! Contador=%d", counter);
    LOG_DBG("Debug: Timer executou.");
    if (counter % 5 == 0) {
        LOG_ERR("Erro simulado a cada 5 mensagens!");
    }

    counter++;
}

int main(void)
{
    LOG_INF("Iniciando Hello World com Timer...");

    k_timer_init(&hello_timer, timer_handler, NULL);
    k_timer_start(&hello_timer, HELLO_INTERVAL, HELLO_INTERVAL);

    while (1) {
        k_sleep(K_FOREVER);
    }

    return 0;
}

