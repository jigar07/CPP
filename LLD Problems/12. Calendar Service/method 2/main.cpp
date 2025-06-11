// Following code is without using shared pointer
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <algorithm>
using namespace std;

// ENUMS
enum class EventType { MEETING, HOLIDAY, REMINDER, BIRTHDAY };
enum class LocationType { URL, PHYSICAL, MEETING_ROOM };
enum class ParticipantType { ADMIN, OWNER, GUEST, EDITOR, ATTENDANCE_MARKER, GUEST_SPL };
enum class RSVPStatus { ACCEPT, REJECT, UNKNOWN };

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
    virtual void save(const Event& event) = 0;
    virtual Event get(const string& eventId) = 0;
    virtual vector<Event> getUserEvents(const User& user, long start, long end) = 0;
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
    Location(string id, string title, ILocationTypeData* typeData, LocationType locationType)
        : id(move(id)), title(move(title)), typeData(typeData), locationType(locationType) {}

private:
    string id;
    string title;
    ILocationTypeData* typeData;  // raw pointer (assumes ownership managed elsewhere)
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
    Participant(const User& user, ParticipantType type, RSVPStatus rsvpStatus)
        : user(user), type(type), rsvpStatus(rsvpStatus) {}

    const User& getUser() const { return user; }

private:
    User user;
    ParticipantType type;
    RSVPStatus rsvpStatus;
};

class Event {
public:
    Event(string id, const Slot& slot, vector<Participant> participants,
          const Location& location, EventType type)
        : id(move(id)), slot(slot), participants(move(participants)),
          location(location), eventType(type) {}

    const Slot& getSlot() const { return slot; }

    bool hasUser(const User& user) const {
        for (const auto& p : participants) {
            if (p.getUser() == user) return true;
        }
        return false;
    }

private:
    string id;
    Slot slot;
    vector<Participant> participants;
    Location location;
    EventType eventType;
};

// INFRASTRUCTURE
class InMemoryEventRepository : public IEventRepository {
public:
    void save(const Event& event) override {
        // events[event.getSlot().getStart()] = event;
        events.emplace(event.getSlot().getStart(), event);
    }

    Event get(const string& eventId) override {
        for (const auto& [_, event] : events) {
            if (eventId == to_string(event.getSlot().getStart())) return event;
        }
        throw runtime_error("Event not found");
    }

    vector<Event> getUserEvents(const User& user, long start, long end) override {
        vector<Event> result;
        for (const auto& [_, event] : events) {
            if (event.hasUser(user) && event.getSlot().between(start, end)) {
                result.push_back(event);
            }
        }
        return result;
    }

private:
    map<long, Event> events;
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
    CalendarService(IEventRepository& repo, NotificationService& notif, SlotService& slotServ)
        : eventRepository(repo), notificationService(notif), slotService(slotServ) {}

    void createEvent(const Event& event) {
        eventRepository.save(event);
        notificationService.notify();
    }

    Event getEvent(const string& eventId) {
        return eventRepository.get(eventId);
    }

    vector<Slot> getFreeSlots(const vector<User>& users, long start, long end,
                                   int increment, int duration) {
        auto allSlots = slotService.getAllSlots(start, end, increment, duration);

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
    vector<Slot> getBookedSlots(const User& user, long start, long end) {
        auto events = eventRepository.getUserEvents(user, start, end);
        vector<Slot> slots;
        for (const auto& e : events) slots.push_back(e.getSlot());
        return slots;
    }

    IEventRepository& eventRepository;
    NotificationService& notificationService;
    SlotService& slotService;
};

// MAIN FUNCTION
int main() {
    User user("u1");

    LocationTypeDataPhysical locData(12.3, 45.6, "123 Main St");
    Location location("1", "Office", &locData, LocationType::PHYSICAL);

    Slot slot(1000, 2000);
    vector<Participant> participants = { Participant(user, ParticipantType::OWNER, RSVPStatus::ACCEPT) };
    Event event("e1", slot, participants, location, EventType::MEETING);

    InMemoryEventRepository repo;
    NotificationService notifier;
    SlotService slotService;
    CalendarService calendar(repo, notifier, slotService);

    calendar.createEvent(event);

    auto freeSlots = calendar.getFreeSlots({ user }, 900, 3000, 500, 500);
    cout << "Free slots:\n";
    for (const auto& s : freeSlots) {
        cout << "Start: " << s.getStart() << ", End: " << s.getEnd() << "\n";
    }

    return 0;
}
