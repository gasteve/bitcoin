bin_PROGRAMS = bitcoin bitcoind bitcoinTest

COMMON_SRC_FILES = \
    CAccount.cpp \
    CAccountingEntry.cpp \
    CAddrDB.cpp \
    CAddress.cpp \
    CAlert.cpp \
    CAutoBN_CTX.cpp \
    CAutoFile.cpp \
    CBigNum.cpp \
    CBlock.cpp \
    CBlockIndex.cpp \
    CBlockLocator.cpp \
    CCriticalBlock.cpp \
    CCriticalSection.cpp \
    CDB.cpp \
    CDataStream.cpp \
    CDiskBlockIndex.cpp \
    CDiskTxPos.cpp \
    CFlatData.cpp \
    CInPoint.cpp \
    CInv.cpp \
    CKey.cpp \
    CKeyPool.cpp \
    CMerkleTx.cpp \
    CMessageHeader.cpp \
    CNode.cpp \
    COutPoint.cpp \
    CRequestTracker.cpp \
    CReserveKey.cpp \
    CScript.cpp \
    CTransaction.cpp \
    CTryCriticalBlock.cpp \
    CTxDB.cpp \
    CTxDiskPos.cpp \
    CTxIn.cpp \
    CTxIndex.cpp \
    CTxOut.cpp \
    CUnsignedAlert.cpp \
    CWalletDB.cpp \
    CWalletKey.cpp \
    CWalletTx.cpp \
    db.cpp \
    init.cpp \
    irc.cpp \
    main.cpp \
    miner.cpp \
    peer.cpp \
    rpc.cpp \
    script.cpp \
    sha256.cpp \
    uint160.cpp \
    uint256.cpp \
    util.cpp \
    wallet.cpp \
    cryptopp/sha.cpp \
    cryptopp/cpu.cpp

bitcoin_SOURCES = $(COMMON_SRC_FILES) \
    gui/CAboutDialog.cpp \
    gui/CAddressBookDialog.cpp \
    gui/CGetTextFromUserDialog.cpp \
    gui/CMainFrame.cpp \
    gui/CMyApp.cpp \
    gui/CMyTaskBarIcon.cpp \
    gui/COptionsDialog.cpp \
    gui/CSendDialog.cpp \
    gui/CSendingDialog.cpp \
    gui/CTxDetailsDialog.cpp \
    gui/ui.cpp \
    gui/uibase.cpp

bitcoind_SOURCES=$(COMMON_SRC_FILES) \
    cli/dmain.cpp

bitcoinTest_SOURCES=$(COMMON_SRC_FILES) test/bitcoinTest.cpp

CXX=llvm-g++
DEPSDIR=../deps

DEFS=-D__WXMAC_OSX__ -DNOPCH -DMSG_NOSIGNAL=0 -DUSE_SSL

#DEFS=$(SHELL $(DEPSDIR)/bin/wx-config --cxxflags) -D__WXMAC_OSX__ -DNOPCH -DMSG_NOSIGNAL=0 -DUSE_SSL
WXDEFS = \
    -I$(DEPSDIR)/lib/wx/include/osx_cocoa-unicode-static-2.9 \
    -I$(DEPSDIR)/include/wx-2.9 \
    -D_FILE_OFFSET_BITS=64 \
    -DwxDEBUG_LEVEL=0 \
    -D__WXMAC__ \
    -D__WXOSX__ \
    -D__WXOSX_COCOA__

DEBUGFLAGS=-g -DwxDEBUG_LEVEL=0
INCLUDEPATHS= \
    -I$(DEPSDIR)/include

LIBPATHS= \
    -L"$(DEPSDIR)/lib"

#The following is based on the output of:
#  $(DEPSDIR)/bin/wx-config --libs --static
WXLIBS = \
    -L$(DEPSDIR)/lib \
    -framework IOKit \
    -framework Carbon \
    -framework Cocoa \
    -framework AudioToolbox \
    -framework System \
    -framework OpenGL \
    -framework QuickTime \
    -framework WebKit \
    $(DEPSDIR)/lib/libwx_osx_cocoau_xrc-2.9.a \
    $(DEPSDIR)/lib/libwx_osx_cocoau_html-2.9.a \
    $(DEPSDIR)/lib/libwx_osx_cocoau_qa-2.9.a \
    $(DEPSDIR)/lib/libwx_osx_cocoau_adv-2.9.a \
    $(DEPSDIR)/lib/libwx_osx_cocoau_core-2.9.a \
    $(DEPSDIR)/lib/libwx_baseu_xml-2.9.a \
    $(DEPSDIR)/lib/libwx_baseu_net-2.9.a \
    $(DEPSDIR)/lib/libwx_baseu-2.9.a \
    -lwxregexu-2.9 \
    -lwxexpat-2.9 \
    -lwxtiff-2.9 \
    -lwxjpeg-2.9 \
    -lwxpng-2.9 \
    -lwxzlib-2.9 \
    -lpthread \
    -liconv

LIBS= -dead_strip \
 $(DEPSDIR)/lib/libdb_cxx-4.7.a \
 $(DEPSDIR)/lib/libboost_system.a \
 $(DEPSDIR)/lib/libboost_filesystem.a \
 $(DEPSDIR)/lib/libboost_program_options.a \
 $(DEPSDIR)/lib/libboost_thread.a \
 $(DEPSDIR)/lib/libssl.a \
 $(DEPSDIR)/lib/libcrypto.a

AM_CPPFLAGS=-mmacosx-version-min=10.5 -arch x86_64 -O3 -Wno-invalid-offsetof -Wformat $(DEBUGFLAGS) $(DEFS) $(INCLUDEPATHS)
bitcoin_CPPFLAGS=$(AM_CPPFLAGS) $(WXDEFS) -DGUI

AM_LDFLAGS=$(LIBPATHS) $(LIBS)
bitcoin_LDFLAGS=$(AM_LDFLAGS) $(WXLIBS)
bitcoinTest_LDFLAGS=$(AM_LDFLAGS) -lboost_unit_test_framework
