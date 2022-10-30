#pragma once
#include "io.hpp"

static U64 count = 0;

template<BoardState state, typename Chess>
_Compiletime void evaluate(Chess &chess) {
  count++;
}

template<BoardState state>
struct NegaMax {
  template<typename Chess>
  _NoInline void operator() (Chess &chess, int alpha, int beta, int depth) {
    if (depth == 0) {evaluate<state>(chess); return;}

    int l = chess.whiteOrigIndex;
    int t = chess.indices[l].second;
    Turn *turn = &chess.turns[l][t];

    std::vector<Move> moves = std::vector<Move>();
    moves.reserve(40);
    MoveGen::EnumerateMoves<state, chess.t>(turn, l, t, moves);

    for (auto &move : moves) {
      ChessFunc::PlayMove<state>(chess, move, NegaMax<state.SilentMove()>(), alpha, beta, depth - 1);
    }
  }
};