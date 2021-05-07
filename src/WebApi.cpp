#include "WebApi.h"

#include <algorithm>

bool CommandRequestHandler::canHandle(HTTPMethod method, String uri)
{
    _log.debug("canHandle: method=%d, uri=%s", method, uri.c_str());

    if (method == HTTP_GET && uri.startsWith("/api")) {
        return true;
    }

    return false;
}

bool CommandRequestHandler::handle(
    ESP8266WebServer::WebServerType& server,
    HTTPMethod requestMethod,
    String requestUri
) {
    _log.debug("handle: requestMethod=%d, requestUri=%s", requestMethod, requestUri.c_str());

    constexpr char CommandPath[] = "/api/command/";

    auto result = HandlerResult::NotHandled;

    if (requestMethod == HTTP_GET && requestUri.startsWith(CommandPath)) {
        result = _commandHandler(requestUri.substring(sizeof(CommandPath) - 1));
    }

    switch (result) {
        case HandlerResult::NotHandled:
            server.send(400);
            break;

        case HandlerResult::Handled:
            server.send(200);
            break;

        case HandlerResult::InternalError:
            server.send(500);
            break;
    }

    return true;
}

void CommandRequestHandler::setCommandHandler(CommandHandler&& handler)
{
    _commandHandler = std::move(handler);
}

WebApi::WebApi()
{
    _log.info("setting up command handling");

    _commandRequestHandler.setCommandHandler([this](const String& commandPath) {
        return onIncomingCommandRequest(commandPath);
    });

    _webServer.addHandler(&_commandRequestHandler);

    _log.info("starting the web server");

    _webServer.begin();
}

void WebApi::task()
{
    _webServer.handleClient();
}

void WebApi::setCommandHandler(CommandHandler&& handler)
{
    _commandHandler = std::move(handler);
}

CommandRequestHandler::HandlerResult WebApi::onIncomingCommandRequest(const String& commandPath)
{
    if (commandPath.isEmpty()) {
        return CommandRequestHandler::HandlerResult::NotHandled;
    }

    _log.debug("incoming command request: commandPath=%s", commandPath.c_str());

    constexpr char UpPath[] = "up/";
    constexpr char DownPath[] = "down/";

    if (commandPath.startsWith(UpPath)) {
        return handleCommand(Command::ShutterUp, commandPath.substring(sizeof(UpPath) - 1));
    }
    
    if (commandPath.startsWith(DownPath)) {
        return handleCommand(Command::ShutterDown, commandPath.substring(sizeof(DownPath) - 1));
    }

    return CommandRequestHandler::HandlerResult::NotHandled;
}

CommandRequestHandler::HandlerResult WebApi::handleCommand(const Command command, const String& path)
{
    _log.debug("handleCommand: command=%s, path=%s", toString(command), path.c_str());

    if (!_commandHandler) {
        _log.warning("handleCommand: command handler is null");
        return CommandRequestHandler::HandlerResult::InternalError;
    }

    if (path == "all") {
        _commandHandler(command, std::numeric_limits<uint8_t>::max());
        return CommandRequestHandler::HandlerResult::Handled;
    }

    if (path.length() >= 1 && path.length() <= 2) {
        if (std::none_of(path.begin(), path.end(), [](const char c) { return !isdigit(c); })) {
            const auto deviceIndex = atoi(path.c_str());
            _commandHandler(command, deviceIndex);
            return CommandRequestHandler::HandlerResult::Handled;
        }
    }

    return CommandRequestHandler::HandlerResult::NotHandled;
}

const char* toString(const WebApi::Command command)
{
    switch (command) {
        case WebApi::Command::ShutterUp:
            return "ShutterUp";

        case WebApi::Command::ShutterDown:
            return "ShutterDown";
    }

    return "Unknown";
}