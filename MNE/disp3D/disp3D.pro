#--------------------------------------------------------------------------------------------------------------
#
# @file     disp3D.pro
# @author   Lorenz Esch <Lorenz.Esch@tu-ilmenauu.de>;
#           Matti Hamalainen <msh@nmr.mgh.harvard.edu>
# @version  1.0
# @date     November, 2015
#
# @section  LICENSE
#
# Copyright (C) 2015, Lorenz Esch and Matti Hamalainen. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without modification, are permitted provided that
# the following conditions are met:
#     * Redistributions of source code must retain the above copyright notice, this list of conditions and the
#       following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
#       the following disclaimer in the documentation and/or other materials provided with the distribution.
#     * Neither the name of MNE-CPP authors nor the names of its contributors may be used
#       to endorse or promote products derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
# WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
# PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
# INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
# NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#
#
# @brief    This project file builds the new display 3D library which depends on the qt3d module.
#
#--------------------------------------------------------------------------------------------------------------

include(../../mne-cpp.pri)

TEMPLATE = lib

QT       += widgets 3dcore 3drender 3dinput

DEFINES += DISP3DNEW_LIBRARY

TARGET = Disp3D
TARGET = $$join(TARGET,,MNE$${MNE_LIB_VERSION},)
CONFIG(debug, debug|release) {
    TARGET = $$join(TARGET,,,d)
}

RESOURCES += $$PWD/disp3d.qrc \

LIBS += -L$${MNE_LIBRARY_DIR}
CONFIG(debug, debug|release) {
    LIBS += -lMNE$${MNE_LIB_VERSION}Genericsd \
            -lMNE$${MNE_LIB_VERSION}Fsd \
            -lMNE$${MNE_LIB_VERSION}Fiffd \
            -lMNE$${MNE_LIB_VERSION}Mned \
            -lMNE$${MNE_LIB_VERSION}Inversed \
            -lMNE$${MNE_LIB_VERSION}Dispd
}
else {
    LIBS += -lMNE$${MNE_LIB_VERSION}Generics \
            -lMNE$${MNE_LIB_VERSION}Fs \
            -lMNE$${MNE_LIB_VERSION}Fiff \
            -lMNE$${MNE_LIB_VERSION}Mne \
            -lMNE$${MNE_LIB_VERSION}Inverse \
            -lMNE$${MNE_LIB_VERSION}Disp
}

DESTDIR = $${MNE_LIBRARY_DIR}

contains(MNECPP_CONFIG, build_MNECPP_Static_Lib) {
    CONFIG += staticlib
    DEFINES += BUILD_MNECPP_STATIC_LIB
}
else {
    CONFIG += dll

    #
    # win32: copy dll's to bin dir
    # unix: add lib folder to LD_LIBRARY_PATH
    #
    win32 {
        FILE = $${DESTDIR}/$${TARGET}.dll
        BINDIR = $${DESTDIR}/../bin
        FILE ~= s,/,\\,g
        BINDIR ~= s,/,\\,g
        QMAKE_POST_LINK += $${QMAKE_COPY} $$quote($${FILE}) $$quote($${BINDIR}) $$escape_expand(\\n\\t)
    }
}

SOURCES += \
    view3D.cpp \
    3DObjects/brain/brain.cpp \
    3DObjects/brain/braintreemodel.cpp \
    3DObjects/brain/braintreemetaitem.cpp \
    3DObjects/brain/brainsurfacetreeitem.cpp \
    3DObjects/brain/brainsurfacesettreeitem.cpp \
    3DObjects/brain/brainannotationtreeitem.cpp \
    3DObjects/brain/brainhemispheretreeitem.cpp \
    3DObjects/brain/braintreedelegate.cpp \
    3DObjects/brain/brainrtsourcelocdatatreeitem.cpp \
    3DObjects/brain/brainrtconnectivitydatatreeitem.cpp \
    helpers/abstracttreeitem.cpp \
    helpers/renderable3Dentity.cpp \
    helpers/custommesh.cpp \
    helpers/window.cpp \
    control/control3dwidget.cpp \
    rt/rtSourceLoc/rtsourcelocdataworker.cpp \
    3DObjects/brain/brainsourcespacetreeitem.cpp \
    materials/shadermaterial.cpp

HEADERS += \
    view3D.h \
    3DObjects/brain/brain.h \
    3DObjects/brain/braintreemodel.h \
    3DObjects/brain/braintreemetaitem.h \
    3DObjects/brain/brainsurfacetreeitem.h \
    3DObjects/brain/brainsurfacesettreeitem.h \
    3DObjects/brain/brainannotationtreeitem.h \
    3DObjects/brain/brainhemispheretreeitem.h \
    3DObjects/brain/braintreedelegate.h \
    3DObjects/brain/brainrtsourcelocdatatreeitem.h \
    3DObjects/brain/brainrtconnectivitydatatreeitem.h \
    helpers/abstracttreeitem.h \
    helpers/renderable3Dentity.h \
    helpers/custommesh.h \
    helpers/window.h \
    helpers/types.h \
    control/control3dwidget.h \
    disp3D_global.h \
    rt/rtSourceLoc/rtsourcelocdataworker.h \
    3DObjects/brain/brainsourcespacetreeitem.h \
    materials/shadermaterial.h

FORMS += \
    control/control3dwidget.ui \

INCLUDEPATH += $${EIGEN_INCLUDE_DIR}
INCLUDEPATH += $${MNE_INCLUDE_DIR}

# Install headers to include directory
header_files.files = ./*.h
header_files.path = $${MNE_INCLUDE_DIR}/disp3D

INSTALLS += header_files

unix: QMAKE_CXXFLAGS += -isystem $$EIGEN_INCLUDE_DIR

