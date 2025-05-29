Design and implement an in-memory, multi-threaded queue system similar to Amazon Simple Queue Service (SQS) that allows for asynchronous message processing. The system should provide a reliable way to transfer messages between different parts of an application, ensuring that each message is processed by a consumer. The queue should support multiple producers and consumers operating concurrently.

Example: AdditionWorker - 5 seconds

Message: 2 numbers.



sqs -> 5 consumers -> only one of them will process the message

kafka -> topic -> 5 consumers of type1 -> one of these 5 consumers should get the message  
4 consumers of type 2: one of these 4 consumers should get the message

How SQS works (at a high level):
    Producer sends a message to the queue.
    SQS stores the message securely.
    Consumer pulls the message from the queue.
    After successful processing, the consumer deletes the message from the queue.

### Thought process
- For any problem HLD or LLD or concurrency or any problem (even in trading, life etc.) can start from small and then expand
- First think of how single consumer of type 1 can consume one message (SQS)
  - SQS - publish, registration (registration of new Consumer and starts thread for it. Maybe can implement ConsumerWorker similar to TaskScheduler)
    - ConsumerWorker can do(It will be created for all the consumers. So, threads of ConsumerWorkers are created which is equalt to number of consumers) - lock handling, state handling and retrive the message and then call consumer.consume. (So, it acts as middleman). Right now this is done in SQS class itself
  - Consumer - process message
- Then think of how multiple consumer of type 1 can consume one message (SQS)
- SQS has queue to store the messages, mutex and condition_variable for concurrency handling.
  - This can be passed to ConsumerWorker and ConsumerWorker uses it for processing and call consumer.consume if and message is found