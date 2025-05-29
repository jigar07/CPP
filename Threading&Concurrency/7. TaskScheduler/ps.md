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