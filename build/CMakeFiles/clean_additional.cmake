# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "")
  file(REMOVE_RECURSE
  [[CMakeFiles\KeyStarControl_autogen.dir\AutogenUsed.txt]]
  [[CMakeFiles\KeyStarControl_autogen.dir\ParseCache.txt]]
  "KeyStarControl_autogen"
  )
endif()
