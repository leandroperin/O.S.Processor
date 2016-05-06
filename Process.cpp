/*
 * File:   Process.cpp
 * Author: <preencher>
 *
 * Created on August 31, 2015, 7:49 AM
 */

#include "Debug.h"
#include "Process.h"
#include "Thread.h"
#include "System.h"
#include "Simulator.h"

Process::Process(unsigned int parentId) {
    this->_entity = Simulator::getInstance()->getEntity(); // simulation purposes only
    this->_entity->setPointer(this); // simulation purposes only
    
    this->_idInfo._processId = Process::getNewId();
    this->_idInfo._parentId = parentId;
    this->_idInfo._priority = Simulator::generate_uniform_distribution(0, 10);
    // random memory info about the new process
    this->_memInfo._segments = new std::list<MemorySegment*>();
    this->_processSize = 0;
    unsigned int numSegments = Simulator::generate_uniform_distribution(2, 5);
    for (unsigned int i = 0; i < numSegments; i++) {
        unsigned int segmentSize = Simulator::generate_uniform_distribution(250 / (numSegments + 3), 250 / numSegments);
        MemorySegment* segment = new MemorySegment(segmentSize);
        _memInfo._segments->insert(_memInfo._segments->begin(), segment);
        this->_processSize += segmentSize;
    }    
    // INSERT YOUR CODE HERE
    // ...
}
Process::Process(const Process& orig) {
}

Process::~Process() {
}

unsigned int Process::getParentId() const {
    return _idInfo._parentId;
}

unsigned int Process::getId() const {
    return _idInfo._processId;
}

/**
 * Retorna o tamanho do processo
 **/
unsigned int Process::getProcessSize() const {
   return this->_processSize;
}

/**
 * Processos chegarão ao sistema quando for invocada a chamada static Process* Process::exec(), que precisa
 * ser implementada. A implementação dessa chamada de sistema deve criar um Process, alocar memória para ele invocando
 * a chamada de sistema Partition* MemoryManagement::alloc(Process* process), inicializar seus atributos, colocá-lo na lista
 * de processos no sistema e criar uma thread (correspondente a "main") para esse processo invocando a chamada
 * static Thread* Thread::thread_create(Process* parent). O metodo deve retornar o processo criado.
 **/
Process* Process::exec() { /*static*/
    Debug::cout(Debug::Level::trace, "Process::exec()");

    // Cria um novo processo
    Process* newProc = new Process(0);

    // Aloca memória para o processo
    newProc->_memInfo._partition = System::memoryManager()->allocateMemory(newProc->getProcessSize());

    // Insere o processo na lista de processos do sistema
    Process::getProcessesList()->push_back(newProc);

    // Cria a thread "main" do processo
    Thread::thread_create(newProc);

    // Retorna o processo
    return newProc;
}

/**
 * Processos serão finalizados quando for invocada a chamada static void Process::exit(int status = 0), que precisa ser
 * implementada. A implementação dessa chamada deve desalocar a memória do processo invocando
 * void MemoryManagement::desalloc(Process* process), excluir todas as threads desse processo, excluir o processo (destruir
 * o objeto Process), invocar o escalonador para escolher outra thread, invocando Thread* Scheduler::choose() e então
 * o despachador para iniciar a execução da thread escolhida pelo escalonador, invocando
 * void Dispatcher::dispatch(Thread* previous, Thread* next).
 **/
void Process::exit(int status) { /*static*/
    Debug::cout(Debug::Level::trace, "Process::exit(" + std::to_string(status) + ")");

    // Seleciona o processo que está executando
    Process* runningProcess = Thread::running()->getProcess();

    // Desaloca memória do processo
    System::memoryManager()->deallocateMemory(runningProcess->_memInfo._partition);

    // Obtém a lista de threads do sistema
    std::list<Thread*>* threadsList = Thread::getThreadsList();
    std::list<Thread*>::iterator it;

    // Verifica toda a lista de threads do sistema. Sempre que uma thread pertencente ao processo que está sendo finalizado
    // for encontrada, a sua lista contendo threads esperando por sua finalização é limpa, inserindo todas as threads na
    // fila de pronto novamente, e a thread é excluída
    for (it=threadsList->begin(); it!=threadsList->end(); ++it) {
        if ((*it)->getProcess() == runningProcess) {
            Thread::cleanWaitingList((*it));
            delete (*it);
        }   
    }

    // Deleta o processo
    delete runningProcess;

    // O escalonador escolhe uma nova thread para executar e despacha a mesma
    Thread* nextThread = System::scheduler()->choose();
    Thread::dispatch(nullptr, nextThread);
}
