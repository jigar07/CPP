# Requirements
Customers can log a complaint against any unsuccessful transaction.
Customer Service Agents can search for customer issues with issue ID or customer details (email).
Agents can view their assigned issues and mark them resolved once they are resolved.
System should assign the issue to agents based on an assigning strategy.
System should allow the admin to onboard new agents.
System should allow the admin to view the agent's work history.

Interaction/Runner layer  - API/CommandLine/Unit Tests - A layer through which your client/user or whoever is sort of using it, is interacting with it.
    - IntModel(Input model) - Request Response Model - DTOs
Transformer - IntModel -> BusModel, BusModel -> IntModel
Business Layer(Service) - Core part of the system which is implementing the business functionality.
    - BusModel (Business model)
Transformer - BusModel -> DBModel, DBModel -> BusModel
Persistence Layer - DB, Redis, In memory
    - DBModel - DAOs


- DTO - data transfer object
- DAO - data access object


- Following asked one of the mentee of Udit:
  - You give ticket id and ask to assign that ticket to a agent.
  - You create ticket and keep open tickets in some queue, and then agent asks for assignment, and you pick one ticket and give to the agent.
  - `You` means system
- Following asked to Udi
  - There is no physical agents, auto resolutions.

/api/v1/admin

# Thought Process
- Classes
- Properties of classes
- Top-Down method:
  - Ticket service
    - createTicket - create ticket and store in ticketRepository
    - assignTicket
      - Most important part of problem - **Finding right agent for the ticket**
      - **Like Food ordering system(filtering and sorting based on selection criteria) and Ride sharing (filter and sort ride), we will get all the agents and then filter application agents**
        - **IPreferenceStrategy** to filter preference
        - So, one strategy to filter the agents and another strategy to sort the agents
        - This is very common pattern might be used at different places. In parking lot also it can be used
        - `orCondtion`, `andCondition` can also be used (Just like amazon filter)
      - `getAllAgents` added in `AgentService`
        - `isAvailable` added in `Agent` (Would be removed from here later)
    - resolveTicket - resolve ticket and update ticketRepository
  - IPostTicketResolutionSubscriber added for notification
  - PostTicketResolutionPubSub to notify all IPostTicketResolutionSubscriber

# Notes
- Unless given in the problem, start with business layer (If given in problem or atlassian then don't forget to add interaction/runner layer)
- Every layer (Interaction, Business, Persistence layer) has own model
  - We don't write all the models every time as we have to write transform layer if any model is added
  - So, if model are same then write it (Even in company/ production it is also to skip if models are similar)
  - Usually IntModels and BusModel are kept as single
  - We usually do not write DBModel as it is DB schema and we need to think about normalization and all for DB schema. Whatever models we have written are mostly business model only
  - Most of the system Udit has seen have single model between IntModel and BusModel
  - In amazon it is suggested to keep separate model for Interaction, Business and Persistence layer
  - DB layer should be separate from Interaction/Business
- Repository do conversion, DB connection handling, DAO conversion
  - If conversion become too big then add conversion/transformer class in repository
- Scoping out requirement with interviewer is very important. Check whether interviewer wants breadth or depth
  - Even Udit faced similar issue (Atlassian example)
  - Keeping interviewer aligned with requirement is very important
- Important part of strategy is, it is easy to tell strategy needs to be used. But difficult to implement.
- Your code will not be perfect from day 1. You might need to do refactoring in future
  - If requirement drastically change then have to rewrite code
- **ORM**
  - Hibernate is ORM
  - ORM is used to write queries for DB
  - But ORM makes system very slow if a lot of join. For every relation it calls DB different times. So, it will increase latency
  - As your system becomes complex, ORM takes time to execute queries. This time we might need to write own queries instead of using ORM
  - That's why Udit does not suggest keeping relation in same DB
  - Putting relations make system performance heavy. If we use lazy loading(of ORM) to solve this then also it will not be helpful as sometimes we might require all the relations(lazy loading means fetching relation only if it is required)
  - So, that's why custom queries will be useful instead of Hibernate. Hibernate has projection. So, can use projection to write custom queries using Hibernate
  - Udit hasn't ORM. 90% time there will be complex queries so better not to use ORM. Can use ORM if simple queries
- Whatever comes to your mind call out to interviewer
- For interview(machine coding round), stick to core requirement. Extended requirement can just be call out. But don't try to implement it at start
# Design
## Classes
- Model: Customer, Ticket, Agent, Admin, TicketDomain, TicketStatus, AgentStatus, Priority
  - Priority is not mentioned in ticket
## Properties
- Agent:
  - id, name, List<TicketDomain>, isOOO, email
- Ticket: Most interesting class in our system
  - id, details, status, List<String> linkedReferenceIds, TicketDomain, TicketStatus
  - Priority, createdAt, resolvedAt, slaDate (These properties are increasing requirement. Marked as comment then will use it if time)
## Top-down method
- Service:
  - `TicketService`
    - createTicket method:
      - Instead of `List<Ticket>`, `ITicketRepository` and `InMemoryTicketRepository` added
    - assignTicket method:
      - `IAgentRepository` added
      - `AgentService` added in `TicketService`
      - Most important part of problem - **Finding right agent for the ticket**
      - **Like Food ordering system(filtering and sorting based on selection criteria) and Ride sharing (filter and sort ride), we will get all the agents and then filter application agents**
        - So, one strategy to filter the agents and another strategy to sort the agents
        - This is very common pattern might be used at different places. In parking lot also it can be used
        - `orCondtion`, `andCondition` can also be used (Just like amazon filter)
      - `getAllAgents` added in `AgentService`
        - `isAvailable` added in `Agent` (Would be removed from here later)
      - OCP will break if more filter are added, or priority is assigned to any filter. But if it is not in requirement then don't try to solve it. Just call out
      - `assignableAgents` in `AgentService` can be converted to interface mechanism using strategy
      - Picking one - 
        - `IPreferenceStrategy` and `RandomPreferenceStrategy` added
        - `List<IPreferenceStrategy>` added in `TicketService`
    - resolveTicket method
    - `IPostTicketResolutionSubscriber` added
    - `AgentWorkLoggerPostTicketResolutionSubscriber` added
    - `PostTicketResolutionPubSub` added (This class should be singleton) (Spring (not java) has way to implement singleton)
    - `CustomerNotifyPostTicketResolutionSubscriber` added