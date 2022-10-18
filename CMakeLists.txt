cmake_minimum_required(VERSION 3.16)
project(MR_ImageProcessingNew)

find_package(ITK)

if(ITK_FOUND)
    include(${ITK_USE_FILE})
else(ITK_FOUND)
    message(fatal_error
	"Canno build without ITK. Please set ITK_DIR.")
endif(ITK_FOUND)

add_executable(MR_ImageProcessingNew MR_ImageProcessingNew.cxx )

target_link_libraries(MR_ImageProcessingNew ${ITK_LIBRARIES})
