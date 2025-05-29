Design and implement an in-memory task scheduler library that allows scheduling tasks to be executed at specific times or at fixed intervals. The library should provide a simple yet powerful interface for scheduling tasks, ensuring efficient management of resources and adherence to the scheduling constraints.

Functional Requirements
Task Submission with Execution Time:

Allow submitting a task along with a specific execution time.
Method signature: schedule(task, time)
Task Scheduling at Fixed Intervals:

Enable scheduling tasks that should be executed at fixed intervals.
The first execution should be immediate, and subsequent executions should happen at the specified interval after the completion of the previous task execution.
Method signature: scheduleAtFixedInterval(task, interval) where interval is in seconds.
Configurable Worker Threads:

The number of worker threads used for executing tasks should be configurable.
Efficient management of these worker threads is essential to handle the execution of scheduled tasks without overloading the system.
Modularity and Design Patterns:

The design and implementation of the library should be modular, following good design principles and patterns to ensure maintainability and scalability.
No External Scheduling Libraries:

The implementation should not rely on external or internal libraries that provide scheduling functionalities.
Core programming language APIs and constructs should be used to build the scheduler.

For alarm clock also same code can be used. Naming needs to be done as per alarm

### Note
ConsumerWorker is very very difficult for this. 3 while loops are there(1 extra while loop)

### Thought process
- Basically we want TaskScheduler.
- TaskScheduler add the consumerWorkers which do the tasks. So, it has registerConsumer function. Which takes the Consumer object and starts consumerWorkers to consumer this Consumer object
- TaskScheduler has scheduledAfter method which add the recurring tasks which needs to be executed. So, this are the actual task which will be triggered at scheduled time by the Consumer Worker. So, these tasks are added to ConsumerWorker queue.
- Consumer has consume method which does the actual task when it is scheduled
- ConsumerWorkers actual trigger task consume method for the Consumer object
- So, ConsumerWorker is the most important tasks, because:
  - It has list of tasks which needs to scheduled (This will be shared across different ConsumerWorker threads). So, locking on this required (priority queue)
  - Consumer object which actually executes the task.
  - mutex which is shared across different ConsumerWorker threads
  - condition_variable which is shared across different ConsumerWorker threads
- As priority queue, mutex and condition_variable are shared across differnt ConsumerWorkers, they are created in TaskScheduler and whenevern new ConsumerWorker is created, they are passed.