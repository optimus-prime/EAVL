#-------------------------------------------------
#
# Project created by QtCreator 2011-03-08T13:20:12
#
#-------------------------------------------------

QT       -= core gui

TARGET = lib/eavl
TEMPLATE = lib
CONFIG += staticlib

# hack in the CUDA files as c plus plus files
QMAKE_EXT_CPP = .cu
QMAKE_CXXFLAGS += -x c++

INCLUDEPATH += src/common src/executor src/exporters src/functors src/filters src/importers src/math src/operations src/rendering

win32 {
  INCLUDEPATH += config-windows
}

SOURCES += \
 src/common/eavlArray.cpp \
 src/common/eavlAtomicProperties.cpp \
 src/common/eavlCUDA.cpp \
 src/common/eavlCellComponents.cpp \
 src/common/eavlCellSetExplicit.cpp \
 src/common/eavlDataSet.cpp \
 src/common/eavlExecutor.cpp \
 src/common/eavlNewIsoTables.cpp \
 src/common/eavlOperation.cpp \
 src/common/eavlTimer.cpp \
 src/exporters/eavlVTKExporter.cpp \
 src/exporters/eavlPNMExporter.cpp \
 src/filters/eavl3X3AverageMutator.cu \
 src/filters/eavlElevateMutator.cpp \
 src/filters/eavlExternalFaceMutator.cpp \
 src/filters/eavlIsosurfaceFilter.cu \
 src/filters/eavlSurfaceNormalMutator.cu \
 src/filters/eavlTesselate2DFilter.cpp \
 src/filters/eavlThresholdMutator.cpp \
 src/importers/eavlBOVImporter.cpp \
 src/importers/eavlImporterFactory.cpp \
 src/importers/eavlMADNESSImporter.cpp \
 src/importers/eavlPDBImporter.cpp \
 src/importers/eavlVTKImporter.cpp \
 src/math/eavlMatrix4x4.cpp \
 src/math/eavlPoint3.cpp \
 src/math/eavlVector3.cpp \
 src/rendering/eavlColor.cpp

HEADERS += $$files(src/common/*.h) $$files(src/exporters/*.h) $$files(src/filters/*.h) $$files(src/importers/*.h) $$files(src/math/*.h) $$files(src/rendering/*.h)
