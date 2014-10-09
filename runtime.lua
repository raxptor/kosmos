KOSMOSPATH = path.getdirectory(_SCRIPT)
KOSMOSRT_INCLUDES = { KOSMOSPATH .. "/src/runtime" }

function kosmos_use_runtime_lib()
        putki_typedefs_runtime("src/types", false, KOSMOSPATH)
        includedirs ( KOSMOSRT_INCLUDES )
        links {"kosmos-runtime"}
        configuration "windows"
			defines {"GLEW_STATIC"}
	        includedirs { KOSMOSPATH .. "/prebuilt/glew-1.11.0/include" }
		   	links (KOSMOSPATH .. "/prebuilt/glew-1.11.0/lib/Release/x64/glew32s" )
			links {"OpenGL32"}
       	configuration {}
end


project "kosmos-runtime"

        language "C++"
        targetname "kosmos-runtime"
        kind "StaticLib"
        
        includedirs { "src/runtime/kosmos" }
        
        files { "src/runtime/**.cpp" }
        files { "src/runtime/**.h" }
        
        putki_use_runtime_lib()
        putki_typedefs_runtime("src/types", true)

		configuration "windows"
			defines {"GLEW_STATIC"}
	       	includedirs { KOSMOSPATH .. "/prebuilt/glew-1.11.0/include" }
        configuration {}

        

