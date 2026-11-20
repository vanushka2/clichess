#include <format>
#include <utility>

#include "handler.hpp"
#include "hook.hpp"

namespace lichess {

hook::hook(hook::map_t&& handlers, std::shared_ptr<context> ctx)
  : handlers_(std::move(handlers)), ctx_(std::move(ctx)) {}

void hook::operator()(std::string_view body) {
  if (body.empty())
    return;

  auto json = nlohmann::json::parse(body);
  if (!json.is_object())
    return fail("body is not an JSON object");

  const auto type = json.find("type");
  if (type == json.end())
    return fail("type field is missing");

  const auto name = type->get<std::string>();
  auto handler = handlers_.find(name);
  if (handler == handlers_.end())
    return fail(std::format("handler \"{}\" not found", name));

  handler->second(json);
}

void hook::fail(std::string_view msg) { ctx_->log->error(msg); }

std::shared_ptr<hook::board_stream>
hook::board_stream::spawn(std::shared_ptr<context> ctx) {
  auto stream = std::make_shared<hook::board_stream>(std::move(ctx));
  stream->init_hook();
  return stream;
}

hook::board_stream::board_stream(std::shared_ptr<context> ctx)
  : ctx_(ctx), hook_({}, ctx) {}

void hook::board_stream::init_hook() {
  hook_ = hook({{"gameFull",
                 [self = shared_from_this()](const nlohmann::json& body) {
                   try {
                     handler::board_stream::game_full(self->ctx_).handle(body);
                   } catch (const std::exception& ex) {
                     self->ctx_->error.store(err::code::game_full);
                   }
                 }},
                {"gameState",
                 [self = shared_from_this()](const nlohmann::json& body) {
                   try {
                     handler::board_stream::game_state(self->ctx_).handle(body);
                   } catch (const std::exception& ex) {
                     self->ctx_->error.store(err::code::game_state);
                   }
                 }},
                {"chatLine",
                 [self = shared_from_this()](const nlohmann::json& body) {
                   try {
                     handler::board_stream::chat_line(self->ctx_).handle(body);
                   } catch (const std::exception& ex) {
                     self->ctx_->error.store(err::code::chat_line);
                   }
                 }}},
               ctx_);
}

hook hook::board_stream::get_hook() const { return hook_; }

std::shared_ptr<hook::event_stream>
hook::event_stream::spawn(std::shared_ptr<context> ctx) {
  auto stream = std::make_shared<hook::event_stream>(std::move(ctx));
  stream->init_hook();
  return stream;
}

hook::event_stream::event_stream(std::shared_ptr<context> ctx)
  : ctx_(ctx), hook_({}, std::move(ctx)) {}

void hook::event_stream::init_hook() {
  hook_ = hook(
    {{"gameStart",
      [self = shared_from_this()](const nlohmann::json& body) {
        try {
          handler::event_stream::game_start(self->ctx_).handle(body);
        } catch (const std::exception& ex) {
          self->ctx_->error.store(err::code::game_start);
        }
      }},
     {"gameFinish",
      [self = shared_from_this()](const nlohmann::json& body) {
        try {
          handler::event_stream::game_finish(self->ctx_).handle(body);
        } catch (const std::exception& ex) {
          self->ctx_->error.store(err::code::game_finish);
        }
      }},
     {"challengeDeclined",
      [self = shared_from_this()](const nlohmann::json& body) {
        try {
          handler::event_stream::challenge_declined(self->ctx_).handle(body);
        } catch (const std::exception& ex) {
          self->ctx_->error.store(err::code::challenge_decline);
        }
      }},
     {"challenge",
      [self = shared_from_this()](auto) {
        // TODO: Clear it: self->ctx_->log->info("challenge event catched
        // here.");
      }}},
    ctx_);
}

hook hook::event_stream::get_hook() const { return hook_; }

} // namespace lichess
