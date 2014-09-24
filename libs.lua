
	dofile "external/libpng/premake.lua"

	KOSMOSPATH = path.getdirectory(_SCRIPT)
	KOSMOSLIB_INCLUDES = { KOSMOSPATH .. "/src/" }
	KOSMOSRT_INCLUDES = { KOSMOSPATH .. "/src/runtime" }
	KOSMOSLIB_LINKS = { "kosmos-builder-lib", "libpng", "libz" }
	
	function kosmos_use_builder_lib()
		putki_typedefs_runtime("src/types", false, KOSMOSPATH)
		includedirs ( KOSMOSLIB_INCLUDES )
		links { KOSMOSLIB_LINKS }
	end
	
	function kosmos_use_runtime_lib()
		putki_typedefs_runtime("src/types", false, KOSMOSPATH)
		includedirs ( KOSMOSRT_INCLUDES )
                links {"kosmos-runtime"}
	end

	project "kosmos-builder-lib"

		kind "StaticLib"

		language "C++"
		targetname "kosmos-databuilder"

		putki_typedefs_builder("src/types", true)
		putki_use_builder_lib()
		
		includedirs { "src" }
		includedirs { "external/libpng"}

		files { "src/builder/**.*" }
		files { "src/binpacker/**.*" }

		links { "libpng" }
		links { "libz" }
	end

	project "kosmos-runtime"

		language "C++"
		targetname "kosmos-runtime"
		kind "StaticLib"
		
		includedirs { "src/untime" }
                files { "src/runtime/**.cpp" }
		files { "src/runtime/**.h" }
		
		putki_use_runtime_lib()
		putki_typedefs_runtime("src/types", true)

	end
	
