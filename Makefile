CXXFLAGS = -g OBJ =
    HTTPServer.o HTTPHeader.o WorkQueue.o server.o EXE = server

                                                                 $(EXE)
    : $(OBJ) g++ $(CXXFLAGS) - o $ @$ ^ -lpthread

                                                                 clean
    : rm - f $(OBJ) $(EXE)
