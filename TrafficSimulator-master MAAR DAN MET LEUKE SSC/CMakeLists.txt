cmake_minimum_required(VERSION 3.16)
project(TrafficSimulator)

set(CMAKE_CXX_STANDARD 14)

add_executable(TrafficSimulator main.cpp Street.cpp Street.h Vehicle.cpp Vehicle.h Influence.cpp Influence.h TransportVehicle.cpp TransportVehicle.h SpecialVehicle.cpp SpecialVehicle.h Network.cpp Network.h Intersection.cpp Intersection.h Simulation.cpp Simulation.h jsonParser.cpp jsonParser.h Util.cpp Util.h StateElimination.cpp StateElimination.h ElimStreet.cpp ElimStreet.h ssc_untouched/SSC.h ssc_untouched/SSC.cpp)