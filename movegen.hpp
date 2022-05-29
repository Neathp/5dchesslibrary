#pragma once
#include "chess.hpp"
#include "io.hpp"

namespace Movelist {
  _Compiletime void CheckBySlider(Turn& turn, U64& checkMask, const U64& sq) {
    checkMask &= Chess_Lookup::PinBetween[sq];
    turn.banMask |= Chess_Lookup::CheckBetween[sq];
  }

  template<bool IsWhite>
  _Compiletime void RegisterPinHV(const Board &brd, const U64 &king, const U64 &enemy, U64 &pinHV) {
    const U64 pinmask = Chess_Lookup::PinBetween[king * 64 + enemy];

    if (pinmask & Color<IsWhite>(brd)) pinHV |= pinmask;
  }

  template<bool IsWhite>
  _Compiletime void RegisterPinD12(const Board &brd, const U64 &king, const U64 &enemy, U64 &pinD12) {
    const U64 pinmask = Chess_Lookup::PinBetween[king * 64 + enemy];

    if (pinmask & Color<IsWhite>(brd)) pinD12 |= pinmask;
  }

  template<bool IsWhite>
  _Compiletime U64 EPSquare(const U64& epTarget) {
	  if constexpr (IsWhite) return epTarget << 8;
	  return epTarget >> 8;
  }

  template<bool IsWhite>
  _Compiletime void RegisterPinEP(Turn& turn, const U64& eRooks) {
    constexpr bool white = IsWhite;
    const Board brd = *turn.board;
    const U64 royalty = Royalty<white>(brd), pawns = Pawns<white>(brd), epTarget = turn.epTarget;

    //King is on EP rank and enemy HV walker is on same rank
    const U64 EPRank = 0xffull << ((SquareOf(epTarget) >> 3) << 3); 
    const U64 royaltyEPRank = EPRank & royalty;
    if (royaltyEPRank && (EPRank & eRooks) && (EPRank & pawns)) {
      const U64 royaltysq = SquareOf(royaltyEPRank);
      const U64 EPLpawn = pawns & ((epTarget & Pawns_NotRight()) >> 1); //Pawn that can EPTake to the left
      const U64 EPRpawn = pawns & ((epTarget & Pawns_NotLeft()) << 1);  //Pawn that can EPTake to the right
      if (EPLpawn) {
        const U64 afterEPocc = (brd.Occ ^ royalty) & ~(epTarget | EPLpawn);
        if ((Lookup::Rook(royaltysq, afterEPocc) & EPRank) & eRooks) {
          turn.epTarget = 0;
        }
      }
      if (EPRpawn) {
        const U64 afterEPocc = (brd.Occ ^ royalty) & ~(epTarget | EPRpawn);
        if ((Lookup::Rook(royaltysq, afterEPocc) & EPRank) & eRooks) {
          turn.epTarget = 0;
        }
      }
    }
  }

  template<bool IsWhite>
  _Compiletime void Refresh(Turn& turn, U64 &pinD12, U64 &pinHV, U64 &doublePin, U64 (&checkMasks)[65], U64 (&pinsD12)[64], U64 (&pinsHV)[64]) {
    constexpr bool white = IsWhite, enemy = !IsWhite;
    const Board brd = *turn.board;
    const U64 occ = brd.Occ;

    // Generating banmask.

    // Enemy Pawns/Brawns
    const U64 ePawns = PawnMovement<enemy>(brd);
    const U64 ePL = Pawn_AttackLeft<enemy>(ePawns & Pawns_NotLeft()), ePR = Pawn_AttackRight<enemy>(ePawns & Pawns_NotRight());
    turn.banMask |= ePL | ePR;

    // Enemy Knights
    const U64 eKnights = Knights<enemy>(brd);
    {
      U64 knights = eKnights;
      Bitloop (knights) {
        turn.banMask |= Lookup::Knight(SquareOf(knights));
      }
    }
    
    // Enemy Bishops/Princesses/Queens/RoyalQueens
    const U64 eBishops = BishopMovement<enemy>(brd);
    {
      U64 bishops = eBishops;
      Bitloop(bishops) {
        turn.banMask |= Lookup::Bishop(SquareOf(bishops), occ);
      }
    }

    // Enemy Rooks/Princesses/Queens/RoyalQueens
    const U64 eRooks = RookMovement<enemy>(brd);
    {
      U64 rooks = eRooks;
      Bitloop(rooks) {
        turn.banMask |= Lookup::Rook(SquareOf(rooks), occ);
      }
    }
  
    // Enemy Kings
    const U64 eKings = KingMovement<enemy>(brd);
    {
      U64 kings = eKings;
      Bitloop (kings) {
        turn.banMask |= Lookup::King(SquareOf(kings));
      }
    }

    // Generating pinmasks and checkmasks.
    int n = 0;

    // RoyalQueens
    {
      U64 royalQueens = RoyalQueens<white>(brd);
      Bitloop (royalQueens) {
        const U64 royalqueensq = SquareOf(royalQueens);
        const U64 royalqueenbit = 1ull << royalqueensq;

        U64 bishopPin = 0, rookPin = 0;
        // Since double check from leaper pieces cannot happen onto one royal queen, this mask will only ever contain at most one bit set.
        U64 checkMask = Pawn_AttackRight<white>(ePL & royalqueenbit) | Pawn_AttackLeft<white>(ePR & royalqueenbit) | (Lookup::Knight(royalqueensq) & eKnights) | (Lookup::King(royalqueensq) & eKings);
        checkMask |= -(checkMask == 0);

        if (Chess_Lookup::RookMask[royalqueensq] & eRooks) {
          U64 atkHV = Lookup::Rook(royalqueensq, occ ^ Royalty<white>(brd)) & eRooks;
          Bitloop(atkHV) {
            CheckBySlider(turn, checkMask, (royalqueensq << 6) + SquareOf(atkHV));
          }

          U64 pinnersHV = Lookup::Rook_Xray(royalqueensq, occ) & eRooks;
          Bitloop(pinnersHV) {
            RegisterPinHV<white>(brd, royalqueensq, SquareOf(pinnersHV), rookPin);
          }
        }

        if (Chess_Lookup::BishopMask[royalqueensq] & eBishops) {
          U64 atkD12 = Lookup::Bishop(royalqueensq, occ ^ Royalty<white>(brd)) & eBishops;
          Bitloop(atkD12) {
            CheckBySlider(turn, checkMask, (royalqueensq << 6) + SquareOf(atkD12));
          }

          U64 pinnersD12 = Lookup::Bishop_Xray(royalqueensq, occ) & eBishops;
          Bitloop(pinnersD12) {
            RegisterPinD12<white>(brd, royalqueensq, SquareOf(pinnersD12), bishopPin);
          }
        }
        
        for (int i = 0; i < n; i++) {
          checkMasks[i] &= checkMask;
          doublePin |= (bishopPin | rookPin) & (pinsD12[i] | pinsHV[i]);
        }
        pinD12 |= bishopPin; pinHV |= rookPin;
        pinsD12[n] = bishopPin; pinsHV[n] = rookPin;
        checkMasks[n + 1] = checkMasks[n] & checkMask;
        n++;
      }
    }

    // Kings
    {
      U64 kings = Kings<white>(brd);
      Bitloop (kings) {
        const U64 kingsq = SquareOf(kings);
        const U64 kingbit = 1ull << kingsq;

        U64 bishopPin = 0, rookPin = 0;
        // Since double check from leaper pieces cannot happen onto one king, this mask will only ever contain at most one bit set.
        U64 checkMask = Pawn_AttackRight<white>(ePL & kingbit) | Pawn_AttackLeft<white>(ePR & kingbit) | (Lookup::Knight(kingsq) & eKnights) | (Lookup::King(kingsq) & eKings);
        checkMask |= -(checkMask == 0);

        if (Chess_Lookup::RookMask[kingsq] & eRooks) {
          U64 atkHV = Lookup::Rook(kingsq, occ ^ Royalty<white>(brd)) & eRooks;
          Bitloop(atkHV) {
            CheckBySlider(turn, checkMask, (kingsq << 6) + SquareOf(atkHV));
          }

          U64 pinnersHV = Lookup::Rook_Xray(kingsq, occ) & eRooks;
          Bitloop(pinnersHV) {
            RegisterPinHV<white>(brd, kingsq, SquareOf(pinnersHV), rookPin);
          }
        }

        if (Chess_Lookup::BishopMask[kingsq] & eBishops) {
          U64 atkD12 = Lookup::Bishop(kingsq, occ ^ Royalty<white>(brd)) & eBishops;
          Bitloop(atkD12) {
            CheckBySlider(turn, checkMask, (kingsq << 6) + SquareOf(atkD12));
          }

          U64 pinnersD12 = Lookup::Bishop_Xray(kingsq, occ) & eBishops;
          Bitloop(pinnersD12) {
            RegisterPinD12<white>(brd, kingsq, SquareOf(pinnersD12), bishopPin);
          }
        }
            
        for (int i = 0; i < n; i++) {
          checkMasks[i] &= checkMask;
          doublePin |= (bishopPin | rookPin) & (pinsD12[i] | pinsHV[i]);
        }
        pinD12 |= bishopPin; pinHV |= rookPin;
        pinsD12[n] = bishopPin; pinsHV[n] = rookPin;
        checkMasks[n + 1] = checkMasks[n] & checkMask;
        n++;
      }
    }

    if (turn.epTarget) {
      RegisterPinEP<white>(turn, eRooks);
    }

    turn.checkMask = checkMasks[n];
  }

  template<bool IsWhite>
  _Compiletime void Pawn_PruneLeft(U64& pawn, const U64 &pinD12, const U64 (&pinsD12)[64], const int &n) {
    const U64 unpinned = pawn & ~pinD12;
    U64 pinned = 0;
    for (int i = 0; i < n; i++) pinned |= pawn & pinsD12[i] & Pawn_InvertLeft<IsWhite>(pinsD12[i] & Pawns_NotRight());

    pawn = (pinned | unpinned);
  }

  template<bool IsWhite>
  _Compiletime void Pawn_PruneRight(U64& pawn, const U64 &pinD12, const U64 (&pinsD12)[64], const int &n) {
    const U64 unpinned= pawn & ~pinD12;
    U64 pinned = 0;
    for (int i = 0; i < n; i++) pinned |= pawn & pinsD12[i] & Pawn_InvertRight<IsWhite>(pinsD12[i] & Pawns_NotLeft());

    pawn = (pinned | unpinned);
  }

  template<bool IsWhite>
  _Compiletime void Pawn_PruneLeftEP(U64& pawn, const U64 &pinD12, const U64 (&pinsD12)[64], const int &n) {
    const U64 unpinned = pawn & ~pinD12;
    U64 pinned = 0; 
    for (int i = 0; i < n; i++) pinned |= pawn & pinsD12[i] & Pawn_InvertLeft<IsWhite>(pinsD12[i] & Pawns_NotRight());

    pawn = (pinned | unpinned);
  }

  template<bool IsWhite>
  _Compiletime void Pawn_PruneRightEP(U64& pawn, const U64 &pinD12, const U64 (&pinsD12)[64], const int &n) {
    const U64 unpinned = pawn & ~pinD12;
    U64 pinned = 0;
    for (int i = 0; i < n; i++) pinned |= pawn & pinsD12[i] & Pawn_InvertRight<IsWhite>(pinsD12[i] & Pawns_NotLeft());

    pawn = (pinned | unpinned); 
  }

  template<bool IsWhite>
  _Compiletime void Pawn_PruneMove(U64& pawn, const U64 &pinHV, const U64 (&pinsHV)[64], const int &n) {
    const U64 unpinned = pawn & ~pinHV;
    U64 pinned = 0;
    for (int i = 0; i < n; i++) pinned |= pawn & pinsHV[i] & Pawn_Backward<IsWhite>(pinsHV[i]);

    pawn = (unpinned | pinned);
  }

  template<bool IsWhite>
  _Compiletime void Pawn_PruneMove2(U64& pawn, const U64 &pinHV, const U64 (&pinsHV)[64], const int &n) {
    const U64 unpinned = pawn & ~pinHV;
    U64 pinned = 0;
    for (int i = 0; i < n; i++) pinned |= pawn & pinsHV[i] & Pawn_Backward2<IsWhite>(pinsHV[i]);
    

    pawn = (unpinned | pinned);
  }

  template<bool IsWhite, class Callback_Move>
  _Compiletime void _enumerate(Turn &turn, const U64 &pinD12, const U64 &pinHV, const U64 &doublePin, const U64 &checkMask, const U64 &pastMask, 
    const U64 (&checkMasks)[65], const U64 (&pinsD12)[64], const U64 (&pinsHV)[64], std::vector<Move> &moves) {

    constexpr bool white = IsWhite, enemy = !IsWhite;

    const Board brd = *turn.board;

    const U64 kingban = turn.banMask, epTarget = turn.epTarget;
    const U64 movableSquare = EnemyOrEmpty<white>(brd) & checkMask;

    // Royalty moves
    int n = 0;
    {
      const U64 movable = EnemyOrEmpty<white>(brd) & ~turn.banMask & pastMask;
      // Royal Queen moves
      {
        U64 royalQueens = RoyalQueens<white>(brd);
        Bitloop (royalQueens) {
          const U64 sq = SquareOf(royalQueens);
          U64 move = Lookup::Queen(sq, brd.Occ) & movable & checkMasks[n];
          while (move) {
            Callback_Move::template RoyalQueenmove<white>(brd, 1ull << sq, PopBit(move), moves);
          }
          n++;
        }
      }
      
      // King moves
      {
        U64 kings = Kings<white>(brd);
        Bitloop (kings) {
          const U64 sq = SquareOf(kings);
          U64 move = Lookup::King(sq) & movable & checkMasks[n];
          while (move) {
            Callback_Move::template Kingmove<white>(brd, 1ull << sq, PopBit(move), moves);
          }
          n++;
        }

        // Castling
        if (checkMask == 0xffffffffffffffffull) {
          kings = Kings<white>(brd) & brd.UnMoved;
          while(kings) {
            const U64 king = PopBit(kings), sq = SquareOf(king), rank = 0xffull << ((sq >> 3) << 3);
            const U64 qKing = (king >> 2) & rank, kKing = (king << 2) & rank;

            const U64 kingLow = king - 1;
            const U64 rooks = Rooks<white>(brd) & brd.UnMoved & rank, legal = rank & ~kingban, empty = brd.Occ ^ rooks;

            // Queenside Castling
            const U64 qRook = rooks & (1ull << (63 - __builtin_clzll((kingLow & rooks) | 1ull))), qLegal = king - qKing;
            if (qRook && ((qLegal & legal) == qLegal) && (((king - qRook) & empty) == 0)) {
              Callback_Move::template KingCastle<white>(brd, (king | qKing), (qRook | king >> 1), moves);
            }

            // Kingside Castling
            const U64 kRook = _blsi_u64(~kingLow & rooks), kLegal = (kKing - king) << 1;
            if (kRook && ((kLegal & legal) == kLegal) && ((((kRook - king) << 1) & empty) == 0)) {
              Callback_Move::template KingCastle<white>(brd, (king | kKing), (kRook | king << 1), moves);
            }
          }
        }
      }
    }

    // Pawn/Brawn moves
    {
      const U64 pawns = Pawns<white>(brd) & ~doublePin, brawns = Brawns<white>(brd), pMovement = pawns | brawns;
      const U64 pMovementLR = pMovement & ~pinHV, pMovementHV = pMovement & ~pinD12;
      const U64 empty = Empty(brd), capturableEnemy = Color<enemy>(brd) & checkMask;

      //These 4 are basic pawn moves
      U64 fPMovement = pMovementHV & Pawn_Backward<white>(empty);	
      U64 pPMovement = fPMovement & brd.UnMoved & Pawn_Backward2<white>(empty & checkMask);  
      U64 lPMovement = pMovementLR & Pawn_InvertLeft<white>(capturableEnemy & Pawns_NotRight());
      U64 rPMovement = pMovementLR & Pawn_InvertRight<white>(capturableEnemy & Pawns_NotLeft());

      //checkmask moved here to use fpawn for faster Ppawn calc: Pawn on P2 can only push - not move
      fPMovement &= Pawn_Backward<white>(checkMask); 

      //These 4 basic moves get pruned with pin information
      Pawn_PruneMove<white>(fPMovement, pinHV, pinsHV, n);
      Pawn_PruneMove2<white>(pPMovement, pinHV, pinsHV, n);
      Pawn_PruneLeft<white>(lPMovement, pinD12, pinsD12, n);
      Pawn_PruneRight<white>(rPMovement, pinD12, pinsD12, n);

      // Enpassant
      if (epTarget) {
        //The eppawn must be an enemy since its only ever valid for a single move
        const U64 validEPTarget = epTarget & checkMask;
        U64 epL = pMovementLR & Pawns_NotLeft() & (validEPTarget >> 1); //Pawn that can EPTake to the left
        U64 epR = pMovementLR & Pawns_NotRight() & (validEPTarget << 1);  //Pawn that can EPTake to the right
                
        //Todo: bench if slower or faster
        if (epL | epR) {
          Pawn_PruneLeftEP<white>(epL, pinD12, pinsD12, n);
          Pawn_PruneRightEP<white>(epR, pinD12, pinsD12, n);

          if (epL & pawns) Callback_Move::template PawnEnpassantTake<white>(brd, epL, epL << 1, Pawn_AttackLeft<white>(epL), moves);
          if (epL & brawns) Callback_Move::template BrawnEnpassantTake<white>(brd, epL, epL << 1, Pawn_AttackLeft<white>(epL), moves);
          if (epR & pawns) Callback_Move::template PawnEnpassantTake<white>(brd, epR, epR >> 1, Pawn_AttackRight<white>(epR), moves);
          if (epR & brawns) Callback_Move::template BrawnEnpassantTake<white>(brd, epR, epR >> 1, Pawn_AttackRight<white>(epR), moves);
        }
      }

      //We have pawns that can move on last rank
      if ((fPMovement | pPMovement | lPMovement | rPMovement) & Pawns_LastRank<white>()) {
        const U64 pLastRank = pawns & Pawns_LastRank<white>(), bLastRank = brawns & Pawns_LastRank<white>();
        const U64 pNotLastRank = pawns & ~Pawns_LastRank<white>(), bNotLastRank = brawns & ~Pawns_LastRank<white>();

        U64 proFPawns = fPMovement & pLastRank, proFBrawns = fPMovement & bLastRank; 
        U64 proPPawns = pPMovement & pLastRank, proPBrawns = pPMovement & bLastRank; 
        U64 proLPawns = lPMovement & pLastRank, proLBrawns = lPMovement & bLastRank;  
        U64 proRPawns = rPMovement & pLastRank, proRBrawns = rPMovement & bLastRank; 
        
        U64 noProFPawns = fPMovement & pNotLastRank, noProFBrawns = fPMovement & bNotLastRank;
        U64 noProPPawns = pPMovement & pNotLastRank, noProPBrawns = pPMovement & bNotLastRank;
        U64 noProLPawns = lPMovement & pNotLastRank, noProLBrawns = lPMovement & bNotLastRank; 
        U64 noProRPawns = rPMovement & pNotLastRank, noProRBrawns = rPMovement & bNotLastRank;

        while (proFPawns) {const U64 pos = PopBit(proFPawns); Callback_Move::template Pawnpromote<white>(brd, pos, Pawn_Forward<white>(pos), moves);}
        while (proFBrawns) {const U64 pos = PopBit(proFBrawns); Callback_Move::template Brawnpromote<white>(brd, pos, Pawn_Forward<white>(pos), moves);}
        while (proPPawns) {const U64 pos = PopBit(proPPawns); Callback_Move::template Pawnpromote<white>(brd, pos, Pawn_Forward2<white>(pos), moves);}
        while (proPBrawns) {const U64 pos = PopBit(proPBrawns); Callback_Move::template Brawnpromote<white>(brd, pos, Pawn_Forward2<white>(pos), moves);}
        while (proLPawns) {const U64 pos = PopBit(proLPawns); Callback_Move::template Pawnpromote<white>(brd, pos, Pawn_AttackLeft<white>(pos), moves);}
        while (proLBrawns) {const U64 pos = PopBit(proLBrawns); Callback_Move::template Brawnpromote<white>(brd, pos, Pawn_AttackLeft<white>(pos), moves);}
        while (proRPawns) {const U64 pos = PopBit(proRPawns); Callback_Move::template Pawnpromote<white>(brd, pos, Pawn_AttackRight<white>(pos), moves);}
        while (proRBrawns) {const U64 pos = PopBit(proRBrawns); Callback_Move::template Brawnpromote<white>(brd, pos, Pawn_AttackRight<white>(pos), moves);}

        while (noProFPawns) {const U64 pos = PopBit(noProFPawns); Callback_Move::template Pawnpromote<white>(brd, pos, Pawn_Forward<white>(pos), moves);}
        while (noProFBrawns) {const U64 pos = PopBit(noProFBrawns); Callback_Move::template Brawnpromote<white>(brd, pos, Pawn_Forward<white>(pos), moves);}
        while (noProPPawns) {const U64 pos = PopBit(noProPPawns); Callback_Move::template Pawnpromote<white>(brd, pos, Pawn_Forward2<white>(pos), moves);}
        while (noProPBrawns) {const U64 pos = PopBit(noProPBrawns); Callback_Move::template Brawnpromote<white>(brd, pos, Pawn_Forward2<white>(pos), moves);}
        while (noProLPawns) {const U64 pos = PopBit(noProLPawns); Callback_Move::template Pawnpromote<white>(brd, pos, Pawn_AttackLeft<white>(pos), moves);}
        while (noProLBrawns) {const U64 pos = PopBit(noProLBrawns); Callback_Move::template Brawnpromote<white>(brd, pos, Pawn_AttackLeft<white>(pos), moves);}
        while (noProRPawns) {const U64 pos = PopBit(noProRPawns); Callback_Move::template Pawnpromote<white>(brd, pos, Pawn_AttackRight<white>(pos), moves);}
        while (noProRBrawns) {const U64 pos = PopBit(noProRBrawns); Callback_Move::template Brawnpromote<white>(brd, pos, Pawn_AttackRight<white>(pos), moves);}
      } else {
        U64 fPawns = fPMovement & pawns, fBrawns = fPMovement & brawns;
        U64 pPawns = pPMovement & pawns, pBrawns = pPMovement & brawns;
        U64 lPawns = lPMovement & pawns, lBrawns = lPMovement & brawns; 
        U64 rPawns = rPMovement & pawns, rBrawns = rPMovement & brawns;
        

        while (fPawns) {const U64 pos = PopBit(fPawns); Callback_Move::template Pawnmove<white>(brd, pos, Pawn_Forward<white>(pos), moves);}
        while (fBrawns) {const U64 pos = PopBit(fBrawns); Callback_Move::template Brawnmove<white>(brd, pos, Pawn_Forward<white>(pos), moves);}
        while (pPawns) {const U64 pos = PopBit(pPawns); Callback_Move::template Pawnpush<white>(brd, pos, Pawn_Forward2<white>(pos), moves);}
        while (pBrawns) {const U64 pos = PopBit(pBrawns); Callback_Move::template Brawnpush<white>(brd, pos, Pawn_Forward2<white>(pos), moves);}
        while (lPawns) {const U64 pos = PopBit(lPawns); Callback_Move::template Pawnatk<white>(brd, pos, Pawn_AttackLeft<white>(pos), moves);}
        while (lBrawns) {const U64 pos = PopBit(lBrawns); Callback_Move::template Brawnatk<white>(brd, pos, Pawn_AttackLeft<white>(pos), moves);}
        while (rPawns) {const U64 pos = PopBit(rPawns); Callback_Move::template Pawnatk<white>(brd, pos, Pawn_AttackRight<white>(pos), moves);}
        while (rPawns) {const U64 pos = PopBit(rBrawns); Callback_Move::template Brawnatk<white>(brd, pos, Pawn_AttackRight<white>(pos), moves);}
      }
    }

    // Knight moves
    {
      U64 knights = Knights<white>(brd) & ~(pinHV | pinD12);
      Bitloop(knights) {
        const U64 sq = SquareOf(knights);
        U64 move = Lookup::Knight(sq) & movableSquare;

        while (move) { 
          const U64 to = PopBit(move); 
          Callback_Move::template Knightmove<white>(brd, 1ull << sq, to, moves); 
        }
      }
    }

    // Bishop moves
    {
      const U64 bishops = Bishops<white>(brd) & ~pinHV & ~doublePin;

      U64 pinBishops = bishops & pinD12, nopinBishops = bishops ^ pinBishops;
      for (int i = 0; i < n; i++) {
        U64 pBishops = pinBishops & pinsD12[i];
        Bitloop(pBishops) {
          const U64 sq = SquareOf(pBishops);
          U64 move = Lookup::Bishop(sq, brd.Occ) & movableSquare & pinsD12[i];
          while (move) { 
            const U64 to = PopBit(move); 
            Callback_Move::template Bishopmove<white>(brd, 1ull << sq, to, moves); 
          } 
        }
      }
      
      Bitloop(nopinBishops) {
        const U64 sq = SquareOf(nopinBishops);
        U64 move = Lookup::Bishop(sq, brd.Occ) & movableSquare;

        while (move) { 
          const U64 to = PopBit(move); 
          Callback_Move::template Bishopmove<white>(brd, 1ull << sq, to, moves); 
        }
      }
    }

    // Rook moves
    {
      const U64 rooks = Rooks<white>(brd) & ~pinD12 & ~doublePin;

      U64 pinRooks = rooks & pinHV, nopinRooks = rooks ^ pinRooks;
      for (int i = 0; i < n; i++) {
        U64 pRooks = pinRooks & pinsHV[i];
        Bitloop(pRooks) {
          const U64 sq = SquareOf(pRooks);
          U64 move = Lookup::Rook(sq, brd.Occ) & movableSquare & pinsHV[i];

          while (move) { 
            const U64 to = PopBit(move); 
            Callback_Move::template Rookmove<white>(brd, 1ull << sq, to, moves); 
          } 
        }
      }
      
      Bitloop(nopinRooks) {
        const U64 sq = SquareOf(nopinRooks);
        U64 move = Lookup::Rook(sq, brd.Occ) & movableSquare;

        while (move) { 
          const U64 to = PopBit(move); 
          Callback_Move::template Rookmove<white>(brd, 1ull << sq, to, moves); 
        } 
      }
    }

    // Princess moves
    {
      const U64 princesses = Princesses<white>(brd) & ~doublePin;
      U64 pinD12Princesses = princesses & pinD12;
      U64 pinHVPrincesses = princesses & pinHV;
      U64 nopinPrincesses = princesses ^ (pinD12Princesses | pinHVPrincesses);
      for (int i = 0; i < n; i++) {
        U64 D12Princesses = pinD12Princesses & pinsD12[i];
        Bitloop(D12Princesses) {
          const U64 sq = SquareOf(D12Princesses);
          U64 move = Lookup::Bishop(sq, brd.Occ) & movableSquare & pinsD12[i];

          while (move) { 
            const U64 to = PopBit(move); 
            Callback_Move::template Princessmove<white>(brd, 1ull << sq, to, moves); 
          }
        }
        U64 HVPrincesses = pinHVPrincesses & pinsHV[i];
        Bitloop(HVPrincesses) {
          const U64 sq = SquareOf(HVPrincesses);
          U64 move = Lookup::Rook(sq, brd.Occ) & movableSquare & pinsHV[i];

          while (move) { 
            const U64 to = PopBit(move); 
            Callback_Move::template Princessmove<white>(brd, 1ull << sq, to, moves); 
          }
        }
      }
      Bitloop(nopinPrincesses) {
        const U64 sq = SquareOf(nopinPrincesses);
        U64 move = Lookup::Queen(sq, brd.Occ) & movableSquare;

        while (move) { 
          const U64 to = PopBit(move); 
          Callback_Move::template Princessmove<white>(brd, 1ull << sq, to, moves); 
        }
      }
    }

    // Queen moves
    {
      const U64 queens = Queens<white>(brd) & ~doublePin;
      U64 pinD12Queens = queens & pinD12;
      U64 pinHVQueens = queens & pinHV;
      U64 nopinQueens = queens ^ (pinD12Queens | pinHVQueens);
      for (int i = 0; i < n; i++) {
        U64 D12Queens = pinD12Queens & pinsD12[i];
        Bitloop(D12Queens) {
          const U64 sq = SquareOf(D12Queens);
          U64 move = Lookup::Bishop(sq, brd.Occ) & movableSquare & pinsD12[i];

          while (move) { 
            const U64 to = PopBit(move); 
            Callback_Move::template Queenmove<white>(brd, 1ull << sq, to, moves); 
          }
        }
        U64 HVQueens = pinHVQueens & pinsHV[i];
        Bitloop(HVQueens) {
          const U64 sq = SquareOf(HVQueens);
          U64 move = Lookup::Rook(sq, brd.Occ) & movableSquare & pinsHV[i];

          while (move) { 
            const U64 to = PopBit(move); 
            Callback_Move::template Queenmove<white>(brd, 1ull << sq, to, moves); 
          }
        }
      }
      Bitloop(nopinQueens) {
        const U64 sq = SquareOf(nopinQueens);
        U64 move = Lookup::Queen(sq, brd.Occ) & movableSquare;

        while (move) { 
          const U64 to = PopBit(move); 
          Callback_Move::template Queenmove<white>(brd, 1ull << sq, to, moves); 
        }
      }
    }
    

    // Common King moves
    {
      const U64 commonKings = CommonKings<white>(brd) & ~doublePin;
      U64 pinD12CKings = commonKings & pinD12;
      U64 pinHVCKings = commonKings & pinHV;
      U64 nopinCKings = commonKings ^ (pinD12CKings | pinHVCKings);
      for (int i = 0; i < n; i++) {
        U64 D12CKIngs = pinD12CKings & pinsD12[i];
        Bitloop(D12CKIngs) {
          const U64 sq = SquareOf(D12CKIngs);
          U64 move = Lookup::King(sq) & movableSquare & pinsD12[i];

          while (move) { 
            const U64 to = PopBit(move); 
            Callback_Move::template CommonKingmove<white>(brd, 1ull << sq, to, moves); 
          }
        }
        U64 HVCKings = pinHVCKings & pinsHV[i];
        Bitloop(HVCKings) {
          const U64 sq = SquareOf(HVCKings);
          U64 move = Lookup::King(sq) & movableSquare & pinsHV[i];

          while (move) { 
            const U64 to = PopBit(move); 
            Callback_Move::template CommonKingmove<white>(brd, 1ull << sq, to, moves); 
          }
        }
      }
      Bitloop(nopinCKings) {
        const U64 sq = SquareOf(nopinCKings);
        U64 move = Lookup::King(sq) & movableSquare;

        while (move) { 
          const U64 to = PopBit(move); 
          Callback_Move::template CommonKingmove<white>(brd, 1ull << sq, to, moves); 
        }
      }
    }
  }

  template<bool IsWhite, class Callback_Move>
  _Compiletime void EnumerateMoves(Turn &turn, std::vector<Move> &moves) {
    constexpr bool white = IsWhite; 

    U64 pastMask = BoardMask::PastCheckMask<white>(*turn.boardMask, *turn.board);
    if ((pastMask & (pastMask - 1)) != 0) {return;}
    pastMask |= -(pastMask == 0);

    U64 pinD12 = 0, pinHV = 0, doublePin = 0;
    U64 checkMasks[65], pinsD12[64], pinsHV[64];
    checkMasks[0] = 0xffffffffffffffffull;
    Refresh<white>(turn, pinD12, pinHV, doublePin, checkMasks, pinsD12, pinsHV);

    const U64 checkMask = turn.checkMask & pastMask;
    if (checkMask != 0) {
      _enumerate<white, Callback_Move>(turn, pinD12, pinHV, doublePin, checkMask, pastMask, checkMasks, pinsD12, pinsHV, moves);
    } else {
      const Board brd = *turn.board;
      int n = 0;
      const U64 movable = EnemyOrEmpty<white>(brd) & ~turn.banMask & pastMask;
      // Royal queen moves
      {
        U64 royalQueens = RoyalQueens<white>(brd);
        Bitloop (royalQueens) {
          const U64 sq = SquareOf(royalQueens);
          U64 move = Lookup::Queen(sq, brd.Occ) & movable & checkMasks[n];
          while (move) {
            Callback_Move::template RoyalQueenmove<white>(brd, 1ull << sq, PopBit(move), moves);
          }
          n++;
        }
      }
      
      // King moves
      {
        U64 kings = Kings<white>(brd);
        Bitloop (kings) {
          const U64 sq = SquareOf(kings);
          U64 move = Lookup::King(sq) & movable & checkMasks[n];
          while (move) {
            Callback_Move::template Kingmove<white>(brd, 1ull << sq, PopBit(move), moves);
          }
          n++;
        }
      }
    }
  }
};

class MoveReciever {
  public:

  #define ENABLEPRINT 0
  #define IFPRN if constexpr (ENABLEPRINT) 

  template<bool IsWhite>
  _Compiletime void Pawnmove(const Board& brd, const U64 from, const U64 to, std::vector<Move> &moves) {
    const Board next = Board::Move<Piece::Pawn, IsWhite, false>(brd, from, to);
    IFPRN std::cout << "Pawnmove:\n" << printBoard(next); 

    moves.push_back(Move(next, 0, 0, 0, 0, 0, 0)); 
  }

  template<bool IsWhite>
  _Compiletime void Pawnpush(const Board& brd, const U64 from, const U64 to, std::vector<Move> &moves) {
    const Board next = Board::Move<Piece::Pawn, IsWhite, false>(brd, from, to);
    IFPRN std::cout << "Pawnpush:\n" << printBoard(next); 

    moves.push_back(Move(next, to, 0, 0, 0, 0, 0));  
  }

  template<bool IsWhite>
  _Compiletime void Pawnatk(const Board& brd, const U64 from, const U64 to, std::vector<Move> &moves) {
    const Board next = Board::Move<Piece::Pawn, IsWhite, true>(brd, from, to);
    IFPRN std::cout << "Pawntake:\n" << printBoard(next); 

    moves.push_back(Move(next, 0, 0, 0, 0, 0, 0));
  }

  template<bool IsWhite>
  _Compiletime void PawnEnpassantTake(const Board& brd, const U64 from, const U64 enemy, const U64 to, std::vector<Move> &moves) {
    const Board next = Board::MoveEP<IsWhite, true>(brd, from, enemy, to);
    IFPRN std::cout << "PawnEnpassantTake:\n" << printBoard(next); 

    moves.push_back(Move(next, 0, 0, 0, 0, 0, 0));
  }

  template<bool IsWhite>
  _Compiletime void Pawnpromote(const Board& brd, const U64 from, const U64 to, std::vector<Move> &moves) {
    const Board next1 = Board::MovePromote<Piece::Queen, IsWhite, true>(brd, from, to);
    const Board next2 = Board::MovePromote<Piece::Knight, IsWhite, true>(brd, from, to);
    const Board next3 = Board::MovePromote<Piece::Bishop, IsWhite, true>(brd, from, to);
    const Board next4 = Board::MovePromote<Piece::Rook, IsWhite, true>(brd, from, to);
    IFPRN std::cout << "Pawnpromote:\n" << printBoard(next1) << printBoard(next2) << printBoard(next3) << printBoard(next4); 

    moves.push_back(Move(next1, 0, 0, 0, 0, 0, 0));
    moves.push_back(Move(next2, 0, 0, 0, 0, 0, 0));
    moves.push_back(Move(next3, 0, 0, 0, 0, 0, 0));
    moves.push_back(Move(next4, 0, 0, 0, 0, 0, 0));
  }

  template<bool IsWhite>
  _Compiletime void Brawnmove(const Board& brd, const U64 from, const U64 to, std::vector<Move> &moves) {
    const Board next = Board::Move<Piece::Pawn, IsWhite, false>(brd, from, to);
    IFPRN std::cout << "Brawnmove:\n" << printBoard(next); 

    moves.push_back(Move(next, 0, 0, 0, 0, 0, 0));
  }

  template<bool IsWhite>
  _Compiletime void Brawnpush(const Board& brd, const U64 from, const U64 to, std::vector<Move> &moves) {
    const Board next = Board::Move<Piece::Pawn, IsWhite, false>(brd, from, to);
    IFPRN std::cout << "Brawnpush:\n" << printBoard(next); 

    moves.push_back(Move(next, to, 0, 0, 0, 0, 0));
  }

  template<bool IsWhite>
  _Compiletime void Brawnatk(const Board& brd, const U64 from, const U64 to, std::vector<Move> &moves) {
    const Board next = Board::Move<Piece::Pawn, IsWhite, true>(brd, from, to);
    IFPRN std::cout << "Brawntake:\n" << printBoard(next); 

    moves.push_back(Move(next, 0, 0, 0, 0, 0, 0));  
  }

  template<bool IsWhite>
  _Compiletime void BrawnEnpassantTake(const Board& brd, const U64 from, const U64 enemy, const U64 to, std::vector<Move> &moves) {
    const Board next = Board::MoveEP<IsWhite, true>(brd, from, enemy, to);
    IFPRN std::cout << "BrawnEnpassantTake:\n" << printBoard(next); 

    moves.push_back(Move(next, 0, 0, 0, 0, 0, 0)); 
  }

  template<bool IsWhite>
  _Compiletime void Brawnpromote(const Board& brd, const U64 from, const U64 to, std::vector<Move> &moves) {
    const Board next1 = Board::MovePromote<Piece::Queen, IsWhite, true>(brd, from, to);
    const Board next2 = Board::MovePromote<Piece::Knight, IsWhite, true>(brd, from, to);
    const Board next3 = Board::MovePromote<Piece::Bishop, IsWhite, true>(brd, from, to);
    const Board next4 = Board::MovePromote<Piece::Rook, IsWhite, true>(brd, from, to);
    IFPRN std::cout << "Pawnpromote:\n" << printBoard(next1) << printBoard(next2) << printBoard(next3) << printBoard(next4); 


    moves.push_back(Move(next1, 0, 0, 0, 0, 0, 0));
    moves.push_back(Move(next2, 0, 0, 0, 0, 0, 0));
    moves.push_back(Move(next3, 0, 0, 0, 0, 0, 0));
    moves.push_back(Move(next4, 0, 0, 0, 0, 0, 0));
  }

  template<bool IsWhite>
  _Compiletime void Knightmove(const Board& brd, const U64 from, const U64 to, std::vector<Move> &moves) {
    const Board next = Board::Move <Piece::Knight, IsWhite>(brd, from, to, to & Color<!IsWhite>(brd));
    IFPRN std::cout << "Knightmove:\n" << printBoard(next); 

    moves.push_back(Move(next, 0, 0, 0, 0, 0, 0));
  }

  template<bool IsWhite>
  _Compiletime void Bishopmove(const Board& brd, const U64 from, const U64 to, std::vector<Move> &moves) {
    const Board next = Board::Move<Piece::Bishop, IsWhite>(brd, from, to, to & Color<!IsWhite>(brd));
    IFPRN std::cout << "Bishopmove:\n" << printBoard(next);

    moves.push_back(Move(next, 0, 0, 0, 0, 0, 0));
  }

  template<bool IsWhite>
  _Compiletime void Rookmove(const Board& brd, const U64 from, const U64 to, std::vector<Move> &moves) {
    const Board next = Board::Move<Piece::Rook, IsWhite>(brd, from, to, to & Color<!IsWhite>(brd));
    IFPRN std::cout << "Rookmove:\n" << printBoard(next); 

    moves.push_back(Move(next, 0, 0, 0, 0, 0, 0));  
  }

  template<bool IsWhite>
  _Compiletime void Princessmove(const Board& brd, const U64 from, const U64 to, std::vector<Move> &moves) {
    const Board next = Board::Move<Piece::Princess, IsWhite>(brd, from, to, to & Color<!IsWhite>(brd));
    IFPRN std::cout << "Princessmove:\n" << printBoard(next); 

    moves.push_back(Move(next, 0, 0, 0, 0, 0, 0));
  }

  template<bool IsWhite>
  _Compiletime void Unicornmove(const Board& brd, const U64 from, const U64 to, std::vector<Move> &moves) {
    const Board next = Board::Move<Piece::Unicorn, IsWhite>(brd, from, to, to & Color<!IsWhite>(brd));
    IFPRN std::cout << "Unicornmove:\n" << printBoard(next); 

    moves.push_back(Move(next, 0, 0, 0, 0, 0, 0));  
  }

  template<bool IsWhite>
  _Compiletime void Dragonmove(const Board& brd, const U64 from, const U64 to, std::vector<Move> &moves) {
    const Board next = Board::Move<Piece::Dragon, IsWhite>(brd, from, to, to & Color<!IsWhite>(brd));
    IFPRN std::cout << "Dragonmove:\n" << printBoard(next); 

    moves.push_back(Move(next, 0, 0, 0, 0, 0, 0));
  }

  template<bool IsWhite>
  _Compiletime void Queenmove(const Board& brd, const U64 from, const U64 to, std::vector<Move> &moves) {
    const Board next = Board::Move<Piece::Queen, IsWhite>(brd, from, to, to & Color<!IsWhite>(brd));
    IFPRN std::cout << "Queenmove:\n" << printBoard(next); 

    moves.push_back(Move(next, 0, 0, 0, 0, 0, 0));
  }

  template<bool IsWhite>
  _Compiletime void RoyalQueenmove(const Board& brd, const U64 from, const U64 to, std::vector<Move> &moves) {
    const Board next = Board::Move<Piece::RQueen, IsWhite>(brd, from, to, to & Color<!IsWhite>(brd));
    IFPRN std::cout << "RoyalQueenmove:\n" << printBoard(next); 

    moves.push_back(Move(next, 0, 0, 0, 0, 0, 0));
  }

  template<bool IsWhite>
  _Compiletime void CommonKingmove(const Board& brd, const U64 from, const U64 to, std::vector<Move> &moves) {
    const Board next = Board::Move<Piece::CKing, IsWhite>(brd, from, to, to & Color<!IsWhite>(brd));
    IFPRN std::cout << "CommonKingmove:\n" << printBoard(next); 

    moves.push_back(Move(next, 0, 0, 0, 0, 0, 0)); 
  }

  template<bool IsWhite>
  _Compiletime void Kingmove(const Board& brd, const U64 from, const U64 to, std::vector<Move> &moves) {
    const Board next = Board::Move<Piece::King, IsWhite>(brd, from, to, to & Color<!IsWhite>(brd));
    IFPRN std::cout << "Kingmove:\n" << printBoard(next); 

    moves.push_back(Move(next, 0, 0, 0, 0, 0, 0)); 
  }

  template<bool IsWhite>
  _Compiletime void KingCastle(const Board& brd, const U64 kingswitch, const U64 rookswitch, std::vector<Move> &moves) {
    const Board next = Board::MoveCastle<IsWhite>(brd, kingswitch, rookswitch);
    IFPRN std::cout << "KingCastle:\n" << printBoard(next);

    moves.push_back(Move(next, 0, 0, 0, 0, 0, 0)); 
  }
};