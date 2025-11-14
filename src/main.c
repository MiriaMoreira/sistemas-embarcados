#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/random/random.h>
#include <string.h>

typedef struct {
    char tipo;      // 'T' = temperatura, 'U' = umidade
    int valor;
} sensor_msg_t;

typedef struct {
    char texto[64];
} log_msg_t;

K_MSGQ_DEFINE(msgq_in,  sizeof(sensor_msg_t), 10, 4);
K_MSGQ_DEFINE(msgq_out, sizeof(sensor_msg_t), 10, 4);
K_MSGQ_DEFINE(logq,     sizeof(log_msg_t),    10, 4);



void enviar_para_log(const char *msg)
{
    log_msg_t lm;
    strncpy(lm.texto, msg, sizeof(lm.texto) - 1);
    lm.texto[sizeof(lm.texto) - 1] = '\0';

    k_msgq_put(&logq, &lm, K_NO_WAIT);
}


void produtor_temp(void)
{
    while (1) {
        sensor_msg_t msg = {
            .tipo = 'T',
            .valor = 18 + (sys_rand32_get() % 20)  // entre 18 e 37
        };

        k_msgq_put(&msgq_in, &msg, K_FOREVER);
        printk("[TEMP] Produzido: %d C\n", msg.valor);
        k_sleep(K_MSEC(1500));
    }
}

void produtor_umid(void)
{
    while (1) {
        sensor_msg_t msg = {
            .tipo = 'U',
            .valor = 30 + (sys_rand32_get() % 50)  // entre 30 e 79
        };

        k_msgq_put(&msgq_in, &msg, K_FOREVER);
        printk("[UMID] Produzido: %d %%\n", msg.valor);
        k_sleep(K_MSEC(1700));
    }
}

void filtro(void)
{
    sensor_msg_t entrada;

    while (1) {
        k_msgq_get(&msgq_in, &entrada, K_FOREVER);

        bool valido = false;

        if (entrada.tipo == 'T') {
            valido = (entrada.valor >= 18 && entrada.valor <= 30);
        } else if (entrada.tipo == 'U') {
            valido = (entrada.valor >= 40 && entrada.valor <= 70);
        }

        if (valido) {
            k_msgq_put(&msgq_out, &entrada, K_FOREVER);
        } else {
            char log_msg[64];
            snprintf(log_msg, sizeof(log_msg),
                     "Dado invalido (%c=%d)", entrada.tipo, entrada.valor);
            enviar_para_log(log_msg);
        }
    }
}

void consumidor(void)
{
    sensor_msg_t msg;

    while (1) {
        k_msgq_get(&msgq_out, &msg, K_FOREVER);

        if (msg.tipo == 'T') {
            printk("[CONS] Temperatura: %d C\n", msg.valor);
        } else {
            printk("[CONS] Umidade: %d %%\n", msg.valor);
        }
    }
}

void thread_log(void)
{
    log_msg_t lm;

    while (1) {
        k_msgq_get(&logq, &lm, K_FOREVER);
        printk("[LOG] %s\n", lm.texto);
    }
}

K_THREAD_DEFINE(th_temp,   1024, produtor_temp,  NULL, NULL, NULL, 3, 0, 0);
K_THREAD_DEFINE(th_umid,   1024, produtor_umid,  NULL, NULL, NULL, 3, 0, 0);
K_THREAD_DEFINE(th_filtro, 1024, filtro,         NULL, NULL, NULL, 2, 0, 0);
K_THREAD_DEFINE(th_cons,   1024, consumidor,     NULL, NULL, NULL, 1, 0, 0);
K_THREAD_DEFINE(th_log,    1024, thread_log,     NULL, NULL, NULL, 4, 0, 0);
