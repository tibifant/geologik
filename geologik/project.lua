ProjectName = "geologik"
project(ProjectName)

dependson { "gamelib" }

  --Settings
  kind "ConsoleApp"
  language "C++"
  flags { "FatalWarnings" }
  staticruntime "On"

  cppdialect "C++17"

  filter {"system:windows"}
    buildoptions { '/MP' }
    ignoredefaultlibraries { "msvcrt" }
  
  filter { }
  
  defines { "_CRT_SECURE_NO_WARNINGS", "SSE2", "GLEW_STATIC" }
  
  objdir "intermediate/obj"

  files { "src/**.c", "src/**.cc", "src/**.cpp", "src/**.cxx", "src/**.h", "src/**.hh", "src/**.hpp", "src/**.inl", "src/**rc" }
  files { "assets/shaders/*.frag", "assets/shaders/*.vert" }

  files { "project.lua" }
  
  includedirs { "src**" }
  includedirs { "../gamelib/include/" }
  includedirs { "../3rdParty/SDL2/include" }
  includedirs { "../3rdParty/glew/include" }
  includedirs { "../3rdParty/stb/include" }
  
  targetname(ProjectName)
  targetdir "../builds/bin"
  debugdir "../builds/bin"
  
filter {}

filter {"configurations:Release"}
  links { "../builds/lib/gamelib.lib" }
filter {"configurations:Debug"}
  links { "../builds/lib/gamelibD.lib" }

filter {}

filter { "system:windows" }
  libdirs { "../3rdParty/SDL2/lib/" }
  links { "SDL2.lib", "SDL2main.lib" }
  links { "../3rdParty/glew/lib/libglew32.lib" }
  links { "opengl32.lib", "glu32.lib" }

  postbuildcommands { "{COPY} assets ../builds/bin" }

filter {}
warnings "Extra"

filter {"configurations:Release"}
  targetname "%{prj.name}"
filter {"configurations:Debug"}
  targetname "%{prj.name}D"

filter { }
  exceptionhandling "Off"
  rtti "Off"
  floatingpoint "Fast"

filter { "configurations:Debug*" }
	defines { "_DEBUG" }
	optimize "Off"
	symbols "FastLink"

filter { "configurations:Release*" }
	defines { "NDEBUG" }
	optimize "Speed"
	flags { "NoBufferSecurityCheck" }
  omitframepointer "On"
  symbols "On"

filter { "system:windows" }
	defines { "WIN32", "_WINDOWS" }
  flags { "NoPCH", "NoMinimalRebuild" }
  links { "kernel32.lib", "user32.lib", "gdi32.lib", "winspool.lib", "comdlg32.lib", "advapi32.lib", "shell32.lib", "ole32.lib", "oleaut32.lib", "uuid.lib", "odbc32.lib", "odbccp32.lib", "winmm.lib", "setupapi.lib", "version.lib", "Imm32.lib", "Ws2_32.lib", "Wldap32.lib", "Crypt32.lib" }

filter { "system:windows", "configurations:Release" }
  flags { "NoIncrementalLink" }

filter { "system:windows", "configurations:Debug" }
  ignoredefaultlibraries { "libcmt" }
filter { }
