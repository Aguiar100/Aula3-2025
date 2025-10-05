#include "sjf.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "msg.h"
#include "queue.h"

void sjf_scheduler(uint32_t current_time_ms, queue_t *rq, pcb_t **cpu_task) {
    // Se já há tarefa a correr
    if (*cpu_task) {
        (*cpu_task)->ellapsed_time_ms += TICKS_MS;
        if ((*cpu_task)->ellapsed_time_ms >= (*cpu_task)->time_ms) {
            // terminou
            msg_t msg = {
                .pid = (*cpu_task)->pid,
                .request = PROCESS_REQUEST_DONE,
                .time_ms = current_time_ms
            };
            write((*cpu_task)->sockfd, &msg, sizeof(msg_t));
            free(*cpu_task);
            *cpu_task = NULL;
        }
        return;
    }

    if (!rq || !rq->head) return;

    queue_elem_t *it = rq->head, *prev = NULL;
    queue_elem_t *shortest = it, *prev_shortest = NULL;

    uint32_t best_rem = shortest->pcb->time_ms - shortest->pcb->ellapsed_time_ms;

    while (it) {
        uint32_t rem = it->pcb->time_ms - it->pcb->ellapsed_time_ms;
        if (rem < best_rem) {
            best_rem = rem;
            shortest = it;
            prev_shortest = prev;
        }
        prev = it;
        it = it->next;
    }

    if (prev_shortest) prev_shortest->next = shortest->next;
    else rq->head = shortest->next;
    if (rq->tail == shortest) rq->tail = prev_shortest;

    *cpu_task = shortest->pcb;
    free(shortest);
}