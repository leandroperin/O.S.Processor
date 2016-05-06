/*
 * File:   TimerMediator.cpp
 * Author: cancian
 *
 * Created on October 9, 2015, 2:55 PM
 */

#include "TimerMediator.h"
#include "Simulator.h"
#include "Debug.h"
#include "System.h"
#include "Traits.h"
#include "Thread.h"

/**
 * O sistema simulado possui ainda um timer, que gera interrupções periodicamente. O timer foi inicialmente configurado
 * para gerar interrupções a cada 100 unidades de tempo (ver arquivo Traits.h). Sempre que uma interrupção do timer ocorre, automaticamente é
 * invocado o método static void TimerMediator::interrupt_handler(), que precisa ser implementado. Esse método trata uma
 * interrupção do timer, e sua implementação depende do algoritmo de escalonamento. Ele pode ser usado para preemptar
 * uma thread caso seu time slice tenha sido atingido, no caso do algoritmo round-robin, por exemplo.
 */
void TimerMediator::interrupt_handler() {
    Debug::cout(Debug::Level::trace, "TimerMediator::interrupt_handler()");

    if (System::scheduler()->preemptive) {
        // Escalonador escolhe uma nova thread para executar e então despacha a mesma
        Thread* previousThread = Thread::running();
        Thread* nextThread = System::scheduler()->choose();
        Thread::dispatch(previousThread, nextThread);
    }
}
