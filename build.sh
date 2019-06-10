#!/usr/bin/env bash
rm -rf obj/
ndk-build NDK_PROJECT_PATH=`pwd` NDK_APPLICATION_MK=`pwd`/Application.mk
