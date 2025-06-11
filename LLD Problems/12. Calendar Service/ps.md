# Requirement
Design a calendar Application (similar like Google Calendar) 
* Ability to create, update, delete an Event
* Get Calendar for a user Ui
* Get Event details.
* For a given set of users[U1, U2,....Un] identity a common free slot of time.
* An event once created, can be either accepted or rejected by the constituent users - if neither it should be in neutral state.


* An event would typically consist of {start, end, location, Owner, user-list, title}.
* Events can either be like meetings(with a dedicated location and appropriate guest-list) or as well be like holidays, birthdays, reminders etc.

# Thought Process
- Start with entity(classes) and then properties
- Then core requirement(core method - top down method)
  - so, calendarService:
    - Which has different method based on requirement:
      - for ex. createEvent 

# Notes
- create, update, delete an event core requirement. create an event more important (Update, delete would be similar)
- Race condition or multi threading might need to handle. Clarify or callout to interviewer. Even if they say tell that first I would handle core requirement and then multithreading at the end
- 1. Whenever in confusion, proceed with one option. Later when you implement method you will get more clear idea. 2. If it looks for production we need to store in DB then better to crate repository
- Recurring event also good call out to inform to interviewer, but also call out that not considering it
- Callout that methods can be broken to different service if multiple methods in single service
  - If required or it service is getting complex then go with different service
- get the feel of the interviewer. Try to check if he is getting convinced or not
- get used to with stream programming. It makes code smaller and more readable

# Design
## Classes:
- Calendar, Event, Slot, User
## Properties:
- Slot
  - startTime, endTime
  - Can break into timeStamp also
- User
  - id
  - List<Event> events (1. Whenever in confusion, proceed with one option. Later when you implement method you will get more clear idea. 2. If it looks for production we need to store in DB then better to crate repository)
    - Hence, events will be stored in repository instead of User class
    - For interview if we want we can put into User class. But for longer term better to keep in repository except the Game type of design as for Game state remains in-memory.
    - `IEventRepository` added
- Event
  - id, slot, `User owner`, `List<User> guests`, eventType, Location
  - EventType (might not required but adding it as of now)
  - Location
    - title, id, ILocationTypeData, LocationType
## Services (method - top down methods) (Business logic layer comes here)
- Validation logic comes here. For ex. to createEvent we required validation. Call out to interviewer that if it is not important then will implement it later-
  - If validation related to input then it should go to input layer (model folder), but if it is related to business logic then it should be in service layer
- CalenderService
  - `createEvent`
    - `NotificationService` added (Though not implemented. But it is required in production. Again clarify with interviewer)
    - NotificationService.notify(), meetingRoomService.block() are post event creation work. We don't want to block event creation for this. (Hard coupling and OCP also breaking along with performance. Latency is HLD problem)
    - Pub-sub model can be used here. Create event and asyncynously calls event notification, meeting room block
      - For engineboogie, Udit started with sync only. Then slowly when use case increased he started moving to async (pub-sub)
      - Sync is not reliable(fault-tolerant), as if notification fails then it will create problem.
      - For interview can go with sync and can call out that for actual system we can go with pub sub
  - `getEvent` - simpler as need to get from eventRepository
  - `getFreeSlots`
    - `getAllSlots` - This is related to slotService. Hence, `SlotService` created and `getAllSlots` method added
    - `getBookedSlots` method added. Then `getUserEvents` of `IEventRepository` called
    - `doesOverlap` added in `Slot`
- An event once created, can be either accepted or rejected by the constituent users - if neither it should be in neutral state.
  - Event class:
    - What can break (OCP break)? What is not implemented? Following are limitation:
      - Accept/Reject status is not implemented
      - Different types of personas - editor, admin
      - More than one organiser
    - To solve this, `Participant`, `ParticipantType`, `RSVPStatus` added
      - Problem with keeping ParticipantType in Partition:
        - Behavior might change - like sending notification based on different partition type, permission might change, display might get changed based on partition type. Hence, it is not OCP complient
        - So, instead of keeping `ParticipantType` in `Participant`: `List<Permission>`, added in `Participant` class
          - As permission is changing we added List<Permission> (Composition over inheritance)
        - For interview `ParticipantType` could be fine
        - 