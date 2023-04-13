# Edit following two lines to set component requirements (see docs)
set(COMPONENT_REQUIRES mqtt espmqtt )
set(COMPONENT_PRIV_REQUIRES mqtt espmqtt nvs_flash bt)

set(COMPONENT_SRCS "main.c")
set(COMPONENT_ADD_INCLUDEDIRS "")

register_component()
