#pragma once

#include "Logger.h"

#include <functional>

#include <ESP8266WebServer.h>

class CommandRequestHandler : public RequestHandler
{
public:
    bool canHandle(HTTPMethod method, String uri) override;
    bool handle(ESP8266WebServer::WebServerType& server, HTTPMethod requestMethod, String requestUri) override;

    enum class HandlerResult {
        NotHandled,
        Handled,
        InternalError
    };

    using CommandHandler = std::function<HandlerResult (const String& commandPath)>;
    void setCommandHandler(CommandHandler&& handler);

private:
    Logger _log{ "WebAPI:CommandRequestHandler" };
    CommandHandler _commandHandler;
};

class WebApi
{
public:
    WebApi();

    void task();

    enum class Command
    {
        ShutterUp,
        ShutterDown
    };

    using CommandHandler = std::function<void (Command command, uint8_t deviceIndex)>;
    void setCommandHandler(CommandHandler&& handler);

    friend const char* toString(Command command);

private:
    Logger _log{ "WebAPI" };
    ESP8266WebServer _webServer;
    CommandRequestHandler _commandRequestHandler;

    CommandHandler _commandHandler;

    CommandRequestHandler::HandlerResult onIncomingCommandRequest(const String& commandPath);
    CommandRequestHandler::HandlerResult handleCommand(Command command, const String& path);
};