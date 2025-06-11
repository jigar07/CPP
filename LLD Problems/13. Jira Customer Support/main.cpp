#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <ctime>
#include <cstdlib>
#include <algorithm>
#include <optional>
#include <memory>
using namespace std;

// Enums
enum class TicketDomain { PAYMENT, MUTUAL_FUND, GOLD, INSURANCE, USER_REGISTRATION };
enum class TicketStatus { CREATED, IN_PROGRESS, COMPLETED };

// Models
class Agent {
public:
    string id;
    string name;
    vector<TicketDomain> domains;
    bool isOOO;
    string email;

    // STL containers like unordered_map require that the value type either:
    // Be default-constructible (i.e. has a no-argument constructor), or
    // Be inserted using a method like .emplace() that explicitly constructs the object with required arguments.
    //     default constructor or ticketToAgent[ticket.id] = agent; agentToTicket[agent.id]
    //                replaced by ticketToAgent.emplace(ticket.id, agent); agentToTicket.emplace(agent.id, ticket.id);

    Agent() = default;

    Agent(string id, string name, vector<TicketDomain> domains, bool isOOO, string email)
        : id(move(id)), name(move(name)), domains(move(domains)), isOOO(isOOO), email(move(email)) {}
};

class Customer {
public:
    string id;
    explicit Customer(string id) : id(move(id)) {}
};

class Ticket {
public:
    string id;
    string details;
    string title;
    vector<string> linkedReferenceIds;
    TicketDomain domain;
    TicketStatus status;
    time_t resolvedAt;

    // STL containers like unordered_map require that the value type either:
    // Be default-constructible (i.e. has a no-argument constructor), or
    // Be inserted using a method like .emplace() that explicitly constructs the object with required arguments.
    //     default constructor or tickets[ticket.id] = ticket; replaced by tickets.emplace(ticket.id, ticket);

    Ticket() = default;

    Ticket(string id, string details, string title, vector<string> linkedReferenceIds,
           TicketDomain domain, TicketStatus status)
        : id(move(id)), details(move(details)), title(move(title)),
          linkedReferenceIds(move(linkedReferenceIds)), domain(domain), status(status), resolvedAt(0) {}
};

// Interfaces
class ITicketRepository {
public:
    virtual ~ITicketRepository() = default;
    virtual Ticket save(const Ticket& ticket) = 0;
    virtual Ticket get(const string& ticketId) = 0;
    virtual void saveTicketAgent(const Ticket& ticket, const Agent& agent) = 0;
    virtual optional<Ticket> getTicketForAgent(const Agent& agent) = 0;
    virtual void markResolved(Ticket& ticket) = 0;
    virtual optional<Agent> getAgentForTicket(const Ticket& ticket) = 0;
};

class IAgentRepository {
public:
    virtual ~IAgentRepository() = default;
    virtual vector<Agent> getAllAgents() = 0;
};

// In-memory implementation
class InMemoryTicketRepository : public ITicketRepository {
private:
    unordered_map<string, Ticket> tickets;
    unordered_map<string, Agent> ticketToAgent;
    unordered_map<string, string> agentToTicket;

public:
    Ticket save(const Ticket& ticket) override {
        tickets[ticket.id] = ticket;
        return ticket;
    }

    Ticket get(const string& ticketId) override {
        return tickets.at(ticketId);
    }

    void saveTicketAgent(const Ticket& ticket, const Agent& agent) override {
        ticketToAgent[ticket.id] = agent;
        agentToTicket[agent.id] = ticket.id;
    }

    optional<Ticket> getTicketForAgent(const Agent& agent) override {
        if (agentToTicket.find(agent.id) != agentToTicket.end()) {
            string ticketId = agentToTicket[agent.id];
            return tickets[ticketId];
        }
        return nullopt;
    }

    void markResolved(Ticket& ticket) override {
        ticket.resolvedAt = time(nullptr);
        tickets[ticket.id] = ticket;
    }

    optional<Agent> getAgentForTicket(const Ticket& ticket) override {
        if (ticketToAgent.find(ticket.id) != ticketToAgent.end()) {
            return ticketToAgent[ticket.id];
        }
        return nullopt;
    }
};

class InMemoryAgentRepository : public IAgentRepository {
private:
    vector<Agent> agents;

public:
    InMemoryAgentRepository() {
        agents.emplace_back("1", "Alice", vector<TicketDomain>{TicketDomain::PAYMENT}, false, "alice@example.com");
        agents.emplace_back("2", "Bob", vector<TicketDomain>{TicketDomain::MUTUAL_FUND, TicketDomain::PAYMENT}, true, "bob@example.com");
        agents.emplace_back("3", "Charlie", vector<TicketDomain>{TicketDomain::PAYMENT}, false, "charlie@example.com");
    }

    vector<Agent> getAllAgents() override {
        return agents;
    }
};

// Strategies
class IPreferenceStrategy {
public:
    virtual ~IPreferenceStrategy() = default;
    virtual bool canSupport(const string& strategy) = 0;
    virtual Agent accept(const vector<Agent>& agents) = 0;
};

class RandomPreferenceStrategy : public IPreferenceStrategy {
public:
    bool canSupport(const string& strategy) override {
        return strategy == "random";
    }

    Agent accept(const vector<Agent>& agents) override {
        int randIndex = rand() % agents.size();
        return agents[randIndex];
    }
};

// Services
class AgentService {
private:
    IAgentRepository& agentRepository;

public:
    explicit AgentService(IAgentRepository& repo) : agentRepository(repo) {}

    vector<Agent> getAllAgents() {
        return agentRepository.getAllAgents();
    }

    void addWorkLog(const Ticket& ticket, const Agent& agent) {
        cout << "Work log added for Agent: " << agent.name << " on Ticket ID: " << ticket.id << endl;
    }
};

class TicketService {
private:
    ITicketRepository& ticketRepo;
    AgentService& agentService;
    vector<shared_ptr<IPreferenceStrategy>> preferenceStrategies;

public:
    TicketService(ITicketRepository& ticketRepo, AgentService& agentService,
                  vector<shared_ptr<IPreferenceStrategy>> strategies)
        : ticketRepo(ticketRepo), agentService(agentService), preferenceStrategies(move(strategies)) {}

    Ticket createTicket(Ticket ticket) {
        ticket.id = to_string(rand());
        return ticketRepo.save(ticket);
    }

    Ticket resolveTicket(Ticket ticket) {
        ticketRepo.markResolved(ticket);
        auto agentOpt = ticketRepo.getAgentForTicket(ticket);
        if (agentOpt) {
            agentService.addWorkLog(ticket, *agentOpt);
        }
        return ticket;
    }

    Agent assignTicket(const string& ticketId, const string& strategy) {
        Ticket ticket = ticketRepo.get(ticketId);
        auto allAgents = agentService.getAllAgents();
        vector<Agent> eligibleAgents;

        for (const auto& agent : allAgents) {
            bool supportsDomain = find(agent.domains.begin(), agent.domains.end(), ticket.domain) != agent.domains.end();
            bool notOOO = !agent.isOOO;
            bool hasTicket = ticketRepo.getTicketForAgent(agent).has_value();
            if (supportsDomain && notOOO && !hasTicket) {
                eligibleAgents.push_back(agent);
            }
        }

        for (const auto& pref : preferenceStrategies) {
            if (pref->canSupport(strategy)) {
                Agent selected = pref->accept(eligibleAgents);
                ticketRepo.saveTicketAgent(ticket, selected);
                return selected;
            }
        }

        throw runtime_error("No suitable strategy found or no eligible agents.");
    }
};

// Main
int main() {
    srand(static_cast<unsigned int>(time(nullptr)));

    InMemoryAgentRepository agentRepo;
    InMemoryTicketRepository ticketRepo;
    AgentService agentService(agentRepo);

    auto randomStrategy = make_shared<RandomPreferenceStrategy>();
    vector<shared_ptr<IPreferenceStrategy>> strategies = {randomStrategy};

    TicketService ticketService(ticketRepo, agentService, strategies);

    // Create ticket
    Ticket ticket("", "Fix payment issue", "Payment Failed", {}, TicketDomain::PAYMENT, TicketStatus::CREATED);
    Ticket createdTicket = ticketService.createTicket(ticket);
    cout << "Ticket created with ID: " << createdTicket.id << endl;

    // Assign agent
    try {
        Agent assigned = ticketService.assignTicket(createdTicket.id, "random");
        cout << "Assigned agent: " << assigned.name << endl;

        // Resolve ticket
        ticketService.resolveTicket(createdTicket);
        cout << "Ticket resolved." << endl;
    } catch (const exception& ex) {
        cerr << "Error assigning ticket: " << ex.what() << endl;
    }

    return 0;
}
