#include <chrono>
#include <iostream>
#include <algorithm>
#include <random>
#include "engine.hpp"

template<BoardState state, typename Chess>
_Compiletime void analyze(Chess &chess) {
  auto begin = std::chrono::high_resolution_clock::now();
  NegaMax<state>()(chess, 0, 0, 6);
  auto end = std::chrono::high_resolution_clock::now();
  std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(end-begin).count()/1000000000.0 << " s\n";

  std::cout << count << "\n";
}

int main() {
  // To allow unicode characters
  system("chcp 65001 > nul");
  
  Chess<32,128> chess = Chess<32,128>();
  std::string fen1 = "[r*nbqk*bnr*/p*p*p*p*p*p*p*p*/8/8/8/8/P*P*P*P*P*P*P*P*/R*NBQK*BNR*:0:1:w]\n";
  importFEN(chess, fen1);
  std::cout << print(chess);
  _analyze(0b10110000, chess);

  return 0;
}