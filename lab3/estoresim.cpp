#include <cstring>
#include <cstdlib>
#include <stdio.h>

#include "EStore.h"
#include "TaskQueue.h"

class Simulation
{
    public:
    TaskQueue supplierTasks;
    TaskQueue customerTasks;
    EStore store;

    int maxTasks;
    int numSuppliers;
    int numCustomers;

    explicit Simulation(bool useFineMode) : store(useFineMode) { }
};

/*
 * ------------------------------------------------------------------
 * supplierGenerator --
 *
 *      The supplier generator thread. The argument is a pointer to
 *      the shared Simulation object.
 *
 *      Enqueue arg->maxTasks requests to the supplier queue, then
 *      stop all supplier threads by enqueuing arg->numSuppliers
 *      stop requests.
 *
 *      Use a SupplierRequestGenerator to generate and enqueue
 *      requests.
 *
 *      This thread should exit when done.
 *
 * Results:
 *      Does not return. Exit instead.
 *
 * ------------------------------------------------------------------
 */
static void*
supplierGenerator(void* arg)
{
    // TODO: Your code here.
    printf("HELLO FROM SUPPLIER GENERATOR\n");
    return NULL; // Keep compiler happy.
}

/*
 * ------------------------------------------------------------------
 * customerGenerator --
 *
 *      The customer generator thread. The argument is a pointer to
 *      the shared Simulation object.
 *
 *      Enqueue arg->maxTasks requests to the customer queue, then
 *      stop all customer threads by enqueuing arg->numCustomers
 *      stop requests.
 *
 *      Use a CustomerRequestGenerator to generate and enqueue
 *      requests.  For the fineMode argument to the constructor
 *      of CustomerRequestGenerator, use the output of
 *      store.fineModeEnabled() method, where store is a field
 *      in the Simulation class.
 *
 *      This thread should exit when done.
 *
 * Results:
 *      Does not return. Exit instead.
 *
 * ------------------------------------------------------------------
 */
static void*
customerGenerator(void* arg)
{
    // TODO: Your code here.
    printf("HELLO FROM CUSTOMER GENERATOR\n");
    return NULL; // Keep compiler happy.
}

/*
 * ------------------------------------------------------------------
 * supplier --
 *
 *      The main supplier thread. The argument is a pointer to the
 *      shared Simulation object.
 *
 *      Dequeue Tasks from the supplier queue and execute them.
 *
 * Results:
 *      Does not return.
 *
 * ------------------------------------------------------------------
 */
static void*
supplier(void* arg)
{
    // TODO: Your code here.
    printf("HELLO FROM SUPPLIER\n");
    return NULL; // Keep compiler happy.
}

/*
 * ------------------------------------------------------------------
 * customer --
 *
 *      The main customer thread. The argument is a pointer to the
 *      shared Simulation object.
 *
 *      Dequeue Tasks from the customer queue and execute them.
 *
 * Results:
 *      Does not return.
 *
 * ------------------------------------------------------------------
 */
static void*
customer(void* arg)
{
    // TODO: Your code here.
    printf("HELLO FROM CUSTOMER\n");
    return NULL; // Keep compiler happy.
}

/*
 * ------------------------------------------------------------------
 * startSimulation --
 *      Create a new Simulation object. This object will serve as
 *      the shared state for the simulation. 
 *
 *      Create the following threads:
 *          - 1 supplier generator thread.
 *          - 1 customer generator thread.
 *          - numSuppliers supplier threads.
 *          - numCustomers customer threads.
 *
 *      After creating the worker threads, the main thread
 *      should wait until all of them exit, at which point it
 *      should return.
 *
 *      Hint: Use sthread_join.
 *
 * Results:
 *      None.
 *
 * ------------------------------------------------------------------
 */
static void
startSimulation(int numSuppliers, int numCustomers, int maxTasks, bool useFineMode)
{
    // Create a new Simlation object and setup
    Simulation* s = new Simulation(useFineMode);
    s->numSuppliers = numSuppliers;
    s->numCustomers = numCustomers;
    s->supplierTasks = TaskQueue();
    s->customerTasks = TaskQueue();
    s->maxTasks = maxTasks;

    // Declares thread structs
    sthread_t supplier_genertator_thread, customer_genertator_thread;
    sthread_t *supplier_threads, *customer_threads;
    supplier_threads = (sthread_t*) malloc(sizeof(sthread_t) * numSuppliers);
    customer_threads = (sthread_t*) malloc(sizeof(sthread_t) * numCustomers);

    // Creare threads
    sthread_create(&supplier_genertator_thread, supplierGenerator, NULL);  // Maybe changeme
    sthread_create(&customer_genertator_thread, customerGenerator, NULL);

    for(int i = 0; i < numSuppliers; i++){
        sthread_create(&supplier_threads[i], supplier, NULL);
    }
    for(int i = 0; i < numCustomers; i++){
        sthread_create(&customer_threads[i], customer, NULL);
    }

    // Join threads
    sthread_join(supplier_genertator_thread);
    sthread_join(customer_genertator_thread);

    for(int i = 0; i < numSuppliers; i++){
        sthread_join(supplier_threads[i]);
    }
    for(int i = 0; i < numCustomers; i++){
        sthread_join(customer_threads[i]);
    }

    // Memory cleanup
    free(supplier_threads);
    free(customer_threads);
    delete s;
}

int main(int argc, char **argv)
{
    bool useFineMode = false;

    // Seed the random number generator.
    // You can remove this line or set it to some constant to get deterministic
    // results, but make sure you put it back before turning in.
    srand(time(NULL));

    if (argc > 1)
        useFineMode = strcmp(argv[1], "--fine") == 0;
    startSimulation(10, 10, 100, useFineMode);
    return 0;
}

