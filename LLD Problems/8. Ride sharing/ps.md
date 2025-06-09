# Requirements
- https://enginebogie.com/public/question/low-level-design-ride-sharing-application/808
The application allows users to share rides on a route.

Users can either offer a shared ride (Driver) or consume a shared ride (Passenger).
Users can search and select one from multiple available rides on a route with the same source and destination.

Usecases:
Application should allow to register users:
create_user(name:"John",gender:M,age:26)

Users should be able to add vehicle (a user can have multiple vehicles):
create_vehicle(name:"John", vehicle:"swift", regNo:"KA-09-32321")

Users should be able offer rides:
offer_ride(name: "John", vehicle: "swift", seats_available: 3, origin: Bangalore, destination: Mysore, start: 'March 21, 2024, 10 AM', duration: 3h)

Users should be able to select rides based on origin, destination, and selection preference.
select_ride(name:"smith",origin:bangalore,destination:mysore,seats_requierd:2,earliest_ending_ride)

list(name:"smith",origin:bangalore,destination:mysore,seats_requierd:2,earliest_ending_ride)
book(ride_id);

Users can have preferences like:

Earliest ending ride or lowest duration ride
Preferred Vehicle (Activa/Polo/XUV)
Most Vacant.
Users should be able to get all the rides offered and taken by the user.

# Thought Process
- Main services:
  - selectRide
    - Matchride based on comparator. So, following required:
      - IRideComparatorStrategy: Comparator to match the rides
  - createBooking
  - Check java code for detail

# Note
- If 45 minutes round then LLD, if 90 mins then mostly machine coding
- Do basic setup before interview only like project setup, junit setup etc. Also, inform to interviewer about this. If they are not fine then create new project. (99% they will be fine)
- If coding in java and googling how to write for loop in java considered as -ve point. If using treeMap and googling how to use then it might be not considered as -ve point
- Goal is to complete first then think whether it is right or not. How can we decide whether it is right or not? Using OCP
- While thinking about properties, think about what ownership classes should have
- Don't try to think much about metadata which are not given in problem statement
- Always put properties whatever you think might required then remove it later. Or vice versa, where you add required properties and then put whichever not required. Udit prefer on demand, so put properties whenever required
- Beset ways to designing model class is, visualise system in front of you. Think who owns what.
- Latitude, longitude required for prod code. But don't complecate for interview to avoid complication
- Udit asked in interview, where it can fails if Interface IVehicle is used
- Note: **This controller keep breaking up in production. Let's say right now driver and passenger are managed in RideController. It becomes too big. To solve this, PassengerController and DriverController are forked**
- Controller is logical division, not a principle base division.
- In Udan, Udit was asked to run code via postman. Other than Udan, it was not asked in other company. (As need to run via postman API support should be added via controller). If APIs are not required then can directly start with service instead of controller. (Controller required to convert input to object)
- Do we need paginator? Ask interview and confirm. In company ask PM
  - So, proposed and then confirm and also let them know if that is important requirement or not.
  - If yes then cover it
- Clarify whether you want better performance also or extensibility is main concern
- Clarify whether interview want all the classes, better design or they want completeness?
  - zeta, gojek wants better design so create all the classes (including repository). It is ok to create empty class. But atleast create clsas
  - For swiggy they look for completeness. Hence, can avoid creating few classes
  - Again, clarify with interviewer
- For 2-3 parameter better to send via parameter then creating POJO for that
- Don't start coming up with strategy directly. If you are unable to find where is the OCP problem, then write simple code. Figure out where is OCP problem then solve it via strategy. Don't start with strategy directly if haven't done that problem. If unknown problem and come up with strategy first then people sometimes get stuck "to fit that strategy to their problem"

# Folders
- model
  - For database related
- controller
  - Its job is to convert API to your own object. If not required then can directly starts with service. No need to create this hierarchy. If required then create own object and then call service
  - If command line input then command pattern and from command handler call service
- service
  - **Whatever interaction with external service world, at the end you will be calling service. And service will be entry point for business logic**
- repository
  - For storing objects. services will use repository for data storing
  - As DB is not used for interview, Udit usually prefer repository related things in service folder only. He does not create repository folder
- strategy
  - As we use strategy pattern most

# Design
## Classes (**model** folder)
- Car, Ride(or Trip), User, Booking, Passenger, Driver
## Properties of classes
- `Booking`:
  - Booking does not own source, destination.
  - Date, Ride, User, Passenger, numberOfSeats (Only one of the property would be there among User and Passenger)
    - StartLocation, EndLocation in Booking can be used if person is joining/ending from/to any other location then source/destination. However, this is not given in requirement. So can skip this
- `Car` Implements IVehicle. But new vehicle added then there will be a lot of duplicate code. Also, If there is Electric bike and normal bike then also there will be lot of classes
  - So, It is like creating hierarchy of classes.
  - **If we have a lot of combination then better to go with composition instead of inheritance**
  - So class `Vehicle` created with `VehicleType` created
  - Similar applies to User and Passenger class
  - Inheritance looks very good in the beginning. But as the level of inheritance grows, it becomes difficult to maintain and would face issue in future. (In interview it might not matter, but in production code it matters)
  - **VehicleType** has Car as well as Bike. Udit is not happy with this. Might address this at the end
- Class `User` (Passenger class not required)
- Class `Ride`
  - Booking has Ride, and Ride has Booking. So, circular dependencies. Right now we are not fetching from DB. So, it is okay to keep at both the places. We will use it at only one place
  - enum `RideStatus`
  - In Booking we thought of Different source and Destination. `List<RideStop>` added
    - Ride will show all the stops. Where Booking will tell stop for particular Booking
    - RideStop will tell what are the stops Ride going to have. So, User can book accordingly
    - We are learning so adding this. In actual interview don't include this and complicate it. Even in production this can be discussed with PM
## Going top down
- **controller**
  - Controller required to convert input to object (View would be API or command line supported?)
  - Top down means start implementing method from main. So, in controller we will add top controller
  - `RiderController`, should be top down class.
    - Anything related to ride can be put in RideController
    - RideController has `offerRide` method
    - Ride object is passed to this method. In production or interview where API needs to be supported:
      - json is passed to `offerRide`
      - Then convert json body to `Ride` object
      - Then `rideService.createRide(ride)` is called
  - `UserController`: Anything related to User can be put here
  - If API via json or REST not asked then can start directly with service. controller might not be required in that case as input to object conversion not required. We can write our test or main such that directly object is used. So, we will start with service first
  - Note: If input is via command then similar to `controller`, we will have `commandController` which will accepts command instead of `json` (We will use command pattern. So, from commandHandler we will call `createRide` method)
  - **Whatever interaction with external service world, at the end you will be calling service. And service will be entry point for business logic**
- service
  - Entry facing methods
  - `RiderService` class
    - `createRide` method
      - **`InMemoryRideRepository` and `IRideRepository` created which will store rides in memory. In case in future if required to store in DB then we can add required storage class in repository folder**
    - `selectRide` method
      - It will have the following arguments:
        - User, Location start, Location end, numberOfSeats, UserSelectRidePreference
          - `UserSelectRidePreference` is enum which has `EARLY_ENDING`, `PREFERRED_VEHICLE` etc. But name of this enum is changed to `SortingName` later
        - All these arguments can be converted to single POJO class. We can think about this later if required
        - It is POJO breaking. But Udit is fine with POJO breaking then OCP breaking
        - Keep the number of parameters less than 7. It will be good user experience.
        - So, Udit created `SelectRideRequest` class
        - `UserSelectRidePreference` might required multiple preference. (AND/OR preferences like seraching problem which was discussed in previous class)
      - Business logic:
        - rideRepository.getAllRides
        - hasPath and getAvailableSeats in Ride class
        - created `IRidePreferencesMatchingStrategy`
        - BookingService.createBooking is called
        - ```
          allRides = getAllRides
          soFarMatchingRides = allRides.stream.filter(ride -> ride.hasPath).filter(ride -> ride.getAvailableSeats).collect // this is filter
          
          strat = ridePreferenceMatchingStrategy.getUserPreference()
          ride = strat.match(soFarMatchingRides).stream.findFirst().orelse(null) // This is sorting. We provided same object and same object was returned with different order. Hence, this is not filter it is sorting. Person is saying me preference is XUV. He is not saying I will not go if XUV not assigned

          booking = booking.CreateBooking(request.getUser(), ride)
          ride.getBooking().add(booking)
          return booking
          ```
  - `BookingService` (new service as logical division and SRP could be followed here)
    - IBookingRepository
    - createBooking method in BookingService
- Improvement:
  - Initially two things were hardcoded in RiderService::selectRides: 1. matching Ride filter is hardcoded (This is required filter: source and destination are mandatory), 2. instead of findAny to find ride we can have ridePickingStrategy (This is optional filter)
    - preference is sorting, it is not filter
    - Hence, enum `RideFilterName` added
    - Also, in `SelectRideRequest`, `List<Sorting> sortings` and `List<Filter> filters` added. `UserSelectRidePreference` renamed to `Sorting`
    - `Filter` class created
    - This filter is same like ecommerce(watch it multiple times). Filter is very reusable, if you show that to interviewer it will go to strong hire. orFilter, andFilter should be implemented here also. Though Udit did not add due to time constraint
    - `ISortingData`, `Sorting` etc added