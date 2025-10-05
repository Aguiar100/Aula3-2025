#include "sjf.h"
#include <stdlib.h>
#include <stdio.h>
#include "msg.h"
#include <unistd.h>

void sjf_scheduler(uint32_t current_time_ms, queue_t *rq, pcb_t **cpu_task) {
    if (*cpu_task) {
        (*cpu_task)->ellapsed_time_ms += TICKS_MS;      // Add to the running time of the application/task
        if ((*cpu_task)->ellapsed_time_ms >= (*cpu_task)->time_ms) {
            // Task finished
            // Send msg to application
            msg_t msg = {
                .pid = (*cpu_task)->pid,
                .request = PROCESS_REQUEST_DONE,
                .time_ms = current_time_ms
            };
            if (write((*cpu_task)->sockfd, &msg, sizeof(msg_t)) != sizeof(msg_t)) {
                perror("write");
            }
            free((*cpu_task));
            (*cpu_task) = NULL;
        }
    }
    if (*cpu_task == NULL && rq->head != NULL) {
        queue_elem_t *it = rq->head;
        queue_elem_t *shortest_elem = it;
        pcb_t *shortest = it->pcb;

        while (it != NULL) {
            if (it->pcb->time_ms < shortest->time_ms) {
                shortest = it->pcb;
                shortest_elem = it;
            }
            it = it->next;
        }

        // Remove o processo mais curto da fila
        queue_elem_t *removed = remove_queue_elem(rq, shortest_elem);
        if (removed) {
            *cpu_task = removed->pcb;
            free(removed); // liberta nó da fila, n o processo
        }
    }
}