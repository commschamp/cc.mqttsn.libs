
#include "GwDiscover.h"

#include <boost/asio.hpp>

#include <csignal>
#include <iostream>
#include <stdexcept>

int main(int argc, const char* argv[])
{
    int result = 0U;
    try {
        boost::asio::io_context io;

        boost::asio::signal_set signals(io, SIGINT, SIGTERM);
        signals.async_wait(
            [&io, &result](const boost::system::error_code& ec, int sigNum)
            {
                if (ec == boost::asio::error::operation_aborted) {
                    return;
                }

                if (ec) {
                    std::cerr << "ERROR: Unexpected error in signal handling: " << ec.message() << std::endl;
                    result = 150;
                    io.stop();
                    return;
                }

                std::cerr << "Terminated with signal " << sigNum << std::endl;
                result = 100;
                io.stop();
            });

        cc_mqttsn_client_app::GwDiscover app(io, result);

        if (!app.start(argc, argv)) {
            return -1;
        }

        io.run();
    }
    catch (const std::exception& ec)
    {
        std::cerr << "ERROR: Unexpected exception: " << ec.what() << std::endl;
        result = 200;
    }

    return result;
}