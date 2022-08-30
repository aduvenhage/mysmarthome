-- premake5.lua
workspace "MySmartHome"
   location "build"
   configurations { "Debug", "Release" }

project "MySmartHome"
   location "build/MySmartHome"
   kind "ConsoleApp"
   language "C++"
   cppdialect "C++17"
   targetdir "bin/%{cfg.buildcfg}"
   
   sysincludedirs {""}
   includedirs {""}

   files {
      "display_app/**.h",
      "display_app/**.ino",
      "libraries/**.h",
      "libraries/**.ino",
      "sensor_xs/**.h",
      "sensor_xs/**.ino",
      "siren_app/**.h",
      "sizren_app/**.ino",
      "*.lua",
      "README.md"}
      
   removefiles {""}
