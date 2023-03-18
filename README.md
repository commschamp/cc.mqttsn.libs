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
of MQTT-SN client and gateway, which are
completely generic and allow usage of MQTT-SN protocol over any I/O link. See 
details below.

# Client Library
This repository provides the MQTT-SN **client** library. It is implemented 
using C++(11) programming language, but provides **C** interface. The library's
code doesn't use [RTTI](https://en.wikipedia.org/wiki/Run-time_type_information)
or exceptions, but by default
the library's implementation uses C++ STL data types, such as 
[std::string](http://en.cppreference.com/w/cpp/string/basic_string) and 
[std::vector](http://en.cppreference.com/w/cpp/container/vector). However,
it is possible to compile the library not to use any dynamic memory allocation,
and make it suitable for bare-metal environment without any heap. Please
refer to [doc/custom_client_build.md](doc/custom_client_build.md) for instructions on
how to do it. 

The doxygen generated documentation of the library with its full tutorial can
be downloaded from the [release artefacts](releases).

# Client Applications
The [Client Library](#client-library) described above is I/O link agnostic,
it allows to do any additional packatisation of the sent data to allow
correct delivery of the messages. This repository also provides a couple of
MQTT-SN **client** "publish"(**cc_mqttsn_pub_udp**) and "subscribe" 
(**cc_mqttsn_sub_udp**) example applications, which use
UDP/IP as its datagram transport layer. These applications are also using
[Qt5](https://doc.qt.io/qt-5/) framework for their operation. It means that if
proper QT5 libraries are not installed or can not be found, these applications won't
be compiled.

# Gateway Library
Just like the [Client Library](#client-library) allows additional data
packatisation for correct delivery, the gateway application must implement
the same functionality. As the result there is a need for a gateway library,
which provides the required core functionality and allows implementation of any 
gateway application, suitable for the type
of I/O link to the client(s) being used.

This repository provides such a library. It is implemented using C++(11)
programming language and provides both **C++** and **C** interfaces to use.
The gateway uses [v3.1.1](http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.pdf)
of the MQTT protocol to connect to and communicate with the broker.

The doxygen generated documentation of the library with its full tutorial can
be downloaded from the [release artefacts](releases).

# Gateway Applications
The [Gateway Library](#gateway-library) described above is I/O link agnostic,
it allows to do any additional packatisation of the sent data to allow
correct delivery of the messages. This repository also provides the 
MQTT-SN gateway(**cc_mqttsn_gateway_udp**) example application, which uses
UDP/IP as its datagram transport layer. The application also using
[Qt5](https://doc.qt.io/qt-5/) framework for its operation. It means that if
proper QT5 libraries are not installed or can not be found, the gateway example
application won't be compiled.

# Spec Deviations
There are a couple of deviations from official MQTT-SN 
[v1.2](https://www.oasis-open.org/committees/download.php/66091/MQTT-SN_spec_v1.2.pdf)
spec.

- The [client](#client-library) does NOT send **GWINFO** messages as a response
to **SEARCHGW** message from other clients, on behalf of the gateway.
- The [gateway](#gateway-library) sends **GWINFO** as a response to the 
**SEARCHGW** message directly to the requesting client, not broadcasting it
like the protocol specifies.

All other behaviour is implemented as specified.

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

