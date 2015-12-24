cmake_minimum_required(VERSION 3.0)

project (utils)

# FIXME compile these and then run them to generate the pp header files
# the number of supported pp-args should be defined in the project that
# pulls-in this library as a configuration option.
# the generated .hpp files should be placed into a build output directory
# and the application then somehow needs to find those generated headers

#pp_for_each.cpp
#pp_args.cpp
#pp_arith.cpp

add_library (utils STATIC
  utils.cpp
)
