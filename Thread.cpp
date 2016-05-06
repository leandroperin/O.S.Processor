/*
 * File:   Thread.cpp
 * Author: <preencher>
 *
 * Created on September 27, 2015, 10:30 AM
 */

#include "Thread.h"
#include "Debug.h"
#include "System.h"
#include "Simulator.h"
#include "CPU.h"

Thread* Thread::_running;

Thread::Thread(Process* task, Thread::State state) {
    this->_id = Thread::getNewId();
    this->_process = task;
    this->_state = state;
    // INSERT YOUR CODE HERE
    // ...
}

Thread::Thread(const Thread& orig) {
}

Thread::~Thread() {
    delete this->_queue;
}

Thread* Thread::running() {
    return _running;
}

Process* Thread::getProcess() {
    return _process;
}

int Thread::getPriority() const {
    return _accountInfo._priority;
}

void Thread::sleep(Queue<Thread*>* q) {
    Debug::cout(Debug::Level::trace, "Thread::sleep(" + std::to_string(reinterpret_cast<unsigned long> (q)) + ")");
    // INSERT YOUR CODE HERE
    // ...
}

void Thread::wakeup(Queue<Thread*>* q) {
    Debug::cout(Debug::Level::trace, "Thread::wakeup(" + std::to_string(reinterpret_cast<unsigned long> (q)) + ")");
    // INSERT YOUR CODE HERE
    // ...
}

/**
 * Threads podem decidir deixar a CPU invocando a chamada void Thread::yield(), que precisa ser implementada. A implementação
 * dessa chamada deve colocar a thread que está executando no estado READY, incluí-la na fila de threads prontas, chamar
 * o escalonador para escolher outra thread, invocando Thread* Scheduler::choose() e então o despachador para iniciar
 * a execução da thread escolhida pelo escalonador, invocando static void Thread::dispatch(Thread* previous, Thread* next).
 **/
void Thread::yield() {
    Debug::cout(Debug::Level::trace, "Thread::yield()");

    // Seleciona a thread em execução
    Thread* runningThread = Thread::running();

    // Altera o estado da thread para PRONTO e a insere na fila de prontos
    runningThread->_state = State::READY;
    System::scheduler()->insert(runningThread);

    // Escalonador seleciona uma nova thread para execução e a despacha
    Thread* newThread = System::scheduler()->choose();
    Thread::dispatch(runningThread, newThread);
}

/**
 * Threads aguardarão pelo término de outras threads quando for invocada a chamada void Thread::join(), que precisa ser 
 * implementada. A implementação dessa chamada deve verificar se a thread this passada como parâmetro está no estado FINISHING 
 * ou não. Se estiver, então a thread que invocou a chamada (a que está executando --running--) simplesmente continua sua execução. 
 * Se não estiver, então a thread executando deve ser colocada no estado WAITING e colocada na fila de threads esperando pela 
 * thread this. Ainda nesse caso, deve-se chamar o escalonador para escolher outra thread, invocando Thread* Scheduler::choose() 
 * e então o despachador para iniciar a execução da thread escolhida pelo escalonador, invocando 
 * static void Thread::dispatch(Thread* previous, Thread* next). Para melhor ilustrar essa chamada, se o código da thread T1 
 * tiver a chamada T2->join(), então T1 é quem está executando (running) e T2 é quem foi invocada (this), e é T1 que deve ser 
 * bloqueada esperando por T2 (se T2 não estiver FINISHING).
 **/
int Thread::join() {
    Debug::cout(Debug::Level::trace, "Thread::join()");

    if (this->_state != State::FINISHING) {
        // Seleciona a thread atualmente em execução
        Thread* runningThread = Thread::running();
        
        // Verifica se a thread tenta dar join nela mesma
        // Não sabia se devia por isso no código, portanto deixei comentado
        /*if (runningThread == this) {
            return -1;
        }*/
        
        // Altera o estado da thread invocadora para WAITING e a insere na lista de "threads em espera" da thread invocada
        runningThread->_state = State::WAITING;
        this->_queue->push_back(runningThread);
        
        // Escalonador seleciona uma nova thread para executar e despacha a mesma
        Thread* nextThread = System::scheduler()->choose();
        Thread::dispatch(runningThread, nextThread);
    }
    
    // Se a thread invocada estiver no estado FINISHING então não é necessário esperar ela terminar, portanto retorna 0
    return 0;
}

/**
 * Threads serão finalizadas quando for invocada a chamada void Thread::exit(), que precisa ser implementada. A implementação
 * dessa chamada deve colocar a thread que está executando no estado FINISHED, verificar se há alguma thread na lista
 * de threads bloqueadas esperando por essa thread. Se houver, todas as threads na lista devem ser colocadas no estado
 * READY e colocadas na fila de threads pronas para executar. Em qualquer caso, deve-se ainda chamar o escalonador para
 * escolher outra thread, invocando Thread* Scheduler::choose() e então o despachador para iniciar a execução da thread
 * escolhida pelo escalonador, invocando static void Thread::dispatch(Thread* previous, Thread* next).
 **/
void Thread::exit(int status) {
    Debug::cout(Debug::Level::trace, "Thread::exit(" + std::to_string(status) + ")");

    // Seleciona a thread atualmente em execução, que vai ser finalizada
    Thread* finishingThread = Thread::running();
    
    // Altera o estado da thread para FINISHING
    finishingThread->_state = State::FINISHING;

    // A lista de threads que esperam pela finalização da finishingThread é limpa, inserindo todas as threads na
    // fila de pronto novamente
    Thread::cleanWaitingList(finishingThread);
    
    // Escalonador seleciona uma nova thread para executar e despacha a mesma
    Thread* newThread = System::scheduler()->choose();
    Thread::dispatch(finishingThread, newThread);
}


/**
 * Função destinada a limpar a waitingList de uma thread. 
 * waitingList é a lista, a qual toda thread possui, que armazena todas as threads que executaram join() em uma determinada thread
 **/
void Thread::cleanWaitingList(Thread* t) {
    // Enquanto a lista de threads em espera não ficar vazia, a primeira thread desta lista tem seu estado alterado para PRONTO,
    // é inserida na fila de prontos e depois removida da waitingList
    while (!t->_queue->empty()) {
        t->_queue->front()->_state = State::READY;
        System::scheduler()->insert(t->_queue->front());
        t->_queue->pop_front();
    }
}

/**
 * Threads são despachadas, ou seja, têm seus contextos trocados, quando se invoca a chamada 
 * static void Thread::dispatch(Thread* previous, Thread* next), que precisa ser implementada. A implementaçao desse  método 
 * deve inicialmente verificar se a próxima thread (next) é nula ou não. Se for, nada precisa ser feito (isso só ocorre quando 
 * a fila de prontos é vazia e não há thread para ser escalonada). Se não for, então deve ser verificado se a thread anterior 
 * (previous) é diferente de nula e também se é diferente da próxma thread. Se não for, então basta restaurar o contexto da 
 * próxima thread, invocando static void CPU::restore_context(Thread* next). Se for, então é preciso verificar se a thread 
 * anterior estava no estado RUNNING e caso sim, então a thread anterior deve passar para o estado READY. Após esse teste 
 * deve-se fazer a troca de contexto entre as threads, invocando o método static void CPU::switch_context(Thread* previous, Thread* next).
 * **/
void Thread::dispatch(Thread* previous, Thread* next) {
    Debug::cout(Debug::Level::trace, "Thread::dispatch(" + std::to_string(reinterpret_cast<unsigned long> (previous)) + "," + std::to_string(reinterpret_cast<unsigned long> (next)) + ")");

    // Seta a thread que entrará em execução
    _running = next;
    
    // Procede com o despachamento caso tenha alguma thread para colocar em execução
    if (next != nullptr) {
        // Altera o estado da thread next para RUNNING e a remove da lista de prontos
        next->_state = State::RUNNING;
        System::scheduler()->remove(next);
        
        // Fará a troca de contexto caso a thread a ser executada seja diferente da thread em execução
        if ((previous != nullptr) && (previous != next)) {
            // Seleciona o tempo atual da simulação
            int currentTime = Simulator::getInstance()->getTnow();
            
            // Seta o tempo de chegada da thread next na CPU
            next->_accountInfo._arrivalTime = currentTime;
            
            // Seta o tempo em que a thread previous permaneceu na CPU
            previous->_accountInfo._cpuTime = currentTime - previous->_accountInfo._arrivalTime;
            
            // Faz uma nova previsão de rajada de CPU para a thread previous, calculando a média aritmética entre a previsão anterior
            // e o tempo que a thread permaneceu na CPU
            previous->_accountInfo._priority = (int) ((previous->_accountInfo._priority + previous->_accountInfo._cpuTime) / 2);
            
            // Se a thread anteriormente em execução estiver no estado RUNNING, então tem seu estado alterado para READY e é inserida
            // na fila de prontos
            if (previous->_state == State::RUNNING) {
                previous->_state = State::READY;
                System::scheduler()->insert(previous);
            }
            
            // Efetua a troca de contexto entre as threads
            CPU::switch_context(previous, next);
        } else {
            // Caso a thread escolhida pelo escalonador ja esteja em execução ou, caso a CPU não esteja com nenhuma thread rodando,
            // então o contexto é apenas restaurado para a thread next
            CPU::restore_context(next);
        }
    }    
    
}

/**
 * Threads criarão novas threads quando for invocada a chamada static Thread* Thread::thread_create(), que precisa ser
 * implementada. A implementação dessa chamada deve criar um objeto Thread, inicializar seus novos atributos, colocá-la na
 * lista de threads no sistema, inseri-la na fila de prontos inovando o método void Scheduler::insert(Thread* thread), 
 * e se o escolador for preemptivo, então deve chamar o escalonador para escolher outra thread, invocando 
 * Thread* Scheduler::choose() e então o despachador para iniciar a execução da thread escolhida pelo escalonador, 
 * invocando static void Thread::dispatch(Thread* previous, Thread* next).
 **/
Thread* Thread::thread_create(Process* parent) {
    Debug::cout(Debug::Level::trace, "Thread::create(" + std::to_string(reinterpret_cast<unsigned long> (parent)) + ")");

    // Cria uma nova thread, passando como parâmetro o processo PAI e setando o estado para PRONTO
    Thread* newThread = new Thread(parent, State::READY);
    
    // Cria a lista de threads em espera
    newThread->_queue = new std::list<Thread*>();
    
    // Coloca a previsão inicial de rajada de CPU da thread no atributo PRIORITY e o tempo atual no atributo ARRIVALTIME
    newThread->_accountInfo._priority = 10;
    newThread->_accountInfo._arrivalTime = Simulator::getInstance()->getTnow();

    // Insere a thread na lista de threads do sistema
    Thread::getThreadsList()->push_back(newThread);
    
    // Insere a thread na fila de prontos
    System::scheduler()->insert(newThread);
    
    // Seleciona a thread atualmente em execução
    Thread* runningThread = Thread::running();
    
    // Se o escalonador for preemptivo, uma nova thread é escolhida da fila de prontos e despachada para execução
    if (System::scheduler()->preemptive) {
        // Se a thread criada tiver prioridade maior do que a thread em execução, ou caso não há nenhuma thread em execução,
        // então o escalonador é invocado para selecionar e despachar uma nova thread da fila de prontos
        if (runningThread != nullptr) {
            if (newThread->_accountInfo._priority < runningThread->_accountInfo._priority) {
                Thread* nextThread = System::scheduler()->choose();
                Thread::dispatch(runningThread, nextThread);
            }
        } else {
            Thread* nextThread = System::scheduler()->choose();
            Thread::dispatch(runningThread, nextThread);
        }
    }
     
    // Retorna a thread criada
    return newThread;
}