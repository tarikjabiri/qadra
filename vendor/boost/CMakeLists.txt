set(BOOST_INCLUDE_LIBRARIES geometry)
set(BOOST_ENABLE_CMAKE ON)

FetchContent_Declare(
  boost
  URL                     https://github.com/boostorg/boost/releases/download/boost-1.90.0/boost-1.90.0-cmake.tar.xz
  URL_HASH                SHA256=aca59f889f0f32028ad88ba6764582b63c916ce5f77b31289ad19421a96c555f
  USES_TERMINAL_DOWNLOAD  TRUE
  DOWNLOAD_NO_EXTRACT     FALSE
)

FetchContent_MakeAvailable(boost)
