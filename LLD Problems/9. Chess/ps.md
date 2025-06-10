# Problem statement
- Chess Design
- 2 players playing it
- One with white pieces, another with black pieces
- White takes the first turn
- They keep changing turns one by one alternatively
- Each piece has different way of movement
  - Queen moves in all the direction by any number of amounts
  - Rook moves horizontally
  - Bishow moves diagonally
- If you land into another team's pieces, that other team's piece is killed
- Whoever kills other oponent's king first, wins

# Thought process
- core requirement is different types of move for piece.
  - As piece is moved, piece also have currentAddress as an object
  - Then jumping and killing etc
  - For that Player gives input. Hence, PlayerInput is created in which player choose Piece and destination for the Piece

# Notes
- We will create classes in single file only
- For in-memory classes in interviews can do it in single file. For production should be in individual folder based on functionality
- Chess not asked much in machine coding. Usually asked in LLD
- Core requirement is movement in chess, so for extensibililty we need to think what if different types of moves comes. For which we created IPiece. 2nd requirement is jumping, killing etc
- Only thing which is changing, we are putting to interface. Other things which are not changing we are just composing it. Again composistion over interface
- Try to guess/get feedback of interviewer. See if interviewer is able to follow up or not. You would be having less time in interview.
  - If interviewer is very strict with his/her approach and you think you are right then don't worry. Sometimes, interviewer might be wrong. Its not about getting the soultion which they have. Some solution will have pros and cons both
- For parkinglot, don't start with permission and all if not asked

# Design
- Classes
  - Game, Board, Cell, Piece, IPiece, Player
  - enum: PieceType (This may not required)
- Properties of classes:
  - Game:
    - POJO - `Class Size`
  - Cell:
    - `enum Color`, Piece, CellAddress
  - Player:
    - name, Color, hasWon
  - Piece:
    - PieceType, Color, isAlive
    - IPiece can also be used as an interface for all type of piece.
    - Pros/cons of Piece: move method has `if` condition. So, Piece class will not be OCP complient. However, as per Udit Piece is better than having individual class for PieceType. Though Udit used `IPiece` in the implementation
    - Pros/cons of IPiece: move method based on PieceType, but so many classes have duplicate properties.
- Fill the methods/behaviors. (Start with top down)
  - `Class Chess`: (Chess or ChessGame)
    - List<Player> players gives more extensibility than Player1 and Player2
    - Board board
    - Player winner
    - start() method
      - `final Queue<Player> queue = new LinkedList<>(players);` is algorithm improvement. It can be injected by client also. We can use `currentIndex` and then traverse `List<Player>` on it. Anything is fine
      - `winner==null` -> Downside is only single winner.
      - `class PlayerInput` created
      - `takeInput` method added in Player class
        ``` java
        // move the piece to that destination
        // - check if possible
        //      - there should be no obstruction in between
        //      - if it can actually move to that destination as per the movement criteria.
        //      - Destination should be free - for only self.
        // - move piece to that location
        // - kill the opponent piece if required .
        ```
      - `isMoveAllowed` and `isDestinationReachable` added for `Rook` class
        - currentLocation part of IPiece,
      - Cell[][] added in `Class Board`
        - `getLeftCell`, `getRightCell`, `getCell` added in Board class
      - `aliveOpponentKind` and `aliveOpponentKind.isEmpty()` added
  - However, there are still problems:
    - IPiece also breaks OCP. It breaks OCP if IPiece is upgraded to more directions
    - Another problem: Code redundency/duplication between the classes. Multiple IPiece would have similar type of problem. If issue with some logic then need to change in all the classes if similar logic is used in other classes.
    - As we went with inheritance over composition we need to create multiple combination of classes inherited from IPiece
    - Only thing which changes is movement of Piece. That's why common properties are coming in IPiece. We are not composing multiple behaviors of piece. We are telling Piece will have particular type of behavior.
    - To solve this `IMoveDirection` added
      - List<IMoveDirection> added in `Piece` class
      - `isAllowed` added in `Piece` class
      - Only thing which is changing, we are putting to interface. Other things which are not changing we are just composing it.
      - Then can create `Piece king`, `Piece Queen` based on this
    - `IMoveDirection moveDirection : allowedMoveDirections` added in `isAllowed` of `Piece` class
    - `getAllCellsUpto` method added
    - `PiecesFactory` added