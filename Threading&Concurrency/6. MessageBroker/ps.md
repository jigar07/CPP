Design message broker which supports multiple subscribers on a topic.

Example: 
AdditionSubscriber - 4 seconds - 1 consumer
MinusSubscriber - 2 seconds - 2 consumers

Message: 2 numbers.

kafka -> topic -> 5 consumers of type1 -> one of these 5 consumers should get the message  
4 consumers of type 2: one of these 4 consumers hsould get the message

Extension:
Offset reset.

{1, 2} {2, 3} {4, 5}

### Thought process
- For any problem HLD or LLD or concurrency or any problem (even in trading, life etc.) can start from small and then expand
- First think of how single consumer of type 1 can consume one message (SQS)
  - SQS - publish, registration
  - ConsumerWorker - lock handling, state handling and retrive the message and then call consumer.consume. (So, it acts as middlement)
  - Consumer - process message
- Then think of how multiple consumer of type 1 can consume one message (SQS)
- Then think of how multiple consumer of type 1, type 2 and type 3(all three) can consume same message (map<type, SQS>)