install(FILES CMakeLists.txt DESTINATION foo COMPONENT test)

set(CPACK_DEBIAN_PACKAGE_VERSION "5.0.1-71-g884852e")

if(PACKAGING_TYPE STREQUAL "COMPONENT")
  set(CPACK_COMPONENTS_ALL test)
endif()
