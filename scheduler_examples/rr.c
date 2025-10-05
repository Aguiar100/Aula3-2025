#include "rr.h"
#include <stdlib.h>
#include <stdio.h>
#include "msg.h"
#include <unistd.h>

#define TIME_SLICE_MS 500

void rr_scheduler(uint32_t current_time_ms, queue_t *rq, pcb_t **cpu_task) {
    if (*cpu_task) {
        // Atualiza tempo total e quantum
        (*cpu_task)->ellapsed_time_ms += TICKS_MS;
        (*cpu_task)->quantum_elapsed_ms += TICKS_MS;

        // Se a tarefa terminou
        if ((*cpu_task)->ellapsed_time_ms >= (*cpu_task)->time_ms) {
            msg_t msg = {
                .pid = (*cpu_task)->pid,
                .request = PROCESS_REQUEST_DONE,
                .time_ms = current_time_ms
            };
            if (write((*cpu_task)->sockfd, &msg, sizeof(msg_t)) != sizeof(msg_t)) {
                perror("write");
            }
            free(*cpu_task);
            *cpu_task = NULL;
        }
        // Se o quantum expirou mas a tarefa não terminou
        else if ((*cpu_task)->quantum_elapsed_ms >= TIME_SLICE_MS) {
            (*cpu_task)->quantum_elapsed_ms = 0;
            // Volta para o fim da fila
            if (!enqueue_pcb(rq, *cpu_task)) {
                fprintf(stderr, "Erro ao enfileirar tarefa no RR\n");
                free(*cpu_task);
            }
            *cpu_task = NULL;
        }
    }

    // CPU livre? pega a próxima tarefa da fila
    if (*cpu_task == NULL && rq->head != NULL) {
        *cpu_task = dequeue_pcb(rq);
        if (*cpu_task) {
            (*cpu_task)->quantum_elapsed_ms = 0;
        }
    }
}