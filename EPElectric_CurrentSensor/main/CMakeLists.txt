
set(COMPONENT_SRCDIRS ".")
set(COMPONENT_ADD_INCLUDEDIRS ".")

register_component()
spiffs_create_partition_image(storage ../spiffs_image FLASH_IN_PROJECT)


# idf_component_register(SRCS "app_main.c"
#                     INCLUDE_DIRS ".")

# spiffs_create_partition_image(storage ../spiffs FLASH_IN_PROJECT)