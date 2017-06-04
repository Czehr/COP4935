#$(rose-config cxx) $(rose-config cppflags) $(rose-config cxxflags) -o loopNestingInfoProcessing loopNestingInfoProcessing.cpp $(rose-config ldflags)

cxx=g++-4.9
cppflags=-I/home/demo/opt/rose_inst/include/rose     -I/home/demo/opt/yaml/0.5.3/boost-1.61.0/gcc-4.9.3-default/include        -I/home/demo/opt/dlib/18.18 -pthread -I/home/demo/opt/boost/1.61.0/gcc-4.9.3-default/include
cxxflags=-g -rdynamic -Wall -Wno-unused-local-typedefs -Wno-attributes -g -rdynamic -O0 -Wall -Wno-unused-local-typedefs -Wno-attributes -Wall -Wno-unused-local-typedefs -Wno-attributes
ldflags=-L/home/demo/opt/rose_inst/lib -lrose -pthread  -L/home/demo/opt/boost/1.61.0/gcc-4.9.3-default/lib -lboost_date_time -lboost_thread -lboost_filesystem -lboost_program_options -lboost_regex -lboost_system -lboost_serialization -lboost_wave -lboost_iostreams -L/home/demo/opt/jvm/jdk1.7.0_51/jre/lib/amd64/server -ljvm  -ldl          -L/home/demo/opt/yaml/0.5.3/boost-1.61.0/gcc-4.9.3-default/lib -lyaml-cpp   -lm


default: all

all: loopNestingInfoProcessing

loopNestingInfoProcessing: loopNestingInfoProcessing.cpp
	$(cxx) $(cppflags) $(cxxflags) -o loopNestingInfoProcessing loopNestingInfoProcessing.cpp $(ldflags)

