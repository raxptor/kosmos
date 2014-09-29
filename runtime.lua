KOSMOSPATH = path.getdirectory(_SCRIPT)
KOSMOSRT_INCLUDES = { KOSMOSPATH .. "/src/runtime" }

function kosmos_use_runtime_lib()
        putki_typedefs_runtime("src/types", false, KOSMOSPATH)
        includedirs ( KOSMOSRT_INCLUDES )
        links {"kosmos-runtime"}
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

