// ChainStrikingPlayerStrategy is having actual chain strategy using self and next.

#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <algorithm>
using namespace std;

// ------------------ Enums ------------------
enum class BallType { NORMAL, NO_BALL, WIDE, WICKET };
enum class RunType { PLAYER_RUN, FOUR, SIX };
enum class WicketType { CATCH, CLEAN_BOLD, RO_STRIKE, RO_NON_STRIKE };

// ------------------ Run ------------------
class Run {
public:
    int value;
    RunType type;

    Run(int v = 0, RunType t = RunType::PLAYER_RUN) : value(v), type(t) {}

    int getRuns() const {
        switch (type) {
            case RunType::SIX: return 6;
            case RunType::FOUR: return 4;
            case RunType::PLAYER_RUN: return value;
        }
        return 0;
    }

    bool is4() const { return type == RunType::FOUR; }
    bool is6() const { return type == RunType::SIX; }
};

// Forward declaration
class Player;

// ------------------ Ball ------------------
class Ball {
public:
    int ballNum;
    BallType type;
    Run run;
    Player* bowler;

    Ball(int num, BallType t, const Run& r, Player* b)
        : ballNum(num), type(t), run(r), bowler(b) {}
};

// ------------------ Player ------------------
class Player {
public:
    string id;
    string teamId;
    vector<Ball> ballsPlayed;

    Player(string id, string teamId) : id(move(id)), teamId(move(teamId)) {}

    void play(const Ball& ball) {
        if (ball.type != BallType::NO_BALL && ball.type != BallType::WIDE) {
            ballsPlayed.push_back(ball);
        }
    }

    int getTotalRuns() const {
        int total = 0;
        for (const auto& ball : ballsPlayed) {
            total += ball.run.getRuns();
        }
        return total;
    }

    int get4s() const {
        return count_if(ballsPlayed.begin(), ballsPlayed.end(),
            [](const Ball& ball) { return ball.run.is4(); });
    }

    int get6s() const {
        return count_if(ballsPlayed.begin(), ballsPlayed.end(),
            [](const Ball& ball) { return ball.run.is6(); });
    }

    int getTotalBalls() const {
        return static_cast<int>(ballsPlayed.size());
    }

    bool operator==(const Player& other) const {
        return id == other.id;
    }
};

// ------------------ Team ------------------
class Team {
public:
    vector<Player*> players;
    vector<Player*> battingOrder;

    int getTotalScore() const {
        int total = 0;
        for (const auto* player : players) {
            total += player->getTotalRuns();
        }
        return total;
    }

    int getTotalWickets() const {
        return 0; // Placeholder
    }

    int getTotalOversPlayed() const {
        int balls = 0;
        for (const auto* player : players) {
            balls += player->getTotalBalls();
        }
        return balls / 6;
    }
};

// ------------------ MatchState ------------------
class MatchState {
public:
    Player* strikingPlayer;
    Player* nonStrikingPlayer;
    Team* team;

    void swapStrikingPlayer() {
        swap(strikingPlayer, nonStrikingPlayer);
    }

    void setStrikingPlayer(Player* p) {
        strikingPlayer = p;
    }
};

// ------------------ Strategy Interface ------------------
class IStrikingPlayerStrategy {
public:
    virtual void updateMatchState(MatchState& state, const Ball& ball) {
        if (isApplicable(state, ball)) {
            _updateMatchState(state, ball);
        }
    }

    virtual ~IStrikingPlayerStrategy() = default;
    virtual void _updateMatchState(MatchState& state, const Ball& ball) = 0;
    virtual bool isApplicable(const MatchState& state, const Ball& ball) const = 0;
};

// ------------------ RunBasedStrategy ------------------
class RunBasedStrikingPlayerStrategy : public IStrikingPlayerStrategy {
protected:
    void _updateMatchState(MatchState& state, const Ball& ball) override {
        state.swapStrikingPlayer();
    }

    bool isApplicable(const MatchState&, const Ball& ball) const override {
        return ball.type == BallType::NORMAL &&
               ball.run.type == RunType::PLAYER_RUN &&
               ball.run.value % 2 == 1;
    }
};

// ------------------ OverChangeStrategy ------------------
class OverChangeStrikingPlayerStrategy : public IStrikingPlayerStrategy {
private:
    int lastBallNum;

public:
    explicit OverChangeStrikingPlayerStrategy(int lastBall = 6) : lastBallNum(lastBall) {}

protected:
    void _updateMatchState(MatchState& state, const Ball&) override {
        state.swapStrikingPlayer();
    }

    bool isApplicable(const MatchState&, const Ball& ball) const override {
        return ball.ballNum == lastBallNum;
    }
};

// ------------------ WicketBasedStrategy ------------------
class WicketBasedStrikingPlayerStrategy : public IStrikingPlayerStrategy {
protected:
    void _updateMatchState(MatchState& state, const Ball& ball) override {
        const auto& order = state.team->battingOrder;
        auto it = find(order.begin(), order.end(), state.strikingPlayer);
        if (it != order.end()) {
            int index = distance(order.begin(), it) + 1;
            while (index < order.size() && order[index] == state.nonStrikingPlayer) {
                ++index;
            }
            if (index < order.size()) {
                state.setStrikingPlayer(order[index]);
            }
        }
    }

    bool isApplicable(const MatchState&, const Ball& ball) const override {
        return ball.type == BallType::WICKET;
    }
};

// ------------------ ChainStrategy ------------------
class ChainStrikingPlayerStrategy : public IStrikingPlayerStrategy {
private:
    unique_ptr<IStrikingPlayerStrategy> self;
    unique_ptr<ChainStrikingPlayerStrategy> next;

public:
    ChainStrikingPlayerStrategy(unique_ptr<IStrikingPlayerStrategy> s)
        : self(move(s)) {}

    void setNext(unique_ptr<ChainStrikingPlayerStrategy> nextChain) {
        next = move(nextChain);
    }

protected:
    void _updateMatchState(MatchState& state, const Ball& ball) override {
        if (self && self->isApplicable(state, ball)) {
            self->updateMatchState(state, ball);
        }
        if (next) {
            next->updateMatchState(state, ball);
        }
    }

    bool isApplicable(const MatchState&, const Ball&) const override {
        return true;
    }
};

// ------------------ MatchScoreService ------------------
class MatchScoreService {
public:
    void handleThrowBall(Player* bowler, Player* batsman, const Ball& ball) {
        batsman->play(ball);
    }

    void printScore(const Team& team) const {
        for (const auto* player : team.players) {
            cout << player->id << " "
                      << player->getTotalRuns() << " "
                      << player->get4s() << " "
                      << player->get6s() << " "
                      << player->getTotalBalls() << "\n";
        }

        cout << "Total: " << team.getTotalScore() << "/" << team.getTotalWickets() << "\n";
        cout << "Overs: " << team.getTotalOversPlayed() << "\n";
    }
};

// ------------------ MatchService ------------------
class MatchService {
private:
    MatchState state;
    MatchScoreService scoreService;
    unique_ptr<ChainStrikingPlayerStrategy> chain;

public:
    explicit MatchService(MatchState s) : state(move(s)) {}

    void setupStrategies() {
        auto run = make_unique<RunBasedStrikingPlayerStrategy>();
        auto wicket = make_unique<WicketBasedStrikingPlayerStrategy>();
        auto over = make_unique<OverChangeStrikingPlayerStrategy>();

        auto chain1 = make_unique<ChainStrikingPlayerStrategy>(move(run));
        auto chain2 = make_unique<ChainStrikingPlayerStrategy>(move(wicket));
        auto chain3 = make_unique<ChainStrikingPlayerStrategy>(move(over));

        chain2->setNext(move(chain3));
        chain1->setNext(move(chain2));

        chain = move(chain1);
    }

    void handleThrowBall(const Ball& ball) {
        scoreService.handleThrowBall(ball.bowler, state.strikingPlayer, ball);
        if (chain) chain->updateMatchState(state, ball);
    }

    void printScore() {
        scoreService.printScore(*state.team);
    }
};

// ------------------ Main ------------------
int main() {
    // Players
    Player p1("P1", "T1");
    Player p2("P2", "T1");
    Player p3("P3", "T1");
    Player bowler("B1", "T2");

    // Team
    Team team;
    team.players = { &p1, &p2, &p3 };
    team.battingOrder = { &p1, &p2, &p3 };

    // Match state
    MatchState state;
    state.strikingPlayer = &p1;
    state.nonStrikingPlayer = &p2;
    state.team = &team;

    // Service
    MatchService matchService(state);
    matchService.setupStrategies();

    // Deliver balls
    matchService.handleThrowBall(Ball(1, BallType::NORMAL, Run(1, RunType::PLAYER_RUN), &bowler)); // odd run -> swap
    matchService.handleThrowBall(Ball(2, BallType::NORMAL, Run(4, RunType::FOUR), &bowler));       // even -> no swap
    matchService.handleThrowBall(Ball(3, BallType::WICKET, Run(), &bowler));                       // wicket -> new batter
    matchService.handleThrowBall(Ball(6, BallType::NORMAL, Run(0, RunType::PLAYER_RUN), &bowler)); // 6th ball -> swap

    // Print
    matchService.printScore();

    return 0;
}
