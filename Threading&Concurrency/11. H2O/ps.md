Implement a thread synchronization system to simulate the formation of water molecules (H₂O). There are two types of threads, **oxygen** and **hydrogen**, and the goal is to ensure that threads bond together in the correct ratio to form water molecules.

Each thread type is provided with a specific method:
- **Oxygen threads** call `releaseOxygen()`.
- **Hydrogen threads** call `releaseHydrogen()`.

These threads interact with a **barrier** where they must wait until enough threads are present to form a complete water molecule. The synchronization must ensure the following:

1. **Thread Grouping**:
    - Each group must consist of **one oxygen thread** and **two hydrogen threads**.
    - No thread may proceed until all three required threads for a water molecule are available.

2. **Execution Order**:
    - Threads forming one molecule must complete their execution and bond before any threads from the next molecule are allowed to proceed.

3. **Fairness**:
    - Threads should not be starved; all waiting threads must eventually contribute to forming a molecule.

###### Behavior Rules:
- If an oxygen thread arrives at the barrier but fewer than two hydrogen threads are present, it must wait.
- If a hydrogen thread arrives but there is no oxygen thread or a second hydrogen thread, it must wait.
- Threads do not need to explicitly pair with specific counterparts. Instead, the system should simply ensure that every group of three consists of one oxygen and two hydrogen threads.

###### Output Requirement:
Each time a complete molecule is formed, the **`releaseOxygen`** and **`releaseHydrogen`** methods should execute in a way that corresponds to the formation of the molecule, maintaining the 2:1 ratio.


###### Implemetation
- Solution 1:
  - `H2O` class
  - `State` class
  - `WaterMoleculeWorker` class
  - `new Thread(new WaterMoleculeWorker(state)).start();` in constructor of H2O
  - What if some heavy operation in WaterMoleculeWorker `run()`?
    - We can keep heavy operation outside synchronized lock
    - what if multiple threads try to take lock in `while (!state.isWaterPossible())`
      - Not possible as only one thread will get the lock when notifyAll is called (after `wait()`) (note: this is due to while loop. In case we have used `if` condition then multiple `if` condition would have been required)
  - As we are doing common operation, we are taking lock on `state` in `releaseOxygen` and `releaseHydrogen`. If there is different requirement then we could have taken lock on `Oxygen` and `Hydrogen` object
    - Also, worker is waiting on `state`, so we can't take lock `Oxygen` and `Hydrogen`
  - `H2ORunner` added
  - Proving correctness of multithreading is complex :D (There is no sure shot way). (Maybe we can use command line input with sleep. But that will also not guarantee proving correctness)
    - UT can be added. UT is also difficult as adding sleep will increase build time
    - Can also prove correctness by writing command line input and then adding logging
- Solution 2:
  - Can we do without `WaterMoleculeWorker` class? yes! Using following
    - `H2OV2` and `H2OV2Runner` added
    - Instead of relying common worker in `releaseOxygen` and `releaseHydrogen` only we will check if molecule can be formed or not. Another requirement is we should accumulate hydrogen or oxygen, if we have two hydrogen or one oxygen. We should get HH0, or OHH, or HOH etc. We should not get HOO, or OHO etc
    - Initially we added `if (state.oxygen == 1)` but then changed to `while (state.oxygen == 1)`
      - Most of the case it should be `while` instead of `if`. Because once comes out of `wait`, we should revalidate again
    - If something is going to `wait` then make sure that someone `notify` it.
- All of this we can solve in two ways:
1. Synchronized using background thread (using queue etc) (solution 1)
2. Synchronized among the incoming threads (solution 2)
- It depends on based on the requirement
  - If we don't want to client to wait then use worker (WaterMoleculeWorker) or alternative (solution 1)