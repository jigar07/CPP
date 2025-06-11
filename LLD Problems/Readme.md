# Though Process
1. model folder - Start with entity(Classes, struct and enumbs)
2. Then parameters for these entity(Properties)
3. service folder - Then top-down methods. Create services for this
4. repository folder - Classes to store data. InMemory or ondisk storage related classses
5. Strategy folder - strategy based on whatever can be changed

## Main concepts
- strategy pattern is most import pattern and OCP compliance is most important SOLID principal
- While implementing any method/requirement, if something can be changed then create strategy for that. Strategies are also stored list/vector
  - Then, all strategies are traversed with following.
    - strategy.doesSupport(input)
      - strategy.apply()
- How, to treat or pass input is also important.

## Requirement clarification
- During requirement clarification think in terms of ##Main concepts