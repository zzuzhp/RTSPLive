APP_CFLAGS     := -fPIC -DNDEBUG -fno-strict-aliasing
APP_CPPFLAGS   := -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -DASIO_STANDALONE -std=c++11 -frtti -fexceptions -fpermissive -ffunction-sections -fdata-sections
APP_OPTIM      := release
APP_LDFLAGS    := -Wl,--gc-sections

APP_PLATFORM   := android-16
APP_ABI        := arm64-v8a armeabi-v7a x86_64 x86
APP_STL        := c++_static
APP_BUILD_SCRIPT := Android.mk
