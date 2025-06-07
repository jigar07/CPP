LLD Cohort January - 2024

Splitwise: https://enginebogie.com/public/question/design-expense-sharing-app-like-splitwise/327


#### Sample Input:

```
SHOW 
SHOW user1 
EXPENSE user1 1000 4 user1 user2 user3 user4 EQUAL 
SHOW user4 
SHOW user1 
EXPENSE user1 1250 2 user2 user3 EXACT 370 880 
SHOW 
SHOW user1 
SHOW
```

#### Sample Output:
```
No balances 
No balances 
User4 owes User1: 250 
User2 owes User1: 250 
User3 owes User1: 250 
User4 owes User1: 250 
User2 owes User1: 620 
User3 owes User1: 1130 
User4 owes User1: 250 
User1 owes User4: 230 
User2 owes User1: 620 
User3 owes User1: 1130 
User1 owes User4: 230 
User2 owes User1: 620 
User2 owes User4: 240 
User3 owes User1: 1130 
User3 owes User4: 240 
```

#### Requirements:
User: Each user should have a userId, name, email, mobile number.
Expense: Could either be EQUAL, EXACT
Users can add any amount, select any type of expense, and split with any of the available users.
In the case of exact, you need to verify if the total sum of shares is equal to the total amount or not.
The application should have the capability to show expenses for a single user as well as balances for everyone.
When asked to show balances, the application should show the balances of a user with all the users where there is a non-zero balance.
The amount should be rounded off to two decimal places. Say if User1 paid 100 and amount is split equally among 3 people. Assign 33.34 to first-person and 33.33 to others.

#### Solution


```java
class User {
	private String id;
	private String name;
	private String email;
	private String phone;
	// other metadata properties
}

enum ExpenseSplitType {
	EQUAL,
	EXACT
}

enum CurrencyType {
	INR,
}

class Currency {
	CurrencyType type;
	String label; //"Indian Rupee"
	String symbol;

}

class Amount {
	private Double value;
	private Currency currency;
}

class SplitParticipant {

	private User participant;
	private Amount share;
}

class Expense {
	private Amount totalAmount;
	User payer;
	// private List<User> participants;
	private ExpenseSplitType splitType;
	private List<User> participants;
	private List<Amount> exactParticipantsAmount;
	private List<Double> exactPercents

	// private List<SplitParticipant> splits;

	private String desc;
	// Other metadata properties
}

class ExpenseStorage {

	private List<Expense> expenses;

	public void add(Expense expense) {
		expenses.add(expense);
	}
}

interface IExpenseParser {
	public boolean canHandle(String input);
	public Expense parse(String input);
}

class StringExpenseParser implements IExpenseParser {
	public boolean canHandle(String input) {

	}

	public Expense parse(String input) {
		String payerId = commandParams[0];
		String totalAmount = commandParams[1];
		Integer numberOfParticipants = Integer.parseInt(commandParams[2]);
		List<SplitParticipant> participants = new ArrayList<>();
		for (int i = 0; i < numberOfParticipants; i++) {
			String payerId = commandParams[3 + i];
			participants.add(new User(payerId));
		}
		ExpenseSplitType splitType = ExpenseSplitType.valueOf(commandParams[3 + i]);
		if (splitType == EXACT) {
			for (int j = 0; j < numberOfParticipants; j++) {
				String participantShare = commandParams[3 + i + 1 + j];
				participants.get(j).setShare(participantShare)
			}	
		} else {
			for (int j = 0; j < numberOfParticipants; j++) {
				participants.get(j).setShare(totalAmount/numberOfParticipants)
			}	
		}
		Expense expense = new Expense(totalAmount, new User(payerId), participants);

	}
}


class JsonExpenseParser implements IExpenseParser {
	public boolean canHandle(String input) {

	}

	public Expense parse(String input) {
		Json inputJson = JsonParser.parse(input);			
		String payerId = inputJson["payerId"];
		String totalAmount = inputJson[1];
		Integer numberOfParticipants = Integer.parseInt(inputJson[2]);
		List<SplitParticipant> participants = new ArrayList<>();
		for (int i = 0; i < numberOfParticipants; i++) {
			String payerId = inputJson[3 + i];
			participants.add(new User(payerId));
		}
		ExpenseSplitType splitType = ExpenseSplitType.valueOf(inputJson[3 + i]);
		if (splitType == EXACT) {
			for (int j = 0; j < numberOfParticipants; j++) {
				String participantShare = inputJson[3 + i + 1 + j];
				participants.get(j).setShare(participantShare)
			}	
		} else {
			for (int j = 0; j < numberOfParticipants; j++) {
				participants.get(j).setShare(totalAmount/numberOfParticipants)
			}	
		}
		Expense expense = new Expense(totalAmount, new User(payerId), participants);

	}
}

interface ICommandHandler {
	void handleCommand(String commandName, String[] commandParams);
	boolean doesSupport(String commandName);
}

class ExpenseCommandHandler implements ICommandHandler {
	private ExpenseStorage storage;
	private ISplitTypeParamsParser splitTypeParamsParsers;

	boolean doesSupport(String commandName) {
		return commandName.equals("EXPENSE");
	}

	public void handleCommand(String commandName, String[] commandParams) {
		String payerId = commandParams[0];
		String totalAmount = commandParams[1];
		Integer numberOfParticipants = Integer.parseInt(commandParams[2]);
		List<Uer> participants = new ArrayList<>();
		for (int i = 0; i < numberOfParticipants; i++) {
			String payerId = commandParams[3 + i];
			participants.add(new User(payerId));
		}
		ExpenseSplitType splitType = ExpenseSplitType.valueOf(commandParams[3 + i]);

		Expense expense = new Expense(totalAmount, new User(payerId), splitType, participants, null, null);
		
		ISplitStrategy strategy = null;
		for (ISplitStrategy ss: strategies) {
			if (ss.doesSupport(expense.splitType)) {
				strategy = ss;
				break;
			}
		}

		if (strategy == null) {
			throw new RuntimeException("Unsupported Split Type");
		}
		strategy.someMethod(expense, commandParams[3 + i to last]);
		strategy.verify(expense);
		storage.add(expense);
	}
}

interface ISplitStrategy {
	boolean doesSupport(ExpenseSplitType splitType);
	void someMethod(Expense expense, String splitTypeParams);
	public List<SplitParticipant> calculateSplitParticipants(Expense expense);
}

class EqualSplitStrategy implements ISplitStrategy {
	boolean doesSupport(ExpenseSplitType splitType) {
		return splitType == EQUAL;
	}

	public List<SplitParticipant> calculateSplitParticipants(Expense expense) {
		final Amount totalAmount = expense.getTotalAmount();
		return expense.getParticipants().stream().map(participant -> new SplitParticipant(participant, totalAmount/participants/size())).collect(Collectors:toList);
	}

	void someMethod(Expense expense, String splitTypeParams) {
		// No op
	}
}

class ExactSplitStrategy implements ISplitStrategy {
	public List<SplitParticipant> calculateSplitParticipants(Expense expense) {
		return // merge participants and exactParticipants amount
	}

	void someMethod(Expense expense, String splitTypeParams) {
		List<Amount> exactParticipantsAmount = new ArrayList<>();
		for (int j = 0; j < numberOfParticipants; j++) {
			String participantShare = splitTypeParams[j];
			exactParticipantsAmount.add(participantShare)
		}
		expense.setExactParticipantsAmount(exactParticipantsAmount);
	}

}

class PercentSplitStrategy implements ISplitStrategy {
	public List<SplitParticipant> calculateSplitParticipants(Expense expense) {
		return // merge participants and exactParticipants amount
	}

	void someMethod(Expense expense, String splitTypeParams) {
		List<Amount> exactPercents = new ArrayList<>();
		for (int j = 0; j < numberOfParticipants; j++) {
			Double percent = PercentParser.parse(splitTypeParams[j]);
			exactPercents.add(participantShare)
		}	
		expense.setExactPercents(exactPercents);
	}

}

class ShowCommandHandler implements ICommandHandler {
	private ExpenseStorage storage;

	boolean doesSupport(String commandName) {
		return commandName.equals("SHOW");
	}

	public void handleCommand(String commandName, String[] commandParams) {
		String userId = commandParams[0];
		Amount result = Amount.ZERO;
		for (Expense expense: storage.getExpenses()) {
			
			ISplitStrategy strategy = null;
			for (ISplitStrategy ss: strategies) {
				if (ss.doesSupport(expense.splitType)) {
					strategy = ss;
					break;
				}
			}

			if (strategy == null) {
				throw new RuntimeException("Unsupported Split Type");
			}

			List<SplitParticipant> splitParticipants = strategy.calculateSplitParticipants(expense);
			for (SplitParticipant sp: splitParticipants) {
				if (sp.participant.equals(userId)) {
					result += sp.share;
				}
			}
		}
		print result
	}
}


class CommanLineManager {

	// EXPENSE user1 1000 4 user1 user2 user3 user4 EQUAL
	// EXPENSE user1 1250 2 user2 user3 EXACT 370 880 
	// EXPENSE user1 1000 4 user1 user2 user3 user4 DOUBLEEQUAL
	// EXPENSE user1 1000 4 user1 user2 user3 user4 Percent 10% 20%

	// u2 owes 250 u1
	// u2 owes 370 u1
	// Map<String, Amount> sums; 
	//		user1	100
	//		user2	620
	// - fastest
	// - Memory complexity
	// - addition is slowed down
	// - maintainance of extra data
	// - original expense is lost

	// Map<String, <String, Amount>> sums; 
	//		user2	user1	620
	//		user2	user5	100
	//		user3	user1	1130
	// - faster

	// List<Expense> allExpenses - slower - because we have to iterate through all the expenses
	// 

	List<ISplitStrategy> strategies;
	List<ICommandHandler> chs;
	String commandInputDelimiter;

	public void execute(String commandInput) {
		final String[] inputParts = commandInput.split(commandInputDelimiter);
		// validate the input format.

		String commandName = inputParts[0];
		String commandParams = inputParts[1-inputParts.size()];

		ICommandHandler chToUse = null;
		for (ICommandHandler ch: chs) {
			if (ch.doesSupport(commandName)) {
				chToUse = ch;
				break;
			}
		}

		if (chToUse == null) {
			throw new RuntimeException("Unsupported Command");
		}

		chToUse.handleCommand(commandName, commandParams);
	}
}



```
#### Problems
- duplication of code if json parser is introduced. - solved
	- Creating an interface for input handler
- adding a new split type. - ocp - solved
- adding a new type of command. - ocp - solved.
- adding new split type with its own different parsing logic, will need change in parser. - ocp

- input params delimiter - ocp
- More than one payees (Can call out to interviwer that this won't be supported if it is not in requirement. This might lead to lot of change in design and it will be complex to implement in an hour)

# Design
## Classes (entity)
- User
- ExpenseSplitType
- Expense
- Amount, Currency, CurrencyType
- What will be entry point? (Interact through API, or command, or command line, or API)
  - Here input will be command line.(In the problem statement it was given)
  - If in requirement it is asking for API then implement using API as main method. If command line in requirement then CommanLineManager
    - Expense command: EXACT and EQUAL
      - StringExpenseParser::parse method part of CommandLineManager initially. (StringExpenseParser class not created yet)
    - Now, we need to calculate who owes how much to whom. This can be done after 1/ creating Expense object, or 2/ while running show command(In which will go over all the expense one by one)
      - Storing Expense object would be better choice if you required faster solution while running show command. However, it requires extra space, slower query while adding expense, maintenance is complex. (For LLD this should be not discussed, it should be pass of HLD. Hence, we will go with getting user expense while running SHOW command only by traversing object). Also, with first solution we are losing original data also(To save this we can store this also. All those are part of HLD as latency comes into picture). If there is requirement of low latency during show command then might need to go with first approach.
      - Whichever approach you go call out to interviewer and ask whether he/she agrees or not
      - So, we are going with approach 2 and ExpenseStorage is created with using list
- Extensibility with new requirements:
  - duplication of code if json parser is introduced.
    - IExpenseParser created. But this might create duplication of code if business logic of parsing and split strategy (which create Expense object) are added in IExpenseParse. We are seeing this issue because we are creating Expense object during input command only.
    - To solve this we will remove `private List<SplitParticipant> splits;` from Expense class and ExpenseSplitType, List<User> and List<Amount> added in this class. (For Equal input type, List<Amount> will be sent as empty for the Expense object). calculateSplitParticipants method inside Expense class.
      - However, this is also not exntesible. To solve which ISplitStrategy interface added
  - adding a new split type.
    - ISplitStrategy solves adding a new split type
  - Adding a new type of command
    - Use command pattern. (Command pattern uses strategy pattern internally)
    - Interface ICommandHandler added
    - List<ICommandHandler> chs; and for loop on chs added
  - adding new split type with its own different parsing logic, will need change in parser
    - PercentSplitStrategy can be added for split type
    - How about parsing logic in ExpenseCommandHandler?
      - There could be splitType EXACT or PERCENT in ExpenseCommandHandler
      - To solve this inside ExpenseCommandHandler, ISplitStrategy is used
      - **ISplitStrategy added in ExpenseCommandHandler (Initially ISplitTypeParamsParser added but then ISplitStrategy was used. ISplitTypeParamsParser removed to avoid duplication. Also, it my lead to runtime error because in future people might add in one of the interface (ISplitTypeParamsParser) and forgets to add in another type of interface (ISplitStrategy) and causing run time error). This is reverse ISP. We splited interface into many interface so that we ran into run time error. To solve this we need to keep only one interface**
        - Udit has never read about reverse ISP. He figure this out while working on engine boogie
        - If conditions are same then keep into one interface only

- If in china delimiter is different then it make sense to add delimiter interace. It can be based on something else as well. **So, whenever there is a behavior difference, same thing can happen different way then we should use interface**. So, instead of SpaceDelimiter, we can add CountryBaseDelimiter or LaungaugeBaseDelimiter.

## Other
- Amount, Country etc are always composite values
  - Amount is composition of value and type of currency. Hence, Amount class added
  - Country is composition of display name, currency etc
  - Also, don't think much about this. Try to make it simpler. But as it came to mind better to add it and changes are minimal here
  - Keep moving further. In case you can't think of better solution go with simpler solution. Then later point of time think of making soultion better
- Goal should be complete and then analyze. Just note down whichever is not fulfilling OCP and then improve that later
- Whenever you have options check pros and cons.
- Usually if else or switch case then mostly it is strategy
- Inject the strategy through list (Command pattern) of run time we need to decide which strategy to use. Inject the strategy command line (strategy pattern) if only single strategy
- If expense is calculated during insertion time then strategy needs to be changed at storage time