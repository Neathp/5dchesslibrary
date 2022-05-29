#pragma once
#include "chess.hpp"
#include <string>
#include <iostream>

_Compiletime void CharToBit(char ch, int &sq, U64 (&p)[25]) {
  switch (ch) {
    case 'p': p[0] |= (1ull << sq); sq += 1; break;
    case 'w': p[1] |= (1ull << sq); sq += 1; break;
    case 'n': p[2] |= (1ull << sq); sq += 1; break;
    case 'b': p[3] |= (1ull << sq); sq += 1; break;
    case 'r': p[4] |= (1ull << sq); sq += 1; break;
    case 's': p[5] |= (1ull << sq); sq += 1; break;
    case 'u': p[6] |= (1ull << sq); sq += 1; break;
    case 'd': p[7] |= (1ull << sq); sq += 1; break;
    case 'q': p[8] |= (1ull << sq); sq += 1; break;
    case 'y': p[9] |= (1ull << sq); sq += 1; break;
    case 'c': p[10] |= (1ull << sq); sq += 1; break;
    case 'k': p[11] |= (1ull << sq); sq += 1; break;
    case 'P': p[12] |= (1ull << sq); sq += 1; break;
    case 'W': p[13] |= (1ull << sq); sq += 1; break;
    case 'N': p[14] |= (1ull << sq); sq += 1; break;
    case 'B': p[15] |= (1ull << sq); sq += 1; break;
    case 'R': p[16] |= (1ull << sq); sq += 1; break;
    case 'S': p[17] |= (1ull << sq); sq += 1; break;
    case 'U': p[18] |= (1ull << sq); sq += 1; break;
    case 'D': p[19] |= (1ull << sq); sq += 1; break;
    case 'Q': p[20] |= (1ull << sq); sq += 1; break;
    case 'Y': p[21] |= (1ull << sq); sq += 1; break;
    case 'C': p[22] |= (1ull << sq); sq += 1; break;
    case 'K': p[23] |= (1ull << sq); sq += 1; break;
    case '*': p[24] |= (1ull << (sq - 1)); break;
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

static void importFEN(Chess *chess, const std::string fen) { // assumes valid fen; currently only implemented for a single 8x8 board
  U64 p[25] = {};
  int i = 0, j = 0, sq = 56; // start on a8
  while (fen[i] != ':') {
    CharToBit(fen[i], sq, p);
    i++;
  }
  j = fen.find_first_of(':', i + 1);
  const int l = stoi(fen.substr(i + 1, j - 1)) + chess->whiteOrigIndex;

  i = j;
  j = fen.find_first_of(':', i + 1);
  const bool white = fen[j + 1] == 'w';
  const int t = 2 * stoi(fen.substr(i + 1, j - 1)) + (white ? 4 : 5);

  const Board *brd = new Board(p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], p[8], p[9], p[10], p[11], p[12], p[13], p[14], p[15], p[16], p[17], p[18], p[19], p[20], p[21], p[22], p[23], p[24]);
  if (white) importBoards<false>(chess, brd, l, t);
  else importBoards<true>(chess, brd, l, t);
}

static std::string BitToChar(U64 bit, const Board& brd) {
  if (bit & brd.BPawn)     return "p";
  if (bit & brd.BBrawn)    return "w";
  if (bit & brd.BKnight)   return "n";
  if (bit & brd.BBishop)   return "b";
  if (bit & brd.BRook)     return "r";
  if (bit & brd.BPrincess) return "s";
  if (bit & brd.BUnicorn)  return "u";
  if (bit & brd.BDragon)   return "d";
  if (bit & brd.BQueen)    return "q";
  if (bit & brd.BRQueen)   return "y";
  if (bit & brd.BCKing)    return "c";
  if (bit & brd.BKing)     return "k";
  if (bit & brd.WPawn)     return "P";
  if (bit & brd.WBrawn)    return "W";
  if (bit & brd.WKnight)   return "N";
  if (bit & brd.WBishop)   return "B";
  if (bit & brd.WRook)     return "R";
  if (bit & brd.WPrincess) return "S";
  if (bit & brd.WUnicorn)  return "U";
  if (bit & brd.WDragon)   return "D";
  if (bit & brd.WQueen)    return "Q";
  if (bit & brd.WRQueen)   return "Y";
  if (bit & brd.WCKing)    return "C";
  if (bit & brd.WKing)     return "K";
  return ".";
}

static std::string printTimeline(Chess *chess, int l) {
  Timeline timeline = *chess->timelines[l];
  int head = timeline.headIndex, tail = timeline.tailIndex;
  std::string output;

  for (int i = tail; i <= head; i++) {
    std::string ltLabel = std::to_string(l - chess->whiteOrigIndex) + "," + std::to_string((i - 4) / 2);
    output += "╔" + ltLabel;
    for (int j = ltLabel.length(); j < 8; j++) output += "═";
    output += "╗";
  }
  output += "\n";

  for (int i = 7; i >= 0; i--) {
    for (int j = tail; j <= head; j++) {
      output += "║";
      for (int k = 0; k < 8; k++) {
        output += BitToChar(1ull << (i * 8 + k), *timeline.turns[j]->board);
      }
      output += "║";
    }
    output += "\n";
  }

  for (int i = tail; i <= head; i++) output += "╚════════╝";
  output += "\n";

  return output;
}

static std::string printBoard(const Board &brd) {
  std::string output = "╔════════╗\n";
  for (int i = 7; i >= 0; i--) {
    output += "║";
    for (int k = 0; k < 8; k++) {
      output += BitToChar(1ull << (i * 8 + k), brd);
    }
    output += "║\n";
  }
  output += "╚════════╝\n";
  return output;
}

// Debugging Only
static U64 dirMaskOr(const DirMask *dirMask) {
  return (dirMask->center | dirMask->orthN | dirMask->orthE | dirMask->orthS | dirMask->orthW | dirMask->diagNE | dirMask->diagNW | dirMask->diagSE | dirMask->diagSW);
}

// Debugging only
static std::string printBoardMask(Chess *chess, const int l, const int t) {
  const BoardMask *mask = chess->timelines[l]->turns[t]->boardMask;
  std::string output = "";
  output += "pawn: " + std::to_string(mask->pawn) + "\n";
  output += "brawn: " + std::to_string(mask->brawn) + "\n";
  output += "knight: " + std::to_string(mask->knight) + "\n";
  output += "king: " + std::to_string(mask->king) + "\n";
  output += "orthN: " + std::to_string(dirMaskOr(mask->orthN)) + "\n";
  output += "orthE: " + std::to_string(dirMaskOr(mask->orthE)) + "\n";
  output += "orthS: " + std::to_string(dirMaskOr(mask->orthS)) + "\n";
  output += "diagNE: " + std::to_string(dirMaskOr(mask->diagNE)) + "\n";
  output += "diagSE: " + std::to_string(dirMaskOr(mask->diagSE)) + "\n";
  output += "diagSW: " + std::to_string(dirMaskOr(mask->diagSW)) + "\n";
  output += "diagNW: " + std::to_string(dirMaskOr(mask->diagNW)) + "\n";
  
  return output;
}

// Debugging only
static std::string printBoardMask(const BoardMask *mask) {
  std::string output = "";
  output += "pawn: " + std::to_string(mask->pawn) + "\n";
  output += "brawn: " + std::to_string(mask->brawn) + "\n";
  output += "knight: " + std::to_string(mask->knight) + "\n";
  output += "king: " + std::to_string(mask->king) + "\n";
  output += "orthN: " + std::to_string(dirMaskOr(mask->orthN)) + "\n";
  output += "orthE: " + std::to_string(dirMaskOr(mask->orthE)) + "\n";
  output += "orthS: " + std::to_string(dirMaskOr(mask->orthS)) + "\n";
  output += "diagNE: " + std::to_string(dirMaskOr(mask->diagNE)) + "\n";
  output += "diagSE: " + std::to_string(dirMaskOr(mask->diagSE)) + "\n";
  output += "diagSW: " + std::to_string(dirMaskOr(mask->diagSW)) + "\n";
  output += "diagNW: " + std::to_string(dirMaskOr(mask->diagNW)) + "\n";
  
  return output;
}

// Debugging Only
static std::string printDirMask(const DirMask *mask) {
  std::string output = "";
  output += "center: " + std::to_string(mask->center) + "\n";
  output += "orthN: " + std::to_string(mask->orthN) + "\n";
  output += "orthE: " + std::to_string(mask->orthE) + "\n";
  output += "orthS: " + std::to_string(mask->orthS) + "\n";
  output += "orthW: " + std::to_string(mask->orthW) + "\n";
  output += "diagNE: " + std::to_string(mask->diagNE) + "\n";
  output += "diagSE: " + std::to_string(mask->diagSE) + "\n";
  output += "diagSW: " + std::to_string(mask->diagSW) + "\n";
  output += "diagNW: " + std::to_string(mask->diagNW) + "\n";
  
  return output;
}
