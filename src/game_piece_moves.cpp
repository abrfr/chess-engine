#include <array>

#include "game.h"
#include "movement.h"

namespace abra {

using namespace movement;

inline static bool is_valid_dimension(square i) { return 0 <= i && i < 8; }

inline static auto get_offset_components(square off) {
  auto x = square{36};  // a central square
  auto y = x + off;
  return std::make_pair(get_row(y) - get_row(x), get_col(y) - get_col(x));
}

constexpr auto make_jump_table(const std::vector<square>& offsets) {
  std::array<bitboard, 64> moves{};
  for (square s = 0; s < 64; s++) {
    int r = get_row(s), c = get_col(s);
    for (auto x : offsets) {
      auto [dr, dc] = get_offset_components(x);
      auto i = r + dr, j = c + dc;
      if (!is_valid_dimension(i) || !is_valid_dimension(j)) continue;
      set_bit(moves[s], square{8 * i + j});
    }
  }
  return moves;
}

// pawns

bitboard game::get_pawn_moves(bitboard b, color c) const {
  auto moves = bitboard{0};
  auto pawn_dir = get_pawn_direction(c);
  auto start_row = (c == color::white) ? 6 : 1;
  auto vacant = ~bitboard{board.black | board.white};

  for (square s = 0; s < 64; s++) {
    if (!test_bit(b, s)) continue;
    auto row = get_row(s), col = get_col(s);

    square t = s + pawn_dir;

    // pawn push
    if (test_bit(vacant, t)) {
      set_bit(moves, t);
      if (row == start_row) {
        t += pawn_dir;  // double pawn push
        if (test_bit(vacant, t)) set_bit(moves, t);
      }
    }

    // pawn captures

    // add pawn capture along direction (including en passant)
    auto add_capture = [&](square dir) {
      auto edge = (dir == left ? 0 : 7);
      if (col == edge) return;  // edge of board
      auto sq = s + pawn_dir + dir;
      // if square has piece or is an en passant target
      if (!test_bit(vacant, sq) || sq == en_passant) set_bit(moves, sq);
    };

    add_capture(left);
    add_capture(right);
  }

  return moves;
}

bitboard game::get_pawn_attacks(bitboard b, color c) const {
  const static auto pawn_attacks = make_jump_table({left, right});
  auto moves = bitboard{0};
  if (c == color::white)
    b <<= 8;  // shift all bits up
  else
    b >>= 8;  // shift all bits down
  for (square s = 0; s < 64; s++)
    if (test_bit(b, s)) moves |= pawn_attacks[s];

  return moves;
}

// short range pieces

bitboard game::get_knight_moves(bitboard b) const {
  const static auto knight_moves = make_jump_table(
      {2 * up + left, 2 * up + right, 2 * down + left, 2 * down + right,
       up + 2 * left, down + 2 * left, up + 2 * right, down + 2 * right});
  auto moves = bitboard{0};
  for (square s = 0; s < 64; s++)
    if (test_bit(b, s)) moves |= knight_moves[s];
  return moves;
}

bitboard game::get_king_moves(bitboard b) const {
  const static auto king_moves =
      make_jump_table({up, down, left, right, up + left, up + right,
                       down + left, down + right});
  auto moves = bitboard{0};
  for (square s = 0; s < 64; s++)
    if (test_bit(b, s)) moves |= king_moves[s];
  return moves;
}

bitboard game::get_bishop_moves(bitboard b) const {
  constexpr std::array<std::pair<int, int>, 4> dirs{
      {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}}};
  auto moves = bitboard{0};
  auto occupied = bitboard{board.black | board.white};
  for (square s = 0; s < 64; s++) {
    if (!test_bit(b, s)) continue;
    int r = get_row(s), c = get_col(s);
    for (auto [dr, dc] : dirs) {
      auto i = r + dr, j = c + dc;
      while (is_valid_dimension(i) && is_valid_dimension(j)) {
        set_bit(moves, square{8 * i + j});
        if (test_bit(occupied, square{8 * i + j})) break;
        i += dr;
        j += dc;
      }
    }
  }
  return moves;
}

bitboard game::get_rook_moves(bitboard b) const {
  constexpr std::array<std::pair<int, int>, 4> dirs{
      {{1, 0}, {-1, 0}, {0, 1}, {0, -1}}};
  auto moves = bitboard{0};
  auto occupied = bitboard{board.black | board.white};
  for (square s = 0; s < 64; s++) {
    if (!test_bit(b, s)) continue;
    int r = get_row(s), c = get_col(s);
    for (auto [dr, dc] : dirs) {
      auto i = r + dr, j = c + dc;
      while (is_valid_dimension(i) && is_valid_dimension(j)) {
        auto sq = square{8 * i + j};
        set_bit(moves, sq);
        if (test_bit(occupied, sq)) break;
        i += dr;
        j += dc;
      }
    }
  }
  return moves;
}

bitboard game::get_queen_moves(bitboard b) const {
  return bitboard{get_bishop_moves(b) | get_rook_moves(b)};
}

}  // namespace abra
