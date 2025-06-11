# Requirement
Design a cricket scorecard that will show the score for a team along with score of each player.

You will be given the number of players in each team, the number of overs and their batting order as input. Then, we can input overs ball by ball with the runs scored on that ball (could be wide, no ball or a wicket as well).
You are expected to print individual scores, number of balls faced, number of 4s, number of 6s for all the players from the batting side at the end of every over. You also need to print total score, total wickets. Essentially, you need to keep a track of all the players, strike changes (at the end of the over or after taking singles or 3s) and maintain their scores, also keep track of extra bowls that are being bowled (like wides or no balls). You also need to print which team won the match at the end.
This is the bare minimum solution which is expected for the problem. You can add some more features once you are done with these, like maintaining a bowlers record (total overs bowled, runs conceded, wickets taken, maiden overs, dot balls, economy, etc.). Total team extras, batsman strike rates, etc. can be added too. But these are "good to have" features, please try to complete the bare minimum first.

Make sure your code is readable and maintainable and preferably object oriented. It should be modular and extensible, to add new features if needed.

Sample input and output:
```
No. of players for each team: 5
No. of overs: 2
Batting Order for team 1:
P1
P2
P3
P4
P5
Over 1:
1
1
1
1
1
2


Scorecard for Team 1:
PlayerName Score 4s 6s Balls
P1* 3 0 0 3
P2* 4 0 0 3
P3 0 0 0 0
P4 0 0 0 0
P5 0 0 0 0
Total: 7/0
Overs: 1


Over 2:
W
4
4
Wd
W
1
6


Scorecard for Team 1:
Player Name Score 4s 6s Balls
P1 3 0 0 4
P2* 10 0 1 4
P3 8 2 0 3
P4* 1 0 0 1
P5 0 0 0 0
Total: 23/2
Overs: 2


Batting Order for team 2:
P6
P7
P8
P9
P10


Over 1:
4
6
W
W
1
1


Scorecard for Team 2:
Player Name Score 4s 6s Balls
P6 10 1 1 3
P7* 1 0 0 1
P8 0 0 0 1
P9* 1 0 0 1
P10 0 0 0 0
Total: 12/1
Overs: 1


Over 2:
6
1
W
W


Scorecard for Team 2:
Player Name Score 4s 6s Balls
P6 10 1 1 2
P7* 8 0 1 3
P8 0 0 0 1
P9 1 0 0 2
P10 0 0 0 1
Total: 19/4
Overs: 1.4


Result: Team 1 won the match by 4 runs
```
# Thought Process
- Classes
- Properties
- Top-down methods:
  - MatchScoreService:
    - Method: handleThrowBall -> updateScoreCard
    - Method: printScore (Team team)
    - strikingPlayer, nonStrikingPlayer added in MatchScoreService. But we are getting player, ball as input only. So, this we should be getting from above only.
  - We added `MatchService` and strikingPlayer, nonStrikingPlayer could be added there. MatchScoreService should be completely independent of MatchService as handling ball and match are different thing
    - strikingPlayer, nonStrikingPlayer, match
    - handleThrowBall(@NonNull final Ball ball) method
      - **IStrikingPlayerStrategy** and **MatchState** created.
        - Update MatchState based on ball using IStrikingPlayerStrategy
        - So, IStrikingPlayerStrategy updateMatchState method first check whether isApplicable(Ball ball) is true or not
          - If true then update MatchState in `updateMatchState` method based on to ball type
          - OverChangeStrikingPlayerStrategy, RunBasedStrikingPlayerStrategy, WicketBasedStrikingStrategy added for this
          - **`_isApplicable` called in `updateMatchState` method of `IStrikingPlayerStrategy`. By doing this, we are making sure that isApplicable always gets called(so service side/or IStrikingPlayerStrategy child class forgets then it doesn't create an issue)**
      - Now how would we ensure all the IStrikingPlayerStrategy have precedence ensure(For ex. over change and run score then how would we change strike)
        - 1. One easy solution is ordering using List only (Client gives list or we defined the order, which is risky but one of the possible solution)
        - 2. We can have priority queue or Integer priority as member for IStrikingPlayerStrategy
        - 3. Design solution: **USING CHAINING**
      - `ChainStrikingPlayerStrategy` added
        - Even though chain is set by developer, they won't be making mistake here
# Notes
- Lot of classes for this. But don't create classes outside of requirements
- This problem is more about entities. We will get very less scope for extensibility in this problem
- `AND`, `OR` can also be used for striking player strategy (like search)
- Enforcing function call isApplicable
- Enforcing precedence using chaining
- Always start simple. If you start with complicate things then you will end up complication which is not needed. If you start simple then you will be able to find out what might break, what might not work well or what might needs to be extend. And, then make only that part extensible 
- Keep making call out wherever improvement could be done if less time. (Especially for SDE3+ position)
- when to call updateScoreCard?
  - sync or async (this is latency which comes under HLD but still call out. For LLD interview need not worry about latency. But still call it out if applying for senior position)
# Design
## Classes
- Player, Over, Team, Match, Scorecard, Run(Interger value, RunType), RunType, Wicket, Ball, BallType
## Properties
- Over: `List<Ball> balls;`
- Ball: Over, BallType, Run, Player batsman, Player bowler
- Match: Team team1, Team team2, Integer totalNumberOfOvers
  - List<Team> is not required as there won't be any design where more than 2 team would be playing one match. That's why we have hard coded team1 and team2
- Team: List<Player> players, List<Player> battingOrders, Team winner (At present we will keep here. Will see if required or not in future)
- Scorecard is more of database property which has business logic. So, this needs to be added into repository instead of Match class
- Player: List<Ball>
## Method (Top-down)
- `MatchScoreService`
  - We can take full match as input which is not good idea. If we go over as input then it does not match with our requirement. So, we will go with ball by ball as input. Ball by ball is more granular level. Hence, we can extend it easily in the future. using this we can write wrapper over it to create over
  - handleThrowBall(Player bowler, Player batsman, Ball ball) method added
    - Can take business object here(**controller Layer** on top of it will convert input id to business object) or take can input id in service logic and service logic will convert input id to business object
    - `play` method added in `Player` class
    - batsman.play(ball) called from handleThrowBall
  - strikingPlayer, nonStrikingPlayer added in MatchScoreService. But we are getting player, ball as input only. So, this we should be getting from above only.
- That's why we added `MatchService` and strikingPlayer, nonStrikingPlayer could be added there. MatchScoreService should be completely independent of MatchService as handling ball and match are different thing
  - MatchService will have:
    - strikingPlayer, nonStrikingPlayer, match
    - handleThrowBall(@NonNull final Ball ball) method
      - `matchScoreService.handleThrowBall(ball.getBowler(), strikingPlayer, ball);`
      - now we need to handle changing of striking bowler. So added `IStrikingPlayerStrategy`
        - `MatchState` added and `strikingPlayer, nonStrikingPlayer` moved here. Then `MatchState` is used in MatchService instead of `strikingPlayer, nonStrikingPlayer` and also `IStrikingPlayerStrategy strikingPlayerStrategy` used
        - `updateMatchState` method added
        - `matchScoreService.handleThrowBall(ball.getBowler(), strikingPlayer, ball);` changed to `matchScoreService.handleThrowBall(ball.getBowler(), state.getStrikingPlayer(), ball);`
        - `strikingPlayerStrategy.updateMatchState` called from hanleThrowBall
    - Now for `IStrikingPlayerStrategy` we need to think about 3 things:
      - 1. Over change
      - 2. Wicket happening
      - 3. Run
    - OverChangeStrikingPlayerStrategy added. (`_updateMatchState` added inside this class)
      - swapStrikingPlayer method added in MatchState
    - RunBasedStrikingPlayerStrategy added
    - `List<IStrikingPlayerStrategy> strikingPlayerStrategies` added
    - **`_isApplicable` called in `updateMatchState` method of `IStrikingPlayerStrategy`. By doing this, we are making sure that isApplicable always gets called(so service side/or IStrikingPlayerStrategy child class forgets then it doesn't create an issue)**
    - Now, how would we ensure all the IStrikingPlayerStrategy have precedence ensure(For ex. over change and run score then how would we change strike)
      - 1. One easy solution is ordering using List only (Client gives list or we defined the order, which is risky but one of the possible solution)
      - 2. We can have priority queue or Integer priority as member for IStrikingPlayerStrategy
      - 3. Design solution: **using chaining**
    - `ChainStrikingPlayerStrategy` added
      - Even though chain is set by developer, they won't be making mistake here
- `printScore` added in `MatchScoreService` (Always start simple)
  - `getTotalRuns`, `get4s`, `get6s`, `getTotalBalls` added in `Player` (null checks not put. For production make sure to put null checks)
  - `getRuns` added in `Run`
  - `getTotalScore`, `getTotalWickets`, `getTotalOverPlayed` added in `Team
  - For printer so many lines. Call out to interviewer that we might need printer here to simplify it (If time then implement. For SDE3 or more it becomes important)
- `List<Ball>` should not be in `Player`. Instead it should be in database. So, repository comes into picture here for actual production. For model class or business class it is okay. For production call it out that we will have separate repository for this or class for this
- `EngineBoogie` example from 2:20:00
  - UI became slow so Udit separated User, Booking classes as hibernate fetching everything became slow 
- `_updateMatchState` of WicketBasedStrikingStrategy implemented
  - `Wicket` added to `Ball`
  - `wicketee` and `WicketType` added in Wicket
