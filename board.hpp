#ifndef CHESS_BOARD
#define CHESS_BOARD

#include <map>
#include <iostream>
#include <string>
#include "vertex.hpp"

class chessBoard
{

    template <class type>
    struct Move
    {
        type from, to;
        int score;
    };

public:
    /*position struct*/
    class Pos
    {
    public:
        friend class chessBoard;
        Pos(const Pos &p, int dx = 0, int dy = 0)
        {
            *this = p;
            x += dx;
            y += dy;
        };
        Pos(int _x, int _y)
        {
            x = _x;
            y = _y;
        };
        bool operator<(const Pos &p) const { return (x < p.x) || (x == p.x && y < p.y); };
        bool operator==(const Pos &p) const { return x == p.x && y == p.y; };
        Pos()
        {
            x = -1;
            y = -1;
        };

    private:
        int x, y;
    };
    chessBoard()
    {
        reset();
    };
    ~chessBoard()
    {
        reset();
    };
    /*reset board to default*/
    void reset()
    {
        play = player::white;
        white_pieces.clear();
        black_pieces.clear();
        for (int i = 1; i < 9; ++i)
        {
            white_pieces[Pos(i, 7)] = piece::Pawn;
            black_pieces[Pos(i, 2)] = piece::Pawn;
        }
        int n = 1;
        for (auto piece : {piece::Rook, piece::Knight, piece::Bishop, piece::King})
        {
            white_pieces[Pos(n, 8)] = white_pieces[Pos(9 - n, 8)] = black_pieces[Pos(n, 1)] = black_pieces[Pos(9 - n, 1)] = piece;
            n++;
        }
        white_pieces[Pos(4, 8)] = black_pieces[Pos(4, 1)] = piece::Queen;
    };

    void flipTurn() { play = play == player::white ? player::black : player::white; };

    bool makeMove(Pos from, Pos to)
    {
        vertex<Pos> *allowed = new vertex<Pos>;
        vertex<Pos>::edgeIterator item;

        allowed = possibleMoves(from);
        bool mulligan = false;
        if (allowed->begin() == allowed->end())
        {
            return false;
        }
        item = allowed->begin();
        while (item != NULL)
        {
            if (to == *item)
            {
                mulligan = true;
                break;
            }
            item++;
        }
        if (mulligan == true)
        {
            // actual peice movement
            oppPieces().erase(to);
            PC_Pieces()[to] = PC_Pieces()[from];
            PC_Pieces().erase(from);
            /*promotes paws to queen if @ end */
            if ((PC_Pieces()[to] == piece::Pawn) && (to.y == 1 || to.y == 8))
            {
                PC_Pieces()[to] = piece::Queen;
            }
            flipTurn();
        }
        delete allowed;
        return mulligan;
    };

    vertex<Pos> *possibleMoves(const Pos &from)
    {
        vertex<Pos> *moves = new vertex<Pos>;
        auto isOwn = [&](int dx, int dy) -> bool { return PC_Pieces().count(Pos(from, dx, dy)); };
        auto isOpponent = [&](int dx, int dy) -> bool { return oppPieces().count(Pos(from, dx, dy)); };
        auto isInsideBoard = [&](int dx, int dy) -> bool { Pos p(from,dx,dy); return p.x < 9 && p.x > 0 && p.y < 9 && p.y > 0; };
        auto isFree = [&](int dx, int dy) -> bool {
            if (isOwn(dx, dy) || !isInsideBoard(dx, dy) || isOpponent(dx, dy))
            {
                return false;
            }

            return true;
        };

        auto addMove = [&](int dx, int dy) -> bool {
            if (isFree(dx, dy) || isOpponent(dx, dy))
            {
                /*  if (kingPath(from, dx, dy))
                {
                    pieceV.at(piece::King) = 100000;
                }*/
                moves->addEdge(Pos(from, dx, dy));
                return true;
            }
            return false;
        };

        if (!isOwn(0, 0))
        {
            return moves;
        }

        auto moving_piece = PC_Pieces()[from];
        switch (moving_piece)
        {
        case piece::Pawn:
            if (play == player::white)
            {
                if (isFree(0, -1) && !isOpponent(0, -1))
                {
                    addMove(0, -1);
                }
                if (isFree(0, -1) && isFree(0, -2) && from.y == 7 && !isOpponent(0, -2))
                {
                    addMove(0, -2);
                }
                if (isOpponent(-1, -1))
                {
                    addMove(-1, -1);
                }
                if (isOpponent(1, -1))
                {
                    addMove(1, -1);
                }
                break;
            }
            if (play == player::black)
            {
                if (isFree(0, 1) && !isOpponent(0, -1))
                {
                    addMove(0, 1);
                }
                if (isFree(0, 1) && isFree(0, 2) && from.y == 2 && !isOpponent(0, -1))
                {
                    addMove(0, 2);
                }
                if (isOpponent(-1, 1))
                {
                    addMove(-1, 1);
                }
                if (isOpponent(1, 1))
                {
                    addMove(1, 1);
                }
                break;
            }
        case piece::Knight: // Knight position tracker start {2,1}{7,1} or {2,8}{7,8}
            addMove(-2, -1);
            addMove(-2, 1);
            addMove(2, -1);
            addMove(2, 1);
            addMove(-1, -2);
            addMove(-1, 2);
            addMove(1, -2);
            addMove(1, 2);
            break;

        case piece::King:
            for (auto dy : {-1, 0, 1})
            {
                for (auto dx : {-1, 0, 1})
                {
                    if (isFree(dy, dx))
                    {
                        addMove(dy, dx);
                    }
                }
            }
            break;

        case piece::Queen:
        case piece::Rook:
            for (int n = 1; n < 9 && isFree(0, n); ++n)
            {
                addMove(0, n);
            }
            for (int n = 1; n < 9 && isFree(0, -n); ++n)
            {
                addMove(0, -n);
            }
            for (int n = 1; n < 9 && isFree(n, 0); ++n)
            {
                addMove(n, 0);
            }
            for (int n = 1; n < 9 && isFree(-n, 0); ++n)
            {
                addMove(-n, 0);
            }
            if (moving_piece != piece::Queen)
            {
                break;
            }

        case piece::Bishop: // needed: coordinants to track bishop start {6,8}{3,8} white or {3,1}{6,1} black
            for (int n = 1; n < 9 && isFree(n, n); ++n)
            {
                addMove(n, n);
            }
            for (int n = 1; n < 9 && isFree(n, -n); ++n)
            {
                addMove(n, -n);
            }
            for (int n = 1; n < 9 && isFree(-n, n); ++n)
            {
                addMove(-n, n);
            }
            for (int n = 1; n < 9 && isFree(-n, -n); ++n)
            {
                addMove(-n, -n);
            }
            break;
        }

        return moves;
    };
    void printBoard() /*Modify in future to build graphic interface */
    {
        static std::map<piece, char> sprites =
            {{piece::Pawn, 'P'},
             {piece::Rook, 'R'},
             {piece::Knight, 'N'},
             {piece::Bishop, 'B'},
             {piece::King, 'K'},
             {piece::Queen, 'Q'}};

        std::cout << '\n'
                  << "        1     2     3     4     5     6     7     8   " << std::endl;
        std::cout << "      _____ _____ _____ _____ _____ _____ _____ _____ ";
        for (int y = 1; y < 9; ++y)
        {
            if (show_coordinates)
            {
                std::cout << std::endl
                          << "     |1" << y << "   |2" << y << "   |3" << y << "   |4" << y
                          << "   |5" << y << "   |6" << y << "   |7" << y << "   |8" << y << "   |";
            }
            else
            {
                std::cout << std::endl
                          << "     |     |     |     |     |     |     |     |     |";
            }
            std::cout << std::endl
                      << "  " << y << "  ";
            for (int x = 1; x < 9; ++x)
            {
                std::cout << "|  ";
                if (white_pieces.count(Pos(x, y)))
                {
                    std::cout << (char)toupper(sprites[white_pieces[Pos(x, y)]]);
                }
                else if (black_pieces.count(Pos(x, y)))
                {
                    std::cout << (char)tolower(sprites[black_pieces[Pos(x, y)]]);
                }
                else
                {
                    std::cout << " ";
                }
                std::cout << "  ";
            }
            std::cout << "|  " << y << std::endl
                      << "     |_____|_____|_____|_____|_____|_____|_____|_____|";
        }
        std::cout << std::endl
                  << std::endl
                  << "        1     2     3     4     5     6     7     8   " << std::endl
                  << std::endl;
    };
    void printHelp()
    {
        std::cout << std::endl
                  << "* h: help, q: quit, p: show board, c: toggle show coordinates inside squares" << std::endl
                  << "* Input format: yxyx is from-to coordinates, e.g: 1715 moves (x,y)=(1,7) to (x,y)=(1,5)\n"
                  << std::endl;
    };
    /* False to exit */
    bool promptInput()
    {
        std::string move;
        Pos from(-1, -1), to(-1, -1);
    illegalmove:
        if (play == player::white)
            std::cout << "White move: ";
        else
            std::cout << "Black move: ";
        if (move == "")
            std::cin >> move;
        if (move == "q")
        {
            std::cout << "Good bye" << std::endl
                      << std::endl;
            return false;
        }
        if (move == "?" || move == "h" || move == "help")
        {
            printHelp();
            move = "";
            goto illegalmove;
        }
        if (move == "c")
        {
            show_coordinates = !show_coordinates;
            printBoard();
            move = "";
            goto illegalmove;
        }

        if (move == "p")
        {
            printBoard();
            move = "";
            goto illegalmove;
        }
        if (move.length() == 4)
        {
            from.x = move[0] - '0';
            from.y = move[1] - '0';
            to.x = move[2] - '0';
            to.y = move[3] - '0';
        }
        if (!makeMove(from, to))
        {
            std::cout << "* Illegal move" << std::endl;
            move.clear();
            goto illegalmove;
        }
        printBoard();
        return true;
    };

    int score()
    {
        int sumWhite = 0;
        int sumBlack = 0;
        for (auto &p : white_pieces)
        {
            sumWhite += pieceV[p.second];
        }
        for (auto &p : black_pieces)
        {
            sumBlack += pieceV[p.second];
        }
        return sumWhite - sumBlack;
    };

    bool hasKing()
    {
        for (auto &p : PC_Pieces())
        {
            if (p.second == piece::King) // instead check if possible move 1 ahead is 4
            {
                return true;
            }
        }
        return false;
    };

    Move<Pos> minimax(int depth, bool minimize)
    {
        Move<Pos> best_move;
        Move<Pos> option;
        chessBoard branch = *this;
        best_move.score = -1000000 + 2000000 * minimize;
        if (0 == depth)
        {
            best_move.score = score();
            return best_move;
        }

        for (auto &from : PC_Pieces())
        {
            vertex<Pos> *i = possibleMoves(from.first);
            vertex<Pos>::edgeIterator j;
            for (j = i->begin(); j != NULL; j++)
            {
                branch.makeMove(from.first, *j);
                option.score = score();
                if (option.score > best_move.score && !minimize)
                {
                    option = branch.minimax(depth - 1, minimize);
                    best_move.score = option.score;
                    best_move.from = from.first;
                    best_move.to = *j;
                }
                if (option.score < best_move.score && minimize)
                {
                    option = branch.minimax(depth - 1, !minimize);
                    best_move.score = option.score;
                    best_move.from = from.first;
                    best_move.to = *j;
                }
            }
            delete i;
        }
        return best_move;
    };
    void AIMove()
    {
        bool minimize = play == player::black ? true : false;
        Move<Pos> m = minimax(4, minimize);
        makeMove(m.from, m.to);
        printBoard();
    };

private:
    enum class player
    {
        white,
        black
    } play;
    enum class piece
    {
        King,
        Queen,
        Pawn,
        Rook,
        Bishop,
        Knight
    };
    // myStack<Pos> stacker;
    // vertex<Pos> *vert;
    std::string Knight[4] = {"21", "71", "28", "78"}; // used to track Knight pos for intelligent movement
                                                      // not yet implemented
    std::string Bishop[4] = {"31", "61", "38", "68"}; // used to track Bishop pos for intelligent movement
                                                      // not yet implemented
    bool show_coordinates = false;
    std::map<Pos, piece> white_pieces, black_pieces;
    std::map<Pos, piece> &PC_Pieces() { return play == player::white ? white_pieces : black_pieces; };
    std::map<Pos, piece> &oppPieces() { return play == player::white ? black_pieces : white_pieces; };
    std::map<piece, int> pieceV{
        // attack values
        {chessBoard::piece::King, 4},
        {chessBoard::piece::Queen, 9},
        {chessBoard::piece::Pawn, 1},
        {chessBoard::piece::Bishop, 3},
        {chessBoard::piece::Knight, 3},
        {chessBoard::piece::Rook, 5},
    };
};

#endif