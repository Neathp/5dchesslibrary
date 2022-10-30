#pragma once
#include <utility>
#include <iostream>
#include "attacktable.hpp"

struct Board {
  Piece board[64];
  U64 bitBoards[28];

  constexpr Board() {}
};

struct DirMask {
  U64 center, north, east, south, west, northEast, southEast, southWest, northWest;

  constexpr DirMask() {}
  constexpr DirMask(U64 c, U64 on, U64 oe, U64 os, U64 ow, U64 dne, U64 dse, U64 dsw, U64 dnw) : 
    center(c), north(on), east(oe), south(os), west(ow), northEast(dne), southEast(dse), southWest(dsw), northWest(dnw) 
  {}
};

struct BoardMask {
  BoardMask *prevMask;
  U64 pawn, brawn, knight, king;
  const DirMask *north, *east, *south, *northEast, *southEast, *southWest, *northWest;

  constexpr BoardMask() {}
  constexpr BoardMask(BoardMask *prev, const U64 pm, const U64 bm, const U64 nm, const U64 km, const DirMask* on, const DirMask* oe, const DirMask* os, const DirMask* dne, const DirMask* dse, const DirMask* dsw, const DirMask* dnw) :
    prevMask(prev), pawn(pm), brawn(bm), knight(nm), king(km), north(on), east(oe), south(os), northEast(dne), southEast(dse), southWest(dsw), northWest(dnw)
  {}
};

struct Move {
  const U8 from, to, special1, special2, type, sTimeline, sTurn, eTimeline, eTurn;
  int score = 0;

  constexpr Move(const U8 f, const U8 t, const U8 s1, const U8 s2, const U8 tp, const U8 sl, const U8 st, const U8 el, const U8 et) :
    from(f), to(t), special1(s1), special2(s2), type(tp), sTimeline(sl), sTurn(st), eTimeline(el), eTurn(et)
  {}
};

namespace BoardFunc {
  template<bool IsWhite, MoveType Type>
  _Compiletime void MakeMove(Board &brd, const Move &move) {
    constexpr Piece friendly = (IsWhite) ? White : Black;
    if (Type == Castle) {
      constexpr Piece king = (IsWhite) ? WKing : BKing;
      constexpr Piece rook = (IsWhite) ? WRook : BRook;

      const U64 kingSwitch = (1ull << move.from) | (1ull << move.to);
      const U64 rookSwitch = (1ull << move.special1) | (1ull << move.special2);
      const U64 totalSwitch = kingSwitch ^ rookSwitch;

      // Mailboxboard information
      brd.board[move.from] = Occ;
      brd.board[move.to] = Occ;
      brd.board[move.special1] = king;
      brd.board[move.special2] = rook;

      // Bitboard information
      brd.bitBoards[king] ^= kingSwitch;
      brd.bitBoards[rook] ^= rookSwitch;
      brd.bitBoards[friendly] ^= totalSwitch;
      brd.bitBoards[Occ] ^= totalSwitch;
      brd.bitBoards[UnMoved] &= ~(kingSwitch | rookSwitch);
    } else {
      const Piece piece = brd.board[move.from];

      const U64 bitFrom = 1ull << move.from;
      const U64 bitTo = (Type == Enpass) ? 1ull << move.special1 : 1ull << move.to;
      const U64 bitFromTo = (Type == Enpass) ? bitFrom | (1ull << move.to) : bitFrom | bitTo;

      if (Type == Cap || Type == Enpass || Type == PromoCap) {
        constexpr Piece enemy = (IsWhite) ? Black : White;
        const Piece cap = (Type == Enpass) ? brd.board[move.special1] : brd.board[move.to];

        brd.bitBoards[enemy] ^= bitTo;
        brd.bitBoards[cap] ^= bitTo;
      }

      brd.board[move.from] = Occ;
      brd.board[move.to] = (Type == Promo || Type == PromoCap) ? Piece(move.special1) : piece;
      if (Type == Enpass) brd.board[move.special1] = Occ;

      if (Type == Promo || Type == PromoCap) brd.bitBoards[move.special1] ^= bitTo;
      brd.bitBoards[friendly] ^= bitFromTo;
      brd.bitBoards[piece] ^= (Type == Promo || Type == PromoCap) ? bitFrom : bitFromTo;
      brd.bitBoards[Occ] ^= (Type == Norm || Type == Push || Type == Promo) ? bitFromTo 
                          : (Type == Cap || Type == PromoCap) ? bitFrom
                          : bitFromTo | bitTo;
      brd.bitBoards[UnMoved] &= ~bitFromTo;
    }
  }

  template<bool IsWhite>
  _Compiletime void MoveTravel(const Board &fromBrd, const Board &toBrd, Board &fromNext, Board &toNext, const U8 from, const U8 to) {
    fromNext = fromBrd; 
    toNext = toBrd;

    // Precalculations
    constexpr Piece us = (IsWhite) ? White : Black;
    const Piece piece = fromNext.board[from];
    const U64 bitFrom = 1ull << from;
    const U64 bitTo = 1ull << to;

    // Mailboxboard information
    fromNext.board[from] = Occ;
    toNext.board[to] = piece;

    // Bitboard information
    fromNext.bitBoards[piece] ^= bitFrom;
    fromNext.bitBoards[us] ^= bitFrom;
    fromNext.bitBoards[Occ] ^= bitFrom;
    fromNext.bitBoards[UnMoved] &= ~bitFrom;
    toNext.bitBoards[piece] ^= bitTo;
    toNext.bitBoards[us] ^= bitTo;
    toNext.bitBoards[Occ] ^= bitTo;
  }

  template<bool IsWhite>
  _Compiletime void MoveTravelCap(const Board &fromBrd, const Board &toBrd, Board &fromNext, Board &toNext, const U8 from, const U8 to) {
    fromNext = fromBrd; 
    toNext = toBrd;

    // Precalculations
    constexpr Piece us = (IsWhite) ? White : Black;
    constexpr Piece them = (IsWhite) ? Black : White;
    const Piece piece = fromNext.board[from];
    const Piece cap = toNext.board[to];
    const U64 bitFrom = 1ull << from;
    const U64 bitTo = 1ull << to;

    // Mailboxboard information
    fromNext.board[from] = Occ;
    toNext.board[to] = piece;

    // Bitboard information
    fromNext.bitBoards[piece] ^= bitFrom;
    fromNext.bitBoards[us] ^= bitFrom;
    fromNext.bitBoards[Occ] ^= bitFrom;
    fromNext.bitBoards[UnMoved] &= ~bitFrom;
    toNext.bitBoards[piece] ^= bitTo;
    toNext.bitBoards[cap] ^= bitTo;
    toNext.bitBoards[us] ^= bitTo;
    toNext.bitBoards[them] ^= bitTo;
    toNext.bitBoards[UnMoved] &= ~bitTo;
  }

  _Compiletime U64 NotLeft() {
    return 0xfefefefefefefefeull;
  }

  _Compiletime U64 NotRight() {
    return 0x7f7f7f7f7f7f7f7full;
  }

  template<bool IsWhite>
  _Compiletime U64 LastRank() {
    return (IsWhite) ? 0xff000000000000ull : 0xff00ull;
  }

  template<bool IsWhite>
  _Compiletime U64 Forward(U64 mask) {
    return (IsWhite) ? mask << 8 : mask >> 8;
  }

  template<bool IsWhite>
  _Compiletime U64 Forward2(U64 mask) {
    return (IsWhite) ? mask << 16 : mask >> 16;
  }

  template<bool IsWhite>
  _Compiletime U64 AttackLeft(U64 mask) {
    return (IsWhite) ? mask << 7 : mask >> 9;
  }

  template<bool IsWhite>
  _Compiletime U64 AttackRight(U64 mask) {
    return (IsWhite) ? mask << 9 : mask >> 7;
  }

  template<bool IsWhite>
  _Compiletime U8 ForwardSquare(U8 sq) {
    return (IsWhite) ? sq + 8 : sq - 8;
  }

  template<bool IsWhite>
  _Compiletime U8 Forward2Square(U8 sq) {
    return (IsWhite) ? sq + 16 : sq - 16;
  }

  template<bool IsWhite>
  _Compiletime U8 AttackLeftSquare(U8 sq) {
    return (IsWhite) ? sq + 7 : sq - 9;
  }

  template<bool IsWhite>
  _Compiletime U8 AttackRightSquare(U8 sq) {
    return (IsWhite) ? sq + 9 : sq - 7;
  }

  template<bool IsWhite>
  _Compiletime U64 Color(const Board &brd) {
    return (IsWhite) ? brd.bitBoards[White] : brd.bitBoards[Black];
  }

  template<bool IsWhite>
  _Compiletime U64 EnemyOrEmpty(const Board &brd) {
    return (IsWhite) ? ~brd.bitBoards[White] : ~brd.bitBoards[Black];
  }

  template<bool IsWhite, PieceType Type>
  _Compiletime U64 PieceBB(const Board &brd) {
    return (IsWhite) ? 
        (Type == PawnType)     ? brd.bitBoards[WPawn]
      : (Type == BrawnType)    ? brd.bitBoards[WBrawn]
      : (Type == KnightType)   ? brd.bitBoards[WKnight]
      : (Type == BishopType)   ? brd.bitBoards[WBishop]
      : (Type == RookType)     ? brd.bitBoards[WRook]
      : (Type == PrincessType) ? brd.bitBoards[WPrincess]
      : (Type == UnicornType)  ? brd.bitBoards[WUnicorn]
      : (Type == DragonType)   ? brd.bitBoards[WDragon]
      : (Type == QueenType)    ? brd.bitBoards[WQueen]
      : (Type == RQueenType)   ? brd.bitBoards[WRQueen]
      : (Type == CKingType)    ? brd.bitBoards[WCKing]
                               : brd.bitBoards[WKing]
      : (Type == PawnType)     ? brd.bitBoards[BPawn]
      : (Type == BrawnType)    ? brd.bitBoards[BBrawn]
      : (Type == KnightType)   ? brd.bitBoards[BKnight]
      : (Type == BishopType)   ? brd.bitBoards[BBishop]
      : (Type == RookType)     ? brd.bitBoards[BRook]
      : (Type == PrincessType) ? brd.bitBoards[BPrincess]
      : (Type == UnicornType)  ? brd.bitBoards[BUnicorn]
      : (Type == DragonType)   ? brd.bitBoards[BDragon]
      : (Type == QueenType)    ? brd.bitBoards[BQueen]
      : (Type == RQueenType)   ? brd.bitBoards[BRQueen]
      : (Type == CKingType)    ? brd.bitBoards[BCKing]
                               : brd.bitBoards[BKing];
  }

  template<bool IsWhite, bool Brawn>
  _Compiletime U64 PawnMovement(const Board &brd) {
    return (Brawn) ? PieceBB<IsWhite, PawnType>(brd) | PieceBB<IsWhite, BrawnType>(brd) : PieceBB<IsWhite, PawnType>(brd);
  }

  template<bool IsWhite, bool Bishop, bool Princess, bool RQueen>
  _Compiletime U64 BishopMovement(const Board &brd) {
    return (Bishop) ? (Princess) ? 
          (RQueen) ? PieceBB<IsWhite, BishopType>(brd) | PieceBB<IsWhite, PrincessType>(brd) | PieceBB<IsWhite, QueenType>(brd) | PieceBB<IsWhite, RQueenType>(brd)
                   : PieceBB<IsWhite, BishopType>(brd) | PieceBB<IsWhite, PrincessType>(brd) | PieceBB<IsWhite, QueenType>(brd)
        : (RQueen) ? PieceBB<IsWhite, BishopType>(brd) | PieceBB<IsWhite, QueenType>(brd) | PieceBB<IsWhite, RQueenType>(brd)
                   : PieceBB<IsWhite, BishopType>(brd) | PieceBB<IsWhite, QueenType>(brd)
      : (Princess) ? 
          (RQueen) ? PieceBB<IsWhite, PrincessType>(brd) | PieceBB<IsWhite, QueenType>(brd) | PieceBB<IsWhite, RQueenType>(brd)
                   : PieceBB<IsWhite, PrincessType>(brd) | PieceBB<IsWhite, QueenType>(brd)
        : (RQueen) ? PieceBB<IsWhite, QueenType>(brd) | PieceBB<IsWhite, RQueenType>(brd)
                   : PieceBB<IsWhite, QueenType>(brd);
  }

  template<bool IsWhite, bool Rook, bool Princess, bool RQueen>
  _Compiletime U64 RookMovement(const Board &brd) {
    return (Rook) ? (Princess) ? 
          (RQueen) ? PieceBB<IsWhite, RookType>(brd) | PieceBB<IsWhite, PrincessType>(brd) | PieceBB<IsWhite, QueenType>(brd) | PieceBB<IsWhite, RQueenType>(brd)
                   : PieceBB<IsWhite, RookType>(brd) | PieceBB<IsWhite, PrincessType>(brd) | PieceBB<IsWhite, QueenType>(brd)
        : (RQueen) ? PieceBB<IsWhite, RookType>(brd) | PieceBB<IsWhite, QueenType>(brd) | PieceBB<IsWhite, RQueenType>(brd)
                   : PieceBB<IsWhite, RookType>(brd) | PieceBB<IsWhite, QueenType>(brd)
      : (Princess) ? 
          (RQueen) ? PieceBB<IsWhite, PrincessType>(brd) | PieceBB<IsWhite, QueenType>(brd) | PieceBB<IsWhite, RQueenType>(brd)
                   : PieceBB<IsWhite, PrincessType>(brd) | PieceBB<IsWhite, QueenType>(brd)
        : (RQueen) ? PieceBB<IsWhite, QueenType>(brd) | PieceBB<IsWhite, RQueenType>(brd)
                   : PieceBB<IsWhite, QueenType>(brd);
  }

  template<bool IsWhite, bool CKing>
  _Compiletime U64 KingMovement(const Board &brd) {
    return (CKing) ? PieceBB<IsWhite, CKingType>(brd) | PieceBB<IsWhite, KingType>(brd) : PieceBB<IsWhite, KingType>(brd);
  }

  template<bool IsWhite, bool RQueen>
  _Compiletime U64 Royalty(const Board &brd) {
    return (RQueen) ? PieceBB<IsWhite, RQueenType>(brd) | PieceBB<IsWhite, KingType>(brd) : PieceBB<IsWhite, KingType>(brd);
  }

  template<bool IsWhite, bool Allocate, bool OccOnly>
  _Compiletime auto Iterate(const DirMask *mask, const Board &brd) {
    const U64 c = mask->center, on = mask->north, oe = mask->east, os = mask->south, ow = mask->west;
    const U64 dne = mask->northEast, dse = mask->southEast, dsw = mask->southWest, dnw = mask->northWest;

    if constexpr (IsWhite) {
      const U64 royal = Royalty<IsWhite, true>(brd), prop = ~(brd.bitBoards[Occ] ^ royal), propE = prop & 0xFFFEFEFEFEFEFEFF, propW = prop & 0xFF7F7F7F7F7F7FFF;
      if constexpr ( Allocate && !OccOnly) return new DirMask((c | royal) & prop, ((on | royal) & prop) << 8, ((oe | royal) & propE) << 1, ((os | royal) & prop) >> 8, ((ow | royal) & propW) >> 1, ((dne | royal) & propE) << 9, ((dse | royal) & propE) >> 7, ((dsw | royal) & propW) >> 9, ((dnw | royal) & propW) << 7);
      if constexpr (!Allocate &&  OccOnly) return DirMask(c & prop, (on & prop) << 8, (oe & propE) << 1, (os & prop) >> 8, (ow & propW) >> 1, (dne & propE) << 9, (dse & propE) >> 7, (dsw & propW) >> 9, (dnw & propW) << 7);
      if constexpr (!Allocate && !OccOnly) return DirMask((c | royal) & prop, ((on | royal) & prop) << 8, ((oe | royal) & propE) << 1, ((os | royal) & prop) >> 8, ((ow | royal) & propW) >> 1, ((dne | royal) & propE) << 9, ((dse | royal) & propE) >> 7, ((dsw | royal) & propW) >> 9, ((dnw | royal) & propW) << 7);

    } else {
      const U64 royal = Royalty<IsWhite, true>(brd), prop = ~(brd.bitBoards[Occ] ^ royal), propE = prop & 0xFFFEFEFEFEFEFEFF, propW = prop & 0xFF7F7F7F7F7F7FFF;
      if constexpr ( Allocate && !OccOnly) return new DirMask((c | royal) & prop, ((on | royal) & prop) << 8, ((oe | royal) & propE) << 1, ((os | royal) & prop) >> 8, ((ow | royal) & propW) >> 1, ((dne | royal) & propE) << 9, ((dse | royal) & propE) >> 7, ((dsw | royal) & propW) >> 9, ((dnw | royal) & propW) << 7);
      if constexpr (!Allocate &&  OccOnly) return DirMask(c & prop, (on & prop) << 8, (oe & propE) << 1, (os & prop) >> 8, (ow & propW) >> 1, (dne & propE) << 9, (dse & propE) >> 7, (dsw & propW) >> 9, (dnw & propW) << 7);
      if constexpr (!Allocate && !OccOnly) return DirMask((c | royal) & prop, ((on | royal) & prop) << 8, ((oe | royal) & propE) << 1, ((os | royal) & prop) >> 8, ((ow | royal) & propW) >> 1, ((dne | royal) & propE) << 9, ((dse | royal) & propE) >> 7, ((dsw | royal) & propW) >> 9, ((dnw | royal) & propW) << 7);
    }
  }

  template<bool Allocate>
  _Compiletime auto Combine(const DirMask *m1, const DirMask &m2) {
    const U64 c1 = m1->center, on1 = m1->north, oe1 = m1->east, os1 = m1->south, ow1 = m1->west;
    const U64 dne1 = m1->northEast, dse1 = m1->southEast, dsw1 = m1->southWest, dnw1 = m1->northWest;

    const U64 c2 = m2.center, on2 = m2.north, oe2 = m2.east, os2 = m2.south, ow2 = m2.west;
    const U64 dne2 = m2.northEast, dse2 = m2.southEast, dsw2 = m2.southWest, dnw2 = m2.northWest;

    if constexpr (Allocate) return new DirMask(c1 | c2, on1 | on2, oe1 | oe2, os1 | os2, ow1 | ow2, dne1 | dne2, dse1 | dse2, dsw1 | dsw2, dnw1 | dnw2);
    else return DirMask(c1 | c2, on1 | on2, oe1 | oe2, os1 | os2, ow1 | ow2, dne1 | dne2, dse1 | dse2, dsw1 | dsw2, dnw1 | dnw2);
  }

  _Compiletime U64 Orth(const DirMask *mask) {
    return mask->north | mask->east | mask->south | mask->west;
  }

  _Compiletime U64 Diag(const DirMask *mask) {
    return mask->northEast | mask->southEast | mask->southWest | mask->northWest;
  }

  template<bool IsWhite>
  _Compiletime U64 PastCheck(const BoardMask *mask, const Board &brd) {
    const U64 queens = PieceBB<!IsWhite, QueenType>(brd) | PieceBB<!IsWhite, RQueenType>(brd);
    const U64 princesses = PieceBB<!IsWhite, PrincessType>(brd) | queens;

    const U64 orthPieces = PieceBB<!IsWhite, RookType>(brd) | princesses, diagPieces = PieceBB<!IsWhite, BishopType>(brd) | princesses;
    const U64 triagPieces = PieceBB<!IsWhite, UnicornType>(brd) | queens, quadPieces = PieceBB<!IsWhite, DragonType>(brd) | queens;

    const DirMask *north = mask->north, *east = mask->east, *south = mask->south;
    const DirMask *northEast = mask->northEast, *southEast = mask->southEast, *southWest = mask->southWest, *northWest = mask->northWest;

    const U64 orthSquares = north->center | east->center | south->center;
    const U64 diagSquares = Orth(north) | Orth(east) | Orth(south) | northEast->center | southEast->center | southWest->center | northWest->center;
    const U64 triagSquares = Diag(north) | Diag(east) | Diag(south) | Orth(northEast) | Orth(southEast) | Orth(southWest) | Orth(northWest);
    const U64 quadSquares = Diag(northEast) | Diag(southEast) | Diag(southWest) | Diag(northWest);

    const U64 sliderPieces = (orthPieces & orthSquares) | (diagPieces & diagSquares) | (triagPieces & triagSquares) | (quadPieces & quadSquares);
    const U64 leaperPieces = (PieceBB<!IsWhite, KnightType>(brd) & mask->knight) | (KingMovement<!IsWhite, true>(brd) & mask->king) | (PawnMovement<!IsWhite, true>(brd) & mask->pawn) | (PieceBB<!IsWhite, BrawnType>(brd) & mask->brawn);

    return sliderPieces | leaperPieces;
  }
};

