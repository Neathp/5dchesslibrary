#pragma once
#include <vector>
#include "chess.hpp"

namespace MoveGen {
  struct Masks {
    U64 banMask = 0, pinD12 = 0, pinHV = 0, doublePin = 0;
    U64 checkMasks[65], pinMasks[64];

    constexpr Masks() {}
  };

  template<bool IsWhite>
  _Compiletime void RegisterPinEP(const U64 occ, const Board &brd, U64 &epTarget, const U64 eRooks) {
    const U64 royalty = BoardFunc::Royalty<IsWhite, true>(brd);
    const U64 pawns = BoardFunc::PieceBB<IsWhite, PawnType>(brd);

    // King is on EP rank and enemy HV walker is on same rank
    const U64 EPRank = 0xffull << ((SquareOf(epTarget) >> 3) << 3); 
    const U64 royaltyEPRank = EPRank & royalty;
    if (royaltyEPRank && (EPRank & eRooks) && (EPRank & pawns)) {
      const U64 royaltysq = SquareOf(royaltyEPRank);
      const U64 EPLpawn = pawns & ((epTarget & BoardFunc::NotRight()) >> 1); //Pawn that can EPTake to the left
      const U64 EPRpawn = pawns & ((epTarget & BoardFunc::NotLeft()) << 1);  //Pawn that can EPTake to the right
      if (EPLpawn) {
        const U64 afterEPocc = (occ ^ royalty) & ~(epTarget | EPLpawn);
        if ((Lookup::Rook(royaltysq, afterEPocc) & EPRank) & eRooks) {
          epTarget = 0;
        }
      }
      if (EPRpawn) {
        const U64 afterEPocc = (occ ^ royalty) & ~(epTarget | EPRpawn);
        if ((Lookup::Rook(royaltysq, afterEPocc) & EPRank) & eRooks) {
          epTarget = 0;
        }
      }
    }
  }

  template<bool IsWhite, bool RQueen>
  _Compiletime void GenCheckPinMasks(const Board &brd, U64 royalty, const U64 ePL, const U64 ePR, const U64 eKnights, const U64 eBishops, const U64 eRooks, const U64 eKings, U8 &n, Masks &masks) {
    U64 pins[64];
    Bitloop (royalty) {
      const U8 sq = SquareOf(royalty);
      const U64 bit = 1ull << sq;
      const U64 offset = sq << 6;

      U64 bishopPin = 0, rookPin = 0;
      // Since double check from leaper pieces cannot happen onto one royal piece, this mask will only ever contain at most one bit set.
      U64 checkMask = BoardFunc::AttackRight<IsWhite>(ePL & bit) | BoardFunc::AttackLeft<IsWhite>(ePR & bit) | (Lookup::Knight(sq) & eKnights) | (Lookup::King(sq) & eKings);
      checkMask |= -(checkMask == 0);

      if (Chess_Lookup::RookMask[sq] & eRooks) {
        U64 atkHV = Lookup::Rook(sq, brd.bitBoards[Occ] ^ BoardFunc::Royalty<IsWhite, RQueen>(brd)) & eRooks;
        Bitloop(atkHV) {
          const U64 sq = offset + SquareOf(atkHV);
          checkMask &= Chess_Lookup::PinBetween[sq];
          masks.banMask |= Chess_Lookup::CheckBetween[sq];
        }

        U64 pinnersHV = Lookup::Rook_Xray(sq, brd.bitBoards[Occ]) & eRooks;
        Bitloop(pinnersHV) {
          const U64 pin = Chess_Lookup::PinBetween[offset + SquareOf(pinnersHV)];
          masks.pinMasks[SquareOf(pin & BoardFunc::Color<IsWhite>(brd))] = pin;
          rookPin |= pin;
        }
      }

      if (Chess_Lookup::BishopMask[sq] & eBishops) {
        U64 atkD12 = Lookup::Bishop(sq, brd.bitBoards[Occ] ^ BoardFunc::Royalty<IsWhite, RQueen>(brd)) & eBishops;
        Bitloop(atkD12) {
          const U64 sq = offset + SquareOf(atkD12);
          checkMask &= Chess_Lookup::PinBetween[sq];
          masks.banMask |= Chess_Lookup::CheckBetween[sq];
        }

        U64 pinnersD12 = Lookup::Bishop_Xray(sq, brd.bitBoards[Occ]) & eBishops;
        Bitloop(pinnersD12) {
          const U64 pin = Chess_Lookup::PinBetween[offset + SquareOf(pinnersD12)];
          masks.pinMasks[SquareOf(pin & BoardFunc::Color<IsWhite>(brd))] = pin;
          bishopPin |= pin;
        }
      }

      const U64 pinMask = bishopPin | rookPin;
      masks.pinD12 |= bishopPin; 
      masks.pinHV |= rookPin; 
      pins[n] = pinMask;
        
      for (int i = 0; i < n; i++) {
        masks.checkMasks[i] &= checkMask;
        masks.doublePin |= pins[i] & pinMask;
      }

      masks.checkMasks[n + 1] = masks.checkMasks[n] & checkMask;
      n++;
    }
  }

  template<PieceType Type>
  _Compiletime void enemyAttack(const U64 occ, U64 piece, U64 &banMask) {
    Bitloop (piece) banMask |= Lookup::PieceMovement<Type>(SquareOf(piece), occ);
  }

  template<BoardState state>
  _Compiletime void Refresh(Turn *turn, Masks &masks) {
    constexpr bool white = state.WhiteMove, enemy = !state.WhiteMove;
    const Board &brd = turn->board; 
    const U64 occ = brd.bitBoards[Occ];

    // Enemy Pawns/Brawns
    const U64 ePawns = BoardFunc::PawnMovement<enemy, state.Brawn>(brd);
    const U64 ePL = BoardFunc::AttackLeft<enemy>(ePawns & BoardFunc::NotLeft()); 
    const U64 ePR = BoardFunc::AttackRight<enemy>(ePawns & BoardFunc::NotRight());
    masks.banMask |= ePL | ePR;

    // Could attempt full directional based lookups for these pieces
    const U64 eKnights = BoardFunc::PieceBB<enemy, KnightType>(brd);
    const U64 eBishops = BoardFunc::BishopMovement<enemy, state.Bishook, state.Princess, state.RQueen>(brd);
    const U64 eRooks = BoardFunc::RookMovement<enemy, state.Bishook, state.Princess, state.RQueen>(brd);
    const U64 eKings = BoardFunc::KingMovement<enemy, state.CKing>(brd);

    enemyAttack<KnightType>(occ, eKnights, masks.banMask);
    enemyAttack<BishopType>(occ, eBishops, masks.banMask);
    enemyAttack<RookType>(occ, eRooks, masks.banMask);
    enemyAttack<KingType>(occ, eKings, masks.banMask);

    // Removing EP if necessary
    if (turn->epTarget) RegisterPinEP<white>(occ, brd, turn->epTarget, eRooks);

    // Generating pinmasks and checkmasks
    U8 n = 0;
    if constexpr (state.RQueen) GenCheckPinMasks<white, true>(brd, BoardFunc::PieceBB<white, RQueenType>(brd), ePL, ePR, eKnights, eBishops, eRooks, eKings, n, masks);
    GenCheckPinMasks<white, state.RQueen>(brd, BoardFunc::PieceBB<white, KingType>(brd), ePL, ePR, eKnights, eBishops, eRooks, eKings, n, masks);

    turn->checkMask = masks.checkMasks[n];
    turn->banMask = masks.banMask;
  }

  template<bool IsWhite>
  _Compiletime void PawnPrune(U64 &pF, U64 &pP, U64 &pL, U64 &pR, const U64 pinHV, const U64 pinD12, const U64 *pinMasks) {
    // Forwards pinned pawns
    U64 pinnedF = pF & pinHV;
    pF ^= pinnedF;
    Bitloop (pinnedF) {
      const U64 sq = SquareOf(pinnedF);
      pF |= (1ull << sq) & BoardFunc::Forward<IsWhite>(pinMasks[sq]);
    }

    // Forwards2 pinned pawns
    U64 pinnedP = pP & pinHV;
    pP ^= pinnedP;
    Bitloop (pinnedP) {
      const U64 sq = SquareOf(pinnedP);
      pP |= (1ull << sq) & BoardFunc::Forward2<IsWhite>(pinMasks[sq]);
    }

    // Left pinned pawns
    U64 pinnedL = pL & pinD12;
    pL ^= pinnedL;
    Bitloop (pinnedL) {
      const U64 sq = SquareOf(pinnedL);
      pL |= (1ull << sq) & BoardFunc::AttackRight<IsWhite>(pinMasks[sq]);
    }

    // Right pinned pawns
    U64 pinnedR = pR & pinD12;
    pR ^= pinnedR;
    Bitloop (pinnedR) {
      const U64 sq = SquareOf(pinnedR);
      pR |= (1ull << sq) & BoardFunc::AttackLeft<IsWhite>(pinMasks[sq]);
    }
  }

  template<bool IsWhite>
  _Compiletime void PawnPruneEP(U64 &epL, U64 &epR, const U64 pinD12, const U64 pinMasks[64]) {
    U64 pinnedL = epL & pinD12;
    epL ^= pinnedL;
    Bitloop (pinnedL) {
      const U64 sq = SquareOf(pinnedL);
      epL |= (1ull << sq) & BoardFunc::AttackRight<IsWhite>(pinMasks[sq]);
    }

    U64 pinnedR = epR & pinD12;
    epR ^= pinnedR;
    Bitloop (pinnedR) {
      const U64 sq = SquareOf(pinnedR);
      epR |= (1ull << sq) & BoardFunc::AttackLeft<IsWhite>(pinMasks[sq]);
    }
  }

  template<bool IsWhite, bool MultPieces, bool Royalty, bool NonPawn>
  _Compiletime void timelineMoves(Turn* turn, const U8 sq, const U8 l, const U8 t, const U8 toL, const U8 toT, const U64 moveSet, std::vector<Move>& moves) {
    if (!turn->valid) return;
    const U64 move = (Royalty) ? moveSet & BoardFunc::EnemyOrEmpty<IsWhite>(turn->board) & turn->checkMask & ~turn->banMask 
                   : (NonPawn) ? moveSet & BoardFunc::EnemyOrEmpty<IsWhite>(turn->board) & turn->checkMask
                               : moveSet & BoardFunc::Color<!IsWhite>(turn->board) & turn->checkMask;

    U64 cap = move & turn->board.bitBoards[Occ];
    Bitloop(cap) {
      const U8 tosq = SquareOf(cap);
      //if constexpr (MultPieces) moves.push_back(Move(tosq, tosq, 0, 0, 8, l, t, toL, toT));
      //else moves.push_back(Move(sq, tosq, 0, 0, 8, l, t, toL, toT));
    }

    if constexpr (NonPawn) {
      U64 nocap = move ^ cap;
      Bitloop(nocap) {
        const U8 tosq = SquareOf(nocap);
        //if constexpr (MultPieces); moves.push_back(Move(tosq, tosq, 0, 0, 7, l, t, toL, toT));
        //else moves.push_back(Move(sq, tosq, 0, 0, 7, l, t, toL, toT));
      }
    }
  }

  template<bool IsWhite, bool Infinite, bool OnlyInfinite, PieceType Type, Direction Dir, int16_t T>
  inline constexpr static U64 buildRingMoves(Turn *turn, const U8 l, const U8 t, const int8_t dist, const U8 sq, U64 tempOcc, std::vector<Move>& moves) {
    constexpr PieceType movementType = (Type == RQueenType) ? QueenType : Type;

    turn += (Dir == North)     ? T      : (Dir == West)      ? -1     : (Dir == South)     ? -T    : (Dir == NorthEast) ? T + 2 
          : (Dir == SouthEast) ? -T - 2 : (Dir == SouthWest) ? -T + 2 : (Dir == NorthWest) ? T - 2                      : 1;

    if (turn->valid == false) return Lookup::PieceMovement<movementType>(sq, tempOcc);

    const U8 toL = (Dir == North)     ? l + dist : (Dir == West)      ? l        : (Dir == South)     ? l - dist : (Dir == NorthEast) ? l + dist 
                 : (Dir == SouthEast) ? l - dist : (Dir == SouthWest) ? l - dist : (Dir == NorthWest) ? l + dist                      : l;
    const U8 toT = (Dir == North)     ? t             : (Dir == West)      ? t - 1         : (Dir == South)     ? t : (Dir == NorthEast) ? t + dist << 1 
                 : (Dir == SouthEast) ? t + dist << 1 : (Dir == SouthWest) ? t - dist << 1 : (Dir == NorthWest) ? t - dist << 1          : t + 1;
    const U64 bit = 1ull << sq;
    const U64 occ = turn->board.bitBoards[Occ];
    const U64 movable = (Type == RQueenType) ? turn->checkMask & BoardFunc::EnemyOrEmpty<IsWhite>(turn->board) & ~turn->banMask 
                                             : turn->checkMask & BoardFunc::EnemyOrEmpty<IsWhite>(turn->board);

    const bool blocked = bit & occ;
    if constexpr (Infinite) {
      if (blocked) {
        if (bit & movable); //moves.push_back(Move(sq, sq, 0, 0, 8, l, t, toL, toT));
        if constexpr (OnlyInfinite) return Lookup::PieceMovement<movementType>(sq, tempOcc);
      } else {
        if (Type != RQueenType || (bit & movable)); //moves.push_back(Move(sq, sq, 0, 0, 7, l, t, toL, toT));
        if constexpr (OnlyInfinite) return buildRingMoves<IsWhite, true, true, Type, Dir, T>(turn, l, t, dist + 1, sq, tempOcc, moves);
      }
    }

    if (dist < 8) {
      const U64 ring = Lookup::Donut(dist, sq);
      tempOcc |= ring & occ;

      const U64 move = (blocked) ? buildRingMoves<IsWhite, false, false, Type, Dir, T>(turn, l, t, dist + 1, sq, tempOcc, moves)
                                 : buildRingMoves<IsWhite, Infinite, false, Type, Dir, T>(turn, l, t, dist + 1, sq, tempOcc, moves);
              
      const U64 moveSlice = ring & move & movable;
      U64 cap = moveSlice & occ;
      U64 nocap = moveSlice ^ cap;
      Bitloop(cap); //moves.emplace_back(sq, SquareOf(cap), 0, 0, 8, l, t, toL, toT);
      Bitloop(nocap); //moves.emplace_back(sq, SquareOf(nocap), 0, 0, 7, l, t, toL, toT);

      return move;
    } else {
      return (Infinite) ? buildRingMoves<IsWhite, Infinite, true, Type, Dir, T>(turn, l, t, dist + 1, sq, tempOcc, moves) : Lookup::PieceMovement<movementType>(sq, tempOcc);
    }
  }

  template<bool IsWhite, PieceType Type, U8 T>
  _Compiletime void royaltyPiece(Turn *turn, const U64 occ, U64 piece, const U64 movableSquare, const U64 *checkMasks, U8 &n, const int l, const int t, std::vector<Move> &moves) {
    Bitloop(piece) {
      const U64 sq = SquareOf(piece);
      const U64 move = Lookup::PieceMovement<Type>(sq, occ) & movableSquare & checkMasks[n];
      U64 cap = move & occ;
      U64 nocap = move ^ cap;
      Bitloop(cap) moves.emplace_back(sq, SquareOf(cap), 0, 0, 1, l, t, l, t);
      Bitloop(nocap) moves.emplace_back(sq, SquareOf(nocap), 0, 0, 0, l, t, l, t);

      //if (checkMasks[n] == 0xffffffffffffffffull) {
      //  if constexpr (Type == RQueenType) {
      //    buildRingMoves<IsWhite, true, false, RQueenType, North, T>(turn, l, t, 0, sq, 0, moves);
      //    buildRingMoves<IsWhite, true, false, RQueenType, West,  T>(turn, l, t, 0, sq, 0, moves);
      //    buildRingMoves<IsWhite, true, false, RQueenType, South, T>(turn, l, t, 0, sq, 0, moves);
      //    buildRingMoves<IsWhite, true, false, RQueenType, NorthEast, T>(turn, l, t, 0, sq, 0, moves);
      //    buildRingMoves<IsWhite, true, false, RQueenType, SouthEast, T>(turn, l, t, 0, sq, 0, moves);
      //    buildRingMoves<IsWhite, true, false, RQueenType, SouthWest, T>(turn, l, t, 0, sq, 0, moves);
      //    buildRingMoves<IsWhite, true, false, RQueenType, NorthWest, T>(turn, l, t, 0, sq, 0, moves);
      //  }
      //  if constexpr (Type == KingType) {
      //    const U64 move = Lookup::King(sq) | 1ull << sq;
      //    timelineMoves<IsWhite, false, true, true>(turn + T, sq, l, t, l + 1, t, move, moves);
      //    timelineMoves<IsWhite, false, true, true>(turn - 1, sq, l, t, l, t - 1, move, moves);
      //    timelineMoves<IsWhite, false, true, true>(turn - T, sq, l, t, l - 1, t, move, moves);
      //    timelineMoves<IsWhite, false, true, true>(turn + T + 2, sq, l, t, l + 1, t + 2, move, moves);
      //    timelineMoves<IsWhite, false, true, true>(turn - T + 2, sq, l, t, l - 1, t + 2, move, moves);
      //    timelineMoves<IsWhite, false, true, true>(turn - T - 2, sq, l, t, l - 1, t - 2, move, moves);
      //    timelineMoves<IsWhite, false, true, true>(turn + T - 2, sq, l, t, l + 1, t - 2, move, moves);
      //  }
      //}
      n++;
    }
  }

  template<bool IsWhite, PieceType Type, U8 T>
  _Compiletime void pinnablePiece(Turn *turn, const U64 occ, const U64 piece, const U64 pins, const U64 movableSquare, const U64 *pinMasks, const int l, const int t, std::vector<Move> &moves) {
    U64 pinPieces = piece & pins;
    U64 nopinPieces = piece ^ pinPieces;
    Bitloop(pinPieces) {
      const U64 sq = SquareOf(pinPieces);
      const U64 move = Lookup::PieceMovement<Type>(sq, occ) & movableSquare & pinMasks[sq];
      U64 cap = move & occ;
      U64 nocap = move ^ cap;
      Bitloop(cap) moves.emplace_back(sq, SquareOf(cap), 0, 0, 1, l, t, l, t);
      Bitloop(nocap) moves.emplace_back(sq, SquareOf(nocap), 0, 0, 0, l, t, l, t);
    }
    Bitloop(nopinPieces) {
      const U64 sq = SquareOf(nopinPieces);
      const U64 move = Lookup::PieceMovement<Type>(sq, occ) & movableSquare;
      U64 cap = move & occ;
      U64 nocap = move ^ cap;
      Bitloop(cap) moves.emplace_back(sq, SquareOf(cap), 0, 0, 1, l, t, l, t);
      Bitloop(nocap) moves.emplace_back(sq, SquareOf(nocap), 0, 0, 0, l, t, l, t);

      //if constexpr (Type == BishopType) {
      //  buildRingMoves<IsWhite, false, false, RookType, North, T>(turn, l, t, 0, sq, 0, moves);
      //  buildRingMoves<IsWhite, false, false, RookType, West,  T>(turn, l, t, 0, sq, 0, moves);
      //  buildRingMoves<IsWhite, false, false, RookType, South, T>(turn, l, t, 0, sq, 0, moves);
      //  buildRingMoves<IsWhite, true, true, NoType, NorthEast, T>(turn, l, t, 0, sq, 0, moves);
      //  buildRingMoves<IsWhite, true, true, NoType, SouthEast, T>(turn, l, t, 0, sq, 0, moves);
      //  buildRingMoves<IsWhite, true, true, NoType, SouthWest, T>(turn, l, t, 0, sq, 0, moves);
      //  buildRingMoves<IsWhite, true, true, NoType, NorthWest, T>(turn, l, t, 0, sq, 0, moves);
      //}
      //if constexpr (Type == RookType) {
      //  buildRingMoves<IsWhite, true, true, RookType, North, T>(turn, l, t, 0, sq, 0, moves);
      //  buildRingMoves<IsWhite, true, true, RookType, West,  T>(turn, l, t, 0, sq, 0, moves);
      //  buildRingMoves<IsWhite, true, true, RookType, South, T>(turn, l, t, 0, sq, 0, moves);
      //}
      //if constexpr (Type == PrincessType) {
      //  buildRingMoves<IsWhite, true, false, PrincessType, North, T>(turn, l, t, 0, sq, 0, moves);
      //  buildRingMoves<IsWhite, true, false, PrincessType, West,  T>(turn, l, t, 0, sq, 0, moves);
      //  buildRingMoves<IsWhite, true, false, PrincessType, South, T>(turn, l, t, 0, sq, 0, moves);
      //  buildRingMoves<IsWhite, true, true, PrincessType, NorthEast, T>(turn, l, t, 0, sq, 0, moves);
      //  buildRingMoves<IsWhite, true, true, PrincessType, SouthEast, T>(turn, l, t, 0, sq, 0, moves);
      //  buildRingMoves<IsWhite, true, true, PrincessType, SouthWest, T>(turn, l, t, 0, sq, 0, moves);
      //  buildRingMoves<IsWhite, true, true, PrincessType, NorthWest, T>(turn, l, t, 0, sq, 0, moves);
      //}
      //if constexpr (Type == QueenType) {
      //  buildRingMoves<IsWhite, true, false, QueenType, North, T>(turn, l, t, 0, sq, 0, moves);
      //  buildRingMoves<IsWhite, true, false, QueenType, West,  T>(turn, l, t, 0, sq, 0, moves);
      //  buildRingMoves<IsWhite, true, false, QueenType, South, T>(turn, l, t, 0, sq, 0, moves);
      //  buildRingMoves<IsWhite, true, false, QueenType, NorthEast, T>(turn, l, t, 0, sq, 0, moves);
      //  buildRingMoves<IsWhite, true, false, QueenType, SouthEast, T>(turn, l, t, 0, sq, 0, moves);
      //  buildRingMoves<IsWhite, true, false, QueenType, SouthWest, T>(turn, l, t, 0, sq, 0, moves);
      //  buildRingMoves<IsWhite, true, false, QueenType, NorthWest, T>(turn, l, t, 0, sq, 0, moves);
      //}
      //if constexpr (Type == CKingType) {
      //  const U64 move = Lookup::King(sq) | 1ull << sq;
      //  timelineMoves<IsWhite, false, false, true>(turn + T, sq, l, t, l + 1, t, move, moves);
      //  timelineMoves<IsWhite, false, false, true>(turn - 1, sq, l, t, l, t - 1, move, moves);
      //  timelineMoves<IsWhite, false, false, true>(turn - T, sq, l, t, l - 1, t, move, moves);
      //  timelineMoves<IsWhite, false, false, true>(turn + T + 2, sq, l, t, l + 1, t + 2, move, moves);
      //  timelineMoves<IsWhite, false, false, true>(turn - T + 2, sq, l, t, l - 1, t + 2, move, moves);
      //  timelineMoves<IsWhite, false, false, true>(turn - T - 2, sq, l, t, l - 1, t - 2, move, moves);
      //  timelineMoves<IsWhite, false, false, true>(turn + T - 2, sq, l, t, l + 1, t - 2, move, moves);
      //}
    }
  }

  template<BoardState state, U16 T>
  _Compiletime void _enumerate(Turn *turn, const U64 epTarget, const U64 checkMask, const U64 pastMask, const Masks &masks, const int l, const int t, std::vector<Move> &moves) {
    constexpr bool white = state.WhiteMove, enemy = !state.WhiteMove;
    constexpr U16 T2 = T << 1;
    const Board &brd = turn->board;
    const U64 occ = brd.bitBoards[Occ];
    const U64 pins = masks.pinD12 | masks.pinHV; 
    const U64 notPinD12 = ~masks.pinD12;
    const U64 notPinHV = ~masks.pinHV;
    const U64 notDoublePin = ~masks.doublePin;

    const U64 enemyOrEmpty = BoardFunc::EnemyOrEmpty<white>(brd);
    const U64 movable = enemyOrEmpty & checkMask;
    const U64 royalMovable = enemyOrEmpty & ~masks.banMask & pastMask;

    U8 n = 0;
    // Royal Queen moves
    if constexpr (state.RQueen) royaltyPiece<white, QueenType, T>(turn, occ, BoardFunc::PieceBB<white, RQueenType>(brd), royalMovable, masks.checkMasks, n, l, t, moves);

    // King moves
    royaltyPiece<white, KingType, T>(turn, occ, BoardFunc::PieceBB<white, KingType>(brd), royalMovable, masks.checkMasks, n, l, t, moves);

    // Castling
    if (checkMask == 0xffffffffffffffffull) {
      U64 kings = BoardFunc::PieceBB<white, KingType>(brd) & brd.bitBoards[UnMoved];
      Bitloop(kings) {
        const U64 sq = SquareOf(kings);
        const U64 bit = 1ull << sq;
        const U64 rank = 0xffull << ((sq >> 3) << 3);

        const U64 kingLow = bit - 1;
        const U64 rooks = BoardFunc::PieceBB<white, RookType>(brd) & brd.bitBoards[UnMoved] & rank;
        const U64 legal = rank & ~masks.banMask;
        const U64 empty = occ ^ rooks;

        const U64 qKing = (bit >> 2) & rank;
        const U64 qRook = rooks & (1ull << (63 - __builtin_clzll((kingLow & rooks) | 1ull)));
        const U64 qLegal = bit - qKing;

        if (qRook && (((bit - qRook) & empty) == 0) && ((qLegal & legal) == qLegal)) moves.emplace_back(sq, sq - 2, SquareOf(qRook), sq - 1, Castle, l, t, l, t);

        const U64 kKing = (bit << 2) & rank;
        const U64 kRook = _blsi_u64(~kingLow & rooks);
        const U64 kLegal = (kKing - bit) << 1;

        if (kRook && ((((kRook - bit) << 1) & empty) == 0) && ((kLegal & legal) == kLegal)) moves.emplace_back(sq, sq + 2, SquareOf(kRook), sq + 1, Castle, l, t, l, t);
      }
    }

    // Pawn/Brawn moves
    const U64 pawns = BoardFunc::PieceBB<white, PawnType>(brd);
    const U64 brawns = BoardFunc::PieceBB<white, BrawnType>(brd);
    const U64 pMove = (pawns | brawns) & notDoublePin;
    const U64 pMoveLR = pMove & notPinHV;
    const U64 pMoveHV = pMove & notPinD12;


    const U64 pMoveUnPin = pMove & ~pins;
    const U64 bUnPin = brawns & ~pins;

    const U64 empty = ~occ;
    const U64 capturableEnemy = BoardFunc::Color<enemy>(brd) & checkMask;

    U64 fPMovement = pMoveHV & BoardFunc::Forward<enemy>(empty);	
    U64 pPMovement = fPMovement & brd.bitBoards[UnMoved] & BoardFunc::Forward2<enemy>(empty & checkMask);
    U64 lPMovement = pMoveLR & BoardFunc::AttackRight<enemy>(capturableEnemy & BoardFunc::NotRight());
    U64 rPMovement = pMoveLR & BoardFunc::AttackLeft<enemy>(capturableEnemy & BoardFunc::NotLeft());

    fPMovement &= BoardFunc::Forward<enemy>(checkMask); 

    PawnPrune<enemy>(fPMovement, pPMovement, lPMovement, rPMovement, masks.pinHV, masks.pinD12, masks.pinMasks);

    // Enpassant
    if (epTarget) {
      const U64 validEPTarget = epTarget & checkMask;
      U64 epL = pMoveLR & BoardFunc::NotLeft() & (validEPTarget << 1);
      U64 epR = pMoveLR & BoardFunc::NotRight() & (validEPTarget >> 1);
                
      if (epL | epR) {
        PawnPruneEP<enemy>(epL, epR, masks.pinD12, masks.pinMasks);
        if (epL) {const U8 sq = SquareOf(epL); moves.emplace_back(sq, BoardFunc::AttackLeftSquare<white>(sq), sq - 1, 0, 3, l, t, l, t);}
        if (epR) {const U8 sq = SquareOf(epR); moves.emplace_back(sq, BoardFunc::AttackRightSquare<white>(sq), sq + 1, 0, 3, l, t, l, t);}
      }
    }

    // We have pawns that can move on last rank
    constexpr U64 lastRank = BoardFunc::LastRank<white>();
    if ((fPMovement | pPMovement | lPMovement | rPMovement) & lastRank) {
      U64 proF = fPMovement & lastRank; 
      U64 proP = pPMovement & lastRank; 
      U64 proL = lPMovement & lastRank; 
      U64 proR = rPMovement & lastRank; 

      U64 noProF = fPMovement ^ proF; 
      U64 noProP = pPMovement ^ proP;
      U64 noProL = lPMovement ^ proL;
      U64 noProR = rPMovement ^ proR;

      constexpr Piece queen = (white) ? WQueen : BQueen;
      constexpr Piece bishop = (white) ? WBishop : BBishop;
      constexpr Piece rook = (white) ? WRook : BRook;
      constexpr Piece knight = (white) ? WKnight : BKnight;

      Bitloop(proF) {
        const U8 sq = SquareOf(proF); 
        const U8 to = BoardFunc::ForwardSquare<white>(sq);
        moves.emplace_back(sq, to, queen, 0, Promo, l, t, l, t);
        moves.emplace_back(sq, to, bishop, 0, Promo, l, t, l, t);
        moves.emplace_back(sq, to, rook, 0, Promo, l, t, l, t);
        moves.emplace_back(sq, to, knight, 0, Promo, l, t, l, t);
      }

      Bitloop(proP) {
        const U8 sq = SquareOf(proP);
        const U8 to = BoardFunc::Forward2Square<white>(sq);
        moves.emplace_back(sq, to, queen, 0, Promo, l, t, l, t);
        moves.emplace_back(sq, to, bishop, 0, Promo, l, t, l, t);
        moves.emplace_back(sq, to, rook, 0, Promo, l, t, l, t);
        moves.emplace_back(sq, to, knight, 0, Promo, l, t, l, t);
      }

      Bitloop(proL) {
        const U8 sq = SquareOf(proL);
        const U8 to = BoardFunc::AttackLeftSquare<white>(sq);
        moves.emplace_back(sq, to, queen, 0, PromoCap, l, t, l, t);
        moves.emplace_back(sq, to, bishop, 0, PromoCap, l, t, l, t);
        moves.emplace_back(sq, to, rook, 0, PromoCap, l, t, l, t);
        moves.emplace_back(sq, to, knight, 0, PromoCap, l, t, l, t);
      }

      Bitloop(proR) {
        const U8 sq = SquareOf(proR); 
        const U8 to = BoardFunc::AttackRightSquare<white>(sq);
        moves.emplace_back(sq, to, queen, 0, PromoCap, l, t, l, t);
        moves.emplace_back(sq, to, bishop, 0, PromoCap, l, t, l, t);
        moves.emplace_back(sq, to, rook, 0, PromoCap, l, t, l, t);
        moves.emplace_back(sq, to, knight, 0, PromoCap, l, t, l, t);
      }

      Bitloop(noProF) {const U8 sq = SquareOf(noProF); moves.emplace_back(sq, BoardFunc::ForwardSquare<white>(sq), 0, 0, 0, l, t, l, t);}
      Bitloop(noProP) {const U8 sq = SquareOf(noProP); moves.emplace_back(sq, BoardFunc::Forward2Square<white>(sq), 0, 0, 2, l, t, l, t);}
      Bitloop(noProL) {const U8 sq = SquareOf(noProL); moves.emplace_back(sq, BoardFunc::AttackLeftSquare<white>(sq), 0, 0, 1, l, t, l, t);}
      Bitloop(noProR) {const U8 sq = SquareOf(noProR); moves.emplace_back(sq, BoardFunc::AttackRightSquare<white>(sq), 0, 0, 1, l, t, l, t);}
    } else {
      Bitloop(fPMovement) {const U8 sq = SquareOf(fPMovement); moves.emplace_back(sq, BoardFunc::ForwardSquare<white>(sq), 0, 0, 0, l, t, l, t);}
      Bitloop(pPMovement) {const U8 sq = SquareOf(pPMovement); moves.emplace_back(sq, BoardFunc::Forward2Square<white>(sq), 0, 0, 2, l, t, l, t);}
      Bitloop(lPMovement) {const U8 sq = SquareOf(lPMovement); moves.emplace_back(sq, BoardFunc::AttackLeftSquare<white>(sq), 0, 0, 1, l, t, l, t);}
      Bitloop(rPMovement) {const U8 sq = SquareOf(rPMovement); moves.emplace_back(sq, BoardFunc::AttackRightSquare<white>(sq), 0, 0, 1, l, t, l, t);}
    }

    //constexpr int16_t indexDir = (white) ? 1 : -1;
    //constexpr int16_t pointerDir = (white) ? T : -T;
    //const U16 toL = l + indexDir;
    //timelineMoves<white, true, false, false>(turn + pointerDir + 2, l, t, toL, t + 2, 0, pMoveUnPin, moves);
    //timelineMoves<white, true, false, false>(turn + pointerDir - 2, l, t, toL, t - 2, 0, pMoveUnPin, moves);
//
//
    //Turn *destTurn = turn + pointerDir;
    //if (destTurn->valid) {
    //  //Single Push timeline
    //  U64 move = pMoveUnPin & ~destTurn->board.bitBoards[Occ];
    //  const U64 pawnPush = move & turn->board.bitBoards[UnMoved];
    //  move &= destTurn->checkMask;
//
    //  Bitloop(move) {
    //    const U8 sq = SquareOf(move);
    //    //moves.push_back(Move(sq, sq, 0, 0, 7, l, t, toL, t));
    //  }
//
    //  //Double push timeline
    //  Turn *destTurn2 = destTurn + pointerDir;
    //  if (destTurn->valid) {
    //    U64 move = pawnPush & ~destTurn2->board.bitBoards[Occ] & destTurn2->checkMask;
    //    Bitloop(move) {
    //      const U8 sq = SquareOf(move);
    //      //moves.push_back(Move(sq, sq, 0, 0, 7, l, t, l + (T << 1), t));
    //    }
    //  }
//
    //  const U64 movable = BoardFunc::Color<!white>(destTurn->board) & destTurn->checkMask;
//
    //  U64 lBrawns = ((movable & BoardFunc::NotRight()) << 1) & bUnPin;
    //  U64 rBrawns = ((movable & BoardFunc::NotLeft()) >> 1) & bUnPin;
    //  const U64 fBrawns = BoardFunc::Forward<enemy>(movable) & bUnPin;
    //  U64 fPromo = fBrawns & BoardFunc::LastRank<white>();
    //  U64 fNoPromo = fBrawns ^ fPromo;
//
    //  Bitloop(lBrawns) {
    //    const U8 sq = SquareOf(lBrawns);
    //    //moves.push_back(Move(sq, sq - 1, 0, 0, 8, l, t, toL, t));
    //  }
//
    //  Bitloop(rBrawns) {
    //    const U8 sq = SquareOf(rBrawns);
    //    //moves.push_back(Move(sq, sq + 1, 0, 0, 8, l, t, toL, t));
    //  }
//
    //  Bitloop(fNoPromo) {
    //    const U8 sq = SquareOf(fNoPromo);
    //    //moves.push_back(Move(sq, BoardFunc::ForwardSquare<white>(sq), 0, 0, 8, l, t, toL, t));
    //  }
//
    //  constexpr Piece queen = (white) ? WQueen : BQueen;
    //  constexpr Piece bishop = (white) ? WBishop : BBishop;
    //  constexpr Piece rook = (white) ? WRook : BRook;
    //  constexpr Piece knight = (white) ? WKnight : BKnight;
//
    //  Bitloop(fPromo) {
    //    //moves.push_back(Move(sq, BoardFunc::ForwardSquare<white>(sq), queen, 0, 8, l, t, toL, t));
    //    //moves.push_back(Move(sq, BoardFunc::ForwardSquare<white>(sq), bishop, 0, 8, l, t, toL, t));
    //    //moves.push_back(Move(sq, BoardFunc::ForwardSquare<white>(sq), rook, 0, 8, l, t, toL, t));
    //    //moves.push_back(Move(sq, BoardFunc::ForwardSquare<white>(sq), knight, 0, 8, l, t, toL, t));
    //  }
    //}
//
    //Turn *destTurnB = turn - 2;
    //if (destTurnB->valid) {
    //  const U16 toT = t - 2;
    //  const U64 fBrawns = BoardFunc::Forward<enemy>(BoardFunc::Color<!white>(destTurn->board) & destTurn->checkMask) & bUnPin;
    //  U64 fPromo = fBrawns & BoardFunc::LastRank<white>();
    //  U64 fNoPromo = fBrawns ^ fPromo;
//
    //  Bitloop(fNoPromo) {
    //    const U8 sq = SquareOf(fNoPromo);
    //    //moves.push_back(Move(sq, BoardFunc::ForwardSquare<white>(sq), 0, 0, 8, l, t, l, toT));
    //  }
//
    //  constexpr Piece queen = (white) ? WQueen : BQueen;
    //  constexpr Piece bishop = (white) ? WBishop : BBishop;
    //  constexpr Piece rook = (white) ? WRook : BRook;
    //  constexpr Piece knight = (white) ? WKnight : BKnight;
//
    //  Bitloop(fPromo) {
    //    //moves.push_back(Move(sq, BoardFunc::ForwardSquare<white>(sq), queen, 0, 8, l, t, l, toT));
    //    //moves.push_back(Move(sq, BoardFunc::ForwardSquare<white>(sq), bishop, 0, 8, l, t, l, toT));
    //    //moves.push_back(Move(sq, BoardFunc::ForwardSquare<white>(sq), rook, 0, 8, l, t, l, toT));
    //    //moves.push_back(Move(sq, BoardFunc::ForwardSquare<white>(sq), knight, 0, 8, l, t, l, toT));
    //  }
    //}

    // Knight moves
    U64 knights = BoardFunc::PieceBB<white, KnightType>(brd) & ~pins;
    if (knights) {
      const U8 sq = SquareOf(knights);
      const U64 bit = 1ull << sq;

      //Spatial
      U64 move = Lookup::Knight(sq) & movable;
      U64 cap = move & occ;
      U64 nocap = move ^ cap;

      Bitloop(cap) moves.emplace_back(sq, SquareOf(cap), 0, 0, 1, l, t, l, t);
      Bitloop(nocap) moves.emplace_back(sq, SquareOf(nocap), 0, 0, 0, l, t, l, t);

      //timelineMoves<white, true, false, true>(turn + T + 4, l, t, l + 1, t + 4, 0, knights, moves);
      //timelineMoves<white, true, false, true>(turn + T - 4, l, t, l + 1, t - 4, 0, knights, moves);
      //timelineMoves<white, true, false, true>(turn - T + 4, l, t, l - 1, t + 4, 0, knights, moves);
      //timelineMoves<white, true, false, true>(turn - T - 4, l, t, l - 1, t - 4, 0, knights, moves);
      //timelineMoves<white, true, false, true>(turn + T2 + 4, l, t, l + 2, t + 2, 0, knights, moves);
      //timelineMoves<white, true, false, true>(turn + T2 - 4, l, t, l + 2, t - 2, 0, knights, moves);
      //timelineMoves<white, true, false, true>(turn - T2 + 4, l, t, l - 2, t + 2, 0, knights, moves);
      //timelineMoves<white, true, false, true>(turn - T2 - 4, l, t, l - 2, t - 2, 0, knights, moves);

      //U64 knight1 = Lookup::Knight1(sq);
      //U64 knight2 = Lookup::Knight2(sq);
      //timelineMoves<white, false, false, true>(turn - 2, sq, l, t, l, t - 2, knight1, moves);
      //timelineMoves<white, false, false, true>(turn - 4, sq, l, t, l, t - 4, knight2, moves);
      //timelineMoves<white, false, false, true>(turn + T, sq, l, t, l + 1, t, knight1, moves);
      //timelineMoves<white, false, false, true>(turn + T2, sq, l, t, l + 2, t, knight2, moves);
      //timelineMoves<white, false, false, true>(turn - T, sq, l, t, l - 1, t, knight1, moves);
      //timelineMoves<white, false, false, true>(turn - T2, sq, l, t, l - 2, t, knight2, moves);

      knights ^= bit;
      Bitloop(knights) {
        const U64 sq = SquareOf(knights);
        const U64 move = Lookup::Knight(sq) & movable;
        U64 cap = move & occ;
        U64 nocap = move ^ cap;

        Bitloop(cap) moves.emplace_back(sq, SquareOf(cap), 0, 0, 1, l, t, l, t);
        Bitloop(nocap) moves.emplace_back(sq, SquareOf(nocap), 0, 0, 0, l, t, l, t);

        //U64 knight1 = Lookup::Knight1(sq);
        //U64 knight2 = Lookup::Knight2(sq);
        //timelineMoves<white, false, false, true>(turn - 2, sq, l, t, l, t - 2, knight1, moves);
        //timelineMoves<white, false, false, true>(turn - 4, sq, l, t, l, t - 4, knight2, moves);
        //timelineMoves<white, false, false, true>(turn + T, sq, l, t, l + 1, t, knight1, moves);
        //timelineMoves<white, false, false, true>(turn + T2, sq, l, t, l + 2, t, knight2, moves);
        //timelineMoves<white, false, false, true>(turn - T, sq, l, t, l - 1, t, knight1, moves);
        //timelineMoves<white, false, false, true>(turn - T2, sq, l, t, l - 2, t, knight2, moves);
      }
    }

    if constexpr (state.Bishook)  pinnablePiece<white, BishopType, T>(turn, occ, BoardFunc::PieceBB<white, BishopType>(brd) & notPinHV & notDoublePin, pins, movable, masks.pinMasks, l, t, moves);
    if constexpr (state.Bishook)  pinnablePiece<white, RookType  , T>(turn, occ, BoardFunc::PieceBB<white, RookType>(brd) & notPinD12 & notDoublePin, pins, movable, masks.pinMasks, l, t, moves);
    if constexpr (state.Princess) pinnablePiece<white, QueenType , T>(turn, occ, BoardFunc::PieceBB<white, PrincessType>(brd) & notDoublePin, pins, movable, masks.pinMasks, l, t, moves);
                                  pinnablePiece<white, QueenType , T>(turn, occ, BoardFunc::PieceBB<white, QueenType>(brd) & notDoublePin, pins, movable, masks.pinMasks, l, t, moves);
    if constexpr (state.CKing)    pinnablePiece<white, KingType  , T>(turn, occ, BoardFunc::PieceBB<white, CKingType>(brd) & notDoublePin, pins, movable, masks.pinMasks, l, t, moves);

    //if constexpr (true) {
    //  U64 unicorns = BoardFunc::PieceBB<white, UnicornType>(brd) & ~pins;
    //  Bitloop(unicorns) {
    //    const U64 sq = SquareOf(unicorns);
    //    buildRingMoves<white, true, false, BishopType, North, T>(turn, l, t, 0, sq, 0, moves);
    //    buildRingMoves<white, true, false, BishopType, West, T>(turn, l, t, 0, sq, 0, moves);
    //    buildRingMoves<white, true, false, BishopType, South, T>(turn, l, t, 0, sq, 0, moves);
    //    buildRingMoves<white, true, false, RookType, NorthEast, T>(turn, l, t, 0, sq, 0, moves);
    //    buildRingMoves<white, true, false, RookType, SouthEast, T>(turn, l, t, 0, sq, 0, moves);
    //    buildRingMoves<white, true, false, RookType, SouthWest, T>(turn, l, t, 0, sq, 0, moves);
    //    buildRingMoves<white, true, false, RookType, NorthWest, T>(turn, l, t, 0, sq, 0, moves);
    //  }
//
    //  U64 dragons = BoardFunc::PieceBB<white, DragonType>(brd) & ~pins;
    //  Bitloop(dragons) {
    //    const U64 sq = SquareOf(dragons);
    //    buildRingMoves<white, true, false, BishopType, NorthEast, T>(turn, l, t, 0, sq, 0, moves);
    //    buildRingMoves<white, true, false, BishopType, SouthEast, T>(turn, l, t, 0, sq, 0, moves);
    //    buildRingMoves<white, true, false, BishopType, SouthWest, T>(turn, l, t, 0, sq, 0, moves);
    //    buildRingMoves<white, true, false, BishopType, NorthWest, T>(turn, l, t, 0, sq, 0, moves);
    //  }
    //}
     
  }

  template<BoardState state, U8 T>
  _Compiletime void EnumerateMoves(Turn *turn, const int l, const int t, std::vector<Move> &moves) {
    constexpr bool white = state.WhiteMove;
    const Board &brd = turn->board;
    //U64 pastMask = BoardFunc::PastCheck<white>(turn->boardMask, brd);
    U64 pastMask = 0;
    if ((pastMask & (pastMask - 1)) != 0) return;
    pastMask |= -(pastMask == 0);

    Masks masks;
    masks.checkMasks[0] = 0xffffffffffffffffull;
    Refresh<state>(turn, masks);

    const U64 checkMask = turn->checkMask & pastMask;
    if (checkMask != 0) {
      _enumerate<state, T>(turn, turn->epTarget, checkMask, pastMask, masks, l, t, moves);
    } else {
      U8 n = 0;
      const U64 occ = brd.bitBoards[Occ];
      const U64 movable = BoardFunc::EnemyOrEmpty<white>(brd) & ~masks.banMask & pastMask;
      if constexpr (state.RQueen) royaltyPiece<white, QueenType, T>(turn, occ, BoardFunc::PieceBB<white, RQueenType>(brd), movable, masks.checkMasks, n, l, t, moves);
      royaltyPiece<white, KingType, T>(turn, occ, BoardFunc::PieceBB<white, KingType>(brd), movable, masks.checkMasks, n, l, t, moves);
    }
  }
};