#include <bits/stdc++.h>
#include <mutex>
#include <condition_variable>
#include <unordered_map>
#include <thread>
#include <atomic>

using namespace std;

struct PlayerStats {
    int score = 0;
};

struct Player {
    mutex mtx;
    PlayerStats stats;

    int getScore() {
        return stats.score;
    }
};

struct LeaderBoardStats {
    unordered_map<string, Player> playerMap;
    vector<pair<PlayerStats, string>> orderedPlayer;

    bool shouldProcess = false;
    mutex mtx;
    condition_variable cv;
};

class LeaderBoardRankingWorker {
    LeaderBoardStats& stats;
    atomic<bool>& stopFlag;

public:
    LeaderBoardRankingWorker(LeaderBoardStats& stats, atomic<bool>& stopFlag)
        : stats(stats), stopFlag(stopFlag) {}

    void run() {
        while (!stopFlag.load()) {
            unique_lock<mutex> lock(stats.mtx);
            stats.cv.wait(lock, [&]() { return stats.shouldProcess || stopFlag.load(); });

            if (stopFlag.load()) break;

            stats.shouldProcess = false;

            // Create snapshot and unlock main mutex
            unordered_map<string, Player>& localMap = stats.playerMap;
            lock.unlock();

            vector<pair<PlayerStats, string>> values;

            for (auto& [playerId, player] : localMap) {
                values.emplace_back(player.stats, playerId);
            }

            sort(values.begin(), values.end(), [](const auto& a, const auto& b) {
                return a.first.score > b.first.score;
            });

            stats.orderedPlayer = std::move(values);
        }
    }
};

class LeaderBoard {
    LeaderBoardStats stats;
    LeaderBoardRankingWorker worker;
    thread workerThread;
    atomic<bool> stopFlag;

public:
    LeaderBoard()
        : stopFlag(false), worker(stats, stopFlag),
          workerThread(&LeaderBoardRankingWorker::run, &worker) {}

    ~LeaderBoard() {
        stopFlag.store(true);
        stats.cv.notify_all();
        if (workerThread.joinable()) {
            workerThread.join();
        }
    }

    void updateScore(const string& playerId, int score) {
        {
            unique_lock<mutex> lock(stats.mtx);
            if (stats.playerMap.find(playerId) == stats.playerMap.end()) {
                stats.playerMap.try_emplace(playerId);
            }
        }

        {
            lock_guard<mutex> playerLock(stats.playerMap[playerId].mtx);
            stats.playerMap[playerId].stats.score += score;
            this_thread::sleep_for(chrono::milliseconds(100));
        }

        {
            unique_lock<mutex> lock(stats.mtx);
            stats.shouldProcess = true;
            stats.cv.notify_one();
        }
    }

    vector<pair<PlayerStats, string>> getOrderedPlayer() {
        lock_guard<mutex> lock(stats.mtx);
        return stats.orderedPlayer;
    }
};

int main() {
    LeaderBoard leaderboard;

    // Writer threads: updating scores
    vector<thread> updateThreads;
    for (int i = 0; i < 50; ++i)
        updateThreads.emplace_back(&LeaderBoard::updateScore, &leaderboard, "p1", 3);

    for (int i = 0; i < 30; ++i)
        updateThreads.emplace_back(&LeaderBoard::updateScore, &leaderboard, "p2", 4);

    // Reader threads: printing leaderboard multiple times
    vector<thread> printThreads;
    for (int i = 0; i < 5; ++i) {
        printThreads.emplace_back([&leaderboard, i]() {
            for (int j = 0; j < 3; ++j) {
                this_thread::sleep_for(chrono::milliseconds(500)); // wait before each print
                auto ordered = leaderboard.getOrderedPlayer();
                cout << "Leaderboard Print #" << i * 3 + j + 1 << ":\n";
                for (const auto& [stats, playerId] : ordered) {
                    cout << playerId << " -> " << stats.score << endl;
                }
                cout << "-----------------------------\n";
            }
        });
    }

    for (auto& t : updateThreads)
        t.join();

    for (auto& t : printThreads)
        t.join();

    auto ordered = leaderboard.getOrderedPlayer();
    cout << "Final Leaderboard:" << endl;
    for (const auto& [stats, playerId] : ordered) {
        cout << playerId << " -> " << stats.score << endl;
    }

    return 0;
}
