cmake_minimum_required(VERSION 3.22)
project(Hish VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(JUCE)

juce_add_gui_app(Hish
    PRODUCT_NAME "Hish"
    COMPANY_NAME "JC Vardy"
)



target_compile_definitions(Hish
    PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:Hish,JUCE_PRODUCT_NAME>"
        JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:Hish,JUCE_VERSION>"
)

target_sources(Hish
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Main.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/MainWindow.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/MainComponent.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/AppMenuBar.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/StatusBar.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/WaveformView.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/SpectrogramView.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/DetailsView.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/ControlBar.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/NavigationBar.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/TimeRuler.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/AmplitudeRuler.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/FrequencyRuler.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/AudioEngine.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/dsp/ProcessingEngine.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/dsp/GainProcessor.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/dsp/FIRFilterProcessor.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/dsp/ConvolutionProcessor.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/dsp/IIRFilterProcessor.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/dsp/FilterResponseCalculator.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/FilterResponseView.cpp
)

target_link_libraries(Hish
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_audio_formats
        juce::juce_dsp
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
