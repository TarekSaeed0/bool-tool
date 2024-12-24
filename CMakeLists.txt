cmake_minimum_required(VERSION 3.31.1)
project(
	bool_tool
	VERSION 1.0
	LANGUAGES C
	DESCRIPTION "Bool Tool GTK Project"
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 REQUIRED IMPORTED_TARGET gtk4)
pkg_check_modules(GMODULE_EXPORT REQUIRED IMPORTED_TARGET gmodule-export-2.0)

find_program(INKSCAPE NAMES inkscape)
find_program(MAGICK NAMES magick)

if(INKSCAPE AND MAGICK)
	add_custom_command(
		OUTPUT
			${CMAKE_CURRENT_BINARY_DIR}/com.github.TarekSaeed0.bool_tool.icons.generated
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		COMMAND
			${CMAKE_COMMAND} -E make_directory
			data/bool_tool/icons/hicolor/16x16/apps
			data/bool_tool/icons/hicolor/32x32/apps
			data/bool_tool/icons/hicolor/48x48/apps
		COMMAND
			${INKSCAPE} -w 16 -h 16 -o
			data/bool_tool/icons/hicolor/16x16/apps/com.github.TarekSaeed0.bool_tool.png
			data/bool_tool/icons/hicolor/scalable/apps/com.github.TarekSaeed0.bool_tool.svg
		COMMAND
			${INKSCAPE} -w 32 -h 32 -o
			data/bool_tool/icons/hicolor/32x32/apps/com.github.TarekSaeed0.bool_tool.png
			data/bool_tool/icons/hicolor/scalable/apps/com.github.TarekSaeed0.bool_tool.svg
		COMMAND
			${INKSCAPE} -w 48 -h 48 -o
			data/bool_tool/icons/hicolor/48x48/apps/com.github.TarekSaeed0.bool_tool.png
			data/bool_tool/icons/hicolor/scalable/apps/com.github.TarekSaeed0.bool_tool.svg
		COMMAND
			${MAGICK}
			data/bool_tool/icons/hicolor/16x16/apps/com.github.TarekSaeed0.bool_tool.png
			data/bool_tool/icons/hicolor/32x32/apps/com.github.TarekSaeed0.bool_tool.png
			data/bool_tool/icons/hicolor/48x48/apps/com.github.TarekSaeed0.bool_tool.png
			data/bool_tool/icons/com.github.TarekSaeed0.bool_tool.ico
		COMMAND
			${CMAKE_COMMAND} -E touch
			${CMAKE_CURRENT_BINARY_DIR}/com.github.TarekSaeed0.bool_tool.icons.generated
		VERBATIM
		MAIN_DEPENDENCY
			data/bool_tool/icons/hicolor/scalable/apps/com.github.TarekSaeed0.bool_tool.svg
	)
else()
	add_custom_command(
		OUTPUT
			${CMAKE_CURRENT_BINARY_DIR}/com.github.TarekSaeed0.bool_tool.icons.generated
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		COMMAND
			${CMAKE_COMMAND} -E touch
			${CMAKE_CURRENT_BINARY_DIR}/com.github.TarekSaeed0.bool_tool.icons.generated
		VERBATIM
		MAIN_DEPENDENCY
			data/bool_tool/icons/hicolor/scalable/apps/com.github.TarekSaeed0.bool_tool.svg
	)
endif()

find_program(GLIB_COMPILE_RESOURCES NAMES glib-compile-resources REQUIRED)

add_custom_command(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/bool_tool.gresource.c
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	COMMAND
		${GLIB_COMPILE_RESOURCES} data/bool_tool.gresource.xml
		--target=${CMAKE_CURRENT_BINARY_DIR}/bool_tool.gresource.c
		--sourcedir=data/bool_tool --generate-source
	VERBATIM
	MAIN_DEPENDENCY data/bool_tool.gresource.xml
	DEPENDS
		data/bool_tool/ui/boolean_function_input.ui
		data/bool_tool/ui/application_window.ui
		data/bool_tool/style.css
		${CMAKE_CURRENT_BINARY_DIR}/com.github.TarekSaeed0.bool_tool.icons.generated
)

add_custom_target(
	bool_tool_compile_gresources ALL
	DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/bool_tool.gresource.c
)

find_program(DESKTOP_FILE_VALIDATE NAMES desktop-file-validate)

if(DESKTOP_FILE_VALIDATE)
	add_custom_command(
		OUTPUT
			${CMAKE_CURRENT_BINARY_DIR}/com.github.TarekSaeed0.bool_tool.desktop.validated
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		COMMAND
			${DESKTOP_FILE_VALIDATE}
			data/bool_tool/applications/com.github.TarekSaeed0.bool_tool.desktop
		COMMAND
			${CMAKE_COMMAND} -E touch
			${CMAKE_CURRENT_BINARY_DIR}/com.github.TarekSaeed0.bool_tool.desktop.validated
		VERBATIM
		MAIN_DEPENDENCY
			data/bool_tool/applications/com.github.TarekSaeed0.bool_tool.desktop
	)

	add_custom_target(
		bool_tool_validate_desktop_file ALL
		DEPENDS
			${CMAKE_CURRENT_BINARY_DIR}/com.github.TarekSaeed0.bool_tool.desktop.validated
	)
endif()

add_executable(
	bool_tool
	${CMAKE_CURRENT_BINARY_DIR}/bool_tool.gresource.c
	src/bool_tool/boolean_environment.c
	src/bool_tool/boolean_expression.c
	src/bool_tool/boolean_variables.c
	src/bool_tool/boolean_minterms.c
	src/bool_tool/boolean_implicants.c
	src/bool_tool/boolean_function.c
	src/bool_tool/boolean_function_input.c
	src/bool_tool/application_window.c
	src/bool_tool/application.c
	src/main.c
)
target_sources(
	bool_tool PRIVATE data/bool_tool/icons/com.github.TarekSaeed0.bool_tool.rc
)
target_include_directories(${PROJECT_NAME} PRIVATE include)
target_link_libraries(
	bool_tool PRIVATE PkgConfig::GTK4 PkgConfig::GMODULE_EXPORT
)
target_compile_options(
	bool_tool
	PRIVATE -Werror
			-Wall
			-Wextra
			-pedantic
			-Wfloat-equal
			-Wundef
			-Wshadow
			-Wpointer-arith
			-Wcast-align
			-Wstrict-prototypes
			-Wstrict-overflow=5
			-Wwrite-strings
			-Wcast-qual
			-Wconversion
			-Wno-overlength-strings
)
if(WIN32)
	target_compile_options(bool_tool PRIVATE -mwindows)
	target_link_options(bool_tool PRIVATE -Wl,-subsystem,console)
endif()

install(TARGETS bool_tool)
install(DIRECTORY data/bool_tool/icons data/bool_tool/applications TYPE DATA)
