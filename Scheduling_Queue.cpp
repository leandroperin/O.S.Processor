/*
 * File:   Scheduling_Queue.cpp
 * Author: cancian
 *
 * Created on October 9, 2015, 9:34 PM
 */

#include "Scheduling_Queue.h"

Scheduling_Queue::Scheduling_Queue() {
    // CHANGE AS NECESSARY IF YOU CHOOSE ANOTHER QUEUE
    _queue = new std::list<Thread*>();
}

Scheduling_Queue::Scheduling_Queue(const Scheduling_Queue& orig) {
}

Scheduling_Queue::~Scheduling_Queue() {
}

bool Scheduling_Queue::empty() {
    // CHANGE AS NECESSARY IF YOU CHOOSE ANOTHER QUEUE
    return _queue->empty();
}

void Scheduling_Queue::pop() {
    // CHANGE AS NECESSARY IF YOU CHOOSE ANOTHER QUEUE
    if (!_queue->empty()) {
        _queue->pop_front();
    }
}

void Scheduling_Queue::remove(Thread* t) {
    _queue->remove(t);
}

Thread* Scheduling_Queue::top() {
    // CHANGE AS NECESSARY IF YOU CHOOSE ANOTHER QUEUE
    return _queue->front();
}

void Scheduling_Queue::insert(Thread* t) {
    // Variável booleana que indica se a thread foi inserida na fila ou não
    bool success = false;

    // Percorre toda a fila até achar alguma thread com prioridade maior, ao encontrar então a thread
    // é inserida na posição desta e a variável booleana "success" é setada como TRUE
    if (!_queue->empty()) {
        int threadPriority = t->getPriority();
        for (it=_queue->begin(); it!=_queue->end(); ++it) {
            if ((*it)->getPriority() > threadPriority) {
                _queue->insert(it, t);
                success = true;
                break;
            }
        }
    }
    
    // Se a thread não achou nenhuma outra com prioridade maior, então ela é inserida no fim da fila
    if (!success) {
        _queue->push_back(t);
    }
}
