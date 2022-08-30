-- premake5.lua
workspace "MySmartHome"
   location "workspace"
   configurations { "Debug", "Release" }

project "MySmartHome"
   location "workspace"
   kind "ConsoleApp"
   language "C++"
   cppdialect "C++17"
   targetdir "workspace/%{cfg.buildcfg}"
   
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
      "siren_app/**.ino",
      "*.lua",
      "README.md",
      "../.gitignore"}
      
   removefiles {""}
