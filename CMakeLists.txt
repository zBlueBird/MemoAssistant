cmake_minimum_required(VERSION 3.21)

project(MemoAssistant VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui Widgets Sql Network)

qt_standard_project_setup()

add_library(memo_core STATIC
    src/models/Memo.h
    src/data/DatabaseManager.h
    src/data/DatabaseManager.cpp
    src/data/MemoRepository.h
    src/data/MemoRepository.cpp
)
target_include_directories(memo_core PUBLIC src)
target_link_libraries(memo_core PUBLIC Qt6::Core Qt6::Sql)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(memo_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

qt_add_executable(MemoAssistant WIN32
    resources/resources.qrc
    src/main.cpp
    src/MemoApplication.h
    src/MemoApplication.cpp
    src/services/SettingsService.h
    src/services/SettingsService.cpp
    src/services/SingleInstanceGuard.h
    src/services/SingleInstanceGuard.cpp
    src/services/ReminderService.h
    src/services/ReminderService.cpp
    src/ui/IconFactory.h
    src/ui/IconFactory.cpp
    src/ui/FloatingBall.h
    src/ui/FloatingBall.cpp
    src/ui/MemoPanel.h
    src/ui/MemoPanel.cpp
    src/ui/DeleteConfirmDialog.h
    src/ui/DeleteConfirmDialog.cpp
    src/ui/ReminderDialog.h
    src/ui/ReminderDialog.cpp
    src/ui/MemoEditorDialog.h
    src/ui/MemoEditorDialog.cpp
    src/ui/MemoItemWidget.h
    src/ui/MemoItemWidget.cpp
)
target_include_directories(MemoAssistant PRIVATE src)
target_link_libraries(MemoAssistant PRIVATE
    memo_core
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Sql
    Qt6::Network
)

if(WIN32)
    target_compile_definitions(MemoAssistant PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(MemoAssistant PRIVATE -Wall -Wextra -Wpedantic)
endif()

include(CTest)
if(BUILD_TESTING)
    find_package(Qt6 6.5 REQUIRED COMPONENTS Test)
    qt_add_executable(MemoAssistantTests
        tests/MemoRepositoryTests.cpp
        src/services/SettingsService.h
        src/services/SettingsService.cpp
        src/services/ReminderService.h
        src/services/ReminderService.cpp
        src/services/SingleInstanceGuard.h
        src/services/SingleInstanceGuard.cpp
    )
    target_include_directories(MemoAssistantTests PRIVATE src)
    target_link_libraries(MemoAssistantTests PRIVATE
        memo_core
        Qt6::Test
        Qt6::Sql
        Qt6::Network
        Qt6::Widgets
    )
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(MemoAssistantTests PRIVATE -Wall -Wextra -Wpedantic)
    endif()
    add_test(NAME MemoAssistantTests
        COMMAND ${CMAKE_COMMAND} -E env
            "PATH=$<TARGET_FILE_DIR:Qt6::Core>;$ENV{PATH}"
            "QT_QPA_PLATFORM=offscreen"
            $<TARGET_FILE:MemoAssistantTests>
    )
endif()

install(TARGETS MemoAssistant
    BUNDLE DESTINATION .
    RUNTIME DESTINATION .
)
