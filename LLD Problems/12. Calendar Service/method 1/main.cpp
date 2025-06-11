#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <map>
#include <algorithm>
#include <optional>
using namespace std;

// ENUMS
enum class EventType {
    MEETING, HOLIDAY, REMINDER, BIRTHDAY
};

enum class LocationType {
    URL, PHYSICAL, MEETING_ROOM
};

enum class ParticipantType {
    ADMIN, OWNER, GUEST, EDITOR, ATTENDANCE_MARKER, GUEST_SPL
};

enum class RSVPStatus {
    ACCEPT, REJECT, UNKNOWN
};

// FORWARD DECLARATIONS
class ILocationTypeData;
class Event;
class User;
class Slot;

// INTERFACES
class ILocationTypeData {
public:
    virtual ~ILocationTypeData() = default;
};

class IEventRepository {
public:
    virtual void save(const shared_ptr<Event>& event) = 0;
    virtual shared_ptr<Event> get(const string& eventId) = 0;
    virtual vector<shared_ptr<Event>> getUserEvents(const shared_ptr<User>& user, long start, long end) = 0;
    virtual ~IEventRepository() = default;
};

// DOMAIN CLASSES
class User {
public:
    explicit User(string id) : id_(move(id)) {}
    const string& getId() const { return id_; }
    bool operator==(const User& other) const { return id_ == other.id_; }

private:
    string id_;
};

class Slot {
public:
    Slot(long start, long end) : startTime(start), endTime(end) {}

    bool doesOverlap(const Slot& other) const {
        return startTime < other.endTime && other.startTime < endTime;
    }

    bool between(long start, long end) const {
        return startTime >= start && endTime <= end;
    }

    long getStart() const { return startTime; }
    long getEnd() const { return endTime; }

private:
    long startTime;
    long endTime;
};

class Location {
public:
    Location(string id, string title, shared_ptr<ILocationTypeData> typeData, LocationType locationType)
        : id(move(id)), title(move(title)), typeData(move(typeData)), locationType(locationType) {}

private:
    string id;
    string title;
    shared_ptr<ILocationTypeData> typeData;
    LocationType locationType;
};

class LocationTypeDataPhysical : public ILocationTypeData {
public:
    LocationTypeDataPhysical(double lat, double lon, string address)
        : latitude(lat), longitude(lon), address(move(address)) {}

private:
    double latitude;
    double longitude;
    string address;
};

class LocationTypeDataURL : public ILocationTypeData {
public:
    explicit LocationTypeDataURL(string url) : url(move(url)) {}

private:
    string url;
};

class Participant {
public:
    Participant(shared_ptr<User> user, ParticipantType type, RSVPStatus rsvpStatus)
        : user(move(user)), type(type), rsvpStatus(rsvpStatus) {}

    shared_ptr<User> getUser() const { return user; }

private:
    shared_ptr<User> user;
    ParticipantType type;
    RSVPStatus rsvpStatus;
};

class Event {
public:
    Event(string id, Slot slot, vector<Participant> participants,
          shared_ptr<Location> location, EventType type)
        : id(move(id)), slot(move(slot)), participants(move(participants)),
          location(move(location)), eventType(type) {}

    const Slot& getSlot() const { return slot; }

    bool hasUser(const shared_ptr<User>& user) const {
        for (const auto& p : participants) {
            if (*p.getUser() == *user) return true;
        }
        return false;
    }

private:
    string id;
    Slot slot;
    vector<Participant> participants;
    shared_ptr<Location> location;
    EventType eventType;
};

// INFRASTRUCTURE
class InMemoryEventRepository : public IEventRepository {
public:
    void save(const shared_ptr<Event>& event) override {
        events[event->getSlot().getStart()] = event;
    }

    shared_ptr<Event> get(const string& eventId) override {
        for (auto& [_, event] : events) {
            if (eventId == to_string(event->getSlot().getStart())) return event;
        }
        return nullptr;
    }

    vector<shared_ptr<Event>> getUserEvents(const shared_ptr<User>& user, long start, long end) override {
        vector<shared_ptr<Event>> result;
        for (const auto& [_, event] : events) {
            if (event->hasUser(user) && event->getSlot().between(start, end)) {
                result.push_back(event);
            }
        }
        return result;
    }

private:
    map<long, shared_ptr<Event>> events;
};

// SERVICES
class NotificationService {
public:
    void notify() {
        cout << "Notification sent.\n";
    }
};

class SlotService {
public:
    vector<Slot> getAllSlots(long start, long end, int increment, int duration) {
        vector<Slot> slots;
        for (long t = start; t + duration <= end; t += increment) {
            slots.emplace_back(t, t + duration);
        }
        return slots;
    }
};

class CalendarService {
public:
    CalendarService(shared_ptr<IEventRepository> repo,
                    shared_ptr<NotificationService> notif,
                    shared_ptr<SlotService> slotServ)
        : eventRepository(move(repo)),
          notificationService(move(notif)),
          slotService(move(slotServ)) {}

    void createEvent(const shared_ptr<Event>& event) {
        eventRepository->save(event);
        notificationService->notify();
    }

    shared_ptr<Event> getEvent(const string& eventId) {
        return eventRepository->get(eventId);
    }

    vector<Slot> getFreeSlots(const vector<shared_ptr<User>>& users, long start, long end,
                                   int increment, int duration) {
        auto allSlots = slotService->getAllSlots(start, end, increment, duration);

        for (const auto& user : users) {
            auto booked = getBookedSlots(user, start, end);
            vector<Slot> filtered;
            for (const auto& slot : allSlots) {
                bool overlaps = any_of(booked.begin(), booked.end(), [&](const Slot& b) {
                    return slot.doesOverlap(b);
                });
                if (!overlaps) filtered.push_back(slot);
            }
            allSlots = move(filtered);
        }
        return allSlots;
    }

private:
    vector<Slot> getBookedSlots(const shared_ptr<User>& user, long start, long end) {
        auto events = eventRepository->getUserEvents(user, start, end);
        vector<Slot> slots;
        for (const auto& e : events) slots.push_back(e->getSlot());
        return slots;
    }

    shared_ptr<IEventRepository> eventRepository;
    shared_ptr<NotificationService> notificationService;
    shared_ptr<SlotService> slotService;
};

// MAIN FUNCTION
int main() {
    auto user = make_shared<User>("u1");
    auto repo = make_shared<InMemoryEventRepository>();
    auto notifier = make_shared<NotificationService>();
    auto slotServ = make_shared<SlotService>();
    auto calendar = CalendarService(repo, notifier, slotServ);

    auto loc = make_shared<Location>("1", "Office", make_shared<LocationTypeDataPhysical>(1.1, 2.2, "123 Street"), LocationType::PHYSICAL);
    Slot slot(1000, 2000);
    vector<Participant> participants = { Participant(user, ParticipantType::OWNER, RSVPStatus::ACCEPT) };
    auto event = make_shared<Event>("e1", slot, participants, loc, EventType::MEETING);

    calendar.createEvent(event);
    auto freeSlots = calendar.getFreeSlots({ user }, 900, 3000, 500, 500);
    cout << "Free slots:\n";
    for (const auto& s : freeSlots) {
        cout << "Start: " << s.getStart() << ", End: " << s.getEnd() << "\n";
    }

    return 0;
}
