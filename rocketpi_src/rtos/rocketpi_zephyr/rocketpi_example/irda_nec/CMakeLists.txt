cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(irda_nec_sample)

target_include_directories(app PRIVATE nec_protocol/lib)

target_sources(app PRIVATE
  nec_protocol/lib/nec_prot.c
  src/ir_recv.c
  src/main.c
)
