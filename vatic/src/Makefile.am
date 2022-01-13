bin_PROGRAMS = client_program server_program

client_program_SOURCES = client_program.cpp ConfUtils.cpp Log.cpp Client.cpp
server_program_SOURCES = server_program.cpp ConfUtils.cpp TrafficSource.cpp Vatic.cpp Log.cpp

AM_CXXFLAGS = --std=c++17 -Wall
# -DLOG_ENABLED
ACLOCAL_AMFLAGS = -I m4

client_program_LDADD = -lboost_program_options -lpthread -lstdc++fs
server_program_LDADD = -lboost_program_options -lpthread -lstdc++fs