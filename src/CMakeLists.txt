set(SOURCE_FILES
    "pluginMain.cpp"
    "inputCurveOptimizer.cpp"
    "inputCurveOptimizer.h"
)

find_package(Maya REQUIRED)

# 🚀 OpenMP 지원 추가 (병렬 처리 최적화)
find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    message(STATUS "OpenMP found - 병렬 처리 활성화")
else()
    message(WARNING "OpenMP not found - 순차 처리만 지원")
endif()

add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})

# UTF-8 인코딩 설정 (Windows 한글 경고 해결)
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /utf-8)
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE Maya::Maya)

# OpenMP 링크 (병렬 처리)
if(OpenMP_CXX_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE OpenMP::OpenMP_CXX)
    target_compile_definitions(${PROJECT_NAME} PRIVATE OPENMP_ENABLED)
endif()

target_include_directories(${PROJECT_NAME} 
    PRIVATE Maya::Maya
    PUBLIC "${CMAKE_CURRENT_BINARY_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}"
)
MAYA_PLUGIN(${PROJECT_NAME})

install(TARGETS ${PROJECT_NAME} ${MAYA_TARGET_TYPE} DESTINATION plug-ins/${MAYA_VERSION})
