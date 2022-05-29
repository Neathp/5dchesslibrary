#pragma once
#include "attacktable.hpp"

namespace Lookup = Chess_Lookup::Lookup_Pext;

enum class Piece {
    Pawn, Brawn, Knight, Bishop, Rook, Princess, Unicorn, Dragon, Queen, RQueen, CKing, King
};

class BoardStatus {
  public:
  const bool WhiteMove;

  constexpr BoardStatus(bool white) : WhiteMove(white) {}
  constexpr BoardStatus(int pat) : WhiteMove((pat & 0b1) != 0) {}

  constexpr BoardStatus SilentMove() const {return BoardStatus(!WhiteMove);}
};

struct Board {
  const U64 BPawn, BBrawn, BKnight, BBishop, BRook, BPrincess, BUnicorn, BDragon, BQueen, BRQueen, BCKing, BKing;
  const U64 WPawn, WBrawn, WKnight, WBishop, WRook, WPrincess, WUnicorn, WDragon, WQueen, WRQueen, WCKing, WKing;
  const U64 Black, White, Occ, UnMoved;

  constexpr Board(U64 bp, U64 bw, U64 bn, U64 bb, U64 br, U64 bs, U64 bu, U64 bd, U64 bq, U64 by, U64 bc, U64 bk,
    U64 wp, U64 ww, U64 wn, U64 wb, U64 wr, U64 ws, U64 wu, U64 wd, U64 wq, U64 wy, U64 wc, U64 wk, U64 um) :
    BPawn(bp), BBrawn(bw), BKnight(bn), BBishop(bb), BRook(br), BPrincess(bs), BUnicorn(bu), BDragon(bd), BQueen(bq), BRQueen(by), BCKing(bc), BKing(bk),
    WPawn(wp), WBrawn(ww), WKnight(wn), WBishop(wb), WRook(wr), WPrincess(ws), WUnicorn(wu), WDragon(wd), WQueen(wq), WRQueen(wy), WCKing(wc), WKing(wk),
    Black(bp | bw | bn | bb | br | bs | bu | bd | bq | by | bk | bc), White(wp | ww | wn | wb | wr | ws | wu | wd | wq | wy | wk | wc), Occ(Black | White), UnMoved(um)
  {}

  template<Piece piece, bool IsWhite, bool IsPawn>
  _Compiletime Board MovePromote(const Board& brd, U64 from, U64 to) {
    const U64 rem = ~(from | to);

    const U64 bp = brd.BPawn, bw = brd.BBrawn, bn = brd.BKnight, bb = brd.BBishop, br = brd.BRook, bs = brd.BPrincess, 
      bu = brd.BUnicorn, bd = brd.BDragon, bq = brd.BQueen, by = brd.BRQueen, bc = brd.BCKing, bk = brd.BKing;

    const U64 wp = brd.WPawn, ww = brd.WBrawn, wn = brd.WKnight, wb = brd.WBishop, wr = brd.WRook, ws = brd.WPrincess, 
      wu = brd.WUnicorn, wd = brd.WDragon, wq = brd.WQueen, wy = brd.WRQueen, wc = brd.WCKing, wk = brd.WKing;

    const U64 um = brd.UnMoved;

    if constexpr (IsWhite) {
      if constexpr (IsPawn) {
        if constexpr (Piece::Knight == piece)   return Board(bp & rem, bw & rem, bn & rem, bb & rem, br & rem, bs & rem, bu & rem, bd & rem, bq & rem, by, bc & rem, bk, wp ^ from, ww, wn ^ to, wb, wr, ws, wu, wd, wq, wy, wc, wk, um & rem);
        if constexpr (Piece::Bishop == piece)   return Board(bp & rem, bw & rem, bn & rem, bb & rem, br & rem, bs & rem, bu & rem, bd & rem, bq & rem, by, bc & rem, bk, wp ^ from, ww, wn, wb ^ to, wr, ws, wu, wd, wq, wy, wc, wk, um & rem);
        if constexpr (Piece::Rook == piece)     return Board(bp & rem, bw & rem, bn & rem, bb & rem, br & rem, bs & rem, bu & rem, bd & rem, bq & rem, by, bc & rem, bk, wp ^ from, ww, wn, wb, wr ^ to, ws, wu, wd, wq, wy, wc, wk, um & rem);
        if constexpr (Piece::Princess == piece) return Board(bp & rem, bw & rem, bn & rem, bb & rem, br & rem, bs & rem, bu & rem, bd & rem, bq & rem, by, bc & rem, bk, wp ^ from, ww, wn, wb, wr, ws ^ to, wu, wd, wq, wy, wc, wk, um & rem);
        if constexpr (Piece::Unicorn == piece)  return Board(bp & rem, bw & rem, bn & rem, bb & rem, br & rem, bs & rem, bu & rem, bd & rem, bq & rem, by, bc & rem, bk, wp ^ from, ww, wn, wb, wr, ws, wu ^ to, wd, wq, wy, wc, wk, um & rem);
        if constexpr (Piece::Dragon == piece)   return Board(bp & rem, bw & rem, bn & rem, bb & rem, br & rem, bs & rem, bu & rem, bd & rem, bq & rem, by, bc & rem, bk, wp ^ from, ww, wn, wb, wr, ws, wu, wd ^ to, wq, wy, wc, wk, um & rem);
        if constexpr (Piece::Queen == piece)    return Board(bp & rem, bw & rem, bn & rem, bb & rem, br & rem, bs & rem, bu & rem, bd & rem, bq & rem, by, bc & rem, bk, wp ^ from, ww, wn, wb, wr, ws, wu, wd, wq ^ to, wy, wc, wk, um & rem);
        if constexpr (Piece::CKing == piece)    return Board(bp & rem, bw & rem, bn & rem, bb & rem, br & rem, bs & rem, bu & rem, bd & rem, bq & rem, by, bc & rem, bk, wp ^ from, ww, wn, wb, wr, ws, wu, wd, wq, wy, wc ^ to, wk, um & rem);
      } else {
        if constexpr (Piece::Knight == piece)   return Board(bp & rem, bw & rem, bn & rem, bb & rem, br & rem, bs & rem, bu & rem, bd & rem, bq & rem, by, bc & rem, bk, wp, ww ^ from, wn ^ to, wb, wr, ws, wu, wd, wq, wy, wc, wk, um & rem);
        if constexpr (Piece::Bishop == piece)   return Board(bp & rem, bw & rem, bn & rem, bb & rem, br & rem, bs & rem, bu & rem, bd & rem, bq & rem, by, bc & rem, bk, wp, ww ^ from, wn, wb ^ to, wr, ws, wu, wd, wq, wy, wc, wk, um & rem);
        if constexpr (Piece::Rook == piece)     return Board(bp & rem, bw & rem, bn & rem, bb & rem, br & rem, bs & rem, bu & rem, bd & rem, bq & rem, by, bc & rem, bk, wp, ww ^ from, wn, wb, wr ^ to, ws, wu, wd, wq, wy, wc, wk, um & rem);
        if constexpr (Piece::Princess == piece) return Board(bp & rem, bw & rem, bn & rem, bb & rem, br & rem, bs & rem, bu & rem, bd & rem, bq & rem, by, bc & rem, bk, wp, ww ^ from, wn, wb, wr, ws ^ to, wu, wd, wq, wy, wc, wk, um & rem);
        if constexpr (Piece::Unicorn == piece)  return Board(bp & rem, bw & rem, bn & rem, bb & rem, br & rem, bs & rem, bu & rem, bd & rem, bq & rem, by, bc & rem, bk, wp, ww ^ from, wn, wb, wr, ws, wu ^ to, wd, wq, wy, wc, wk, um & rem);
        if constexpr (Piece::Dragon == piece)   return Board(bp & rem, bw & rem, bn & rem, bb & rem, br & rem, bs & rem, bu & rem, bd & rem, bq & rem, by, bc & rem, bk, wp, ww ^ from, wn, wb, wr, ws, wu, wd ^ to, wq, wy, wc, wk, um & rem);
        if constexpr (Piece::Queen == piece)    return Board(bp & rem, bw & rem, bn & rem, bb & rem, br & rem, bs & rem, bu & rem, bd & rem, bq & rem, by, bc & rem, bk, wp, ww ^ from, wn, wb, wr, ws, wu, wd, wq ^ to, wy, wc, wk, um & rem);
        if constexpr (Piece::CKing == piece)    return Board(bp & rem, bw & rem, bn & rem, bb & rem, br & rem, bs & rem, bu & rem, bd & rem, bq & rem, by, bc & rem, bk, wp, ww ^ from, wn, wb, wr, ws, wu, wd, wq, wy, wc ^ to, wk, um & rem);
      }
    } else {
      if constexpr (IsPawn) {
        if constexpr (Piece::Knight == piece)   return Board(bp ^ from, bw, bn ^ to, bb, br, bs, bu, bd, bq, by, bc, bk, wp & rem, ww & rem, wn & rem, wb & rem, wr & rem, ws & rem, wu & rem, wd & rem, wq & rem, wy, wc & rem, wk, um & rem);
        if constexpr (Piece::Bishop == piece)   return Board(bp ^ from, bw, bn, bb ^ to, br, bs, bu, bd, bq, by, bc, bk, wp & rem, ww & rem, wn & rem, wb & rem, wr & rem, ws & rem, wu & rem, wd & rem, wq & rem, wy, wc & rem, wk, um & rem);
        if constexpr (Piece::Rook == piece)     return Board(bp ^ from, bw, bn, bb, br ^ to, bs, bu, bd, bq, by, bc, bk, wp & rem, ww & rem, wn & rem, wb & rem, wr & rem, ws & rem, wu & rem, wd & rem, wq & rem, wy, wc & rem, wk, um & rem);
        if constexpr (Piece::Princess == piece) return Board(bp ^ from, bw, bn, bb, br, bs ^ to, bu, bd, bq, by, bc, bk, wp & rem, ww & rem, wn & rem, wb & rem, wr & rem, ws & rem, wu & rem, wd & rem, wq & rem, wy, wc & rem, wk, um & rem);
        if constexpr (Piece::Unicorn == piece)  return Board(bp ^ from, bw, bn, bb, br, bs, bu ^ to, bd, bq, by, bc, bk, wp & rem, ww & rem, wn & rem, wb & rem, wr & rem, ws & rem, wu & rem, wd & rem, wq & rem, wy, wc & rem, wk, um & rem);
        if constexpr (Piece::Dragon == piece)   return Board(bp ^ from, bw, bn, bb, br, bs, bu, bd ^ to, bq, by, bc, bk, wp & rem, ww & rem, wn & rem, wb & rem, wr & rem, ws & rem, wu & rem, wd & rem, wq & rem, wy, wc & rem, wk, um & rem);
        if constexpr (Piece::Queen == piece)    return Board(bp ^ from, bw, bn, bb, br, bs, bu, bd, bq ^ to, by, bc, bk, wp & rem, ww & rem, wn & rem, wb & rem, wr & rem, ws & rem, wu & rem, wd & rem, wq & rem, wy, wc & rem, wk, um & rem);
        if constexpr (Piece::CKing == piece)    return Board(bp ^ from, bw, bn, bb, br, bs, bu, bd, bq, by, bc ^ to, bk, wp & rem, ww & rem, wn & rem, wb & rem, wr & rem, ws & rem, wu & rem, wd & rem, wq & rem, wy, wc & rem, wk, um & rem);
      } else {
        if constexpr (Piece::Knight == piece)   return Board(bp, bw ^ from, bn ^ to, bb, br, bs, bu, bd, bq, by, bc, bk, wp & rem, ww & rem, wn & rem, wb & rem, wr & rem, ws & rem, wu & rem, wd & rem, wq & rem, wy, wc & rem, wk, um & rem);
        if constexpr (Piece::Bishop == piece)   return Board(bp, bw ^ from, bn, bb ^ to, br, bs, bu, bd, bq, by, bc, bk, wp & rem, ww & rem, wn & rem, wb & rem, wr & rem, ws & rem, wu & rem, wd & rem, wq & rem, wy, wc & rem, wk, um & rem);
        if constexpr (Piece::Rook == piece)     return Board(bp, bw ^ from, bn, bb, br ^ to, bs, bu, bd, bq, by, bc, bk, wp & rem, ww & rem, wn & rem, wb & rem, wr & rem, ws & rem, wu & rem, wd & rem, wq & rem, wy, wc & rem, wk, um & rem);
        if constexpr (Piece::Princess == piece) return Board(bp, bw ^ from, bn, bb, br, bs ^ to, bu, bd, bq, by, bc, bk, wp & rem, ww & rem, wn & rem, wb & rem, wr & rem, ws & rem, wu & rem, wd & rem, wq & rem, wy, wc & rem, wk, um & rem);
        if constexpr (Piece::Unicorn == piece)  return Board(bp, bw ^ from, bn, bb, br, bs, bu ^ to, bd, bq, by, bc, bk, wp & rem, ww & rem, wn & rem, wb & rem, wr & rem, ws & rem, wu & rem, wd & rem, wq & rem, wy, wc & rem, wk, um & rem);
        if constexpr (Piece::Dragon == piece)   return Board(bp, bw ^ from, bn, bb, br, bs, bu, bd ^ to, bq, by, bc, bk, wp & rem, ww & rem, wn & rem, wb & rem, wr & rem, ws & rem, wu & rem, wd & rem, wq & rem, wy, wc & rem, wk, um & rem);
        if constexpr (Piece::Queen == piece)    return Board(bp, bw ^ from, bn, bb, br, bs, bu, bd, bq ^ to, by, bc, bk, wp & rem, ww & rem, wn & rem, wb & rem, wr & rem, ws & rem, wu & rem, wd & rem, wq & rem, wy, wc & rem, wk, um & rem);
        if constexpr (Piece::CKing == piece)    return Board(bp, bw ^ from, bn, bb, br, bs, bu, bd, bq, by, bc ^ to, bk, wp & rem, ww & rem, wn & rem, wb & rem, wr & rem, ws & rem, wu & rem, wd & rem, wq & rem, wy, wc & rem, wk, um & rem);
      }
    }
  }
    
  template<bool IsWhite>
  _Compiletime Board MoveCastle(const Board& brd, U64 kingswitch, U64 rookswitch) {
    const U64 bp = brd.BPawn, bw = brd.BBrawn, bn = brd.BKnight, bb = brd.BBishop, br = brd.BRook, bs = brd.BPrincess, 
      bu = brd.BUnicorn, bd = brd.BDragon, bq = brd.BQueen, by = brd.BRQueen, bc = brd.BCKing, bk = brd.BKing;

    const U64 wp = brd.WPawn, ww = brd.WBrawn, wn = brd.WKnight, wb = brd.WBishop, wr = brd.WRook, ws = brd.WPrincess, 
      wu = brd.WUnicorn, wd = brd.WDragon, wq = brd.WQueen, wy = brd.WRQueen, wc = brd.WCKing, wk = brd.WKing;

    const U64 um = brd.UnMoved;

    if constexpr (IsWhite) return Board(bp, bw, bn, bb, br, bs, bu, bd, bq, by, bc, bk, wp, ww, wn, wb, wr ^ rookswitch, ws, wu, wd, wq, wy, wc, wk ^ kingswitch, um & ~(kingswitch | rookswitch));
    else return Board(bp, bw, bn, bb, br ^ rookswitch, bs, bu, bd, bq, by, bc, bk ^ kingswitch, wp, ww, wn, wb, wr, ws, wu, wd, wq, wy, wc, wk, um & ~(kingswitch | rookswitch));
  }

  template<bool IsWhite, bool IsPawn>
  _Compiletime Board MoveEP(const Board& brd, U64 from, U64 enemy, U64 to) {
    const U64 rem = ~enemy, mov = from | to;

    const U64 bp = brd.BPawn, bw = brd.BBrawn, bn = brd.BKnight, bb = brd.BBishop, br = brd.BRook, bs = brd.BPrincess, 
      bu = brd.BUnicorn, bd = brd.BDragon, bq = brd.BQueen, by = brd.BRQueen, bc = brd.BCKing, bk = brd.BKing;

    const U64 wp = brd.WPawn, ww = brd.WBrawn, wn = brd.WKnight, wb = brd.WBishop, wr = brd.WRook, ws = brd.WPrincess, 
      wu = brd.WUnicorn, wd = brd.WDragon, wq = brd.WQueen, wy = brd.WRQueen, wc = brd.WCKing, wk = brd.WKing;

    const U64 um = brd.UnMoved;

    if constexpr (IsWhite) {
      if constexpr (IsPawn) return Board(bp & rem, bw & rem, bn, bb, br, bs, bu, bd, bq, by, bc, bk, wp ^ mov, ww, wn, wb, wr, ws, wu, wd, wq, wy, wc, wk, um & ~from);
      else return Board(bp & rem, bw & rem, bn, bb, br, bs, bu, bd, bq, by, bc, bk, wp, ww ^ mov, wn, wb, wr, ws, wu, wd, wq, wy, wc, wk, um & ~from); 
    } else {
      if constexpr (IsPawn) return Board(bp ^ mov, bw, bn, bb, br, bs, bu, bd, bq, by, bc, bk, wp & rem, ww & rem, wn, wb, wr, ws, wu, wd, wq, wy, wc, wk, um & ~from);
      else return Board(bp, bw ^ mov, bn, bb, br, bs, bu, bd, bq, by, bc, bk, wp & rem, ww & rem, wn, wb, wr, ws, wu, wd, wq, wy, wc, wk, um & ~from);
    }
  }

  template<Piece piece, bool IsWhite>
  _Compiletime Board Move(const Board& existing, U64 from, U64 to, bool IsTaking) {
    if (IsTaking) return Move<piece, IsWhite, true>(existing, from, to);
    else return Move<piece, IsWhite, false>(existing, from, to);
  }

  template<Piece piece, bool IsWhite, bool IsTaking>
  _Compiletime Board Move(const Board& brd, U64 from, U64 to) {
    const U64 mov = from | to;

    const U64 bp = brd.BPawn, bw = brd.BBrawn, bn = brd.BKnight, bb = brd.BBishop, br = brd.BRook, bs = brd.BPrincess, 
      bu = brd.BUnicorn, bd = brd.BDragon, bq = brd.BQueen, by = brd.BRQueen, bc = brd.BCKing, bk = brd.BKing;

    const U64 wp = brd.WPawn, ww = brd.WBrawn, wn = brd.WKnight, wb = brd.WBishop, wr = brd.WRook, ws = brd.WPrincess, 
      wu = brd.WUnicorn, wd = brd.WDragon, wq = brd.WQueen, wy = brd.WRQueen, wc = brd.WCKing, wk = brd.WKing;

    const U64 um = brd.UnMoved;
        
    if constexpr (IsTaking) {
      const U64 rem = ~to;
      if constexpr (IsWhite) {
        if constexpr (Piece::Pawn == piece)     return Board(bp & rem, bw & rem, bn & rem, bb & rem, br & rem, bs & rem, bu & rem, bd & rem, bq & rem, by, bc & rem, bk, wp ^ mov, ww, wn, wb, wr, ws, wu, wd, wq, wy, wc, wk, um & ~mov);
        if constexpr (Piece::Brawn == piece)    return Board(bp & rem, bw & rem, bn & rem, bb & rem, br & rem, bs & rem, bu & rem, bd & rem, bq & rem, by, bc & rem, bk, wp, ww ^ mov, wn, wb, wr, ws, wu, wd, wq, wy, wc, wk, um & ~mov);
        if constexpr (Piece::Knight == piece)   return Board(bp & rem, bw & rem, bn & rem, bb & rem, br & rem, bs & rem, bu & rem, bd & rem, bq & rem, by, bc & rem, bk, wp, ww, wn ^ mov, wb, wr, ws, wu, wd, wq, wy, wc, wk, um & rem);
        if constexpr (Piece::Bishop == piece)   return Board(bp & rem, bw & rem, bn & rem, bb & rem, br & rem, bs & rem, bu & rem, bd & rem, bq & rem, by, bc & rem, bk, wp, ww, wn, wb ^ mov, wr, ws, wu, wd, wq, wy, wc, wk, um & rem);
        if constexpr (Piece::Rook == piece)     return Board(bp & rem, bw & rem, bn & rem, bb & rem, br & rem, bs & rem, bu & rem, bd & rem, bq & rem, by, bc & rem, bk, wp, ww, wn, wb, wr ^ mov, ws, wu, wd, wq, wy, wc, wk, um & ~mov);
        if constexpr (Piece::Princess == piece) return Board(bp & rem, bw & rem, bn & rem, bb & rem, br & rem, bs & rem, bu & rem, bd & rem, bq & rem, by, bc & rem, bk, wp, ww, wn, wb, wr, ws ^ mov, wu, wd, wq, wy, wc, wk, um & rem);
        if constexpr (Piece::Unicorn == piece)  return Board(bp & rem, bw & rem, bn & rem, bb & rem, br & rem, bs & rem, bu & rem, bd & rem, bq & rem, by, bc & rem, bk, wp, ww, wn, wb, wr, ws, wu ^ mov, wd, wq, wy, wc, wk, um & rem);
        if constexpr (Piece::Dragon == piece)   return Board(bp & rem, bw & rem, bn & rem, bb & rem, br & rem, bs & rem, bu & rem, bd & rem, bq & rem, by, bc & rem, bk, wp, ww, wn, wb, wr, ws, wu, wd ^ mov, wq, wy, wc, wk, um & rem);
        if constexpr (Piece::Queen == piece)    return Board(bp & rem, bw & rem, bn & rem, bb & rem, br & rem, bs & rem, bu & rem, bd & rem, bq & rem, by, bc & rem, bk, wp, ww, wn, wb, wr, ws, wu, wd, wq ^ mov, wy, wc, wk, um & rem);
        if constexpr (Piece::RQueen == piece)   return Board(bp & rem, bw & rem, bn & rem, bb & rem, br & rem, bs & rem, bu & rem, bd & rem, bq & rem, by, bc & rem, bk, wp, ww, wn, wb, wr, ws, wu, wd, wq, wy ^ mov, wc, wk, um & rem);
        if constexpr (Piece::CKing == piece)    return Board(bp & rem, bw & rem, bn & rem, bb & rem, br & rem, bs & rem, bu & rem, bd & rem, bq & rem, by, bc & rem, bk, wp, ww, wn, wb, wr, ws, wu, wd, wq, wy, wc ^ mov, wk, um & rem);
        if constexpr (Piece::King == piece)     return Board(bp & rem, bw & rem, bn & rem, bb & rem, br & rem, bs & rem, bu & rem, bd & rem, bq & rem, by, bc & rem, bk, wp, ww, wn, wb, wr, ws, wu, wd, wq, wy, wc, wk ^ mov, um & ~mov);
      } else {
        if constexpr (Piece::Pawn == piece)     return Board(bp ^ mov, bw, bn, bb, br, bs, bu, bd, bq, by, bc, bk, wp & rem, ww & rem, wn & rem, wb & rem, wr & rem, ws & rem, wu & rem, wd & rem, wq & rem, wy, wc & rem, wk, um & ~mov);
        if constexpr (Piece::Brawn == piece)    return Board(bp, bw ^ mov, bn, bb, br, bs, bu, bd, bq, by, bc, bk, wp & rem, ww & rem, wn & rem, wb & rem, wr & rem, ws & rem, wu & rem, wd & rem, wq & rem, wy, wc & rem, wk, um & ~mov);
        if constexpr (Piece::Knight == piece)   return Board(bp, bw, bn ^ mov, bb, br, bs, bu, bd, bq, by, bc, bk, wp & rem, ww & rem, wn & rem, wb & rem, wr & rem, ws & rem, wu & rem, wd & rem, wq & rem, wy, wc & rem, wk, um & rem);
        if constexpr (Piece::Bishop == piece)   return Board(bp, bw, bn, bb ^ mov, br, bs, bu, bd, bq, by, bc, bk, wp & rem, ww & rem, wn & rem, wb & rem, wr & rem, ws & rem, wu & rem, wd & rem, wq & rem, wy, wc & rem, wk, um & rem);
        if constexpr (Piece::Rook == piece)     return Board(bp, bw, bn, bb, br ^ mov, bs, bu, bd, bq, by, bc, bk, wp & rem, ww & rem, wn & rem, wb & rem, wr & rem, ws & rem, wu & rem, wd & rem, wq & rem, wy, wc & rem, wk, um & ~mov);
        if constexpr (Piece::Princess == piece) return Board(bp, bw, bn, bb, br, bs ^ mov, bu, bd, bq, by, bc, bk, wp & rem, ww & rem, wn & rem, wb & rem, wr & rem, ws & rem, wu & rem, wd & rem, wq & rem, wy, wc & rem, wk, um & rem);
        if constexpr (Piece::Unicorn == piece)  return Board(bp, bw, bn, bb, br, bs, bu ^ mov, bd, bq, by, bc, bk, wp & rem, ww & rem, wn & rem, wb & rem, wr & rem, ws & rem, wu & rem, wd & rem, wq & rem, wy, wc & rem, wk, um & rem);
        if constexpr (Piece::Dragon == piece)   return Board(bp, bw, bn, bb, br, bs, bu, bd ^ mov, bq, by, bc, bk, wp & rem, ww & rem, wn & rem, wb & rem, wr & rem, ws & rem, wu & rem, wd & rem, wq & rem, wy, wc & rem, wk, um & rem);
        if constexpr (Piece::Queen == piece)    return Board(bp, bw, bn, bb, br, bs, bu, bd, bq ^ mov, by, bc, bk, wp & rem, ww & rem, wn & rem, wb & rem, wr & rem, ws & rem, wu & rem, wd & rem, wq & rem, wy, wc & rem, wk, um & rem);
        if constexpr (Piece::RQueen == piece)   return Board(bp, bw, bn, bb, br, bs, bu, bd, bq, by ^ mov, bc, bk, wp & rem, ww & rem, wn & rem, wb & rem, wr & rem, ws & rem, wu & rem, wd & rem, wq & rem, wy, wc & rem, wk, um & rem);
        if constexpr (Piece::CKing == piece)    return Board(bp, bw, bn, bb, br, bs, bu, bd, bq, by, bc ^ mov, bk, wp & rem, ww & rem, wn & rem, wb & rem, wr & rem, ws & rem, wu & rem, wd & rem, wq & rem, wy, wc & rem, wk, um & rem);
        if constexpr (Piece::King == piece)     return Board(bp, bw, bn, bb, br, bs, bu, bd, bq, by, bc, bk ^ mov, wp & rem, ww & rem, wn & rem, wb & rem, wr & rem, ws & rem, wu & rem, wd & rem, wq & rem, wy, wc & rem, wk, um & ~mov);
      }
    } else {
      if constexpr (IsWhite) {
        if constexpr (Piece::Pawn == piece)     return Board(bp, bw, bn, bb, br, bs, bu, bd, bq, by, bc, bk, wp ^ mov, ww, wn, wb, wr, ws, wu, wd, wq, wy, wc, wk, um & ~mov);
        if constexpr (Piece::Brawn == piece)    return Board(bp, bw, bn, bb, br, bs, bu, bd, bq, by, bc, bk, wp, ww ^ mov, wn, wb, wr, ws, wu, wd, wq, wy, wc, wk, um & ~mov);
        if constexpr (Piece::Knight == piece)   return Board(bp, bw, bn, bb, br, bs, bu, bd, bq, by, bc, bk, wp, ww, wn ^ mov, wb, wr, ws, wu, wd, wq, wy, wc, wk, um & ~to);
        if constexpr (Piece::Bishop == piece)   return Board(bp, bw, bn, bb, br, bs, bu, bd, bq, by, bc, bk, wp, ww, wn, wb ^ mov, wr, ws, wu, wd, wq, wy, wc, wk, um & ~to);
        if constexpr (Piece::Rook == piece)     return Board(bp, bw, bn, bb, br, bs, bu, bd, bq, by, bc, bk, wp, ww, wn, wb, wr ^ mov, ws, wu, wd, wq, wy, wc, wk, um & ~mov);
        if constexpr (Piece::Princess == piece) return Board(bp, bw, bn, bb, br, bs, bu, bd, bq, by, bc, bk, wp, ww, wn, wb, wr, ws ^ mov, wu, wd, wq, wy, wc, wk, um & ~to);
        if constexpr (Piece::Unicorn == piece)  return Board(bp, bw, bn, bb, br, bs, bu, bd, bq, by, bc, bk, wp, ww, wn, wb, wr, ws, wu ^ mov, wd, wq, wy, wc, wk, um & ~to);
        if constexpr (Piece::Dragon == piece)   return Board(bp, bw, bn, bb, br, bs, bu, bd, bq, by, bc, bk, wp, ww, wn, wb, wr, ws, wu, wd ^ mov, wq, wy, wc, wk, um & ~to);
        if constexpr (Piece::Queen == piece)    return Board(bp, bw, bn, bb, br, bs, bu, bd, bq, by, bc, bk, wp, ww, wn, wb, wr, ws, wu, wd, wq ^ mov, wy, wc, wk, um & ~to);
        if constexpr (Piece::RQueen == piece)   return Board(bp, bw, bn, bb, br, bs, bu, bd, bq, by, bc, bk, wp, ww, wn, wb, wr, ws, wu, wd, wq, wy ^ mov, wc, wk, um & ~to);
        if constexpr (Piece::CKing == piece)    return Board(bp, bw, bn, bb, br, bs, bu, bd, bq, by, bc, bk, wp, ww, wn, wb, wr, ws, wu, wd, wq, wy, wc ^ mov, wk, um & ~to);
        if constexpr (Piece::King == piece)     return Board(bp, bw, bn, bb, br, bs, bu, bd, bq, by, bc, bk, wp, ww, wn, wb, wr, ws, wu, wd, wq, wy, wc, wk ^ mov, um & ~mov);
      } else {
        if constexpr (Piece::Pawn == piece)     return Board(bp ^ mov, bw, bn, bb, br, bs, bu, bd, bq, by, bc, bk, wp, ww, wn, wb, wr, ws, wu, wd, wq, wy, wc, wk, um & ~mov);
        if constexpr (Piece::Brawn == piece)    return Board(bp, bw ^ mov, bn, bb, br, bs, bu, bd, bq, by, bc, bk, wp, ww, wn, wb, wr, ws, wu, wd, wq, wy, wc, wk, um & ~mov);
        if constexpr (Piece::Knight == piece)   return Board(bp, bw, bn ^ mov, bb, br, bs, bu, bd, bq, by, bc, bk, wp, ww, wn, wb, wr, ws, wu, wd, wq, wy, wc, wk, um & ~to);
        if constexpr (Piece::Bishop == piece)   return Board(bp, bw, bn, bb ^ mov, br, bs, bu, bd, bq, by, bc, bk, wp, ww, wn, wb, wr, ws, wu, wd, wq, wy, wc, wk, um & ~to);
        if constexpr (Piece::Rook == piece)     return Board(bp, bw, bn, bb, br ^ mov, bs, bu, bd, bq, by, bc, bk, wp, ww, wn, wb, wr, ws, wu, wd, wq, wy, wc, wk, um & ~mov);
        if constexpr (Piece::Princess == piece) return Board(bp, bw, bn, bb, br, bs ^ mov, bu, bd, bq, by, bc, bk, wp, ww, wn, wb, wr, ws, wu, wd, wq, wy, wc, wk, um & ~to);
        if constexpr (Piece::Unicorn == piece)  return Board(bp, bw, bn, bb, br, bs, bu ^ mov, bd, bq, by, bc, bk, wp, ww, wn, wb, wr, ws, wu, wd, wq, wy, wc, wk, um & ~to);
        if constexpr (Piece::Dragon == piece)   return Board(bp, bw, bn, bb, br, bs, bu, bd ^ mov, bq, by, bc, bk, wp, ww, wn, wb, wr, ws, wu, wd, wq, wy, wc, wk, um & ~to);
        if constexpr (Piece::Queen == piece)    return Board(bp, bw, bn, bb, br, bs, bu, bd, bq ^ mov, by, bc, bk, wp, ww, wn, wb, wr, ws, wu, wd, wq, wy, wc, wk, um & ~to);
        if constexpr (Piece::RQueen == piece)   return Board(bp, bw, bn, bb, br, bs, bu, bd, bq, by ^ mov, bc, bk, wp, ww, wn, wb, wr, ws, wu, wd, wq, wy, wc, wk, um & ~to);
        if constexpr (Piece::CKing == piece)    return Board(bp, bw, bn, bb, br, bs, bu, bd, bq, by, bc ^ mov, bk, wp, ww, wn, wb, wr, ws, wu, wd, wq, wy, wc, wk, um & ~to);
        if constexpr (Piece::King == piece)     return Board(bp, bw, bn, bb, br, bs, bu, bd, bq, by, bc, bk ^ mov, wp, ww, wn, wb, wr, ws, wu, wd, wq, wy, wc, wk, um & ~mov);
      }
    }   
  }
};

static constexpr U64 File1 = 0b1000000010000000100000001000000010000000100000001000000010000000ul;  // Problematic currently
static constexpr U64 File8 = 0b0000000100000001000000010000000100000001000000010000000100000001ul;  // ''
static constexpr U64 Rank2 = 0b0000000000000000000000000000000000000000000000001111111100000000ul;  // ''
static constexpr U64 Rank7 = 0b0000000011111111000000000000000000000000000000000000000000000000ul;  // ''
static constexpr U64 RankMid = 0x0000FFFFFFFF0000;
static constexpr U64 Rank_18 = 0xFF000000000000FF;

_Compiletime U64 Pawns_NotLeft() {
  return ~File1;
}

_Compiletime U64 Pawns_NotRight() {
  return ~File8;
}

template<bool IsWhite>
_Compiletime U64 Pawn_Forward(U64 mask) {
  if constexpr (IsWhite) return mask << 8;
  else return mask >> 8;
}

template<bool IsWhite>
_Compiletime U64 Pawn_Forward2(U64 mask) {
  if constexpr (IsWhite) return mask << 16;
  else return mask >> 16;
}

template<bool IsWhite>
_Compiletime U64 Pawn_Backward(U64 mask) {
  return Pawn_Forward<!IsWhite>(mask);
}

template<bool IsWhite>
_Compiletime U64 Pawn_Backward2(U64 mask) {
  return Pawn_Forward2<!IsWhite>(mask);
}

template<bool IsWhite>
_Compiletime U64 Pawn_AttackLeft(U64 mask) {
  if constexpr (IsWhite) return mask << 9;
  else return mask >> 7;
}

template<bool IsWhite>
_Compiletime U64 Pawn_AttackRight(U64 mask) {
  if constexpr (IsWhite) return mask << 7;
  else return mask >> 9;
}

template<bool IsWhite>
_Compiletime U64 Pawn_InvertLeft(U64 mask) {
  return Pawn_AttackRight<!IsWhite>(mask);
}

template<bool IsWhite>
_Compiletime U64 Pawn_InvertRight(U64 mask) {
  return Pawn_AttackLeft<!IsWhite>(mask);
}

template<bool IsWhite>
constexpr U64 Pawns_LastRank() {
  if constexpr (IsWhite) return Rank7;
  else return Rank2;
}

_Compiletime U64 Empty(const Board& brd) {
  return ~brd.Occ;
}

template<bool IsWhite>
_Compiletime U64 Color(const Board& brd) {
  if constexpr (IsWhite) return brd.White;
  return brd.Black;
}

template<bool IsWhite>
_Compiletime U64 EnemyOrEmpty(const Board& brd) {
  if constexpr (IsWhite) return ~brd.White;
  return ~brd.Black;
}

template<bool IsWhite>
_Compiletime U64 Pawns(const Board& brd) {
  if constexpr (IsWhite) return brd.WPawn;
  else return brd.BPawn;
}

template<bool IsWhite>
_Compiletime U64 Brawns(const Board& brd) {
  if constexpr (IsWhite) return brd.WBrawn;
  else return brd.BBrawn;
}

template<bool IsWhite>
_Compiletime U64 PawnMovement(const Board& brd) {
  if constexpr (IsWhite) return brd.WPawn | brd.WBrawn;
  else return brd.BPawn | brd.BBrawn;
}

template<bool IsWhite>
_Compiletime U64 Knights(const Board& brd) {
  if constexpr (IsWhite) return brd.WKnight;
  return brd.BKnight;
}

template<bool IsWhite>
_Compiletime U64 Bishops(const Board& brd) {
  if constexpr (IsWhite) return brd.WBishop;
  return brd.BBishop;
}

template<bool IsWhite>
_Compiletime U64 BishopMovement(const Board& brd) {
  if constexpr (IsWhite) return brd.WBishop | brd.WPrincess | brd.WQueen | brd.WRQueen;
  return brd.BBishop | brd.BPrincess | brd.BQueen | brd.BRQueen;
}

template<bool IsWhite>
_Compiletime U64 Rooks(const Board& brd) {
  if constexpr (IsWhite) return brd.WRook;
  return brd.BRook;
}

template<bool IsWhite>
_Compiletime U64 RookMovement(const Board& brd) {
  if constexpr (IsWhite) return brd.WRook | brd.WPrincess | brd.WQueen | brd.WRQueen;
  return brd.BRook | brd.BPrincess | brd.BQueen | brd.BRQueen;
}

template<bool IsWhite>
_Compiletime U64 Princesses(const Board& brd) {
  if constexpr (IsWhite) return brd.WPrincess;
  return brd.BPrincess;
}

template<bool IsWhite>
_Compiletime U64 Unicorns(const Board& brd) {
  if constexpr (IsWhite) return brd.WUnicorn;
  return brd.BUnicorn;
}

template<bool IsWhite>
_Compiletime U64 Dragons(const Board& brd) {
  if constexpr (IsWhite) return brd.WDragon;
  return brd.BDragon;
}

template<bool IsWhite>
_Compiletime U64 Queens(const Board& brd) {
  if constexpr (IsWhite) return brd.WQueen;
  return brd.BQueen;
}

template<bool IsWhite>
_Compiletime U64 RoyalQueens(const Board& brd) {
  if constexpr (IsWhite) return brd.WRQueen;
  return brd.BRQueen;
}

template<bool IsWhite>
_Compiletime U64 CommonKings(const Board& brd) {
  if constexpr (IsWhite) return brd.WCKing;
  else return brd.BCKing;
}

template<bool IsWhite>
_Compiletime U64 Kings(const Board& brd) {
  if constexpr (IsWhite) return brd.WKing;
  else return brd.BKing;
}

template<bool IsWhite>
_Compiletime U64 KingMovement(const Board& brd) {
  if constexpr (IsWhite) return brd.WCKing | brd.WKing;
  else return brd.BCKing | brd.BKing;
}

template<bool IsWhite>
_Compiletime U64 Royalty(const Board& brd) {
  if constexpr (IsWhite) return brd.WRQueen | brd.WKing;
  else return brd.BRQueen | brd.BKing;
}

struct Move {
  const Board board;
  const U64 epTarget;
  const int sTimeline, sTurn, eTimeline, eTurn, score;

  Move(const Board brd, const U64 ep, const int sl, const int st, const int el, const int et, const int s) :
    board(brd), epTarget(ep), sTimeline(sl), sTurn(st), eTimeline(el), eTurn(et), score(s) { }
};