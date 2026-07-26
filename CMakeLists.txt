cmake_minimum_required(VERSION 3.20)

project(FLACHEAD VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(SDL3 REQUIRED CONFIG)
find_package(SDL3_ttf REQUIRED CONFIG)
find_package(SDL3_image QUIET CONFIG)

add_executable(FLACHEAD
    src/main.cpp
    src/app/Application.cpp
    src/app/AppManager.cpp
    src/animation/Animator.cpp
    src/animation/AnimationPlayer.cpp
    src/animation/Tween.cpp
    src/audio/AudioService.cpp
    src/core/Logger.cpp
    src/core/Renderer.cpp
    src/core/Time.cpp
    src/filesystem/FileSystem.cpp
    src/graphics/Font.cpp
    src/graphics/FontManager.cpp
    src/graphics/Sprite.cpp
    src/graphics/Texture.cpp
    src/graphics/TextureCache.cpp
    src/input/InputManager.cpp
    src/layout/Layout.cpp
    src/layout/ColumnLayout.cpp
    src/layout/RowLayout.cpp
    src/math/Color.cpp
    src/screens/HomeScreen.cpp
    src/screens/ScreenManager.cpp
    src/services/BatteryManager.cpp
    src/services/SettingsManager.cpp
    src/services/StorageManager.cpp
    src/system/Window.cpp
    src/ui/theme/ThemeManager.cpp
    src/ui/Button.cpp
    src/ui/Canvas.cpp
    src/ui/Card.cpp
    src/ui/Container.cpp
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
    src/ui/Toast.cpp
    src/ui/Widget.cpp
    src/views/AlbumArtWidget.cpp
    src/views/SongInfoWidget.cpp
    src/views/ProgressWidget.cpp
    src/views/BottomBarWidget.cpp
    src/resource/ResourceManager.cpp
    src/models/SongModel.cpp
    src/models/AlbumModel.cpp
    src/models/ArtistModel.cpp
    src/models/PlaylistModel.cpp
    src/models/LibraryModel.cpp
    src/models/BatteryStateModel.cpp
    src/models/SettingsModel.cpp
)
target_link_libraries(FLACHEAD PRIVATE 
    SDL3::SDL3 
    SDL3_ttf::SDL3_ttf
)

if(TARGET SDL3_image::SDL3_image)
    target_link_libraries(FLACHEAD PRIVATE SDL3_image::SDL3_image)
endif()