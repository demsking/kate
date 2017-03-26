project(katejshintplugin) # what is this?
add_definitions(-DTRANSLATION_DOMAIN=\"katejshint\")

# Maybe remove these later // I just copied this from xmltools example
remove_definitions(-DQT_NO_CAST_TO_ASCII)
remove_definitions(-DQT_NO_CAST_FROM_ASCII)
remove_definitions(-DQT_NO_URL_CAST_FROM_STRING)
remove_definitions(-DQT_NO_CAST_FROM_BYTEARRAY)


set(katejshintplugin_PART_SRCS
    plugin_katejshint.cpp
)

# resource for ui file and stuff
qt5_add_resources(katejshintplugin_PART_SRCS plugin.qrc)

add_library(katejshintplugin MODULE ${katejshintplugin_PART_SRCS})
target_link_libraries(katejshintplugin
    KF5::TextEditor
    KF5::Parts
    KF5::IconThemes
    KF5::I18n
    KF5::Service
)

# this did not changed
install(TARGETS katejshintplugin  DESTINATION ${PLUGIN_INSTALL_DIR}/ktexteditor )

# It is to generate json
kcoreaddons_desktop_to_json (katejshintplugin katejshint.desktop)
