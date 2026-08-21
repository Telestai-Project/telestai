# Copyright (c) 2023-present The Telestai Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or https://opensource.org/license/mit/.

add_library(telestai_zmq STATIC EXCLUDE_FROM_ALL
  zmqabstractnotifier.cpp
  zmqnotificationinterface.cpp
  zmqpublishnotifier.cpp
  zmqrpc.cpp
  zmqutil.cpp
)
target_compile_definitions(telestai_zmq
  INTERFACE
    ENABLE_ZMQ=1
)
target_link_libraries(telestai_zmq
  PRIVATE
    core_interface
    univalue
    zeromq
)
