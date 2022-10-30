#pragma once
#include <string>
#include <sstream>
#include <iostream>
#include "movegen.hpp"

_Compiletime void CharToBit(char ch, int &sq, Piece *b, U64 *p) {
  switch (ch) {
    case 'p': b[sq] = BPawn;      p[BPawn]      |= (1ull << sq); sq += 1; break;
    case 'w': b[sq] = BBrawn;     p[BBrawn]     |= (1ull << sq); sq += 1; break;
    case 'n': b[sq] = BKnight;    p[BKnight]    |= (1ull << sq); sq += 1; break;
    case 'b': b[sq] = BBishop;    p[BBishop]    |= (1ull << sq); sq += 1; break;
    case 'r': b[sq] = BRook;      p[BRook]      |= (1ull << sq); sq += 1; break;
    case 's': b[sq] = BPrincess;  p[BPrincess]  |= (1ull << sq); sq += 1; break;
    case 'u': b[sq] = BUnicorn;   p[BUnicorn]   |= (1ull << sq); sq += 1; break;
    case 'd': b[sq] = BDragon;    p[BDragon]    |= (1ull << sq); sq += 1; break;
    case 'q': b[sq] = BQueen;     p[BQueen]     |= (1ull << sq); sq += 1; break;
    case 'y': b[sq] = BRQueen;    p[BRQueen]    |= (1ull << sq); sq += 1; break;
    case 'c': b[sq] = BCKing;     p[BCKing]     |= (1ull << sq); sq += 1; break;
    case 'k': b[sq] = BKing;      p[BKing]      |= (1ull << sq); sq += 1; break;
    case 'P': b[sq] = WPawn;      p[WPawn]      |= (1ull << sq); sq += 1; break;
    case 'W': b[sq] = WBrawn;     p[WBrawn]     |= (1ull << sq); sq += 1; break;
    case 'N': b[sq] = WKnight;    p[WKnight]    |= (1ull << sq); sq += 1; break;
    case 'B': b[sq] = WBishop;    p[WBishop]    |= (1ull << sq); sq += 1; break;
    case 'R': b[sq] = WRook;      p[WRook]      |= (1ull << sq); sq += 1; break;
    case 'S': b[sq] = WPrincess;  p[WPrincess]  |= (1ull << sq); sq += 1; break;
    case 'U': b[sq] = WUnicorn;   p[WUnicorn]   |= (1ull << sq); sq += 1; break;
    case 'D': b[sq] = WDragon;    p[WDragon]    |= (1ull << sq); sq += 1; break;
    case 'Q': b[sq] = WQueen;     p[WQueen]     |= (1ull << sq); sq += 1; break;
    case 'Y': b[sq] = WRQueen;    p[WRQueen]    |= (1ull << sq); sq += 1; break;
    case 'C': b[sq] = WCKing;     p[WCKing]     |= (1ull << sq); sq += 1; break;
    case 'K': b[sq] = WKing;      p[WKing]      |= (1ull << sq); sq += 1; break;
    case '*': p[UnMoved] |= (1ull << (sq - 1)); break;
    case '/': sq -= 16; break;
    case '1': sq += 1; break;
    case '2': sq += 2; break;
    case '3': sq += 3; break;
    case '4': sq += 4; break;
    case '5': sq += 5; break;
    case '6': sq += 6; break;
    case '7': sq += 7; break;
    case '8': sq += 8; break;
    case '9': sq += 9; break;
  }
}

// Assumes valid fen
template<typename Chess>
_ForceInline static void importFEN(Chess &chess, std::string fen) {
  std::istringstream ss(fen);
  std::string board;
  while (getline(ss, board, '\n')) {
    Piece b[64] = 
      {Occ, Occ, Occ, Occ, Occ, Occ, Occ, Occ, Occ, Occ, Occ, Occ, Occ, Occ, Occ, Occ,
       Occ, Occ, Occ, Occ, Occ, Occ, Occ, Occ, Occ, Occ, Occ, Occ, Occ, Occ, Occ, Occ,
       Occ, Occ, Occ, Occ, Occ, Occ, Occ, Occ, Occ, Occ, Occ, Occ, Occ, Occ, Occ, Occ,
       Occ, Occ, Occ, Occ, Occ, Occ, Occ, Occ, Occ, Occ, Occ, Occ, Occ, Occ, Occ, Occ};
    U64 p[28] = {};
    int i = 0, j = 0, sq = 56; // start on a8
    while (board[i] != ':') {
      CharToBit(board[i], sq, b, p);
      i++;
    }
    p[Black] = p[BPawn] | p[BBrawn] | p[BKnight] | p[BBishop] | p[BRook] | p[BPrincess] | p[BUnicorn] | p[BDragon] | p[BQueen] | p[BRQueen] | p[BCKing] | p[BKing];
    p[White] = p[WPawn] | p[WBrawn] | p[WKnight] | p[WBishop] | p[WRook] | p[WPrincess] | p[WUnicorn] | p[WDragon] | p[WQueen] | p[WRQueen] | p[WCKing] | p[WKing];
    p[Occ] = p[Black] | p[White];
    j = board.find_first_of(':', i + 1);
    const int l = stoi(board.substr(i + 1, j - 1)) + chess.whiteOrigIndex;

    i = j;
    j = board.find_first_of(':', i + 1);
    const bool white = (board[j + 1] == 'w');
    const int t = 2 * stoi(board.substr(i + 1, j - 1)) + (white ? 4 : 5);

    Board brd = Board();
    memcpy(brd.board, b, sizeof(b));
    memcpy(brd.bitBoards, p, sizeof(p));

    U64 epTarget = 0;
    MoveGen::Masks masks;
    if (white) {
      ChessFunc::ImportBoard<false>(chess, brd, epTarget, l, t);
      MoveGen::Refresh<BoardState(0b11111111)>(&chess.turns[l][t], masks);
    } else {
      ChessFunc::ImportBoard<true>(chess, brd, epTarget, l, t);
      MoveGen::Refresh<BoardState(0b01111111)>(&chess.turns[l][t], masks);
    }
  }
}

static _ForceInline std::string BitToChar(U64 bit, const Board& brd) {
  if (bit & brd.bitBoards[BPawn])     return "p";
  if (bit & brd.bitBoards[BBrawn])    return "w";
  if (bit & brd.bitBoards[BKnight])   return "n";
  if (bit & brd.bitBoards[BBishop])   return "b";
  if (bit & brd.bitBoards[BRook])     return "r";
  if (bit & brd.bitBoards[BPrincess]) return "s";
  if (bit & brd.bitBoards[BUnicorn])  return "u";
  if (bit & brd.bitBoards[BDragon])   return "d";
  if (bit & brd.bitBoards[BQueen])    return "q";
  if (bit & brd.bitBoards[BRQueen])   return "y";
  if (bit & brd.bitBoards[BCKing])    return "c";
  if (bit & brd.bitBoards[BKing])     return "k";
  if (bit & brd.bitBoards[WPawn])     return "P";
  if (bit & brd.bitBoards[WBrawn])    return "W";
  if (bit & brd.bitBoards[WKnight])   return "N";
  if (bit & brd.bitBoards[WBishop])   return "B";
  if (bit & brd.bitBoards[WRook])     return "R";
  if (bit & brd.bitBoards[WPrincess]) return "S";
  if (bit & brd.bitBoards[WUnicorn])  return "U";
  if (bit & brd.bitBoards[WDragon])   return "D";
  if (bit & brd.bitBoards[WQueen])    return "Q";
  if (bit & brd.bitBoards[WRQueen])   return "Y";
  if (bit & brd.bitBoards[WCKing])    return "C";
  if (bit & brd.bitBoards[WKing])     return "K";
  return ".";
}

template<typename Chess>
static _ForceInline std::string print(Chess &chess) {
  int top = chess.whiteOrigIndex + chess.whiteNum, bot = chess.whiteOrigIndex - chess.blackNum;
  int min = chess.indices[bot].first;
  for (int i = bot; i < top; i++) min = std::min(min, chess.indices[i].first);

  std::string output;
  for (int i = top; i >= bot; i--) {
    int head = chess.indices[i].second, tail = chess.indices[i].first;

    for (int j = min; j < tail; j++) output += "          ";

    for (int j = tail; j <= head; j++) {
      std::string ltLabel = std::to_string(i - chess.whiteOrigIndex) + "," + std::to_string((j - 4) / 2);
      output += "╔" + ltLabel;
      for (int k = ltLabel.length(); k < 8; k++) output += "═";
      output += "╗";
    }
    output += "\n";
    
    for (int j = 7; j >= 0; j--) {
      for (int k = min; k < tail; k++) output += "          ";

      for (int k = tail; k <= head; k++) {
        output += "║";
        for (int l = 0; l < 8; l++) {
          output += BitToChar(1ull << (j * 8 + l), chess.turns[i][k].board);
        }
        output += "║";
      }
      output += "\n";
    }

    for (int j = min; j < tail; j++) output += "          ";
    for (int j = tail; j <= head; j++) output += "╚════════╝";
    output += "\n";
  }

  return output;
}

// Debugging Only
_Compiletime U64 dirMaskOr(const DirMask *dirMask) {
  return (dirMask->center | dirMask->north | dirMask->east | dirMask->south | dirMask->west | dirMask->northEast | dirMask->northWest | dirMask->southEast | dirMask->southWest);
}

// Debugging only
template<typename Chess>
static _ForceInline std::string printBoardMask(Chess &chess, const int l, const int t) {
  const BoardMask *mask = chess.turns[l][t].boardMask;
  std::string output = "";
  output += "pawn: " + std::to_string(mask->pawn) + "\n";
  output += "brawn: " + std::to_string(mask->brawn) + "\n";
  output += "knight: " + std::to_string(mask->knight) + "\n";
  output += "king: " + std::to_string(mask->king) + "\n";
  output += "orthN: " + std::to_string(dirMaskOr(mask->north)) + "\n";
  output += "orthE: " + std::to_string(dirMaskOr(mask->east)) + "\n";
  output += "orthS: " + std::to_string(dirMaskOr(mask->south)) + "\n";
  output += "diagNE: " + std::to_string(dirMaskOr(mask->northEast)) + "\n";
  output += "diagSE: " + std::to_string(dirMaskOr(mask->southEast)) + "\n";
  output += "diagSW: " + std::to_string(dirMaskOr(mask->southWest)) + "\n";
  output += "diagNW: " + std::to_string(dirMaskOr(mask->northWest)) + "\n";
  
  return output;
}

// Debugging only
static _ForceInline std::string printBoardMask(const BoardMask *mask) {
  std::string output = "";
  output += "pawn: " + std::to_string(mask->pawn) + "\n";
  output += "brawn: " + std::to_string(mask->brawn) + "\n";
  output += "knight: " + std::to_string(mask->knight) + "\n";
  output += "king: " + std::to_string(mask->king) + "\n";
  output += "orthN: " + std::to_string(dirMaskOr(mask->north)) + "\n";
  output += "orthE: " + std::to_string(dirMaskOr(mask->east)) + "\n";
  output += "orthS: " + std::to_string(dirMaskOr(mask->south)) + "\n";
  output += "diagNE: " + std::to_string(dirMaskOr(mask->northEast)) + "\n";
  output += "diagSE: " + std::to_string(dirMaskOr(mask->southEast)) + "\n";
  output += "diagSW: " + std::to_string(dirMaskOr(mask->southWest)) + "\n";
  output += "diagNW: " + std::to_string(dirMaskOr(mask->northWest)) + "\n";
  
  return output;
}

// Debugging Only
static _ForceInline std::string printDirMask(const DirMask *mask) {
  std::string output = "";
  output += "center: " + std::to_string(mask->center) + "\n";
  output += "orthN: " + std::to_string(mask->north) + "\n";
  output += "orthE: " + std::to_string(mask->east) + "\n";
  output += "orthS: " + std::to_string(mask->south) + "\n";
  output += "orthW: " + std::to_string(mask->west) + "\n";
  output += "diagNE: " + std::to_string(mask->northEast) + "\n";
  output += "diagSE: " + std::to_string(mask->southEast) + "\n";
  output += "diagSW: " + std::to_string(mask->southWest) + "\n";
  output += "diagNW: " + std::to_string(mask->northWest) + "\n";
  
  return output;
}
