INCLUDEPATH += $$PWD

olddecoder {
    QMAKE_CXXFLAGS += -DOLD_DECODER=1
    SOURCES += desc.cpp
    HEADERS += desc.h
} else {
    QMAKE_CXXFLAGS += -DOLD_DECODER=0

    include ( value/value.pri )
    include ( decode/decode.pri )

    SOURCES += $$DECODER_SOURCES
}

SOURCES += \
    atsctext.cpp \
    channels.cpp \
    decode.cpp \
    demux.cpp \
    dvb-vb2.cpp \
    feed.cpp \
    functions.cpp \
    listen.cpp \
    log.cpp \
    output.cpp \
    parse.cpp \
    rbuf.cpp \
    stats.cpp \
    tune.cpp \
    hdhr_tuner.cpp \
    hlsfeed.cpp \
    curlhttpget.cpp

HEADERS += \
    atsctext.h \
    channels.h \
    decode.h \
    demux.h \
    dvb-vb2.h \
    feed.h \
    functions.h \
    listen.h \
    log.h \
    output.h \
    outputbase.h \
    parse.h \
    rbuf.h \
    stats.h \
    tune.h \
    hdhr_tuner.h \
    hlsfeed.h \
    dvbtee_config.h \
    curlhttpget.h

unix:!macx:!symbian {
    HEADERS += linuxtv_tuner.h
    SOURCES += linuxtv_tuner.cpp
}
