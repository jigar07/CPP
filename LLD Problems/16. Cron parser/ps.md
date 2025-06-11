# Requirements
Design and implement a cron expression parser that takes a cron expression
and parses it.

Cron expressions consist of five fields: 
minute, hour, day of the month, month, and day of the week, in that order. 
Each field can have a specific value, a range (`-`), a list (`,`), 
or an asterisk (`*`) which represents "every" possible value.

Your cron parser should support the following features:
- Parsing standard cron expressions with 5 fields.
- Handling ranges and lists in each field.
- Support for the `*` character to denote any value in a field.

Follow up
- Calculation of the next execution time from a given timestamp,
  taking into account the current time and the cron expression.

Assume the cron expressions are in UTC time and the input timestamp is also
provided in UTC.

## Example:
```
Input Cron Expression: `*/15 0 1,15 * 1-5 /usr/bin/find`

Output:

seconds        
minute          0 15 30 45
hour            0
day of month    1 15
month           1 2 3 4 5 6 7 8 9 10 11 12
day of week     1 2 3 4 5
command         /usr/bin/find
```

712 
  9

717

Follow ups:
MON: 0
THU: 3
* MON-THU
* JAN-MAR
* Print the next instance of the time 

# Thought Process
- Difficult problem
// Input Cron Expression: `*/15 0 1,15 1,4 1-5 /usr/bin/find`
/**
    * minute          0 15 30 45
    * hour            0
    * day of month    1 15
    * month           1 2 3 4 5 6 7 8 9 10 11 12
    * day of week     1 2 3 4 5
    * command         /usr/bin/find
    */
- Classes
- Properties
- Top-Down methods:
  - CronParser Service:
    - `private Map<Integer, ComponentTypeData> componentTypeDataMap;` - store all 6 component type(minute, hour, day of month, month, day of week, command) in sequence
    - private List<IExpressionTypeParser> parsers; - store different types of parser
    - `parse` method
      - split by spaces and validate length==6
      - `IExpressionTypeParser` created with CommaExpressionTypeParser and HyphenExpressionTypeParser as subclass. It has following methods:
        - doesSupport(String componentExpr) - check whether it contains `,` or `-` based on subclass
        - isValid(String componentExpr, Range validRange) - Check whether range is valid or not
        - parse(String componentExpr) - split values based on `,` or `-` and return List of values (`ComponentTypeDataList`)
      - for all componentExpr from 6 expressions:
        - for all parser in IExpressionTypeParser:
          - if parser.doesSupport(componentExpr)
            - componentTypeData = componentTypeDataMap.get(i)
            - if parser.isValid(componentExpr, componentTypeData.getValidRange())
              - parsedComponents.add(parsedComponent);

# Notes
- Validation is required. But not a core component. If different validation strategy then discuss with interviewer if it required or not. If required then use strategy for that
# Design
## Classes
- Range, CronComponentType, CronExpression, CronComponent, IComponentTypeData, ComponentTypeDataList, ComponentTypeDataString
  - CronExpression is more like response
## Properties
- CronComponent:
  - CronComponentType, IComponentTypeData, 
## Method (topdown)
- CronParser
  - `parse` method
    - IComponentParser, MinuteComponentParser, HourComponentParser created
      - `private Range validRange` as a member variable for above
      - `boolean isValid` method
      - `CronComponent parse` method
  - `List<IComponentParser> parsers` added in CronParse
  - We need to find type using order of the input `*/15 0 1,15 * 1-5 /usr/bin/find` (Call out to interviewer should I have input like `Min: */15 Day: 0 Hour: 1,15 * Week: 1-5 /usr/bin/find`. Based on this answer parse input. For our discussion we will take `*/15 0 1,15 * 1-5 /usr/bin/find` as input. So, by putting key like Min, Hour we can make input safer)
  - `for (int i = 0; i < exprParts.length; i++)` in parse method
``` java
List<IComponentParser> parsers
public CronExpression parse(@NonNull final String cronExpression) {
  final String[] exprParts = cronExpression.split(" ");
  if (exprParts.length != 6) {
      throw new InvalidInputException();
  }

  final List<CronComponent> parsedComponents = new ArrayList<>();
  for (int i = 0; i < exprParts.length; i++) {
    final IComponentParser componentParser = parsers.get(i);
    if(componentParser.isValid(exprParsts[i]) {
      parsedComponents.add(componentParser.parse(exprParsts[i]));
    } else {
      throw exception
    }
  }
  return new CronExpression(parsedComponents);
}
```
- Problem with above: very hard dependency that the caller of this class should inject in the same order
- parsing changes based on input like *, -, /, `,` etc. It does not change based on Minute, Hour etc. Because Minute, Hour, Week each would required parser based on *, -, /, `,` etc. So, it will create a lot of repeatation of code (Even Udit added MinuteComponentParser, HourComponentParser when he wrote first time). So, IComponentParser, MinuteComponentParser, HourComponentParser removed.
  - DefaultComponentParser added in which it has method based on parser (`,`, `-`) etc. But it has OCP problem
  - To solve this `IExpressionTypeParser`, `CommaExpressionTypeParser`, `HyphenExpressionTypeParser` added
  - `parse` method of CronParser modified to use IExpressionTypeParser
  - ComponentTypeData added and used in CronParser (To map with Minute, Hour etc. Can use if else but it will have OCP issue. So, map is used)
    - This map has also problem of "very hard dependency that the caller of this class should inject in the same order" as we are using map based on index
- Follow ups. What if string is given as input instead of integer:
  - MON-THU
  - JAN-MAR
  - To solve this we can map string to integer as our whole logic is based on integer
    - MON: 0
    - THU: 3
- Print the next instance of the time
  - Next time matching the expression
  - This is algorithm question instead of design question
  - `CronService` added with method `parseAndPrint`
  - `printNextInstance` method can be added to get next instance of the time (It is like nextPermutation problem of leetcode)
- What if seconds added
  - To solve this map will change, index handling will get change and `exprParts.length != 5` replaced by `exprParts.length != 6`