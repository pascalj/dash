set(ADDITIONAL_INCLUDES ${ADDITIONAL_INCLUDES} $ENV{HOME}/src/mephisto/alpaka/include)
set(ADDITIONAL_INCLUDES ${ADDITIONAL_INCLUDES} $ENV{BOOST_ROOT}/include)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DALPAKA_ACC_CPU_B_SEQ_T_THREADS_ENABLED")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DALPAKA_ACC_CPU_B_SEQ_T_SEQ_ENABLED")
