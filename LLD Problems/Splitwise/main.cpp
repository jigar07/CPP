#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <map>
#include <stdexcept>
#include <iomanip>
using namespace std;

// ---------- Enums ----------
enum class ExpenseSplitType {
    EQUAL,
    EXACT,
    PERCENT
};

enum class CurrencyType {
    INR
};

// ---------- Currency ----------
class Currency {
public:
    CurrencyType type;
    string label;
    string symbol;

    Currency() : type(CurrencyType::INR), label("Indian Rupee"), symbol("₹") {}
};

// ---------- Amount ----------
class Amount {
public:
    double value;
    Currency currency;

    Amount() : value(0.0), currency() {}
    Amount(double v) : value(v), currency() {}
    Amount(double v, Currency c) : value(v), currency(c) {}

    Amount operator/(double divisor) const {
        return Amount(value / divisor, currency);
    }

    Amount operator+(const Amount& other) const {
        return Amount(value + other.value, currency);
    }

    Amount operator-(const Amount& other) const {
        return Amount(value - other.value, currency);
    }


    void print() const {
        cout << fixed << setprecision(2) << currency.symbol << value;
    }

    static Amount ZERO() {
        return Amount(0.0);
    }
};

// ---------- User ----------
class User {
public:
    string id;
    User() {}
    User(string uid) : id(uid) {}

    bool operator==(const User& other) const {
        return id == other.id;
    }

    bool operator!=(const User& other) const {
        return id != other.id;
    }
};

// ---------- SplitParticipant ----------
class SplitParticipant {
public:
    User participant;
    Amount share;

    SplitParticipant() {}
    SplitParticipant(User u, Amount a) : participant(u), share(a) {}
};

// ---------- Expense ----------
class Expense {
public:
    Amount totalAmount;
    User payer;
    ExpenseSplitType splitType;
    vector<User> participants;
    vector<Amount> exactParticipantsAmount;
    vector<double> exactPercents;
    string desc;

    Expense() {}
    Expense(Amount amt, User p, ExpenseSplitType type, vector<User> parts)
        : totalAmount(amt), payer(p), splitType(type), participants(parts) {}
};

// ---------- ExpenseStorage ----------
class ExpenseStorage {
private:
    vector<Expense> expenses;

public:
    void add(const Expense& expense) {
        expenses.push_back(expense);
    }

    const vector<Expense>& getExpenses() const {
        return expenses;
    }
};

// ---------- ISplitStrategy ----------
class ISplitStrategy {
public:
    virtual bool doesSupport(ExpenseSplitType splitType) = 0;
    virtual void processSplitData(Expense& expense, const vector<string>& params) = 0;
    virtual vector<SplitParticipant> calculateSplitParticipants(const Expense& expense) = 0;
};

// ---------- EqualSplitStrategy ----------
class EqualSplitStrategy : public ISplitStrategy {
public:
    bool doesSupport(ExpenseSplitType splitType) override {
        return splitType == ExpenseSplitType::EQUAL;
    }

    void processSplitData(Expense&, const vector<string>&) override {
        // No extra processing needed for EQUAL
    }

    vector<SplitParticipant> calculateSplitParticipants(const Expense& expense) override {
        vector<SplitParticipant> result;
        Amount share = expense.totalAmount / expense.participants.size();
        for (const auto& user : expense.participants) {
            result.emplace_back(user, share);
        }
        return result;
    }
};

// ---------- ExactSplitStrategy ----------
class ExactSplitStrategy : public ISplitStrategy {
public:
    bool doesSupport(ExpenseSplitType splitType) override {
        return splitType == ExpenseSplitType::EXACT;
    }

    void processSplitData(Expense& expense, const vector<string>& params) override {
        for (const auto& str : params) {
            expense.exactParticipantsAmount.emplace_back(stod(str)); // stod: string to double
        }
    }

    vector<SplitParticipant> calculateSplitParticipants(const Expense& expense) override {
        vector<SplitParticipant> result;
        for (size_t i = 0; i < expense.participants.size(); ++i) {
            result.emplace_back(expense.participants[i], expense.exactParticipantsAmount[i]);
        }
        return result;
    }
};

// ---------- PercentSplitStrategy ----------
class PercentSplitStrategy : public ISplitStrategy {
public:
    bool doesSupport(ExpenseSplitType splitType) override {
        return splitType == ExpenseSplitType::PERCENT;
    }

    void processSplitData(Expense& expense, const vector<string>& params) override {
        for (const auto& str : params) {
            expense.exactPercents.push_back(stod(str));
        }
    }

    vector<SplitParticipant> calculateSplitParticipants(const Expense& expense) override {
        vector<SplitParticipant> result;
        for (size_t i = 0; i < expense.participants.size(); ++i) {
            double percent = expense.exactPercents[i];
            Amount share = Amount((expense.totalAmount.value * percent) / 100.0);
            result.emplace_back(expense.participants[i], share);
        }
        return result;
    }
};

// ---------- ICommandHandler ----------
class ICommandHandler {
public:
    virtual void handleCommand(const string& name, const vector<string>& params) = 0;
    virtual bool doesSupport(const string& name) = 0;
};

// ---------- ExpenseCommandHandler ----------
class ExpenseCommandHandler : public ICommandHandler {
private:
    ExpenseStorage& storage;
    vector<ISplitStrategy*> strategies;

public:
    ExpenseCommandHandler(ExpenseStorage& s) : storage(s) {
        strategies.push_back(new EqualSplitStrategy());
        strategies.push_back(new ExactSplitStrategy());
        strategies.push_back(new PercentSplitStrategy());
    }

    bool doesSupport(const string& name) override {
        return name == "EXPENSE";
    }

    void handleCommand(const string&, const vector<string>& params) override {
        string payerId = params[0];
        double totalAmount = stod(params[1]);
        int numParticipants = stoi(params[2]);

        vector<User> participants;
        for (int i = 0; i < numParticipants; ++i) {
            participants.emplace_back(params[3 + i]);
        }

        string splitTypeStr = params[3 + numParticipants];
        ExpenseSplitType splitType;
        if (splitTypeStr == "EQUAL") splitType = ExpenseSplitType::EQUAL;
        else if (splitTypeStr == "EXACT") splitType = ExpenseSplitType::EXACT;
        else if (splitTypeStr == "PERCENT") splitType = ExpenseSplitType::PERCENT;
        else throw invalid_argument("Invalid split type");

        Expense expense(Amount(totalAmount), User(payerId), splitType, participants);

        ISplitStrategy* splitStrategy = nullptr;
        for (auto* s : strategies) {
            if (s->doesSupport(splitType)) {
                splitStrategy = s;
                break;
            }
        }
        if (!splitStrategy) throw runtime_error("No strategy found");

        vector<string> splitData(params.begin() + 4 + numParticipants, params.end());
        splitStrategy->processSplitData(expense, splitData);

        storage.add(expense);
    }
};

// ---------- ShowCommandHandler ----------
class ShowCommandHandler : public ICommandHandler {
private:
    ExpenseStorage& storage;
    vector<ISplitStrategy*> strategies;

public:
    ShowCommandHandler(ExpenseStorage& s) : storage(s) {
        strategies.push_back(new EqualSplitStrategy());
        strategies.push_back(new ExactSplitStrategy());
        strategies.push_back(new PercentSplitStrategy());
    }

    bool doesSupport(const string& name) override {
        return name == "SHOW";
    }

    void handleCommand(const string&, const vector<string>& params) override {
        if (storage.getExpenses().empty()) {
            cout << "No balances" << endl;
            return;  // Exit early if there are no expenses
        }

        // If no user is specified, we print balances for all users
        if (params.empty()) {
            map<pair<string, string>, Amount> balances;  // Track balances for each user

            // Iterate over all expenses
            for (const auto& expense : storage.getExpenses()) {
                ISplitStrategy* strategy = nullptr;
                for (auto* s : strategies) {
                    if (s->doesSupport(expense.splitType)) {
                        strategy = s;
                        break;
                    }
                }

                if (!strategy) {
                    throw std::runtime_error("No split strategy found for expense");
                }

                // Calculate split details (how much each participant owes)
                vector<SplitParticipant> splits = strategy->calculateSplitParticipants(expense);

                // Calculate what each participant owes or is owed
                for (const auto& sp : splits) {
                    balances[{expense.payer.id, sp.participant.id}] = balances[{expense.payer.id, sp.participant.id}] - sp.share;
                    balances[{sp.participant.id, expense.payer.id,}] = balances[{sp.participant.id, expense.payer.id,}] + sp.share;
                }
            }

            // Print balances for all users
            bool foundBalance = false;
            for (const auto& balance : balances) {
                if (balance.second.value != 0.0) {
                    foundBalance = true;
                    cout << balance.first.first << " owes " << balance.first.second<< ": "<<balance.second.value<<endl;
                }
            }

            if (!foundBalance) {
                cout << "No balances" << endl;
            }
        } else {
            // If a user is specified, print only their balance
            string userId = params[0];
            map<string, Amount> balances;  // Track balances for each user

            // Iterate over all expenses
            for (const auto& expense : storage.getExpenses()) {
                ISplitStrategy* strategy = nullptr;
                for (auto* s : strategies) {
                    if (s->doesSupport(expense.splitType)) {
                        strategy = s;
                        break;
                    }
                }

                if (!strategy) {
                    throw std::runtime_error("No split strategy found for expense");
                }

                // Calculate split details (how much each participant owes)
                vector<SplitParticipant> splits = strategy->calculateSplitParticipants(expense);

                // Calculate what each participant owes or is owed
                for (const auto& sp : splits) {
                    // If the user is the payer, subtract their share
                    if (expense.payer.id == userId) {
                        if (sp.participant.id != userId) {
                            balances[sp.participant.id] = balances[sp.participant.id] - sp.share;
                        } 
                    } else {
                        if (sp.participant.id == userId) {
                            balances[expense.payer.id] = balances[expense.payer.id] + sp.share;
                        }
                    }
                }
            }

            // Print the balance for the specific user
            bool foundBalance = false;
            for (const auto& balance : balances) {
                if (balance.second.value != 0.0) {
                    foundBalance = true;
                    cout << userId << " owes " << balance.first <<": "<<balance.second.value<< endl;
                }
            }

            if (!foundBalance) {
                cout << "No balances" << endl;
            }
        }
    }
};


// ---------- CommandLineManager ----------
class CommandLineManager {
private:
    vector<ICommandHandler*> handlers;

public:
    void registerHandler(ICommandHandler* handler) {
        handlers.push_back(handler);
    }

    void execute(const string& command) {
        istringstream ss(command);
        string cmd;
        ss >> cmd;

        vector<string> params;
        string param;
        while (ss >> param) {
            params.push_back(param);
        }

        for (auto* h : handlers) {
            if (h->doesSupport(cmd)) {
                h->handleCommand(cmd, params);
                return;
            }
        }

        cerr << "Unsupported command: " << cmd << endl;
    }
};

// ---------- Main ----------
int main() {
    ExpenseStorage storage;
    ExpenseCommandHandler ech(storage);
    ShowCommandHandler sch(storage);
    CommandLineManager clm;

    clm.registerHandler(&ech);
    clm.registerHandler(&sch);

    // Example commands
    clm.execute("EXPENSE user1 1000 4 user1 user2 user3 user4 EQUAL");
    clm.execute("EXPENSE user1 1250 2 user2 user3 EXACT 370 880");
    clm.execute("EXPENSE user2 1000 4 user1 user2 user3 user4 PERCENT 10 20 30 40");

    cout<<"-------------------"<<endl;
    cout<<"Show user1 expenses"<<endl;
    cout<<"-------------------"<<endl;
    clm.execute("SHOW user1");
    cout<<"-------------------"<<endl;
    cout<<"Show user2 expenses"<<endl;
    cout<<"-------------------"<<endl;
    clm.execute("SHOW user2");
    cout<<"-------------------"<<endl;
    cout<<"Show user3 expenses"<<endl;
    cout<<"-------------------"<<endl;
    clm.execute("SHOW user3");
    cout<<"-------------------"<<endl;
    cout<<"Show All User expenses"<<endl;
    cout<<"-------------------"<<endl;
    clm.execute("SHOW");

    return 0;
}
