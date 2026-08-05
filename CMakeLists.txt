cmake_minimum_required(VERSION 3.20)

project(FLACHEAD VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(FLACHEAD_SIZE_OPT "Reduce binary size (function/data sections, gc-sections, strip)" ON)

if(FLACHEAD_SIZE_OPT)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -ffunction-sections -fdata-sections")
    if(NOT APPLE)
        set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -Wl,--gc-sections -s")
    endif()
endif()

find_package(SDL3 REQUIRED CONFIG)
find_package(SDL3_ttf REQUIRED CONFIG)
find_package(SDL3_image QUIET CONFIG)
find_package(PNG REQUIRED)
find_package(JPEG REQUIRED)
find_package(Threads REQUIRED)

add_executable(FLACHEAD
    src/main.cpp
    src/app/Application.cpp
    src/app/AppManager.cpp
    src/animation/Animator.cpp
    src/animation/AnimationPlayer.cpp
    src/animation/AnimationManager.cpp
    src/animation/Easing.cpp
    src/animation/Tween.cpp
    src/audio/AudioService.cpp
    src/audio/MpvBackend.cpp
    src/audio/NullBackend.cpp
    src/commands/Command.cpp
    src/commands/CommandCenter.cpp
    src/core/Logger.cpp
    src/core/PathUtils.cpp
    src/core/Renderer.cpp
    src/core/Time.cpp
    src/database/Database.cpp
    src/dap/DapScreen.cpp
    src/dap/LibraryScreens.cpp
    src/dap/PlaybackScreens.cpp
    src/dap/PlaylistScreens.cpp
    src/dap/SettingsScreen.cpp
    src/events/EventBus.cpp
    src/filesystem/FileSystem.cpp
    src/focus/FocusManager.cpp
    src/graphics/Font.cpp
    src/graphics/FontManager.cpp
    src/graphics/ImageCodec.cpp
    src/graphics/Sprite.cpp
    src/graphics/Texture.cpp
    src/graphics/TextureCache.cpp
    src/graphics/AssetManager.cpp
    src/input/GpioInputBackend.cpp
    src/input/GestureRecognizer.cpp
    src/input/InputEvent.cpp
    src/input/InputManager.cpp
    src/layout/BoxLayout.cpp
    src/layout/GridLayout.cpp
    src/layout/Layout.cpp
    src/layout/LayoutEngine.cpp
    src/library/AlbumArtExtractor.cpp
    src/library/LibraryScanner.cpp
    src/library/LibraryService.cpp
    src/math/Color.cpp
    src/metadata/FFprobeExtractor.cpp
    src/metadata/Json.cpp
    src/metadata/Metadata.cpp
    src/metadata/TagLibExtractor.cpp
    src/screens/HomeScreen.cpp
    src/screens/ScreenManager.cpp
    src/services/BatteryManager.cpp
    src/services/BackgroundJobManager.cpp
    src/services/MemoryManager.cpp
    src/services/NotificationManager.cpp
    src/services/SettingsManager.cpp
    src/services/StorageManager.cpp
    src/system/Window.cpp
    src/ui/theme/ThemeManager.cpp
    src/ui/palette/Palette.cpp
    src/ui/wallpaper/WallpaperManager.cpp
    src/ui/overlay/OverlayManager.cpp
    src/ui/Button.cpp
    src/ui/Canvas.cpp
    src/ui/Card.cpp
    src/ui/Dialog.cpp
    src/ui/GridView.cpp
    src/ui/IconButton.cpp
    src/ui/Image.cpp
    src/ui/Label.cpp
    src/ui/ListView.cpp
    src/ui/Modal.cpp
    src/ui/NavigationBar.cpp
    src/ui/Overlay.cpp
    src/ui/Panel.cpp
    src/ui/ProgressBar.cpp
    src/ui/ScrollBar.cpp
    src/ui/ScrollView.cpp
    src/ui/Slider.cpp
    src/ui/TextField.cpp
    src/ui/Toast.cpp
    src/ui/Widget.cpp
    src/views/AlbumArtWidget.cpp
    src/views/SongInfoWidget.cpp
    src/views/ProgressWidget.cpp
    src/views/BottomBarWidget.cpp
    src/apps/AppScreens.cpp
    src/resource/ResourceManager.cpp
    src/models/SongModel.cpp
    src/models/AlbumModel.cpp
    src/models/ArtistModel.cpp
    src/models/PlaylistModel.cpp
    src/models/LibraryModel.cpp
    src/models/BatteryStateModel.cpp
    src/models/SettingsModel.cpp
    src/playback/PlaybackController.cpp
    src/playback/PlaylistEngine.cpp
    src/playback/QueueManager.cpp
    src/shell/ShellScreen.cpp
    src/shell/ShellChrome.cpp
    src/shell/LegacyChromeScreen.cpp
    src/shell/AmbientHomeScreen.cpp
    src/shell/LauncherScreen.cpp
    src/shell/TaskOverviewScreen.cpp
    src/shell/UniversalSearchScreen.cpp
    src/shell/SettingsScreen.cpp
    src/shell/ScreenTransitionManager.cpp
    src/shell/SystemScreen.cpp
)
target_link_libraries(FLACHEAD PRIVATE 
    SDL3::SDL3 
    SDL3_ttf::SDL3_ttf
    sqlite3
    PNG::PNG
    JPEG::JPEG
)
target_include_directories(FLACHEAD PRIVATE third_party/mpv/include)

if(TARGET SDL3_image::SDL3_image)
    target_link_libraries(FLACHEAD PRIVATE SDL3_image::SDL3_image)
endif()

option(FLACHEAD_BUILD_TESTS "Build the FLACHEAD test suite" OFF)

if(FLACHEAD_BUILD_TESTS)
    enable_testing()
    set(FLACHEAD_TEST_CORE
        src/database/Database.cpp
        src/core/Logger.cpp
        src/core/PathUtils.cpp
        src/events/EventBus.cpp
        src/library/AlbumArtExtractor.cpp
        src/library/LibraryScanner.cpp
        src/library/LibraryService.cpp
        src/metadata/FFprobeExtractor.cpp
        src/metadata/Json.cpp
        src/metadata/Metadata.cpp
        src/services/SettingsManager.cpp
    )

    add_executable(flachead_test_library tests/test_library.cpp ${FLACHEAD_TEST_CORE})
    target_include_directories(flachead_test_library PRIVATE third_party/mpv/include)
    target_link_libraries(flachead_test_library PRIVATE sqlite3 ${CMAKE_DL_LIBS})
    target_compile_features(flachead_test_library PRIVATE cxx_std_20)
    add_test(NAME flachead_test_library COMMAND flachead_test_library)

    add_executable(flachead_test_settings tests/test_settings.cpp ${FLACHEAD_TEST_CORE})
    target_include_directories(flachead_test_settings PRIVATE third_party/mpv/include)
    target_link_libraries(flachead_test_settings PRIVATE sqlite3)
    target_compile_features(flachead_test_settings PRIVATE cxx_std_20)
    add_test(NAME flachead_test_settings COMMAND flachead_test_settings)

    add_executable(flachead_test_database tests/test_database.cpp ${FLACHEAD_TEST_CORE})
    target_link_libraries(flachead_test_database PRIVATE sqlite3)
    target_compile_features(flachead_test_database PRIVATE cxx_std_20)
    add_test(NAME flachead_test_database COMMAND flachead_test_database)

    add_executable(flachead_test_metadata tests/test_metadata.cpp ${FLACHEAD_TEST_CORE})
    target_link_libraries(flachead_test_metadata PRIVATE sqlite3)
    target_compile_features(flachead_test_metadata PRIVATE cxx_std_20)
    add_test(NAME flachead_test_metadata COMMAND flachead_test_metadata)

    add_executable(flachead_test_audio
        tests/test_audio.cpp
        src/audio/AudioService.cpp
        src/audio/MpvBackend.cpp
        src/audio/NullBackend.cpp
        src/core/Logger.cpp
        src/events/EventBus.cpp
    )
    target_include_directories(flachead_test_audio PRIVATE third_party/mpv/include)
    target_link_libraries(flachead_test_audio PRIVATE ${CMAKE_DL_LIBS})
    target_compile_features(flachead_test_audio PRIVATE cxx_std_20)
    add_test(NAME flachead_test_audio COMMAND flachead_test_audio)

    add_executable(flachead_test_playback
        tests/test_playback.cpp
        src/playback/QueueManager.cpp
        src/playback/PlaylistEngine.cpp
        src/playback/PlaybackController.cpp
        src/audio/AudioService.cpp
        src/audio/MpvBackend.cpp
        src/audio/NullBackend.cpp
        src/database/Database.cpp
        src/core/Logger.cpp
        src/core/PathUtils.cpp
        src/events/EventBus.cpp
    )
    target_include_directories(flachead_test_playback PRIVATE third_party/mpv/include)
    target_link_libraries(flachead_test_playback PRIVATE sqlite3 ${CMAKE_DL_LIBS})
    target_compile_features(flachead_test_playback PRIVATE cxx_std_20)
    add_test(NAME flachead_test_playback COMMAND flachead_test_playback)

    add_executable(flachead_test_app_wiring
        tests/test_app_wiring.cpp
        src/playback/QueueManager.cpp
        src/playback/PlaybackController.cpp
        src/library/AlbumArtExtractor.cpp
        src/library/LibraryScanner.cpp
        src/library/LibraryService.cpp
        src/audio/AudioService.cpp
        src/audio/MpvBackend.cpp
        src/audio/NullBackend.cpp
        src/database/Database.cpp
        src/core/Logger.cpp
        src/core/PathUtils.cpp
        src/events/EventBus.cpp
        src/metadata/FFprobeExtractor.cpp
        src/metadata/Json.cpp
        src/metadata/Metadata.cpp
        src/services/SettingsManager.cpp
    )
    target_include_directories(flachead_test_app_wiring PRIVATE third_party/mpv/include)
    target_link_libraries(flachead_test_app_wiring PRIVATE sqlite3 ${CMAKE_DL_LIBS})
    target_compile_features(flachead_test_app_wiring PRIVATE cxx_std_20)
    add_test(NAME flachead_test_app_wiring COMMAND flachead_test_app_wiring)

    add_executable(flachead_test_shell_ui
        tests/test_shell_ui.cpp
        src/animation/AnimationManager.cpp
        src/animation/Easing.cpp
        src/commands/Command.cpp
        src/commands/CommandCenter.cpp
        src/focus/FocusManager.cpp
        src/input/GestureRecognizer.cpp
        src/input/InputEvent.cpp
        src/input/InputManager.cpp
        src/layout/BoxLayout.cpp
        src/layout/GridLayout.cpp
        src/layout/LayoutEngine.cpp
        src/math/Color.cpp
        src/shell/ScreenTransitionManager.cpp
        src/ui/palette/Palette.cpp
    )
    target_compile_features(flachead_test_shell_ui PRIVATE cxx_std_20)
    target_link_libraries(flachead_test_shell_ui PRIVATE SDL3::SDL3)
    add_test(NAME flachead_test_shell_ui COMMAND flachead_test_shell_ui)

    add_executable(flachead_test_services
        tests/test_services.cpp
        src/services/BackgroundJobManager.cpp
        src/services/MemoryManager.cpp
        src/services/NotificationManager.cpp
    )
    target_compile_features(flachead_test_services PRIVATE cxx_std_20)
    target_link_libraries(flachead_test_services PRIVATE Threads::Threads)
    add_test(NAME flachead_test_services COMMAND flachead_test_services)
endif()