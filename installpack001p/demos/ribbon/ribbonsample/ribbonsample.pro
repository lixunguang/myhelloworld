TEMPLATE = app

QTITANDIR = $$quote($$(QTITANDIR))

include($$QTITANDIR/src/shared/qtitanribbon.pri)

DESTDIR = $$QTITANDIR/bin
DESTDIR = $$member(DESTDIR, 0)$$QTITAN_LIB_PREFIX

!debug_and_release|build_pass {
    CONFIG(debug, debug|release) {
        TARGET = $$member(TARGET, 0)d
    }
}

HEADERS       = mainwindow.h
SOURCES       = main.cpp \
                mainwindow.cpp

RESOURCES     = ribbonsample.qrc
FORMS         = ribbonsample.ui

include($$QTITANDIR/demos/shared/aboutdialog.pri)
include($$QTITANDIR/demos/ribbon/shared/ribbonwindow.pri)

macx {
    CONFIG-=app_bundle
}

!lessThan(QT_VER_MAJ, 5) {
  QT += printsupport
}
