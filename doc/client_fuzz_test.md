# Client Library Fuzz Testing Application
This repository provides a fuzz testing application called **cc_mqttsn_client_afl_fuzz**.
It expects to receive a sequence of the raw bytes from the **STDIN** and intended to
be used with fuzz testing tools like [AFL++](https://github.com/AFLplusplus/AFLplusplus).

The **cc_mqttsn_client_afl_fuzz** application will try to perform the following steps:

1. Initialize the library.
2. Search for the gateway.
3. Perform "connect" operation (with will if needed) and wait for acknowledgement (**CONNACK**).
4. Subscribe to all topics provided via command line arguments.
5. Echo back all the received messages from the broker, and proceed to the next step only after
   the necessary number of messages has been published (configured via command line).
6. Perform "will update" operation.
7. Enter "sleep" state.
8. Check incoming messages several times (configured via command line)
9. Re-connect to the gateway.
10. Unsubscribe from all the previously subscribed topics
11. Gracefully disconnect from the gateway
12. Return to step 1.

Due to the nature of the fuzz testing in most cases the intended workflow from above won't be
completed as intended due to detected unexpected messages, malformed packets, and/or protocol errors.
The client library as well as fuzzing application is expected to handle all the workflows without
crashing.

# How to Build
To better understand the build instructions below please read the
[BUILD.md](BUILD.md) document as well as refer to the
main [CMakeLists.txt](../CMakeLists.txt) file for the info on available configuration options and
variables.

If you're not familiar with the [AFL++](https://github.com/AFLplusplus/AFLplusplus) it
is highly recommended to read through its [instructions](https://github.com/AFLplusplus/AFLplusplus/blob/stable/docs/fuzzing_in_depth.md)
to properly understand what is being done.

To enable the fuzz testing application use **CC_MQTTSN_AFL_FUZZ** cmake option. When
instrumenting the binaries for the fuzz testing other applications are probably not needed
and their build can be disabled using the **CC_MQTTSN_CLIENT_APPS** and **CC_MQTTSN_GATEWAY_APPS** cmake options.

Also remember to properly set instrumenting compilers provided by the [AFL++](https://github.com/AFLplusplus/AFLplusplus).

```
CC=afl-clang-fast CXX=afl-clang-fast++ cmake /path/to/src -DCC_MQTTSN_AFL_FUZZ=ON -DCC_MQTTSN_CLIENT_APPS=OFF -DCC_MQTTSN_GATEWAY_APPS=OFF ...
```

It is also highly recommended to use "Debug" build to enable all the assertions.
```
CC=afl-clang-fast CXX=afl-clang-fast++ cmake ... -DCMAKE_BUILD_TYPE=Debug ...
```

After the "Debug" build is tested for several days without showing any crashes it is also
beneficial to rebuild the fuzzing application using the "Release" build type and re-run it on the same produced corpus
(passing "-i-" to afl-fuzz) to verify that there are no unexpected pitfalls for the "Release" version.

Due to the fact that [AFL++](https://github.com/AFLplusplus/AFLplusplus) compilers
receive their configuration via environment variables before the build, it is necessary
to disable "ccache" usage in case the sources are going to be built multiple times
with different configurations. Otherwise the build might produced wierd errors of
linked object files being incompatible.

```
CC=afl-clang-fast CXX=afl-clang-fast++ cmake ... -DCC_MQTTSN_USE_CCACHE=OFF ...
```

Enable required sanitizers before the build
```
export AFL_USE_ASAN=1
export AFL_USE_UBSAN=1
```

**WARNING**: It has been noticed that when too many sanitizers are enabled at the same time the
target either fails to compile or fails to produce proper output
("Illegal instruction" is reported) when some failure happens. If such failure happen
try to re-compile the fuzzing application without the sanitizers altogether and see
if any unexpected crash is reported on the generated crash causing input.
After that, try to recompile enabling only one sanitizer enabled at a time and feeding the
same problematic input in attempt to produce reasonable failure report from the
sanitizer.

As the final stage, build the fuzzing application
```
cmake --build . --target install
```

Note that in case of [custom](custom_client_build.md) client libraries are built, the
fuzzing application will be built for each such library and the application name will
reflect the custom name selected for the library.

The typical build sequence may look like this:
```
cd /path/to/cc.mqttsn.libs
mkdir build
cd build
CC=afl-clang-fast CXX=afl-clang-fast++ cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=install \
    -DCMAKE_PREFIX_PATH=/path/to/comms/install\;/path/to/cc.mqttsn.generated/install \
    -DCC_MQTTSN_AFL_FUZZ=ON -DCC_MQTTSN_CLIENT_APPS=OFF -DCC_MQTTSN_GATEWAY_APPS=OFF -DCC_MQTTSN_USE_CCACHE=OFF
export AFL_USE_ASAN=1
export AFL_USE_UBSAN=1
cmake --build . --target install --parallel 8
```

# How to Fuzz Test
In order to start fuzz testing the [AFL++](https://github.com/AFLplusplus/AFLplusplus) requires
creation of the input corpus. Please use "-h" option to list the available command line parameters:
```
./install/bin/cc_mqttsn_client_afl_fuzz -h
```
Note the presence of the "-g" option which can be used to generate a valid input sequence
to perform a full single iteration of the **cc_mqttsn_client_afl_fuzz** described
earlier. The actual fuzzing will use it as a valid input and then deviate from there.
If you intend to use some extra command line arguments in the actual
fuzz testing, provide them when generating the input sequence as well.

For example
```
mkdir -p /path/to/fuzz/input
mkdir -p /path/to/fuzz/output
./install/bin/cc_mqttsn_client_afl_fuzz -g /path/to/fuzz/input/1.bin -t "prefix/+" -t 1 -t 2 --min-pub-count 5
```

Now when the input corpus is created it is possible to actually start fuzz testing. Use the
same command line option as ones used for the generation of the input (excluding the "-g" with parameter of course).
```
afl-fuzz -i /path/to/fuzz/input -o /path/to/fuzz/output -a binary -D -- ./install/bin/cc_mqttsn_client_afl_fuzz -t "prefix/+" -t 1 -t 2 --min-pub-count 5
```

Note that "afl-fuzz" may request to change your machine configuration before being able to fuzz test.

Also use help proved by the "afl-fuzz" itself to see all the available fuzzing options.
```
afl-fuzz -h
```

In case fuzz testing reports any crash please open an issue for this project reporting
a build configuration and attaching the input file(s) that caused the crash.
