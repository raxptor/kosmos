	project "libpng"
		kind "StaticLib"
		language "c"
		targetname "libkosmospng"
		files { "*.c", "*.h" }
		excludes { "example.c" }
		includedirs (ZLIB_INCLUDES)
		links {"libz"}
