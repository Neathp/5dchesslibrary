#pragma once
#include "board.hpp"

DirMask eDirMask = DirMask(0ull, 0ull, 0ull, 0ull, 0ull, 0ull, 0ull, 0ull, 0ull);
BoardMask eBoardMask = BoardMask(nullptr, 0ull, 0ull, 0ull, 0ull, &eDirMask, &eDirMask, &eDirMask, &eDirMask, &eDirMask, &eDirMask, &eDirMask);

struct Turn {
  bool valid = false;
  U64 checkMask = 0xffffffffffffffffull;
  U64 banMask = 0ull;
  U64 epTarget = 0ull;

  Board board;
  BoardMask *boardMask = &eBoardMask;
  constexpr Turn() {}
};

struct TurnData {
  Turn *turns[18];
  std::pair<DirMask, BoardMask> sliderMasks[7];
  BoardMask knightMasks[11];

  constexpr TurnData() {}
};

template<U8 L, U8 T>
struct Chess {
  constexpr static U8 l = L;
  constexpr static U8 t = T;
  int whiteOrigIndex = L/2;
  int blackOrigIndex = L/2;
  int whiteNum = 0;
  int blackNum = 0;
  int whiteActiveNum = 0;
  int blackActiveNum = 0;

  std::pair<int,int> indices[L];
  Turn turns[L][T];
  constexpr Chess() {}
};

namespace ChessFunc {
  template<bool IsWhite, typename Chess>
  _Compiletime void ImportBoard(Chess &chess, const Board &brd, const U64 epTarget, const int l, const int t) {
    if (l < chess.whiteOrigIndex - chess.blackNum) chess.blackNum = chess.whiteOrigIndex - l;
    if (l > chess.whiteOrigIndex + chess.whiteNum) chess.whiteNum = l - chess.whiteOrigIndex;

    Turn &turn = chess.turns[l][t];
    turn.valid = true;
    turn.board = brd;
    turn.epTarget = epTarget;

    std::pair<int,int> &indices = chess.indices[l];
    indices.first = std::max(indices.first, t);
    indices.second = t;

    IterateBoardMask<IsWhite, chess.t>(&turn);
  }

  template<BoardState state, typename Chess, typename Func, class... Args>
  _Compiletime void PlayMove(Chess &chess, const Move &move, Func f, const Args... args) {
    constexpr bool white = state.WhiteMove;
    Turn &eTurn = chess.turns[move.sTimeline][move.sTurn + 1];
    eTurn.board = chess.turns[move.sTimeline][move.sTurn].board;
    eTurn.valid = true;

    std::pair<int,int> &eIndices = chess.indices[move.sTimeline];
    eIndices.second++;

    switch (move.type) {
      case Norm: 
        BoardFunc::MakeMove<white, Norm>(eTurn.board, move); 
        eTurn.epTarget = 0ull;
        break; 
      case Cap:
        BoardFunc::MakeMove<white, Cap>(eTurn.board, move); 
        eTurn.epTarget = 0ull;
        break; 
      case Push:
        BoardFunc::MakeMove<white, Push>(eTurn.board, move); 
        eTurn.epTarget = 1ull << move.to;
        break; 
      case Enpass:
        BoardFunc::MakeMove<white, Enpass>(eTurn.board, move); 
        eTurn.epTarget = 0ull;
        break; 
      case Promo:
        BoardFunc::MakeMove<white, Promo>(eTurn.board, move); 
        eTurn.epTarget = 0ull;
        break; 
      case PromoCap:
        BoardFunc::MakeMove<white, PromoCap>(eTurn.board, move); 
        eTurn.epTarget = 0ull;
        break;
      case Castle:
        BoardFunc::MakeMove<white, Castle>(eTurn.board, move); 
        eTurn.epTarget = 0ull;
        break; 
    }

    TurnData data;
    IterateBoardMask<state, chess.t>(&eTurn, data);

    f(chess, args...);

    for (auto turn : data.turns) turn->boardMask = turn->boardMask->prevMask;
    eIndices.second = move.sTurn;
    eTurn.valid = false;

    //Board &fromBrd = chess.turns[move.sTimeline][move.sTurn].board;
    //  Board &toBrd = chess.turns[move.eTimeline][move.eTurn].board;
    //  Turn &sTurn = chess.turns[move.sTimeline][move.sTurn + 1];
    //  Turn &eTurn = chess.turns[move.eTimeline][move.eTurn + 1];
//
    //  std::pair<int,int> &sIndices = chess.indices[move.sTimeline];
    //  std::pair<int,int> &eIndices = chess.indices[move.eTimeline];
//
    //  if (move.eTurn != eIndices.second) {
    //    const int l = (white) ? chess.whiteOrigIndex + (++chess.whiteNum) : chess.blackOrigIndex - (++chess.blackNum);
    //    if constexpr (white) {
    //      chess.whiteActiveNum = std::min(chess.whiteNum, chess.blackActiveNum + 1);
    //      chess.blackActiveNum = std::min(chess.blackNum, chess.whiteActiveNum + 1);
    //    } else {
    //      chess.blackActiveNum = std::min(chess.blackNum, chess.whiteActiveNum + 1);
    //      chess.whiteActiveNum = std::min(chess.whiteNum, chess.blackActiveNum + 1);
    //    }
//
    //    eTurn = chess.turns[l][move.eTurn + 1];
    //    eIndices = chess.indices[l];
    //    eIndices.first = move.eTurn + 1;
    //  }
//
    //  sIndices.second++;
    //  sTurn.valid = true;
    //  sTurn.epTarget = 0ull;
//
    //  eIndices.second++;
    //  eTurn.valid = true;
    //  eTurn.epTarget = 0ull;
//
    //  switch (move.type) {
    //    case 7: break; // travel
    //    case 8: break; // travel cap
    //    case 9: break; // travel promo cap
    //  }
  }

  //template<BoardState state, U8 Type, typename Chess>
  //_Compiletime void PlayMove(Chess &chess, const Move &move) {
  //  constexpr bool white = state.WhiteMove;
//
  //  if (Type == 0) {
//
  //  } else {
  //    Turn &eTurn = chess.turns[move.sTimeline][move.sTurn + 1];
  //    eTurn.board = chess.turns[move.sTimeline][move.sTurn].board;
  //    eTurn.valid = true;
//
  //    std::pair<int,int> &eIndices = chess.indices[move.sTimeline];
  //    eIndices.second++;
//
  //    if (Type == 1) {
  //      switch (move.type) {
  //        case Norm: 
  //          BoardFunc::MakeMove<white, Norm>(eTurn.board, move); 
  //          eTurn.epTarget = 0ull;
  //          break; 
  //        case Cap:
  //          BoardFunc::MakeMove<white, Cap>(eTurn.board, move); 
  //          eTurn.epTarget = 0ull;
  //          break; 
  //        case Push:
  //          BoardFunc::MakeMove<white, Push>(eTurn.board, move); 
  //          eTurn.epTarget = 1ull << move.to;
  //          break; 
  //        case Enpass:
  //          BoardFunc::MakeMove<white, Enpass>(eTurn.board, move); 
  //          eTurn.epTarget = 0ull;
  //          break; 
  //        case Promo:
  //          BoardFunc::MakeMove<white, Promo>(eTurn.board, move); 
  //          eTurn.epTarget = 0ull;
  //          break; 
  //        case PromoCap:
  //          BoardFunc::MakeMove<white, PromoCap>(eTurn.board, move); 
  //          eTurn.epTarget = 0ull;
  //          break;
  //        case Castle:
  //          BoardFunc::MakeMove<white, Castle>(eTurn.board, move); 
  //          eTurn.epTarget = 0ull;
  //          break; 
  //      }
  //    } else {
//
  //    }
//
  //    
//
  //    TurnData data;
  //    IterateBoardMask<state, chess.t>(&eTurn, data);
//
  //    for (auto turn : data.turns) turn->boardMask = turn->boardMask->prevMask;
  //    eIndices.second = move.sTurn;
  //    eTurn.valid = false;
  //  }
  //  
  //}

  template<bool IsWhite, U16 T, Direction Dir>
  _Compiletime void IterateSlider(Turn *turn, const Board &brd, const DirMask *dirMask, const U64 pawn, const U64 brawn, const U64 d1Knight, const U64 king) {
    BoardMask *cur;
    if constexpr (Dir == East) {
      cur = turn->boardMask;
      BoardMask *prev = (cur == &eBoardMask) ? cur : cur->prevMask;
      turn->boardMask = new BoardMask(prev, cur->pawn, cur->brawn | brawn, cur->knight | d1Knight, cur->king | king, cur->north, BoardFunc::Iterate<IsWhite, true, false>(dirMask, brd), cur->south, cur->northEast, cur->southEast, cur->southWest, cur->northWest);
    } else if (turn->valid != false) {
      DirMask temp = BoardFunc::Iterate<IsWhite, false, false>(dirMask, brd);
      temp = BoardFunc::Iterate<IsWhite, false, true>(&temp, turn->board);
      turn += (Dir == North)     ? T      : (Dir == West)      ? -1     : (Dir == South)     ? -T    : (Dir == NorthEast) ? T + 2 
            : (Dir == SouthEast) ? -T - 2 : (Dir == SouthWest) ? -T + 2 : (Dir == NorthWest) ? T - 2                      : 1;
      
      while (turn->valid != false) {
        temp = BoardFunc::Iterate<IsWhite, false, true>(&temp, turn->board);
        turn += (Dir == North)     ? T      : (Dir == West)      ? -1     : (Dir == South)     ? -T    : (Dir == NorthEast) ? T + 2 
              : (Dir == SouthEast) ? -T - 2 : (Dir == SouthWest) ? -T + 2 : (Dir == NorthWest) ? T - 2                      : 1;
      }

      cur = (--turn)->boardMask;
      BoardMask *prev = (cur == &eBoardMask) ? cur : cur->prevMask;
      turn->boardMask = (Dir == North)     ? new BoardMask(prev, cur->pawn, cur->brawn, cur->knight, cur->king, BoardFunc::Combine<true>(cur->north, temp), cur->east, cur->south, cur->northEast, cur->southEast, cur->southWest, cur->northWest)
                      : (Dir == South)     ? new BoardMask(prev, cur->pawn, cur->brawn, cur->knight, cur->king, cur->north, cur->east, BoardFunc::Combine<true>(cur->south, temp), cur->northEast, cur->southEast, cur->southWest, cur->northWest)
                      : (Dir == NorthEast) ? new BoardMask(prev, cur->pawn, cur->brawn, cur->knight, cur->king, cur->north, cur->east, cur->south, BoardFunc::Combine<true>(cur->northEast, temp), cur->southEast, cur->southWest, cur->northWest)
                      : (Dir == SouthEast) ? new BoardMask(prev, cur->pawn, cur->brawn, cur->knight, cur->king, cur->north, cur->east, cur->south, cur->northEast, BoardFunc::Combine<true>(cur->southEast, temp), cur->southWest, cur->northWest)
                      : (Dir == SouthWest) ? new BoardMask(prev, cur->pawn, cur->brawn, cur->knight, cur->king, cur->north, cur->east, cur->south, cur->northEast, cur->southEast, BoardFunc::Combine<true>(cur->southWest, temp), cur->northWest)
                                           : new BoardMask(prev, cur->pawn, cur->brawn, cur->knight, cur->king, cur->north, cur->east, cur->south, cur->northEast, cur->southEast, cur->southWest, BoardFunc::Combine<true>(cur->northWest, temp));
    } else {
      cur = (--turn)->boardMask;
      BoardMask *prev = (cur == &eBoardMask) ? cur : cur->prevMask;
      turn->boardMask = (IsWhite) ? 
          (Dir == North)     ? new BoardMask(prev, cur->pawn, cur->brawn, cur->knight | d1Knight, cur->king | king, BoardFunc::Iterate<IsWhite, true, false>(dirMask, brd), cur->east, cur->south, cur->northEast, cur->southEast, cur->southWest, cur->northWest)
        : (Dir == South)     ? new BoardMask(prev, cur->pawn, cur->brawn | brawn, cur->knight | d1Knight, cur->king | king, cur->north, cur->east, BoardFunc::Iterate<IsWhite, true, false>(dirMask, brd), cur->northEast, cur->southEast, cur->southWest, cur->northWest)
        : (Dir == NorthEast) ? new BoardMask(prev, cur->pawn, cur->brawn, cur->knight, cur->king | king, cur->north, cur->east, cur->south, BoardFunc::Iterate<IsWhite, true, false>(dirMask, brd), cur->southEast, cur->southWest, cur->northWest)
        : (Dir == SouthEast) ? new BoardMask(prev, cur->pawn | pawn, cur->brawn, cur->knight, cur->king | king, cur->north, cur->east, cur->south, cur->northEast, BoardFunc::Iterate<IsWhite, true, false>(dirMask, brd), cur->southWest, cur->northWest)
        : (Dir == SouthWest) ? new BoardMask(prev, cur->pawn | pawn, cur->brawn, cur->knight, cur->king | king, cur->north, cur->east, cur->south, cur->northEast, cur->southEast, BoardFunc::Iterate<IsWhite, true, false>(dirMask, brd), cur->northWest)
                             : new BoardMask(prev, cur->pawn, cur->brawn, cur->knight, cur->king | king, cur->north, cur->east, cur->south, cur->northEast, cur->southEast, cur->southWest, BoardFunc::Iterate<IsWhite, true, false>(dirMask, brd))
        : (Dir == North)     ? new BoardMask(prev, cur->pawn, cur->brawn | brawn, cur->knight | d1Knight, cur->king | king, BoardFunc::Iterate<IsWhite, true, false>(dirMask, brd), cur->east, cur->south, cur->northEast, cur->southEast, cur->southWest, cur->northWest)
        : (Dir == South)     ? new BoardMask(prev, cur->pawn, cur->brawn, cur->knight | d1Knight, cur->king | king, cur->north, cur->east, BoardFunc::Iterate<IsWhite, true, false>(dirMask, brd), cur->northEast, cur->southEast, cur->southWest, cur->northWest)
        : (Dir == NorthEast) ? new BoardMask(prev, cur->pawn | pawn, cur->brawn, cur->knight, cur->king | king, cur->north, cur->east, cur->south, BoardFunc::Iterate<IsWhite, true, false>(dirMask, brd), cur->southEast, cur->southWest, cur->northWest)
        : (Dir == SouthEast) ? new BoardMask(prev, cur->pawn, cur->brawn, cur->knight, cur->king | king, cur->north, cur->east, cur->south, cur->northEast, BoardFunc::Iterate<IsWhite, true, false>(dirMask, brd), cur->southWest, cur->northWest)
        : (Dir == SouthWest) ? new BoardMask(prev, cur->pawn, cur->brawn, cur->knight, cur->king | king, cur->north, cur->east, cur->south, cur->northEast, cur->southEast, BoardFunc::Iterate<IsWhite, true, false>(dirMask, brd), cur->northWest)
                             : new BoardMask(prev, cur->pawn | pawn, cur->brawn, cur->knight, cur->king | king, cur->north, cur->east, cur->south, cur->northEast, cur->southEast, cur->southWest, BoardFunc::Iterate<IsWhite, true, false>(dirMask, brd));
    }

    if (cur != &eBoardMask) {
      if constexpr (Dir == North)     if (cur->north != &eDirMask)     delete cur->north;
      if constexpr (Dir == East)      if (cur->east != &eDirMask)      delete cur->east;
      if constexpr (Dir == South)     if (cur->south != &eDirMask)     delete cur->south;
      if constexpr (Dir == NorthEast) if (cur->northEast != &eDirMask) delete cur->northEast;
      if constexpr (Dir == SouthEast) if (cur->southEast != &eDirMask) delete cur->southEast;
      if constexpr (Dir == SouthWest) if (cur->southWest != &eDirMask) delete cur->southWest;
      if constexpr (Dir == NorthWest) if (cur->northWest != &eDirMask) delete cur->northWest;
      delete cur;
    }
  }

  _Compiletime void KnightMask(Turn *turn, const U64 knight) {
    BoardMask *cur = turn->boardMask;
    BoardMask *prev = (cur == &eBoardMask) ? cur : cur->prevMask;
    turn->boardMask = new BoardMask(prev, cur->pawn, cur->brawn, cur->knight | knight, cur->king, cur->north, cur->east, cur->south, cur->northEast, cur->southEast, cur->southWest, cur->northWest);
    if (cur != &eBoardMask) delete cur;
  }

  template<bool IsWhite, U16 T>
  _Compiletime void IterateBoardMask(Turn *turn) {
    constexpr U16 T2 = T << 1;
    const Board &brd = turn->board;
    BoardMask *mask = (turn - 1)->boardMask;

    const U64 royalty = BoardFunc::Royalty<IsWhite, true>(brd);
    const U64 brawnForward = BoardFunc::Forward<IsWhite>(royalty);
    const U64 brawn = (royalty & BoardFunc::NotLeft()) >> 1 | (royalty & BoardFunc::NotRight()) << 1 | brawnForward;
    U64 roy = royalty, d1Knight = 0, d2Knight = 0, king = royalty;
    Bitloop (roy) {
      const U64 sq = SquareOf(roy);
      d1Knight |= Lookup::Knight1(sq);
      d2Knight |= Lookup::Knight2(sq);
      king |= Lookup::King(sq);
    }

    IterateSlider<IsWhite, T, North>(turn + T, brd, mask->north, royalty, brawn, d1Knight, king);
    IterateSlider<IsWhite, T, East >(turn + 1, brd, mask->east, royalty, brawnForward, d1Knight, king);
    IterateSlider<IsWhite, T, South>(turn - T, brd, mask->south, royalty, brawn, d1Knight, king);
    IterateSlider<IsWhite, T, NorthEast>(turn + T + 2, brd, mask->northEast, royalty, brawn, d1Knight, king);
    IterateSlider<IsWhite, T, SouthEast>(turn - T + 2, brd, mask->southEast, royalty, brawn, d1Knight, king);
    IterateSlider<IsWhite, T, SouthWest>(turn - T - 2, brd, mask->southWest, royalty, brawn, d1Knight, king);
    IterateSlider<IsWhite, T, NorthWest>(turn + T - 2, brd, mask->northWest, royalty, brawn, d1Knight, king);

    KnightMask(turn + T + 3, royalty);
    KnightMask(turn + T2 + 1, royalty);
    KnightMask(turn + T2 - 3, royalty);
    KnightMask(turn + T - 5, royalty);
    KnightMask(turn - T + 3, royalty);
    KnightMask(turn - T2 + 1, royalty);
    KnightMask(turn - T2 - 3, royalty);
    KnightMask(turn - T - 5, royalty);

    KnightMask(turn + T2, d2Knight);
    KnightMask(turn + 3, d2Knight);
    KnightMask(turn - T2, d2Knight);
  }

  template<bool IsWhite, U16 T, Direction Dir>
  _Compiletime void IterateSlider(Turn *turn, const Board &brd, const DirMask *dirMask, Turn **turns, std::pair<DirMask, BoardMask> *sliderMasks, const U64 pawn, const U64 brawn, const U64 d1Knight, const U64 king) {
    DirMask temp = BoardFunc::Iterate<IsWhite, false, false>(dirMask, brd);
    if constexpr (Dir == East) {
      BoardMask *cur = turn->boardMask;
      sliderMasks->first = temp;
      sliderMasks->second = BoardMask(cur, cur->pawn, cur->brawn | brawn, cur->knight | d1Knight, cur->king | king, cur->north, &sliderMasks->first, cur->south, cur->northEast, cur->southEast, cur->southWest, cur->northWest);
    } else if (turn->valid != false) {
      temp = BoardFunc::Iterate<IsWhite, false, true>(dirMask, turn->board);
      turn += (Dir == North)     ? T      : (Dir == West)      ? -1     : (Dir == South)     ? -T    : (Dir == NorthEast) ? T + 2 
            : (Dir == SouthEast) ? -T - 2 : (Dir == SouthWest) ? -T + 2 : (Dir == NorthWest) ? T - 2                      : 1;

      while (turn->valid != false) {
        temp = BoardFunc::Iterate<IsWhite, false, true>(&temp, turn->board);
        turn += (Dir == North)     ? T      : (Dir == West)      ? -1     : (Dir == South)     ? -T    : (Dir == NorthEast) ? T + 2 
              : (Dir == SouthEast) ? -T - 2 : (Dir == SouthWest) ? -T + 2 : (Dir == NorthWest) ? T - 2                      : 1;
      }

      BoardMask *cur = (--turn)->boardMask;
      sliderMasks->first = (Dir == North)     ? BoardFunc::Combine<false>(cur->north, temp)     : (Dir == South)     ? BoardFunc::Combine<false>(cur->south, temp)
                         : (Dir == NorthEast) ? BoardFunc::Combine<false>(cur->northEast, temp) : (Dir == SouthEast) ? BoardFunc::Combine<false>(cur->southEast, temp)
                         : (Dir == SouthWest) ? BoardFunc::Combine<false>(cur->southWest, temp) :                      BoardFunc::Combine<false>(cur->northWest, temp);

      sliderMasks->second = (Dir == North)     ? BoardMask(cur, cur->pawn, cur->brawn, cur->knight, cur->king, &sliderMasks->first, cur->east, cur->south, cur->northEast, cur->southEast, cur->southWest, cur->northWest) 
                          : (Dir == South)     ? BoardMask(cur, cur->pawn, cur->brawn, cur->knight, cur->king, cur->north, cur->east, &sliderMasks->first, cur->northEast, cur->southEast, cur->southWest, cur->northWest)
                          : (Dir == NorthEast) ? BoardMask(cur, cur->pawn, cur->brawn, cur->knight, cur->king, cur->north, cur->east, cur->south, &sliderMasks->first, cur->southEast, cur->southWest, cur->northWest) 
                          : (Dir == SouthEast) ? BoardMask(cur, cur->pawn, cur->brawn, cur->knight, cur->king, cur->north, cur->east, cur->south, cur->northEast, &sliderMasks->first, cur->southWest, cur->northWest)
                          : (Dir == SouthWest) ? BoardMask(cur, cur->pawn, cur->brawn, cur->knight, cur->king, cur->north, cur->east, cur->south, cur->northEast, cur->southEast, &sliderMasks->first, cur->northWest) 
                                               : BoardMask(cur, cur->pawn, cur->brawn, cur->knight, cur->king, cur->north, cur->east, cur->south, cur->northEast, cur->southEast, cur->southWest, &sliderMasks->first);
    } else {
      BoardMask *cur = (--turn)->boardMask;
      sliderMasks->first = temp;
      sliderMasks->second = (IsWhite) ? 
          (Dir == North)     ? BoardMask(cur, cur->pawn, cur->brawn, cur->knight | d1Knight, cur->king | king, &sliderMasks->first, cur->east, cur->south, cur->northEast, cur->southEast, cur->southWest, cur->northWest)
        : (Dir == South)     ? BoardMask(cur, cur->pawn, cur->brawn | brawn, cur->knight | d1Knight, cur->king | king, cur->north, cur->east, &sliderMasks->first, cur->northEast, cur->southEast, cur->southWest, cur->northWest)
        : (Dir == NorthEast) ? BoardMask(cur, cur->pawn, cur->brawn, cur->knight, cur->king | king, cur->north, cur->east, cur->south, &sliderMasks->first, cur->southEast, cur->southWest, cur->northWest)
        : (Dir == SouthEast) ? BoardMask(cur, cur->pawn | pawn, cur->brawn, cur->knight, cur->king | king, cur->north, cur->east, cur->south, cur->northEast, &sliderMasks->first, cur->southWest, cur->northWest)
        : (Dir == SouthWest) ? BoardMask(cur, cur->pawn | pawn, cur->brawn, cur->knight, cur->king | king, cur->north, cur->east, cur->south, cur->northEast, cur->southEast, &sliderMasks->first, cur->northWest)
                             : BoardMask(cur, cur->pawn, cur->brawn, cur->knight, cur->king | king, cur->north, cur->east, cur->south, cur->northEast, cur->southEast, cur->southWest, &sliderMasks->first)
        : (Dir == North)     ? BoardMask(cur, cur->pawn, cur->brawn | brawn, cur->knight | d1Knight, cur->king | king, &sliderMasks->first, cur->east, cur->south, cur->northEast, cur->southEast, cur->southWest, cur->northWest)
        : (Dir == South)     ? BoardMask(cur, cur->pawn, cur->brawn, cur->knight | d1Knight, cur->king | king, cur->north, cur->east, &sliderMasks->first, cur->northEast, cur->southEast, cur->southWest, cur->northWest)
        : (Dir == NorthEast) ? BoardMask(cur, cur->pawn | pawn, cur->brawn, cur->knight, cur->king | king, cur->north, cur->east, cur->south, &sliderMasks->first, cur->southEast, cur->southWest, cur->northWest)
        : (Dir == SouthEast) ? BoardMask(cur, cur->pawn, cur->brawn, cur->knight, cur->king | king, cur->north, cur->east, cur->south, cur->northEast, &sliderMasks->first, cur->southWest, cur->northWest)
        : (Dir == SouthWest) ? BoardMask(cur, cur->pawn, cur->brawn, cur->knight, cur->king | king, cur->north, cur->east, cur->south, cur->northEast, cur->southEast, &sliderMasks->first, cur->northWest)
                             : BoardMask(cur, cur->pawn | pawn, cur->brawn, cur->knight, cur->king | king, cur->north, cur->east, cur->south, cur->northEast, cur->southEast, cur->southWest, &sliderMasks->first);
    }
    *turns = turn;
    turn->boardMask = &sliderMasks->second;
  }

  _Compiletime void IterateKnight(Turn *turn, Turn **turns, BoardMask *knightMasks, const U64 knight) {
    BoardMask *cur = turn->boardMask;
    *knightMasks = BoardMask(cur, cur->pawn, cur->brawn, cur->knight | knight, cur->king, cur->north, cur->east, cur->south, cur->northEast, cur->southEast, cur->southWest, cur->northWest);
    *turns = turn;
    turn->boardMask = knightMasks;
  }

  template<BoardState state, U16 T>
  _Compiletime void IterateBoardMask(Turn *turn, TurnData &data) {
    constexpr bool IsWhite = state.WhiteMove;
    constexpr U16 T2 = T << 1;

    const Board &brd = turn->board;
    BoardMask *mask = (turn - 1)->boardMask;

    const U64 royalty = BoardFunc::Royalty<IsWhite, state.RQueen>(brd);
    const U64 brawnForward = BoardFunc::Forward<IsWhite>(royalty);
    const U64 brawn = (royalty & BoardFunc::NotLeft()) >> 1 | (royalty & BoardFunc::NotRight()) << 1 | brawnForward;
    U64 roy = royalty, d1Knight = 0, d2Knight = 0, king = royalty;
    Bitloop (roy) {
      const U64 sq = SquareOf(roy);
      d1Knight |= Lookup::Knight1(sq);
      d2Knight |= Lookup::Knight2(sq);
      king |= Lookup::King(sq);
    }

    Turn **turns = data.turns;
    std::pair<DirMask, BoardMask> *sliderMasks = data.sliderMasks;
    IterateSlider<IsWhite, T, North>(turn + T, brd, mask->north, turns, sliderMasks, royalty, brawn, d1Knight, king);
    IterateSlider<IsWhite, T, East >(turn + 1, brd, mask->east, turns + 1, sliderMasks + 1, royalty, brawnForward, d1Knight, king);
    IterateSlider<IsWhite, T, South>(turn - T, brd, mask->south, turns + 2, sliderMasks + 2, royalty, brawn, d1Knight, king);
    IterateSlider<IsWhite, T, NorthEast>(turn + T + 2, brd, mask->northEast, turns + 3, sliderMasks + 3, royalty, brawn, d1Knight, king);
    IterateSlider<IsWhite, T, SouthEast>(turn - T + 2, brd, mask->southEast, turns + 4, sliderMasks + 4, royalty, brawn, d1Knight, king);
    IterateSlider<IsWhite, T, SouthWest>(turn - T - 2, brd, mask->southWest, turns + 5, sliderMasks + 5, royalty, brawn, d1Knight, king);
    IterateSlider<IsWhite, T, NorthWest>(turn + T - 2, brd, mask->northWest, turns + 6, sliderMasks + 6, royalty, brawn, d1Knight, king);

    BoardMask *knightMasks = data.knightMasks;
    IterateKnight(turn + T + 3, turns + 7, knightMasks, royalty);
    IterateKnight(turn + T2 + 1, turns + 8, knightMasks + 1, royalty);
    IterateKnight(turn + T2 - 3, turns + 9, knightMasks + 2, royalty);
    IterateKnight(turn + T - 5, turns + 10, knightMasks + 3, royalty);
    IterateKnight(turn - T + 3, turns + 11, knightMasks + 4, royalty);
    IterateKnight(turn - T2 + 1, turns + 12, knightMasks + 5, royalty);
    IterateKnight(turn - T2 - 3, turns + 13, knightMasks + 6, royalty);
    IterateKnight(turn - T - 5, turns + 14, knightMasks + 7, royalty);
    
    IterateKnight(turn + T2 - 1, turns + 15, knightMasks + 8, d2Knight);
    IterateKnight(turn + 3, turns + 16, knightMasks + 9, d2Knight);
    IterateKnight(turn - T2 - 1, turns + 17, knightMasks + 10, d2Knight);
  }

  //template<BoardState state, U8 L, typename Chess, typename Func, class... Args>
  //_Compiletime void GenerateCombinations(Chess &chess, std::vector<std::vector<Move>*> &moveList, const U64 index, Func f, const Args... args) {
  //  // Final Step
  //  if (L != chess.l && L != moveList.size()) {
  //    // No move case
  //    GenerateCombinations<state, L + 1>(chess, moveList, index, f, args);
//
  //    // All but last move case
  //    for (U64 i = 0; i < index; i++) {
  //      PlayMove<state>(chess, moveList[L]->[i], f, args);
//
  //      GenerateCombinations<state, L + 1>(chess, moveList, index - i, f, args);
  //    }
  //  }
  //  const Move move = moveList[L]->[index];
  //  PlayMove<state>(chess, move, f, args);
//
  //  // Perform permutations here
//
  //  // Recurse!
  //  f(chess, args...);
//
  //  
  //}
};