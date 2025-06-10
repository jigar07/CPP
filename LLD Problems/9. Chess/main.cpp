#include <iostream>
#include <vector>
#include <memory>
#include <queue>
#include <cmath>
#include <optional>
#include <string>
using namespace std;

// ---------------- ENUMS ----------------
enum class Color { WHITE, BLACK };

// ---------------- STRUCTS ----------------
struct CellAddress {
    int x, y;

    bool operator==(const CellAddress& other) const {
        return x == other.x && y == other.y;
    }
};

// ---------------- FORWARD DECLARATIONS ----------------
class IPiece;
class Board;

// ---------------- CELL ----------------
class Cell {
    CellAddress address;
    shared_ptr<IPiece> currentPiece;

public:
    Cell(int x, int y) : address{ x, y } {}

    const CellAddress& getAddress() const { return address; }

    void setPiece(const shared_ptr<IPiece>& piece) {
        currentPiece = piece;
    }

    shared_ptr<IPiece> getPiece() const {
        return currentPiece;
    }

    void clearPiece() {
        currentPiece.reset();
    }
};

// ---------------- BOARD ----------------
class Board {
    vector<vector<shared_ptr<Cell>>> grid;

public:
    Board(int rows = 8, int cols = 8) {
        grid.resize(rows, vector<shared_ptr<Cell>>(cols));
        for (int i = 0; i < rows; ++i)
            for (int j = 0; j < cols; ++j)
                grid[i][j] = make_shared<Cell>(i, j);
    }

    shared_ptr<Cell> getCell(int x, int y) const {
        if (x < 0 || y < 0 || x >= grid.size() || y >= grid[0].size())
            return nullptr;
        return grid[x][y];
    }

    shared_ptr<Cell> getCell(const CellAddress& addr) const {
        return getCell(addr.x, addr.y);
    }
};

// ---------------- IPiece Interface ----------------
class IPiece {
protected:
    Color color;
    bool alive = true;
    shared_ptr<Cell> currentLocation;

public:
    IPiece(Color c) : color(c) {}
    virtual ~IPiece() = default;

    virtual bool isMoveAllowed(const shared_ptr<Cell>& dest, const Board& board) const = 0;
    virtual string name() const = 0;

    void setCurrentLocation(const shared_ptr<Cell>& cell) {
        currentLocation = cell;
        if (cell) cell->setPiece(shared_from_this());
    }

    shared_ptr<Cell> getCurrentLocation() const {
        return currentLocation;
    }

    bool isAlive() const { return alive; }

    void kill() {
        alive = false;
        if (currentLocation)
            currentLocation->clearPiece();
    }

    Color getColor() const { return color; }

    virtual shared_ptr<IPiece> shared_from_this() = 0;
};

// ---------------- KING ----------------
class King : public IPiece, public enable_shared_from_this<King> {
public:
    King(Color c) : IPiece(c) {}

    shared_ptr<IPiece> shared_from_this() override {
        return enable_shared_from_this<King>::shared_from_this();
    }

    string name() const override { return "King"; }

    bool isMoveAllowed(const shared_ptr<Cell>& dest, const Board& board) const override {
        if (!dest) return false;
        auto d = dest->getAddress(), c = currentLocation->getAddress();
        int dx = abs(d.x - c.x), dy = abs(d.y - c.y);
        return (dx <= 1 && dy <= 1) && (!dest->getPiece() || dest->getPiece()->getColor() != color);
    }
};

// ---------------- QUEEN ----------------
class Queen : public IPiece, public enable_shared_from_this<Queen> {
public:
    Queen(Color c) : IPiece(c) {}

    shared_ptr<IPiece> shared_from_this() override {
        return enable_shared_from_this<Queen>::shared_from_this();
    }

    string name() const override { return "Queen"; }

    bool isMoveAllowed(const shared_ptr<Cell>& dest, const Board& board) const override {
        if (!dest) return false;
        auto d = dest->getAddress(), c = currentLocation->getAddress();
        int dx = abs(d.x - c.x), dy = abs(d.y - c.y);
        return (dx == dy || dx == 0 || dy == 0) && (!dest->getPiece() || dest->getPiece()->getColor() != color);
    }
};

// ---------------- ROOK ----------------
class Rook : public IPiece, public enable_shared_from_this<Rook> {
public:
    Rook(Color c) : IPiece(c) {}

    shared_ptr<IPiece> shared_from_this() override {
        return enable_shared_from_this<Rook>::shared_from_this();
    }

    string name() const override { return "Rook"; }

    bool isMoveAllowed(const shared_ptr<Cell>& dest, const Board& board) const override {
        if (!dest) return false;
        auto d = dest->getAddress(), c = currentLocation->getAddress();
        return (d.x == c.x || d.y == c.y) && (!dest->getPiece() || dest->getPiece()->getColor() != color);
    }
};

// ---------------- PAWN ----------------
class Pawn : public IPiece, public enable_shared_from_this<Pawn> {
public:
    Pawn(Color c) : IPiece(c) {}

    shared_ptr<IPiece> shared_from_this() override {
        return enable_shared_from_this<Pawn>::shared_from_this();
    }

    string name() const override { return "Pawn"; }

    bool isMoveAllowed(const shared_ptr<Cell>& dest, const Board& board) const override {
        if (!dest) return false;
        auto d = dest->getAddress(), c = currentLocation->getAddress();
        int dx = d.x - c.x;
        int dy = abs(d.y - c.y);

        int forward = (color == Color::WHITE) ? 1 : -1;

        if (dy == 0 && dx == forward && !dest->getPiece())
            return true;

        if (dy == 1 && dx == forward && dest->getPiece() && dest->getPiece()->getColor() != color)
            return true;

        return false;
    }
};

// ---------------- PLAYER ----------------
struct PlayerInput {
    shared_ptr<IPiece> piece;
    shared_ptr<Cell> destination;
};

class Player {
    string name;
    Color color;
    vector<shared_ptr<IPiece>> pieces;

public:
    Player(const string& name_, Color color_) : name(name_), color(color_) {}

    Color getColor() const { return color; }

    void addPiece(const shared_ptr<IPiece>& piece) {
        pieces.push_back(piece);
    }

    const vector<shared_ptr<IPiece>>& getPieces() const {
        return pieces;
    }

    PlayerInput takeInput(const Board& board) {
        for (const auto& piece : pieces) {
            if (piece->isAlive()) {
                auto loc = piece->getCurrentLocation();
                if (!loc) continue;

                for (int dx = -1; dx <= 1; ++dx) {
                    for (int dy = -1; dy <= 1; ++dy) {
                        auto dest = board.getCell(loc->getAddress().x + dx, loc->getAddress().y + dy);
                        if (dest && piece->isMoveAllowed(dest, board)) {
                            return { piece, dest };
                        }
                    }
                }
            }
        }
        throw runtime_error("No valid move");
    }

    bool hasAliveKing() const {
        for (const auto& p : pieces) {
            if (p->isAlive() && p->name() == "King")
                return true;
        }
        return false;
    }

    const string& getName() const {
        return name;
    }
};

// ---------------- GAME ----------------
class ChessGame {
    Board board;
    shared_ptr<Player> p1, p2;
    optional<shared_ptr<Player>> winner;

public:
    ChessGame(shared_ptr<Player> a, shared_ptr<Player> b) : p1(a), p2(b) {}

    void start() {
        queue<shared_ptr<Player>> q;
        q.push(p1); q.push(p2);

        while (!winner) {
            auto player = q.front(); q.pop();

            try {
                auto input = player->takeInput(board);
                auto dest = input.destination;
                auto curr = input.piece->getCurrentLocation();

                if (!input.piece->isMoveAllowed(dest, board)) {
                    cout << player->getName() << " tried invalid move.\n";
                    q.push(player);
                    continue;
                }

                auto opponentPiece = dest->getPiece();
                if (opponentPiece && opponentPiece->getColor() != player->getColor()) {
                    opponentPiece->kill();
                    cout << player->getName() << " captured " << opponentPiece->name() << "\n";
                }

                curr->clearPiece();
                input.piece->setCurrentLocation(dest);

                if (!(p1->hasAliveKing() && p2->hasAliveKing())) {
                    winner = player;
                    cout << "Winner: " << player->getName() << "\n";
                    break;
                }
            } catch (...) {
                cout << player->getName() << " has no moves. Skipping.\n";
            }

            q.push(player);
        }
    }
};

// ---------------- MAIN ----------------
int main() {
    auto board = make_shared<Board>();

    auto player1 = make_shared<Player>("Alice", Color::WHITE);
    auto player2 = make_shared<Player>("Bob", Color::BLACK);

    auto wKing = make_shared<King>(Color::WHITE);
    auto bKing = make_shared<King>(Color::BLACK);
    auto wQueen = make_shared<Queen>(Color::WHITE);
    auto bRook = make_shared<Rook>(Color::BLACK);
    auto wPawn = make_shared<Pawn>(Color::WHITE);

    board->getCell(0, 4)->setPiece(wKing);
    wKing->setCurrentLocation(board->getCell(0, 4));
    board->getCell(7, 4)->setPiece(bKing);
    bKing->setCurrentLocation(board->getCell(7, 4));
    board->getCell(0, 3)->setPiece(wQueen);
    wQueen->setCurrentLocation(board->getCell(0, 3));
    board->getCell(7, 0)->setPiece(bRook);
    bRook->setCurrentLocation(board->getCell(7, 0));
    board->getCell(1, 4)->setPiece(wPawn);
    wPawn->setCurrentLocation(board->getCell(1, 4));

    player1->addPiece(wKing);
    player1->addPiece(wQueen);
    player1->addPiece(wPawn);
    player2->addPiece(bKing);
    player2->addPiece(bRook);

    ChessGame game(player1, player2);
    game.start();

    return 0;
}
