#pragma once
#include <chrono>
#include <iostream>

#include "movegen.hpp"

static U64 count = 0;

template<class BoardStatus status>
static _ForceInline void evaluate(Chess *chess) {
  //std::cout << printTimeline(chess, chess->whiteOrigIndex);
  count++;
}

template<class BoardStatus status>
static _NoInline void negaMax(Chess *chess, int alpha, int beta, int depth) {
  if (depth == 0) {evaluate<status>(chess); return;}

  int l = chess->whiteOrigIndex;
  Timeline *timeline = chess->timelines[l];
  int t = timeline->headIndex;
  Turn *turn = timeline->turns[t];

  std::vector<Move> moves = std::vector<Move>();
  moves.reserve(40);
  Movelist::EnumerateMoves<status.WhiteMove, MoveReciever>(*turn, moves);

  for (auto &&move : moves) {
    //std::cout << printBoard(move.board);
    chess->actionNum++;
    addBoard<status.WhiteMove>(chess, move.board, l, t + 1);

    negaMax<status.SilentMove()>(chess, alpha, beta, depth - 1);

    removeBoard(chess, l, t + 1);
    chess->actionNum--;
  }
}

PositionToTemplate(negaMax);

int main() {
  // To allow unicode characters
  system("chcp 65001 > nul");
  
  Chess *chess = new Chess(32, 128);
  const std::string fen1 = "[r*nbqk*bnr*/p*p*p*p*p*p*p*p*/8/8/8/8/P*P*P*P*P*P*P*P*/R*NBQK*BNR*:0:1:w]";
  importFEN(chess, fen1);

  std::cout << printTimeline(chess, chess->whiteOrigIndex);

  auto begin = std::chrono::high_resolution_clock::now();
  //_negaMax(true, chess, 0, 0, 5);
  auto end = std::chrono::high_resolution_clock::now();
  std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(end-begin).count()/1000000000.0 << " s\n";
  
  std::cout << count << "\n";

  return 0;
}