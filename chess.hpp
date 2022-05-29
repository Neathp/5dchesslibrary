#pragma once
#include "board.hpp"
#include <vector>

struct DirMask {
  U64 center, orthN, orthE, orthS, orthW, diagNE, diagSE, diagSW, diagNW;

  constexpr DirMask(U64 c, U64 on, U64 oe, U64 os, U64 ow, U64 dne, U64 dse, U64 dsw, U64 dnw) :
    center(c), orthN(on), orthE(oe), orthS(os), orthW(ow), diagNE(dne), diagSE(dse), diagSW(dsw), diagNW(dnw)
  {}

  template<bool IsWhite>
  _Compiletime DirMask *iterate(const DirMask& mask, const Board& brd) {
    const U64 c = mask.center, on = mask.orthN, oe = mask.orthE, os = mask.orthS, ow = mask.orthW;
    const U64 dne = mask.diagNE, dse = mask.diagSE, dsw = mask.diagSW, dnw = mask.diagNW;

    if constexpr (IsWhite) {
      const U64 royal = (brd.WRQueen | brd.WKing), prop = ~(brd.Occ ^ royal), propE = prop & 0xFFFEFEFEFEFEFEFF, propW = prop & 0xFF7F7F7F7F7F7FFF;
      return new DirMask((c | royal) & prop, ((on | royal) & prop) << 8, ((oe | royal) & propE) << 1, ((os | royal) & prop) >> 8, ((ow | royal) & propW) >> 1, 
        ((dne | royal) & propE) << 9, ((dse | royal) & propE) >> 7, ((dsw | royal) & propW) >> 9, ((dnw | royal) & propW) << 7);
    } else {
      const U64 royal = (brd.BRQueen | brd.BKing), prop = ~(brd.Occ ^ royal), propE = prop & 0xFFFEFEFEFEFEFEFF, propW = prop & 0xFF7F7F7F7F7F7FFF;
      return new DirMask((c | royal) & prop, ((on | royal) & prop) << 8, ((oe | royal) & propE) << 1, ((os | royal) & prop) >> 8, ((ow | royal) & propW) >> 1, 
        ((dne | royal) & propE) << 9, ((dse | royal) & propE) >> 7, ((dsw | royal) & propW) >> 9, ((dnw | royal) & propW) << 7);
    }
  }

  template<bool IsWhite, bool Occ>
  _Compiletime DirMask iterateOcc(const DirMask& mask, const Board& brd) {
    const U64 c = mask.center, on = mask.orthN, oe = mask.orthE, os = mask.orthS, ow = mask.orthW;
    const U64 dne = mask.diagNE, dse = mask.diagSE, dsw = mask.diagSW, dnw = mask.diagNW;

    if constexpr (IsWhite) {
      const U64 royal = (brd.WRQueen | brd.WKing), prop = ~(brd.Occ ^ royal), propE = prop & 0xFFFEFEFEFEFEFEFF, propW = prop & 0xFF7F7F7F7F7F7FFF;
      if constexpr (Occ) return DirMask(c & prop, (on & prop) << 8, (oe & propE) << 1, (os & prop) >> 8, (ow & propW) >> 1, (dne & propE) << 9, (dse & propE) >> 7, (dsw & propW) >> 9, (dnw & propW) << 7);
      else return DirMask((c | royal) & prop, ((on | royal) & prop) << 8, ((oe | royal) & propE) << 1, ((os | royal) & prop) >> 8, ((ow | royal) & propW) >> 1, ((dne | royal) & propE) << 9, ((dse | royal) & propE) >> 7, ((dsw | royal) & propW) >> 9, ((dnw | royal) & propW) << 7);
    } else {
      const U64 royal = (brd.BRQueen | brd.BKing), prop = ~(brd.Occ ^ royal), propE = prop & 0xFFFEFEFEFEFEFEFF, propW = prop & 0xFF7F7F7F7F7F7FFF;
      if constexpr (Occ) return DirMask(c & prop, (on & prop) << 8, (oe & propE) << 1, (os & prop) >> 8, (ow & propW) >> 1, (dne & propE) << 9, (dse & propE) >> 7, (dsw & propW) >> 9, (dnw & propW) << 7);
      else return DirMask((c | royal) & prop, ((on | royal) & prop) << 8, ((oe | royal) & propE) << 1, ((os | royal) & prop) >> 8, ((ow | royal) & propW) >> 1, ((dne | royal) & propE) << 9, ((dse | royal) & propE) >> 7, ((dsw | royal) & propW) >> 9, ((dnw | royal) & propW) << 7);
    }
  }

  _Compiletime DirMask *combine(const DirMask& mask1, const DirMask& mask2) {
    const U64 c1 = mask1.center, on1 = mask1.orthN, oe1 = mask1.orthE, os1 = mask1.orthS, ow1 = mask1.orthW;
    const U64 dne1 = mask1.diagNE, dse1 = mask1.diagSE, dsw1 = mask1.diagSW, dnw1 = mask1.diagNW;

    const U64 c2 = mask2.center, on2 = mask2.orthN, oe2 = mask2.orthE, os2 = mask2.orthS, ow2 = mask2.orthW;
    const U64 dne2 = mask2.diagNE, dse2 = mask2.diagSE, dsw2 = mask2.diagSW, dnw2 = mask2.diagNW;

    return new DirMask(c1 | c2, on1 | on2, oe1 | oe2, os1 | os2, ow1 | ow2, dne1 | dne2, dse1 | dse2, dsw1 | dsw2, dnw1 | dnw2);
  }

  _Compiletime U64 orth(const DirMask& mask) {
    return mask.orthN | mask.orthE | mask.orthS | mask.orthW;
  }

  _Compiletime U64 diag(const DirMask &mask) {
    return mask.diagNE | mask.diagSE | mask.diagSW | mask.diagNW;
  }
};

struct BoardMask {
  const BoardMask *prevMask;
  const int actionNum, dirModified;

  const U64 pawn, brawn, knight, king;
  const DirMask *orthN, *orthE, *orthS, *diagNE, *diagSE, *diagSW, *diagNW;

  constexpr BoardMask(const BoardMask *prev, const int num, const int dir, const U64 pm, const U64 bm, const U64 nm, const U64 km, 
    const DirMask* on, const DirMask* oe, const DirMask* os, const DirMask* dne, const DirMask* dse, const DirMask* dsw, const DirMask* dnw) :
    prevMask(prev), actionNum(num), dirModified(dir), pawn(pm), brawn(bm), knight(nm), king(km), orthN(on), orthE(oe), orthS(os), diagNE(dne), diagSE(dse), diagSW(dsw), diagNW(dnw)
  {}

  template<bool IsWhite>
  _Compiletime U64 PastCheckMask(const BoardMask &mask, const Board &brd) {
    constexpr bool enemy = !IsWhite;

    const U64 queens = Queens<enemy>(brd) | RoyalQueens<enemy>(brd);
    const U64 princesses = Princesses<enemy>(brd) | queens;

    const U64 orthPieces = Rooks<enemy>(brd) | princesses, diagPieces = Bishops<enemy>(brd) | princesses;
    const U64 triagPieces = Unicorns<enemy>(brd) | queens, quadPieces = Dragons<enemy>(brd) | queens;

    const DirMask on = *mask.orthN, oe = *mask.orthE, os = *mask.orthS, dne = *mask.diagNE, dse = *mask.diagSE, dsw = *mask.diagSW, dnw = *mask.diagNW;

    const U64 orthSquares = on.center | oe.center | os.center;
    const U64 diagSquares = DirMask::orth(on) | DirMask::orth(oe) | DirMask::orth(os) | dne.center | dse.center | dsw.center | dnw.center;
    const U64 triagSquares = DirMask::diag(on) | DirMask::diag(oe) | DirMask::diag(os) | DirMask::orth(dne) | DirMask::orth(dse) | DirMask::orth(dsw) | DirMask::orth(dnw);
    const U64 quadSquares = DirMask::diag(dne) | DirMask::diag(dse) | DirMask::diag(dsw) | DirMask::diag(dnw);

    const U64 vectorPieces = (orthPieces & orthSquares) | (diagPieces & diagSquares) | (triagPieces & triagSquares) | (quadPieces & quadSquares);
    const U64 miscPieces = (Knights<enemy>(brd) & mask.knight) | (KingMovement<enemy>(brd) & mask.king) | (PawnMovement<enemy>(brd) & mask.pawn) | (Brawns<enemy>(brd) & mask.brawn);

    return vectorPieces | miscPieces;
  }
};

constexpr DirMask eDirMask = DirMask(0, 0, 0, 0, 0, 0, 0, 0, 0);
constexpr BoardMask eBoardMask = BoardMask(nullptr, -1, 0, 0, 0, 0, 0, &eDirMask, &eDirMask, &eDirMask, &eDirMask, &eDirMask, &eDirMask, &eDirMask);

struct Turn {
  U64 checkMask, banMask, epTarget;
  const Board *board;
  const BoardMask *boardMask;
    
  constexpr Turn(const Board* brd, const BoardMask* mask) : 
    checkMask(0xFFFFFFFFFFFFFFFFULL), banMask(0x0), epTarget(0x0), board(brd), boardMask(mask)
  {}
};

struct Timeline {
  int headIndex = 0, tailIndex = 0;
  std::vector<Turn*> turns;

  Timeline(int t) {
    turns.reserve(t);
    for (int i = 0; i < t; i++) {
      turns.push_back(new Turn(nullptr, &eBoardMask));
    }
  }
};

struct Chess {
  int actionNum = 0;
  int numLines, numTurns;

  // Original timeline index
  int whiteOrigIndex, blackOrigIndex;

  // Tracks the highest timeline
  int whiteNum = 0, blackNum = 0;

  // Tracks the highest active timeline
  int whiteActiveNum = 0, blackActiveNum = 0;

  std::vector<Timeline*> timelines;
    
  Chess(int l, int t) : numLines(l + 4), numTurns(t + 8), whiteOrigIndex((l + 1) / 2), blackOrigIndex((l + 1) / 2) {
    timelines.reserve(l);
    for (int i = 0; i < l; i++) {
      timelines.push_back(new Timeline(t));
    }
  }
};

_Compiletime void knightBoardMask(Turn *turn, const U64& royalty, const int& actionNum) {
  const BoardMask *cur = turn->boardMask;
  if (cur->actionNum == actionNum) {
    const BoardMask *prev = cur->prevMask;
    turn->boardMask = new BoardMask(prev, actionNum, 0, cur->pawn, cur->brawn, cur->knight | royalty, cur->king, cur->orthN, cur->orthE, cur->orthS, cur->diagNE, cur->diagSE, cur->diagSW, cur->diagNW);
    delete cur;
  } else {
    const BoardMask *prev = cur;
    turn->boardMask = new BoardMask(prev, actionNum, 0, cur->pawn, cur->brawn, cur->knight | royalty, cur->king, cur->orthN, cur->orthE, cur->orthS, cur->diagNE, cur->diagSE, cur->diagSW, cur->diagNW);
  }
}

_Compiletime void d2KnightBoardMask(Turn *turn, const U64& d2Knight, const int& actionNum) {
  const BoardMask *cur = turn->boardMask;
  if (cur->actionNum == actionNum) {
    const BoardMask *prev = cur->prevMask;
    turn->boardMask = new BoardMask(prev, actionNum, 0, cur->pawn, cur->brawn, cur->knight | d2Knight, cur->king, cur->orthN, cur->orthE, cur->orthS, cur->diagNE, cur->diagSE, cur->diagSW, cur->diagNW);
    delete cur;
  } else {
    const BoardMask *prev = cur;
    turn->boardMask = new BoardMask(prev, actionNum, 0, cur->pawn, cur->brawn, cur->knight | d2Knight, cur->king, cur->orthN, cur->orthE, cur->orthS, cur->diagNE, cur->diagSE, cur->diagSW, cur->diagNW);
  }
}

template<bool IsWhite>
_Compiletime void iterateBoardMask(Chess *chess, const int l, const int t) {
  constexpr bool white = IsWhite;
  const int actionNum = chess->actionNum;

  Timeline const *pos2T = chess->timelines[l + 2];
  Timeline const *posT = chess->timelines[l + 1];
  Timeline const *curT = chess->timelines[l];
  Timeline const *negT = chess->timelines[l - 1];
  Timeline const *neg2T = chess->timelines[l - 2];

  const Board curBrd = *curT->turns[t]->board;
  const BoardMask curMask = *curT->turns[t - 1]->boardMask;

  const U64 royalty = Royalty<white>(curBrd);
  const U64 brawn = (royalty & Pawns_NotLeft()) >> 1 | (royalty & Pawns_NotRight()) << 1 | Pawn_Forward<white>(royalty);
  U64 roy = royalty, d1Knight = 0, d2Knight = 0, king = royalty;
  Bitloop (roy) {
    const U64 sq = SquareOf(roy);
    d1Knight |= Lookup::Knight1(sq);
    d2Knight |= Lookup::Knight2(sq);
    king |= Lookup::King(sq);
  }
  
  // OrthN
  {
    Turn *turn = posT->turns[t];
    if (turn->board != nullptr) {
      DirMask temp = DirMask::iterateOcc<white, true>(DirMask::iterateOcc<white, false>(*curMask.orthN, curBrd), *turn->board);
      turn = chess->timelines[l + 2]->turns[t];
      int i = 2;
      while (turn->board != nullptr) {
        temp = DirMask::iterateOcc<white, true>(temp, *turn->board);
        turn = chess->timelines[l + (++i)]->turns[t];
      }
      Turn *curTurn = chess->timelines[l + i]->turns[t - 1];
      const BoardMask *cur = curTurn->boardMask;
      if (cur->actionNum == actionNum) {
        const BoardMask *prev = cur->prevMask;
        curTurn->boardMask = new BoardMask(prev, actionNum, 1, cur->pawn, cur->brawn, cur->knight, cur->king, 
          DirMask::combine(*cur->orthN, temp), cur->orthE, cur->orthS, cur->diagNE, cur->diagSE, cur->diagSW, cur->diagNW);
        if (cur->dirModified == cur->prevMask->dirModified) delete cur->orthN;
        delete cur;
      } else {
        const BoardMask *prev = cur;
        curTurn->boardMask = new BoardMask(prev, actionNum, 1, cur->pawn, cur->brawn, cur->knight, cur->king, 
          DirMask::combine(*cur->orthN, temp), cur->orthE, cur->orthS, cur->diagNE, cur->diagSE, cur->diagSW, cur->diagNW);
      }
    } else {
      Turn *curTurn = posT->turns[t - 1];
      const BoardMask *cur = curTurn->boardMask;
      const DirMask *mask = DirMask::iterate<white>(*curMask.orthN, curBrd);
      if (cur->actionNum == actionNum) {
        const BoardMask *prev = cur->prevMask;
        if constexpr (white) {
          curTurn->boardMask = new BoardMask(prev, actionNum, 1, cur->pawn, cur->brawn, cur->knight | d1Knight, cur->king | king, 
            mask, cur->orthE, cur->orthS, cur->diagNE, cur->diagSE, cur->diagSW, cur->diagNW);
        } else {
          curTurn->boardMask = new BoardMask(prev, actionNum, 1, cur->pawn, cur->brawn | brawn, cur->knight | d1Knight, cur->king | king, 
            mask, cur->orthE, cur->orthS, cur->diagNE, cur->diagSE, cur->diagSW, cur->diagNW);
        }
        if (cur->dirModified == cur->prevMask->dirModified) delete cur->orthN;
        delete cur;
      } else {
        const BoardMask *prev = cur;
        if constexpr (white) {
          curTurn->boardMask = new BoardMask(prev, actionNum, 1, cur->pawn, cur->brawn, cur->knight | d1Knight, cur->king | king, 
            mask, cur->orthE, cur->orthS, cur->diagNE, cur->diagSE, cur->diagSW, cur->diagNW);
        } else {
          curTurn->boardMask = new BoardMask(prev, actionNum, 1, cur->pawn, cur->brawn | brawn, cur->knight | d1Knight, cur->king | king, 
            mask, cur->orthE, cur->orthS, cur->diagNE, cur->diagSE, cur->diagSW, cur->diagNW);
        }
      }
    }
  }
  
  // OrthE
  {
    Turn *curTurn = curT->turns[t + 1];
    const BoardMask *cur = curTurn->boardMask;
    const DirMask *mask = DirMask::iterate<white>(*curMask.orthE, curBrd);
    if (cur->actionNum == actionNum) {
      const BoardMask *prev = cur->prevMask;
      curTurn->boardMask = new BoardMask(prev, actionNum, 2, cur->pawn, cur->brawn | Pawn_Forward<white>(royalty), cur->knight | d1Knight, cur->king | king, 
        cur->orthN, mask, cur->orthS, cur->diagNE, cur->diagSE, cur->diagSW, cur->diagNW);
      if (cur->dirModified == cur->prevMask->dirModified) delete cur->orthE;
      delete cur;
    } else {
      const BoardMask *prev = cur;
      curTurn->boardMask = new BoardMask(prev, actionNum, 2, cur->pawn, cur->brawn | Pawn_Forward<white>(royalty), cur->knight | d1Knight, cur->king | king, 
        cur->orthN, mask, cur->orthS, cur->diagNE, cur->diagSE, cur->diagSW, cur->diagNW);
    }
  }
  
  // OrthS
  {
    Turn *turn = negT->turns[t];
    if (turn->board != nullptr) {
      DirMask temp = DirMask::iterateOcc<white, true>(DirMask::iterateOcc<white, false>(*curMask.orthS, curBrd), *turn->board);
      turn = chess->timelines[l - 2]->turns[t];
      int i = 2;
      while (turn->board != nullptr) {
        temp = DirMask::iterateOcc<white, true>(temp, *turn->board);
        turn = chess->timelines[l - (++i)]->turns[t];
      }
      Turn *curTurn = chess->timelines[l - i]->turns[t - 1];
      const BoardMask *cur = curTurn->boardMask;
      if (cur->actionNum == actionNum) {
        const BoardMask *prev = cur->prevMask;
        curTurn->boardMask = new BoardMask(prev, actionNum, 3, cur->pawn, cur->brawn, cur->knight, cur->king, 
          cur->orthN, cur->orthE, DirMask::combine(*cur->orthS, temp), cur->diagNE, cur->diagSE, cur->diagSW, cur->diagNW);
        if (cur->dirModified == cur->prevMask->dirModified) delete cur->orthS;
        delete cur;
      } else {
        const BoardMask *prev = cur;
        curTurn->boardMask = new BoardMask(prev, actionNum, 3, cur->pawn, cur->brawn, cur->knight, cur->king, 
          cur->orthN, cur->orthE, DirMask::combine(*cur->orthS, temp), cur->diagNE, cur->diagSE, cur->diagSW, cur->diagNW);
      }
    } else {
      Turn *curTurn = negT->turns[t - 1];
      const BoardMask *cur = curTurn->boardMask;
      const DirMask *mask = DirMask::iterate<white>(*curMask.orthS, curBrd);
      if (cur->actionNum == actionNum) {
        const BoardMask *prev = cur->prevMask;
        if constexpr (white) {
          curTurn->boardMask = new BoardMask(prev, actionNum, 3, cur->pawn, cur->brawn | brawn, cur->knight | d1Knight, cur->king | king, 
            cur->orthN, cur->orthE, mask, cur->diagNE, cur->diagSE, cur->diagSW, cur->diagNW);
        } else {
          curTurn->boardMask = new BoardMask(prev, actionNum, 3, cur->pawn, cur->brawn, cur->knight | d1Knight, cur->king | king, 
            cur->orthN, cur->orthE, mask, cur->diagNE, cur->diagSE, cur->diagSW, cur->diagNW);
        }
        if (cur->dirModified == cur->prevMask->dirModified) delete cur->orthS;
        delete cur;
      } else {
        const BoardMask *prev = cur;
        if constexpr (white) {
          curTurn->boardMask = new BoardMask(prev, actionNum, 3, cur->pawn, cur->brawn | brawn, cur->knight | d1Knight, cur->king | king, 
            cur->orthN, cur->orthE, mask, cur->diagNE, cur->diagSE, cur->diagSW, cur->diagNW);
        } else {
          curTurn->boardMask = new BoardMask(prev, actionNum, 3, cur->pawn, cur->brawn, cur->knight | d1Knight, cur->king | king, 
            cur->orthN, cur->orthE, mask, cur->diagNE, cur->diagSE, cur->diagSW, cur->diagNW);
        }
      }
    }
  }
  
  // DiagNE
  {
    Turn *turn = posT->turns[t + 2];
    if (turn->board != nullptr) {
      DirMask temp = DirMask::iterateOcc<white, true>(DirMask::iterateOcc<white, false>(*curMask.diagNE, curBrd), *turn->board);
      turn = chess->timelines[l + 2]->turns[t + 4];
      int i = 2;
      while (turn->board != nullptr) {
        temp = DirMask::iterateOcc<white, true>(temp, *turn->board);
        turn = chess->timelines[l + (++i)]->turns[t + 2 * i];
      }
      Turn *curTurn = chess->timelines[l + i]->turns[t + 2 * i - 1];
      const BoardMask *cur = curTurn->boardMask;
      if (cur->actionNum == actionNum) {
        const BoardMask *prev = cur->prevMask;
        curTurn->boardMask = new BoardMask(prev, actionNum, 4, cur->pawn, cur->brawn, cur->knight, cur->king, 
          cur->orthN, cur->orthE, cur->orthS, DirMask::combine(*cur->diagNE, temp), cur->diagSE, cur->diagSW, cur->diagNW);
        if (cur->dirModified == cur->prevMask->dirModified) delete cur->diagNE;
        delete cur;
      } else {
        const BoardMask *prev = cur;
        curTurn->boardMask = new BoardMask(prev, actionNum, 4, cur->pawn, cur->brawn, cur->knight, cur->king, 
          cur->orthN, cur->orthE, cur->orthS, DirMask::combine(*cur->diagNE, temp), cur->diagSE, cur->diagSW, cur->diagNW);
      }
    } else {
      Turn *curTurn = posT->turns[t + 1];
      const BoardMask *cur = curTurn->boardMask;
      const DirMask *mask = DirMask::iterate<white>(*curMask.diagNE, curBrd);
      if (cur->actionNum == actionNum) {
        const BoardMask *prev = cur->prevMask;
        if constexpr (white) {
          curTurn->boardMask = new BoardMask(prev, actionNum, 4, cur->pawn, cur->brawn, cur->knight, cur->king | king, 
            cur->orthN, cur->orthE, cur->orthS, mask, cur->diagSE, cur->diagSW, cur->diagNW);
        } else {
          curTurn->boardMask = new BoardMask(prev, actionNum, 4, cur->pawn | royalty, cur->brawn, cur->knight, cur->king | king, 
            cur->orthN, cur->orthE, cur->orthS, mask, cur->diagSE, cur->diagSW, cur->diagNW);
        }
        if (cur->dirModified == cur->prevMask->dirModified) delete cur->diagNE;
        delete cur;
      } else {
        const BoardMask *prev = cur;
        if constexpr (white) {
          curTurn->boardMask = new BoardMask(prev, actionNum, 4, cur->pawn, cur->brawn, cur->knight, cur->king | king, 
            cur->orthN, cur->orthE, cur->orthS, mask, cur->diagSE, cur->diagSW, cur->diagNW);
        } else {
          curTurn->boardMask = new BoardMask(prev, actionNum, 4, cur->pawn | royalty, cur->brawn, cur->knight, cur->king | king, 
            cur->orthN, cur->orthE, cur->orthS, mask, cur->diagSE, cur->diagSW, cur->diagNW);
        }
      }
    }
  }
  
  // DiagSE
  {
    Turn *turn = negT->turns[t + 2];
    if (turn->board != nullptr) {
      DirMask temp = DirMask::iterateOcc<white, true>(DirMask::iterateOcc<white, false>(*curMask.diagSE, curBrd), *turn->board);
      turn = chess->timelines[l - 2]->turns[t + 4];
      int i = 2;
      while (turn->board != nullptr) {
        temp = DirMask::iterateOcc<white, true>(temp, *turn->board);
        turn = chess->timelines[l - (++i)]->turns[t + 2 * i];
      }
      Turn *curTurn = chess->timelines[l - i]->turns[t + 2 * i - 1];
      const BoardMask *cur = curTurn->boardMask;
      if (cur->actionNum == actionNum) {
        const BoardMask *prev = cur->prevMask;
        curTurn->boardMask = new BoardMask(prev, actionNum, 5, cur->pawn, cur->brawn, cur->knight, cur->king, 
          cur->orthN, cur->orthE, cur->orthS, cur->diagNE, DirMask::combine(*cur->diagSE, temp), cur->diagSW, cur->diagNW);
        if (cur->dirModified == cur->prevMask->dirModified) delete cur->diagSE;
        delete cur;
      } else {
        const BoardMask *prev = cur;
        curTurn->boardMask = new BoardMask(prev, actionNum, 5, cur->pawn, cur->brawn, cur->knight, cur->king, 
          cur->orthN, cur->orthE, cur->orthS, cur->diagNE, DirMask::combine(*cur->diagSE, temp), cur->diagSW, cur->diagNW);
      }
    } else {
      Turn *curTurn = negT->turns[t + 1];
      const BoardMask *cur = curTurn->boardMask;
      const DirMask *mask = DirMask::iterate<white>(*curMask.diagSE, curBrd);
      if (cur->actionNum == actionNum) {
        const BoardMask *prev = cur->prevMask;
        if constexpr (white) {
          curTurn->boardMask = new BoardMask(prev, actionNum, 5, cur->pawn | royalty, cur->brawn, cur->knight, cur->king | king, 
            cur->orthN, cur->orthE, cur->orthS, cur->diagNE, mask, cur->diagSW, cur->diagNW);
        } else {
          curTurn->boardMask = new BoardMask(prev, actionNum, 5, cur->pawn, cur->brawn, cur->knight, cur->king | king, 
            cur->orthN, cur->orthE, cur->orthS, cur->diagNE, mask, cur->diagSW, cur->diagNW);
        }
        if (cur->dirModified == cur->prevMask->dirModified) delete cur->diagSE;
        delete cur;
      } else {
        const BoardMask *prev = cur;
        if constexpr (white) {
          curTurn->boardMask = new BoardMask(prev, actionNum, 5, cur->pawn | royalty, cur->brawn, cur->knight, cur->king | king, 
            cur->orthN, cur->orthE, cur->orthS, cur->diagNE, mask, cur->diagSW, cur->diagNW);
        } else {
          curTurn->boardMask = new BoardMask(prev, actionNum, 5, cur->pawn, cur->brawn, cur->knight, cur->king | king, 
            cur->orthN, cur->orthE, cur->orthS, cur->diagNE, mask, cur->diagSW, cur->diagNW);
        }
      }
    }
  }
  
  // DiagSW
  {
    Turn *turn = negT->turns[t - 2];
    if (turn->board != nullptr) {
      DirMask temp = DirMask::iterateOcc<white, true>(DirMask::iterateOcc<white, false>(*curMask.diagSW, curBrd), *turn->board);
      turn = chess->timelines[l - 2]->turns[t - 4];
      int i = 2;
      while (turn->board != nullptr) {
        temp = DirMask::iterateOcc<white, true>(temp, *turn->board);
        turn = chess->timelines[l - (++i)]->turns[t - 2 * i];
      }
      Turn *curTurn = chess->timelines[l - i]->turns[t - 2 * i - 1];
      const BoardMask *cur = curTurn->boardMask;
      if (cur->actionNum == actionNum) {
        const BoardMask *prev = cur->prevMask;
        curTurn->boardMask = new BoardMask(prev, actionNum, 6, cur->pawn, cur->brawn, cur->knight, cur->king, 
          cur->orthN, cur->orthE, cur->orthS, cur->diagNE, cur->diagSE, DirMask::combine(*cur->diagSW, temp), cur->diagNW);
        if (cur->dirModified == cur->prevMask->dirModified) delete cur->diagSW;
        delete cur;
      } else {
        const BoardMask *prev = cur;
        curTurn->boardMask = new BoardMask(prev, actionNum, 6, cur->pawn, cur->brawn, cur->knight, cur->king, 
          cur->orthN, cur->orthE, cur->orthS, cur->diagNE, cur->diagSE, DirMask::combine(*cur->diagSW, temp), cur->diagNW);
      }
    } else {
      Turn *curTurn = negT->turns[t - 3];
      const BoardMask *cur = curTurn->boardMask;
      const DirMask *mask = DirMask::iterate<white>(*curMask.diagSW, curBrd);
      if (cur->actionNum == actionNum) {
        const BoardMask *prev = cur->prevMask;
        if constexpr (white) {
          curTurn->boardMask = new BoardMask(prev, actionNum, 6, cur->pawn | royalty, cur->brawn, cur->knight, cur->king | king, 
            cur->orthN, cur->orthE, cur->orthS, cur->diagNE, cur->diagSE, mask, cur->diagNW);
        } else {
          curTurn->boardMask = new BoardMask(prev, actionNum, 6, cur->pawn | royalty, cur->brawn, cur->knight, cur->king | king, 
            cur->orthN, cur->orthE, cur->orthS, cur->diagNE, cur->diagSE, mask, cur->diagNW);
        }
        if (cur->dirModified == cur->prevMask->dirModified) delete cur->diagSW;
        delete cur;
      } else {
        const BoardMask *prev = cur;
        if constexpr (white) {
          curTurn->boardMask = new BoardMask(prev, actionNum, 6, cur->pawn | royalty, cur->brawn, cur->knight, cur->king | king, 
            cur->orthN, cur->orthE, cur->orthS, cur->diagNE, cur->diagSE, mask, cur->diagNW);
        } else {
          curTurn->boardMask = new BoardMask(prev, actionNum, 6, cur->pawn | royalty, cur->brawn, cur->knight, cur->king | king, 
            cur->orthN, cur->orthE, cur->orthS, cur->diagNE, cur->diagSE, mask, cur->diagNW);
        }
      }
    }
  }

  // DiagNW
  {
    Turn *turn = posT->turns[t - 2];
    if (turn->board != nullptr) {
      DirMask temp = DirMask::iterateOcc<white, true>(DirMask::iterateOcc<white, false>(*curMask.diagNW, curBrd), *turn->board);
      turn = chess->timelines[l + 2]->turns[t - 4];
      int i = 2;
      while (turn->board != nullptr) {
        temp = DirMask::iterateOcc<white, true>(temp, *turn->board);
        turn = chess->timelines[l + (++i)]->turns[t - 2 * i];
      }
      Turn *curTurn = chess->timelines[l + i]->turns[t - 2 * i - 1];
      const BoardMask *cur = curTurn->boardMask;
      if (cur->actionNum == actionNum) {
        const BoardMask *prev = cur->prevMask;
        curTurn->boardMask = new BoardMask(prev, actionNum, 7, cur->pawn, cur->brawn, cur->knight, cur->king, 
          cur->orthN, cur->orthE, cur->orthS, cur->diagNE, cur->diagSE, cur->diagSW, DirMask::combine(*cur->diagNW, temp));
        if (cur->dirModified == cur->prevMask->dirModified) delete cur->diagNW;
        delete cur;
      } else {
        const BoardMask *prev = cur;
        curTurn->boardMask = new BoardMask(prev, actionNum, 7, cur->pawn, cur->brawn, cur->knight, cur->king, 
          cur->orthN, cur->orthE, cur->orthS, cur->diagNE, cur->diagSE, cur->diagSW, DirMask::combine(*cur->diagNW, temp));
      }
    } else {
      Turn *curTurn = posT->turns[t - 3];
      const BoardMask *cur = curTurn->boardMask;
      const DirMask *mask = DirMask::iterate<white>(*curMask.diagNW, curBrd);
      if (cur->actionNum == actionNum) {
        const BoardMask *prev = cur->prevMask;
        if constexpr (white) {
          curTurn->boardMask = new BoardMask(prev, actionNum, 7, cur->pawn, cur->brawn, cur->knight, cur->king | king, 
            cur->orthN, cur->orthE, cur->orthS, cur->diagNE, cur->diagSE, cur->diagSW, mask);
        } else {
          curTurn->boardMask = new BoardMask(prev, actionNum, 7, cur->pawn | royalty, cur->brawn, cur->knight, cur->king | king, 
            cur->orthN, cur->orthE, cur->orthS, cur->diagNE, cur->diagSE, cur->diagSW, mask);
        }
        if (cur->dirModified == cur->prevMask->dirModified) delete cur->diagNW;
        delete cur;
      } else {
        const BoardMask *prev = cur;
        if constexpr (white) {
          curTurn->boardMask = new BoardMask(prev, actionNum, 7, cur->pawn, cur->brawn, cur->knight, cur->king | king, 
            cur->orthN, cur->orthE, cur->orthS, cur->diagNE, cur->diagSE, cur->diagSW, mask);
        } else {
          curTurn->boardMask = new BoardMask(prev, actionNum, 7, cur->pawn | royalty, cur->brawn, cur->knight, cur->king | king, 
            cur->orthN, cur->orthE, cur->orthS, cur->diagNE, cur->diagSE, cur->diagSW, mask);
        }
      }
    }
  }

  {
    knightBoardMask(posT->turns[t + 3], royalty, actionNum);
    knightBoardMask(pos2T->turns[t + 1], royalty, actionNum);
    knightBoardMask(pos2T->turns[t - 3], royalty, actionNum);
    knightBoardMask(posT->turns[t - 5], royalty, actionNum);
    knightBoardMask(negT->turns[t + 3], royalty, actionNum);
    knightBoardMask(neg2T->turns[t + 1], royalty, actionNum);
    knightBoardMask(neg2T->turns[t - 3], royalty, actionNum);
    knightBoardMask(negT->turns[t - 5], royalty, actionNum);
  }

  {
    d2KnightBoardMask(pos2T->turns[t - 1], d2Knight, actionNum);
    d2KnightBoardMask(curT->turns[t + 3], d2Knight, actionNum);
    d2KnightBoardMask(neg2T->turns[t - 1], d2Knight, actionNum);
  }
}

// MOST IMPORTANT PART TO STOP MEMORY LEAKS
_Compiletime void removeMask(Turn *turn, const int& actionNum) {
  const BoardMask *cur = turn->boardMask;
  if (cur->actionNum == actionNum) {
    const BoardMask *prev = cur->prevMask;
    if (cur->orthN != prev->orthN) delete cur->orthN;
    if (cur->orthE != prev->orthE) delete cur->orthE;
    if (cur->orthS != prev->orthS) delete cur->orthS;
    if (cur->diagNE != prev->diagNE) delete cur->diagNE;
    if (cur->diagSE != prev->diagSE) delete cur->diagSE;
    if (cur->diagSW != prev->diagSW) delete cur->diagSW;
    if (cur->diagNW != prev->diagNW) delete cur->diagNW;
    delete cur;
    turn->boardMask = prev;
  }
}

_ForceInline void removeBoardMask(Chess *chess, const int l, const int t) {
  const int actionNum = chess->actionNum;

  Timeline const *pos2T = chess->timelines[l + 2];
  Timeline const *posT = chess->timelines[l + 1];
  Timeline const *curT = chess->timelines[l];
  Timeline const *negT = chess->timelines[l - 1];
  Timeline const *neg2T = chess->timelines[l - 2];

  // OrthN
  {
    Turn *turn = posT->turns[t];
    for (int i = 1; turn->board != nullptr;) {
      removeMask(turn, actionNum);
      turn = chess->timelines[l + (++i)]->turns[t];
    }
    removeMask(turn, actionNum);
  }

  // OrthE
  {
    removeMask(curT->turns[t + 2], actionNum);
  }
  
  // OrthS
  {
    Turn *turn = negT->turns[t];
    for (int i = 1; turn->board != nullptr;) {
      removeMask(turn, actionNum);
      turn = chess->timelines[l - (++i)]->turns[t];
    }
    removeMask(turn, actionNum);
  }
  
  // DiagNE
  {
    Turn *turn = posT->turns[t + 2];
    for (int i = 1; turn->board != nullptr;) {
      removeMask(turn, actionNum);
      turn = chess->timelines[l + (++i)]->turns[t + 2 * i];
    }
    removeMask(turn, actionNum);
  }

  // DiagSE
  {
    Turn *turn = negT->turns[t + 2];
    for (int i = 1; turn->board != nullptr;) {
      removeMask(turn, actionNum);
      turn = chess->timelines[l - (++i)]->turns[t + 2 * i];
    }
    removeMask(turn, actionNum);
  }

  // DiagSW
  {
    Turn *turn = negT->turns[t - 2];
    for (int i = 1; turn->board != nullptr;) {
      removeMask(turn, actionNum);
      turn = chess->timelines[l - (++i)]->turns[t - 2 * i];
    }
    removeMask(turn, actionNum);
  }

  // DiagNW
  {
    Turn *turn = posT->turns[t - 2];
    for (int i = 1; turn->board != nullptr;) {
      removeMask(turn, actionNum);
      turn = chess->timelines[l + (++i)]->turns[t - 2 * i];
    }
    removeMask(turn, actionNum);
  }

  {
    removeMask(posT->turns[t + 4], actionNum);
    removeMask(pos2T->turns[t + 2], actionNum);
    removeMask(pos2T->turns[t - 2], actionNum);
    removeMask(posT->turns[t - 4], actionNum);
    removeMask(negT->turns[t + 4], actionNum);
    removeMask(neg2T->turns[t + 2], actionNum);
    removeMask(neg2T->turns[t - 2], actionNum);
    removeMask(negT->turns[t - 4], actionNum);

    removeMask(pos2T->turns[t], actionNum);
    removeMask(curT->turns[t + 4], actionNum);
    removeMask(neg2T->turns[t], actionNum);
  }
}

// Resizing
_Compiletime void resize(Chess *chess) {
  const int size = (chess->numTurns - 4) << 1;

  for (int i = 0; i < size; i++) {
    Timeline *timeline = chess->timelines[i];
    timeline->turns.reserve(size);
    for (int j = chess->numTurns; j < size; j++) {
      timeline->turns.push_back(new Turn(nullptr, &eBoardMask));
    }
  }
  chess->numTurns = size;
}

template<bool IsWhite>
_Compiletime void importBoards(Chess *chess, const Board *brd, const int l, const int t) {
  Timeline *timeline = chess->timelines[l];
  if (t >= chess->numTurns - 8) {resize(chess);}
  timeline->turns[t]->board = brd;
  timeline->headIndex = t;
  timeline->tailIndex += (timeline->tailIndex == 0) ? t : 0;
  

  iterateBoardMask<IsWhite>(chess, l, t);
}

// Non-branching moves
template<bool IsWhite>
_Compiletime void addBoard(Chess *chess, const Board &brd, const int l, const int t) {
  if (t >= chess->numTurns - 8) {resize(chess);}
  Timeline *timeline = chess->timelines[l];
  timeline->turns[t]->board = &brd;
  timeline->headIndex = t;

  iterateBoardMask<IsWhite>(chess, l, t);
}

// Branching moves
template<bool IsWhite>
_Compiletime void addBoard(Chess *chess, const Board *brd, const int t) {
  int l;
  if constexpr (IsWhite) {
    l = chess->whiteOrigIndex + (++chess->whiteNum);
    chess->whiteNum++;
    chess->whiteActiveNum = std::min(chess->whiteNum, chess->blackActiveNum + 1);
    chess->blackActiveNum = std::min(chess->blackNum, chess->whiteActiveNum + 1);
  } else {
    l = chess->blackOrigIndex - (++chess->blackNum);
    chess->blackActiveNum = std::min(chess->blackNum, chess->whiteActiveNum + 1);
    chess->whiteActiveNum = std::min(chess->whiteNum, chess->blackActiveNum + 1);
  }
  Timeline *timeline = chess->timelines[l];
  if (t >= chess->numTurns - 8) {resize(chess);}
  timeline->turns[t]->board = brd;
  timeline->headIndex = t;
  timeline->tailIndex = t;

  iterateBoardMask<IsWhite>(chess, l, t);
}

_ForceInline void removeBoard(Chess *chess, const int l, const int t) {
  removeBoardMask(chess, l, t - 1);

  Timeline *timeline = chess->timelines[l];
  timeline->turns[t]->board = nullptr;
  timeline->headIndex = t - 1;
}

#define PositionToTemplate(func) \
static inline void _##func(const bool wh, Chess *chess, int alpha, int beta, int depth) { \
if (wh)  return func<BoardStatus(0b1)>(chess, alpha, beta, depth); \
if (!wh) return func<BoardStatus(0b0)>(chess, alpha, beta, depth);}