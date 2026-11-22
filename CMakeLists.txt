cmake_minimum_required(VERSION 3.22)
project(DIRT VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.6
    GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(DIRT
    COMPANY_NAME "syst3me79"
    BUNDLE_ID "com.syst3me79.dirt"
    PLUGIN_MANUFACTURER_CODE Sy79
    PLUGIN_CODE Drt1
    FORMATS VST3 Standalone
    PRODUCT_NAME "DIRT")

juce_generate_juce_header(DIRT)

target_sources(DIRT PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h)

target_compile_definitions(DIRT PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN通常=0)

target_link_libraries(DIRT PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    PUBLIC juce::juce_recommended_config_flags
           juce::juce_recommended_lto_flags
           juce::juce_recommended_warning_flags)
