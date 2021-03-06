include_directories("src")

# Download and extract MNIST
if(UNIX)
  set(MNIST_FILES "train-images-idx3-ubyte"
                  "train-labels-idx1-ubyte"
                  "t10k-images-idx3-ubyte"
                  "t10k-labels-idx1-ubyte")
  set(MNIST_RELATIVE_PATH "data/mnist/")
  set(MNIST_FULL_PATH ${CMAKE_CURRENT_BINARY_DIR}/${MNIST_RELATIVE_PATH})
  file(MAKE_DIRECTORY ${MNIST_FULL_PATH})
  foreach(FILENAME ${MNIST_FILES})
    if(NOT EXISTS ${MNIST_FULL_PATH}/${FILENAME})
      set(FILE "${FILENAME}.gz")
      set(DL_LINK "http://yann.lecun.com/exdb/mnist/${FILE}")
      execute_process(COMMAND wget ${DL_LINK}
                      WORKING_DIRECTORY ${MNIST_FULL_PATH}
                      RESULT_VARIABLE PROC_RET)
      if(${PROC_RET})
        message(FATAL_ERROR "Could not download ${DL_LINK}")
      endif()
      execute_process(COMMAND gunzip ${FILE}
                      WORKING_DIRECTORY ${MNIST_FULL_PATH}
                      RESULT_VARIABLE PROC_RET)
      if(${PROC_RET})
        message(FATAL_ERROR "Could not extract ${FILE}")
      endif()
    endif()
  endforeach()
endif()

# Build each example
file(GLOB_RECURSE SOURCES "src/*.cpp")
foreach(SOURCE ${SOURCES})
  process_source_name(${SOURCE})
  add_executable(${TARGET_NAME} ${SOURCE})
  set_property(TARGET ${TARGET_NAME} PROPERTY COMPUTECPP_INCLUDE_AFTER 1)
  add_sycl_to_target(${TARGET_NAME} ${OUTPUT_SYCL_DIR} ${SOURCE})
endforeach()
