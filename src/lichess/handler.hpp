#pragma once

#include "nlohmann/json_fwd.hpp"
#include <chess.hpp>
#include <mutex>

#include <nlohmann/json.hpp>

namespace lichess::handler::board_stream {

template <typename CtxSPtr>
class game_full {
public:
  explicit game_full(CtxSPtr ctx) : ctx_(std::move(ctx)) {}

  void handle(const nlohmann::json& body) {
    const auto white = body.find("white");
    const auto name = white->find("id");
    const auto state = body.find("state");
    const auto moves = state->find("moves");
    const auto& mvstr = moves->get<std::string>();

    std::lock_guard<std::mutex> lock(ctx_->mtx);
    if (!mvstr.empty()) {
      const auto uci = mvstr.substr(mvstr.size() - 4, 4);
      auto mv = chess::uci::uciToMove(*ctx_->board, uci);
      ctx_->board->makeMove(mv);
    }
    auto side =
      (*name == ctx_->player_name) ? chess::Color::WHITE : chess::Color::BLACK;
    ctx_->side = side;
    ctx_->is_my_turn = (side == chess::Color::WHITE);
  }

private:
  CtxSPtr ctx_;
};

template <typename CtxSPtr>
class game_state {
public:
  explicit game_state(CtxSPtr ctx) : ctx_(std::move(ctx)) {}

  void handle(const nlohmann::json& body) {
    const auto moves = body.find("moves");
    const auto mvstr = moves->get<std::string>();

    const auto last_space = mvstr.rfind(' ');
    const auto at = (last_space == mvstr.npos) ? 0 : last_space + 1;
    const auto sz = mvstr.size() - at;
    const auto uci = mvstr.substr(at, sz);
    std::lock_guard<std::mutex> lock(ctx_->mtx);
    auto board = ctx_->board;
    auto mv = chess::uci::uciToMove(*board, uci);
    ctx_->moves.push_back(mv);
    auto log = ctx_->log;
    auto my_last_move = ctx_->my_last_move.load();
    if (mv.from() != my_last_move.from() && mv.to() != my_last_move.to()) {
      board->makeMove(mv);
      ctx_->is_my_turn.store(true);
    }
    log->info("move {} maked", uci);
  }

private:
  CtxSPtr ctx_;
};

template <typename CtxSPtr>
class chat_line {
public:
  explicit chat_line(CtxSPtr ctx) : ctx_(std::move(ctx)) {}

  void handle(const nlohmann::json& body) {
    const auto& player = body["username"].get<std::string>();
    const auto& text = body["text"].get<std::string>();
    ctx_->log->info("[chat] {}> {}", player, text);
  }

private:
  CtxSPtr ctx_;
};

} // namespace lichess::handler::board_stream
