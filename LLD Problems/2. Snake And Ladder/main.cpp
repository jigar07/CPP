/*
Snake and ladder is Game and it is played
    So, "Game" object and 'play' is the method
Game has "Board"
Game has "Players" (vector<player>)
Board has "Boxes" (vector<box>)
Board has "Snakes" (vector<snake>) (or Box has snake?. Board has snake gives good solution. Because snake is Box as mouth and tail)
    Snakes has mouth and tail which are "Box" object
Board has "Ladders" (vector<ladder>)
    Ladder has start and end which are "Box" object

play Method of Game(actions):
    throw dice
    move position
    player win
    next player turn
*/

#include <random>
#include <iostream>

using namespace std;

class Dice {
    int diceSides;
public:
    Dice(int diceSides) {
        this->diceSides = diceSides;
    }

    int rollDice() {
        return rand()%6 + 1;
    }
};

class Box{
    int position;
public:
    Box(int position) {
        this->position = position;
    }
    int getPosition() const {
        return position;
    }
};


class Snake {
    Box mouth;
    Box tail;
public:
    Snake(const Box& m, const Box& t) : mouth(m), tail(t) {}
    const Box& getMouth() const { return mouth; }
    const Box& getTail() const { return tail; }
};

class Ladder {
    Box start;
    Box end;
public:
    Ladder(const Box& s, const Box& e) : start(s), end(e) {}
    const Box& getStart() const { return start; }
    const Box& getEnd() const { return end; }
};

class Board {
    vector<Box> boxes;
    vector<Snake> snakes;
    vector<Ladder> ladders;
public:
    Board() {
        for(int i=0; i<100; i++) {
            boxes.push_back(Box(i));
        }

        // Add some sample snakes
        snakes.push_back(Snake(getBox(97), getBox(25)));
        snakes.push_back(Snake(getBox(62), getBox(19)));

        // Add some sample ladders
        ladders.push_back(Ladder(getBox(3), getBox(22)));
        ladders.push_back(Ladder(getBox(50), getBox(91)));
    }
    Box& getBox(int position) {
        return boxes[position];
    }
    vector<Box>& getBoxes() {
        return boxes;
    }
    int getNextPosition(int pos) {
        for (auto& snake : snakes) { // not following OCP if wants to add any other entity like reward on some boxes
            if (snake.getMouth().getPosition() == pos) {
                cout << "Oops! Snake from " << pos << " to " << snake.getTail().getPosition() << endl;
                return snake.getTail().getPosition();
            }
        }

        for (auto& ladder : ladders) {
            if (ladder.getStart().getPosition() == pos) {
                cout << "Yay! Ladder from " << pos << " to " << ladder.getEnd().getPosition() << endl;
                return ladder.getEnd().getPosition();
            }
        }

        return pos;
    }
};

class Player {
    Box currentPosition;
public:
    Player(const Box& startPosition): currentPosition(startPosition) {
    }
    void changeCurrentPosition(const Box& b) {
        currentPosition = b;
        // cout<<b.getPosition()<<endl;
    }
    Box& getCurrentPosition() {
        return currentPosition;
    }
}; 

class Game {
    Board board;
    Dice dice;
    vector<Player> players;
    int currentPlayer;
public:
    Game(int playerCount): dice(6), currentPlayer(0) {
        board = Board();
        for(int i=0; i<playerCount; i++) {
            players.push_back(Player(board.getBox(0))); // Player is always starting from 0. Not following OCP if want to change start strategy
        }
        srand(time(nullptr)); // without this player 0 is always winning. So, this will seed the random number generation
    }
public:
    void play() {
        for(auto box: board.getBoxes()) {
            // cout<<box.getPosition()<<endl;
        }
        while(true) {
            Player& player = players[currentPlayer];
            int diceRoll = dice.rollDice();
            int newPos = player.getCurrentPosition().getPosition() + diceRoll; // This is not following OCP if want to introduce skip of player based if snake bite

            if(newPos >= 100) { // win strategy is always >=100. Hence, not following OCP if want to change win strategy
                cout<<"Player " + to_string(currentPlayer) + " won!"<<endl;
                break;
            }
            int finalPos = board.getNextPosition(newPos);
            player.changeCurrentPosition(finalPos);
            cout << "Player " << currentPlayer << " is at " << finalPos << endl;

            if(currentPlayer + 1 == players.size()) cout<<endl;

            currentPlayer = (currentPlayer + 1) % players.size();
        }
    }
};

int main() {
    Game game(4);
    game.play();
    return 0;
}