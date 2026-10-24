cmake_minimum_required(VERSION 3.22)

# macOS: Universal Binary (Apple Silicon + Intel), läuft ab macOS 10.13
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "Minimum macOS version")
set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "macOS architectures")

project(RavenClipper VERSION 1.0.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.4
    GIT_SHALLOW    ON)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(RavenClipper
    COMPANY_NAME                "RavenClipper"
    PLUGIN_MANUFACTURER_CODE    Rvnc
    PLUGIN_CODE                 Rcl1
    FORMATS                     VST3
    PRODUCT_NAME                "RavenClipper"
    IS_SYNTH                    FALSE
    NEEDS_MIDI_INPUT            FALSE
    NEEDS_MIDI_OUTPUT           FALSE
    VST3_CATEGORIES             Fx Distortion
    COPY_PLUGIN_AFTER_BUILD     FALSE)

juce_generate_juce_header(RavenClipper)

target_sources(RavenClipper PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
    Source/ClipperDSP.h
    Source/RavenField.h)

target_compile_definitions(RavenClipper PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_STRICT_REFCOUNTEDPOINTER=1)

target_link_libraries(RavenClipper
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags)
