cmake_minimum_required(VERSION 3.5)

project(FuzzyDroplets VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Concurrent Svg)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Concurrent Svg)

set(PROJECT_SOURCES
        main.cpp

        core/binormal.h
        core/data.h
        core/data.cpp
        core/design.h
        core/design.cpp
        core/geometry.h
        core/colorscheme.h
        core/mean.h
        core/line_feeder.hpp
        core/quadtree.h
        core/hungarianalgorithm.h
        core/hungarianalgorithm.cpp
        core/approximately.h
        core/color.h
        core/fuzzycolor.h
        core/mapped_file.hpp
        core/trim_string_view.hpp
        core/median.h
        core/vectorqueue.h
        core/sortedvector.h
        core/kernel.h
        core/kmeans.h
        core/kmeans.cpp
        core/emclustering.h
        core/gmm.h
        core/gmm.cpp

        gui/plot/primitive.h
        gui/plot/primitive.cpp
        gui/plot/axis.h
        gui/plot/axis.cpp
        gui/plot/continuousaxis.h
        gui/plot/continuousaxis.cpp
        gui/plot/scatterplotbox2.h
        gui/plot/scatterplotbox2.cpp
        gui/plot/boxgraphbase.h
        gui/plot/boxgraphbase.cpp
        gui/plot/polygon.h
        gui/plot/polygon.cpp
        gui/plot/ringmarker.h
        gui/plot/ringmarker.cpp
        gui/plot/textmarker.h
        gui/plot/textmarker.cpp

        gui/generic/command.h
        gui/generic/command.cpp
        gui/generic/commandstack.h
        gui/generic/commandstack.cpp
        gui/generic/comboboxitemdelegate.h
        gui/generic/comboboxitemdelegate.cpp
        gui/generic/nofocusitemdelegate.h
        gui/generic/nofocusitemdelegate.cpp
        gui/generic/themedicon.h
        gui/generic/themedicon.cpp
        gui/generic/flowlayout.h
        gui/generic/flowlayout.cpp
        gui/generic/shelfwidget.h
        gui/generic/shelfwidget.cpp
        gui/generic/launchwidget.h
        gui/generic/launchwidget.cpp
        gui/generic/stackedwidget.h
        gui/generic/stackedwidget.cpp
        gui/generic/slideraction.h

        gui/icons.qrc
        gui/mainwindow.cpp
        gui/mainwindow.h
        gui/samplelistwidget.h
        gui/samplelistwidget.cpp
        gui/dropletgraphwidget.h
        gui/dropletgraphwidget.cpp
        gui/pointcloud.h
        gui/pointcloud.cpp
        gui/paintingwidget.h
        gui/paintingwidget.cpp
        gui/clusteringwidget.h
        gui/clusteringwidget.cpp
        gui/clustermethodwidget.h
        gui/clustermethodwidget.cpp
        gui/assignmentwidget.h
        gui/assignmentwidget.cpp
        gui/assignmentmethodwidget.h
        gui/assignmentmethodwidget.cpp
        gui/experimentaldesignwidget.h
        gui/experimentaldesignwidget.cpp
        gui/kmeanswidget.h
        gui/kmeanswidget.cpp
        gui/emworker.h
        gui/emworker.cpp
        gui/emclustermethodwidget.h
        gui/emclustermethodwidget.cpp
        gui/gmmwidget.h
        gui/gmmwidget.cpp
        gui/dbscanwidget.h
        gui/dbscanwidget.cpp
        gui/gmmassignmentwidget.h
        gui/gmmassignmentwidget.cpp
        gui/nearestneighbourswidget.h
        gui/nearestneighbourswidget.cpp
        gui/randomforestwidget.h
        gui/randomforestwidget.cpp

        ranger/Data.cpp
        ranger/Data.h
        ranger/DataChar.h
        ranger/DataFloat.h
        ranger/DataDouble.h
        ranger/Forest.cpp
        ranger/Forest.h
        ranger/ForestClassification.cpp
        ranger/ForestClassification.h
        ranger/ForestProbability.cpp
        ranger/ForestProbability.h
        ranger/Tree.cpp
        ranger/Tree.h
        ranger/TreeClassification.cpp
        ranger/TreeClassification.h
        ranger/TreeProbability.cpp
        ranger/TreeProbability.h
        ranger/globals.h
        ranger/utility.cpp
        ranger/utility.h

)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(FuzzyDroplets
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET FuzzyDroplets APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(FuzzyDroplets SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(FuzzyDroplets
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(FuzzyDroplets PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Concurrent Qt${QT_VERSION_MAJOR}::Svg)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.FuzzyDroplets)
endif()
set_target_properties(FuzzyDroplets PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS FuzzyDroplets
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(FuzzyDroplets)
endif()

