Design and implement a **queue management system** for a single bathroom used in a voting agency shared by Democrats and Republicans. The system must adhere to strict rules regarding usage and prioritize fairness while maximizing efficiency.

###### Requirements:
1. **Single Bathroom**: The bathroom can accommodate a maximum of **3 people** at a time.
2. **Homogeneous Groups Only**: At any given time, the bathroom must be used by either only Democrats (D) or only Republicans (R). Mixed groups like `(2D, 1R)` or `(1D, 1R)` are **not allowed**.
3. **Queue Management**: People wait in a queue until their group can fully occupy the bathroom according to the constraints.
4. **Usage Duration**: Each person takes a variable amount of time, calculated by a function `f(N)`, where `N` is the person's name.
5. **Fairness**: The system must ensure that eligible people are served as quickly as possible while adhering to the above constraints.
6. **Prioritization**: The system should allow the most eligible group (based on availability and queue conditions) to occupy the bathroom whenever there's room.

###### Input
- A queue of people arriving sequentially, where each person belongs to either group D or R.
- A function `f(N)` that returns the bathroom usage time for each individual.

###### Output
- A schedule of bathroom usage that ensures:
   - Maximum efficiency in bathroom usage.
   - Fairness in serving Democrats and Republicans.
   - Adherence to group-based constraints.

###### Examples

**Example 1**  
Input:  
Queue = `["D1", "D2", "R1", "R2", "D3", "D4", "R3"]`  
`f(N)` returns: `D1=3s, D2=4s, R1=5s, R2=3s, D3=2s, D4=6s, R3=4s`

Output:
- Time 0–7s: Bathroom used by `[D1, D2, D3]`
- Time 7–13s: Bathroom used by `[D4]`
- Time 13–16s: Bathroom used by `[R1, R2, R3]`

**Example: 2**
```
Input:  
Queue = ["R1", "R2", "D1", "D2", "D3", "R3", "D4"] 
f(N) returns: R1=2s, R2=3s, D1=4s, D2=5s, D3=3s, R3=2s, D4=6s

Output:  
- Time 0–5s: Bathroom used by [R1, R2, R3]  
- Time 5–12s: Bathroom used by [D1, D2, D3]  
- Time 12–18s: Bathroom used by [D4]  
```

###### Constraints
- The bathroom usage time `f(N)` must be non-negative.
- Both Democrats and Republicans arrive in random order.

###### Notes
- **Usage Duration**  - This we can skip initially. Initially, we will start with fix amount of duration
- If democrats comes and bathroom is already by democrats then democrats should be able to use it(same applies for republican) (This is due to fairness requirement)
  - In this approach starving can happened. So, call out to interviewer regarding (Can suggest like allow only 100 and then switch)
- One things is command for multithreading is, there will be `while` loop and there will be `wait`
- For interview go with Synchronous for this problem. For H2O Asynchronous was straight forward. But for this Asynchronous is complex
###### Design
- Solution 1: `BathroomDemocratRepublican` class created (Synchronous approach)
  - `democrat` and `republic` method defined
  - `State` class created with `toString` method
  - state object created in BathroomDemocratRepublican with constructor
  - `Bathroom` class created and used in BathroomDemocratRepublican. (`State` is not required)
  - Lock on `Bathrooom` in democrat and republic functions
  - `DRRunner` added
  - **Usage Duration** can be supported by adding `Long millis` as an argument with democrat and republic functions
  - For fairness(Fairness means whichever came first that should be allowed to use bathroom. For ex. if D1 comes before D2 then D1 should be assigned bathroom first) can use `reentrant` fair lock or queue. `myLock` added in BathroomDemocratRepublican.
  - There is a problem of starvation but that is not given in requirement. So, no need to implement. Still better to call it out to interviewer (In company discuss with PM)
- Solution 2: Implement using async with additional fairness (Asynchronous approach)
  - `BDRV2` created
  - `state` object is used
    - `democrat` and `republican` added to `BDRV2`
    - `BathroomV2` created and `bathroom` added to State
      - Initially use Integer variable for democrat and republican in state class
  - `BDRWorker` created
    - run method added. Condition would be difficult with this. So instead separate worker created for democrat and republican. (So, lets write with separate worker, and then we will see if we can use single worker)
    - `BDRDemocratWorker` and `BDRRepublicanWorker` created
  - `BDRRunnerV2` created
  - Added queue in `State` class to print better naming. POJO `Person` also created
    - Fairness would fix(no need to use reentrant lock) with queue
  - `for (int i = 0; i < 3; i++) {` added in BDRV2 constructor because if we don't add then only one thread will be used for scheduling and only one democrat or republican would be able to use bathroom
  - Here, problem is starvation if democrat(or republican) is keep coming then republican(or democrat) would keep waiting
    - To solve this we can have size based approach (number of time it is used by democrat or republican)
    - or we can use have time based approach
    - or based on number of people waiting in queue (if more number of democrat are waiting more than republican)
      - This can be solved using `state.democrat.size() < state.republic.size()` and `state.republic.size() < state.democrat.size()` added (This is business side fairness)
