# Overview
The [MQTT-SN](https://www.oasis-open.org/committees/download.php/66091/MQTT-SN_spec_v1.2.pdf) 
protocol is a sibling of [MQTT](http://mqtt.org). While **MQTT** is designed to be 
used over a **reliable stream** transport protocol (such as TCP/IP), the **MQTT-SN**
was designed to be used over a **datagram** transport protocol. It means, that
if a message is received by the other end on the communication link, it is
received **in full** and **correctly**. 

There are multiple implementations of MQTT-SN client libraries and/or gateway 
applications. Most of them support only UDP/IP and/or ZigBee transport
layers. However, it is possible to use MQTT-SN protocol with any other I/O link, such as
RS-232 serial or CAN bus, as long as additional "packatisation" of the native 
MQTT-SN message is provided, to insure the message is delivered **in full**
and **without errors**. 

This repository provides **single threaded**, **asynchronous**, **non-blocking**, easy to
use, suitable for **embedded** platforms, well documented libraries 
of MQTT-SN **client** and **gateway**, which are
completely generic and allow usage of MQTT-SN protocol over any I/O link. See 
details below.

# Client Library
This repository provides the MQTT-SN **client** library. It is implemented 
using C++(17) programming language, but provides **C** interface. The library's
code doesn't use [RTTI](https://en.wikipedia.org/wiki/Run-time_type_information)
or exceptions. By default
the library's implementation uses C++ STL data types, such as 
[std::string](http://en.cppreference.com/w/cpp/string/basic_string) and 
[std::vector](http://en.cppreference.com/w/cpp/container/vector). However,
it is possible to compile the library not to use any dynamic memory allocation,
and make it suitable for bare-metal environment without any heap. Please
refer to [doc/custom_client_build.md](doc/custom_client_build.md) for instructions on
how to do it. 

The doxygen generated documentation of the library with its full tutorial can
be downloaded from the [release artefacts](releases) or browsed
[online](https://commschamp.github.io/cc_mqttsn_client_doc).

# Client Applications
This repository also provides extra utilities (example applications) which
use the [client library](#client-library) described above.

* **cc_mqttsn_gw_discover** - Discover available gateways client application
* **cc_mqttsn_client_pub** - Publish client application
* **cc_mqttsn_client_sub** - Subscribe client application

These applications use [Boost](https://www.boost.org) libraries,
([boost::program_options](https://www.boost.org/doc/libs/1_83_0/doc/html/program_options.html)
to parse the command line arguments and
[boost::asio](https://www.boost.org/doc/libs/1_83_0/doc/html/boost_asio.html) to run
the events loop and manage network connection(s)). Currently only UDP/IP network
connection type is supported.

# Gateway Library
Just like the [Client Library](#client-library) allows additional data
packatisation for correct delivery, the gateway application must implement
the same functionality. As the result there is a need for a gateway library,
which provides the required core functionality and allows implementation of any 
gateway application, suitable for the type
of I/O link to the client(s) being used.

This repository provides such a library. It is implemented using C++(17)
programming language and provides both **C++** and **C** interfaces to use.
The gateway uses [v3.1.1](http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.pdf)
of the MQTT protocol to connect to and communicate with the broker.

The doxygen generated documentation of the library with its full tutorial can
be downloaded from the [release artefacts](releases) or browsed
[online](https://commschamp.github.io/cc_mqttsn_gateway_doc).

# Gateway Applications
This repository also provides extra applications which
use the [gateway library](#gateway-library) described above.

* **cc_mqttsn_gateway_app** - Transparent gateway application.

These applications use [Boost](https://www.boost.org) libraries,
([boost::program_options](https://www.boost.org/doc/libs/1_83_0/doc/html/program_options.html)
to parse the command line arguments and
[boost::asio](https://www.boost.org/doc/libs/1_83_0/doc/html/boost_asio.html) to run
the events loop and manage network connection(s)). Currently only UDP/IP network
connection type is supported.

# How to Build
Detailed instructions on how to build and install all the components can be
found in [doc/BUILD.md](doc/BUILD.md) file.

# Branching Model
This repository will follow the 
[Successful Git Branching Model](http://nvie.com/posts/a-successful-git-branching-model/).

The **master** branch will always point to the latest release, the
development is performed on **develop** branch. As the result it is safe
to just clone the sources of this repository and use it without
any extra manipulations of looking for the latest stable version among the tags and
checking it out.

# Contact Information
For bug reports, feature requests, or any other question you may open an issue
here in **github** or e-mail me directly to: **arobenko@gmail.com**

