#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/ServerSocket.h"
#include "Poco/Util/ServerApplication.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Net/HTTPResponse.h"
#include "Poco/StreamCopier.h"

#include <nlohmann/json.hpp>
#include "udecoder/dummy.h"

using Poco::Net::ServerSocket;
using Poco::Net::HTTPRequestHandler;
using Poco::Net::HTTPRequestHandlerFactory;
using Poco::Net::HTTPServer;
using Poco::Net::HTTPServerRequest;
using Poco::Net::HTTPServerResponse;
using Poco::Net::HTTPServerParams;
using Poco::Util::ServerApplication;
using Poco::Util::Application;
using Poco::Util::Option;
using Poco::Util::OptionSet;
using Poco::Util::OptionCallback;
using json = nlohmann::json;

class ConverterRequestHandler : public HTTPRequestHandler
{
public:
    ConverterRequestHandler(const std::string& path) : _path(path)
    {
    }
    void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response)
    {
        Application& app = Application::instance();
        std::string s;
        Poco::StreamCopier::copyToString(request.stream(), s, request.getContentLength());
        auto j = json::parse(s);
        std::string from, to;
        from = j["from"];
        to = j["to"];
        std::string a = udecoder::convert(from, to, _path);
        response.setChunkedTransferEncoding(true);
        response.setContentType("message/http");
        if (a == "-400")
        {
            response.setStatus(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
            response.setReason("Bad Request");
            response.send();
        }
        else if (a == "-404")
        {
            response.setStatus(Poco::Net::HTTPResponse::HTTP_NOT_FOUND);
            response.setReason("Not Found");
            response.send();
        }
        else
        {
            response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
            response.send()
                << a;
        }
    }
private:
    std::string _path;
};

class ConverterRequestHandlerFactory : public HTTPRequestHandlerFactory
{
public:
    ConverterRequestHandlerFactory(const std::string& path) :_path(path)
    {
    }
    HTTPRequestHandler* createRequestHandler(const HTTPServerRequest&)
    {
        return new ConverterRequestHandler(_path);
    }
private:
    std::string _path;
};

class WebServerApp : public Poco::Util::ServerApplication
{
protected:
    void initialize(Application& self)
    {
        loadConfiguration();
        ServerApplication::initialize(self);
    }

    void defineOptions(OptionSet& options)
    {
        ServerApplication::defineOptions(options);

        options.addOption(
            Option("rules", "r", "path to file.csv")
            .required(true)
            .repeatable(false)
            .argument("directory", true)
            .callback(OptionCallback<WebServerApp>(this, &WebServerApp::handleRules)));
    }

    void handleRules(const std::string& name, const std::string& directory)
    {
        path = directory;
    }

    int main(const std::vector<std::string>& args)
    {
        unsigned short port = static_cast<unsigned short>(config().getInt("WebServerApp.port", 80));
        ServerSocket svs(port);
        HTTPServer srv(new ConverterRequestHandlerFactory(path), svs, new HTTPServerParams);
        srv.start();
        waitForTerminationRequest();
        srv.stop();
        return Application::EXIT_OK;
    }

private:
    std::string path;
};

int main(int argc, char** argv)
{
    WebServerApp app;
    return app.run(argc, argv);
}

