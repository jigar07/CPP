#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <ctime>
#include <cstdlib>
#include <algorithm>
#include <optional>
#include <memory>

// Enums
enum class TicketDomain { PAYMENT, MUTUAL_FUND, GOLD, INSURANCE, USER_REGISTRATION };
enum class TicketStatus { CREATED, IN_PROGRESS, COMPLETED };

// Models
class Agent {
public:
    std::string id;
    std::string name;
    std::vector<TicketDomain> domains;
    bool isOOO;
    std::string email;

    // STL containers like unordered_map require that the value type either:
    // Be default-constructible (i.e. has a no-argument constructor), or
    // Be inserted using a method like .emplace() that explicitly constructs the object with required arguments.
    //     default constructor or ticketToAgent[ticket.id] = agent; agentToTicket[agent.id]
    //                replaced by ticketToAgent.emplace(ticket.id, agent); agentToTicket.emplace(agent.id, ticket.id);

    Agent() = default;

    Agent(std::string id, std::string name, std::vector<TicketDomain> domains, bool isOOO, std::string email)
        : id(std::move(id)), name(std::move(name)), domains(std::move(domains)), isOOO(isOOO), email(std::move(email)) {}
};

class Customer {
public:
    std::string id;
    explicit Customer(std::string id) : id(std::move(id)) {}
};

class Ticket {
public:
    std::string id;
    std::string details;
    std::string title;
    std::vector<std::string> linkedReferenceIds;
    TicketDomain domain;
    TicketStatus status;
    std::time_t resolvedAt;

    // STL containers like unordered_map require that the value type either:
    // Be default-constructible (i.e. has a no-argument constructor), or
    // Be inserted using a method like .emplace() that explicitly constructs the object with required arguments.
    //     default constructor or tickets[ticket.id] = ticket; replaced by tickets.emplace(ticket.id, ticket);

    Ticket() = default;

    Ticket(std::string id, std::string details, std::string title, std::vector<std::string> linkedReferenceIds,
           TicketDomain domain, TicketStatus status)
        : id(std::move(id)), details(std::move(details)), title(std::move(title)),
          linkedReferenceIds(std::move(linkedReferenceIds)), domain(domain), status(status), resolvedAt(0) {}
};

// Interfaces
class ITicketRepository {
public:
    virtual ~ITicketRepository() = default;
    virtual Ticket save(const Ticket& ticket) = 0;
    virtual Ticket get(const std::string& ticketId) = 0;
    virtual void saveTicketAgent(const Ticket& ticket, const Agent& agent) = 0;
    virtual std::optional<Ticket> getTicketForAgent(const Agent& agent) = 0;
    virtual void markResolved(Ticket& ticket) = 0;
    virtual std::optional<Agent> getAgentForTicket(const Ticket& ticket) = 0;
};

class IAgentRepository {
public:
    virtual ~IAgentRepository() = default;
    virtual std::vector<Agent> getAllAgents() = 0;
};

// In-memory implementation
class InMemoryTicketRepository : public ITicketRepository {
private:
    std::unordered_map<std::string, Ticket> tickets;
    std::unordered_map<std::string, Agent> ticketToAgent;
    std::unordered_map<std::string, std::string> agentToTicket;

public:
    Ticket save(const Ticket& ticket) override {
        tickets[ticket.id] = ticket;
        return ticket;
    }

    Ticket get(const std::string& ticketId) override {
        return tickets.at(ticketId);
    }

    void saveTicketAgent(const Ticket& ticket, const Agent& agent) override {
        ticketToAgent[ticket.id] = agent;
        agentToTicket[agent.id] = ticket.id;
    }

    std::optional<Ticket> getTicketForAgent(const Agent& agent) override {
        if (agentToTicket.find(agent.id) != agentToTicket.end()) {
            std::string ticketId = agentToTicket[agent.id];
            return tickets[ticketId];
        }
        return std::nullopt;
    }

    void markResolved(Ticket& ticket) override {
        ticket.resolvedAt = std::time(nullptr);
        tickets[ticket.id] = ticket;
    }

    std::optional<Agent> getAgentForTicket(const Ticket& ticket) override {
        if (ticketToAgent.find(ticket.id) != ticketToAgent.end()) {
            return ticketToAgent[ticket.id];
        }
        return std::nullopt;
    }
};

class InMemoryAgentRepository : public IAgentRepository {
private:
    std::vector<Agent> agents;

public:
    InMemoryAgentRepository() {
        agents.emplace_back("1", "Alice", std::vector<TicketDomain>{TicketDomain::PAYMENT}, false, "alice@example.com");
        agents.emplace_back("2", "Bob", std::vector<TicketDomain>{TicketDomain::MUTUAL_FUND, TicketDomain::PAYMENT}, true, "bob@example.com");
        agents.emplace_back("3", "Charlie", std::vector<TicketDomain>{TicketDomain::PAYMENT}, false, "charlie@example.com");
    }

    std::vector<Agent> getAllAgents() override {
        return agents;
    }
};

// Strategies
class IPreferenceStrategy {
public:
    virtual ~IPreferenceStrategy() = default;
    virtual bool canSupport(const std::string& strategy) = 0;
    virtual Agent accept(const std::vector<Agent>& agents) = 0;
};

class RandomPreferenceStrategy : public IPreferenceStrategy {
public:
    bool canSupport(const std::string& strategy) override {
        return strategy == "random";
    }

    Agent accept(const std::vector<Agent>& agents) override {
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

    std::vector<Agent> getAllAgents() {
        return agentRepository.getAllAgents();
    }

    void addWorkLog(const Ticket& ticket, const Agent& agent) {
        std::cout << "Work log added for Agent: " << agent.name << " on Ticket ID: " << ticket.id << std::endl;
    }
};

class TicketService {
private:
    ITicketRepository& ticketRepo;
    AgentService& agentService;
    std::vector<std::shared_ptr<IPreferenceStrategy>> preferenceStrategies;

public:
    TicketService(ITicketRepository& ticketRepo, AgentService& agentService,
                  std::vector<std::shared_ptr<IPreferenceStrategy>> strategies)
        : ticketRepo(ticketRepo), agentService(agentService), preferenceStrategies(std::move(strategies)) {}

    Ticket createTicket(Ticket ticket) {
        ticket.id = std::to_string(rand());
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

    Agent assignTicket(const std::string& ticketId, const std::string& strategy) {
        Ticket ticket = ticketRepo.get(ticketId);
        auto allAgents = agentService.getAllAgents();
        std::vector<Agent> eligibleAgents;

        for (const auto& agent : allAgents) {
            bool supportsDomain = std::find(agent.domains.begin(), agent.domains.end(), ticket.domain) != agent.domains.end();
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

        throw std::runtime_error("No suitable strategy found or no eligible agents.");
    }
};

// Main
int main() {
    srand(static_cast<unsigned int>(time(nullptr)));

    InMemoryAgentRepository agentRepo;
    InMemoryTicketRepository ticketRepo;
    AgentService agentService(agentRepo);

    auto randomStrategy = std::make_shared<RandomPreferenceStrategy>();
    std::vector<std::shared_ptr<IPreferenceStrategy>> strategies = {randomStrategy};

    TicketService ticketService(ticketRepo, agentService, strategies);

    // Create ticket
    Ticket ticket("", "Fix payment issue", "Payment Failed", {}, TicketDomain::PAYMENT, TicketStatus::CREATED);
    Ticket createdTicket = ticketService.createTicket(ticket);
    std::cout << "Ticket created with ID: " << createdTicket.id << std::endl;

    // Assign agent
    try {
        Agent assigned = ticketService.assignTicket(createdTicket.id, "random");
        std::cout << "Assigned agent: " << assigned.name << std::endl;

        // Resolve ticket
        ticketService.resolveTicket(createdTicket);
        std::cout << "Ticket resolved." << std::endl;
    } catch (const std::exception& ex) {
        std::cerr << "Error assigning ticket: " << ex.what() << std::endl;
    }

    return 0;
}
