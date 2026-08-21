# Copyright (c) 2023-present The Telestai Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or https://opensource.org/license/mit/.

function(generate_setup_nsi)
  set(abs_top_srcdir ${PROJECT_SOURCE_DIR})
  set(abs_top_builddir ${PROJECT_BINARY_DIR})
  set(CLIENT_URL ${PROJECT_HOMEPAGE_URL})
  set(CLIENT_TARNAME "telestai")
  set(BITCOIN_WRAPPER_NAME "telestai")
  set(BITCOIN_GUI_NAME "telestai-qt")
  set(BITCOIN_DAEMON_NAME "telestaid")
  set(BITCOIN_CLI_NAME "telestai-cli")
  set(BITCOIN_TX_NAME "telestai-tx")
  set(BITCOIN_WALLET_TOOL_NAME "telestai-wallet")
  set(BITCOIN_TEST_NAME "test_telestai")
  set(EXEEXT ${CMAKE_EXECUTABLE_SUFFIX})
  configure_file(${PROJECT_SOURCE_DIR}/share/setup.nsi.in ${PROJECT_BINARY_DIR}/telestai-win64-setup.nsi USE_SOURCE_PERMISSIONS @ONLY)
endfunction()
