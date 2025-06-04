#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <vector>

enum class Party { D, R, NONE };

struct Bathroom {
    int maxOccupantsAllowed;
    Party currentParty;
    int currentOccupants;

    Bathroom(int max) : maxOccupantsAllowed(max), currentParty(Party::NONE), currentOccupants(0) {}
};

class BathroomDemocratRepublican {
private:
    Bathroom& bathroom;
    std::mutex mtx;
    std::condition_variable cv;

public:
    BathroomDemocratRepublican(Bathroom& b) : bathroom(b) {}

    void democrat(const std::string& name, long millis) {
        std::cout << name << " came\n";

        {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [&]() {
                // bathroom.currentParty == Party::D doesn't work because what if Party::NONE.
                return (bathroom.currentParty != Party::R && bathroom.currentOccupants < bathroom.maxOccupantsAllowed);
            });

            bathroom.currentParty = Party::D;
            bathroom.currentOccupants++;
            std::cout << name << " using the bathroom\n";
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(millis));

        {
            std::unique_lock<std::mutex> lock(mtx);
            bathroom.currentOccupants--;
            if (bathroom.currentOccupants == 0) {
                bathroom.currentParty = Party::NONE;
            }
            std::cout << name << " exited the bathroom\n";
            cv.notify_all();
        }
    }

    void republican(const std::string& name, long millis) {
        std::cout << name << " came\n";

        {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [&]() {
                return (bathroom.currentParty != Party::D && bathroom.currentOccupants < bathroom.maxOccupantsAllowed);
            });

            bathroom.currentParty = Party::R;
            bathroom.currentOccupants++;
            std::cout << name << " using the bathroom\n";
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(millis));

        {
            std::unique_lock<std::mutex> lock(mtx);
            bathroom.currentOccupants--;
            if (bathroom.currentOccupants == 0) {
                bathroom.currentParty = Party::NONE;
            }
            std::cout << name << " exited the bathroom\n";
            cv.notify_all();
        }
    }
};

int main() {
    Bathroom bathroom(3);
    BathroomDemocratRepublican bdr(bathroom);

    std::vector<std::thread> threads;

    threads.emplace_back(&BathroomDemocratRepublican::democrat, &bdr, "D1", 5000);
    threads.emplace_back(&BathroomDemocratRepublican::democrat, &bdr, "D2", 5000);
    threads.emplace_back(&BathroomDemocratRepublican::republican, &bdr, "R1", 5000);
    threads.emplace_back(&BathroomDemocratRepublican::democrat, &bdr, "D3", 5000);
    threads.emplace_back(&BathroomDemocratRepublican::democrat, &bdr, "D4", 5000);

    // Only join required, notify not required. Because thread will sleep for some time and will exit
    for (auto& t : threads) {
        t.join();
    }

    return 0;
}
