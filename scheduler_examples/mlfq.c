#include "queue.h"
#include "msg.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define TIME_SLICE_MS 500
#define MLFQ_LEVELS 3

void mlfq_scheduler(uint32_t current_time_ms, queue_t mlfq[MLFQ_LEVELS], pcb_t **cpu_task) {
    // Atualiza a CPU atual
    if (*cpu_task) {
        (*cpu_task)->ellapsed_time_ms += TICKS_MS;
        (*cpu_task)->quantum_elapsed_ms += TICKS_MS;

        // Tarefa terminou
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
        // Quantum expirou
        else if ((*cpu_task)->quantum_elapsed_ms >= TIME_SLICE_MS) {
            (*cpu_task)->quantum_elapsed_ms = 0;
            // Desce de nível se não estiver na última fila
            if ((*cpu_task)->current_queue_level < MLFQ_LEVELS - 1) {
                (*cpu_task)->current_queue_level++;
            }
            // Coloca na fila apropriada
            enqueue_pcb(&mlfq[(*cpu_task)->current_queue_level], *cpu_task);
            *cpu_task = NULL;
        }
    }

    // CPU livre? seleciona o próximo processo da fila mais alta não-vazia
    if (*cpu_task == NULL) {
        for (int i = 0; i < MLFQ_LEVELS; i++) {
            if (mlfq[i].head != NULL) {
                *cpu_task = dequeue_pcb(&mlfq[i]);
                (*cpu_task)->quantum_elapsed_ms = 0;
                break;
            }
        }
    }
}