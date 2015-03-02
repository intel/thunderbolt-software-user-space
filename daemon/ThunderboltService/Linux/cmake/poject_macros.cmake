#define a macro to define binary include and src 
macro (define_bin_layout_vars bin_name)
  set(${bin_name}_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/include")
  set(${bin_name}_SRC "${CMAKE_CURRENT_SOURCE_DIR}/src")
endmacro (define_bin_layout_vars)
