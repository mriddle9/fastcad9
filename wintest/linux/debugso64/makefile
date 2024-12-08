%.o : ../../source/%.cpp
	g++ -D_SYS_LINUX -D_OS64 -D_DEBUG -D_REENTRANT -traditional-cpp -m64 -c -g -fPIC \
	-I../../../../evtk4src/fwkc/sdk -I../../../../evtk4src/fwkw/sdk -o $@ $<

include ../libobjs

wintest: $(LIBOBJS)
	g++ -o wintest -D_OS64 -D_SYS_LINUX -D_DEBUG -D_REENTRANT -traditional-cpp -m64 -g -fPIC \
	$(LIBOBJS) \
	-L../../../../evtk4src/libs/linux/Dd64 -L. \
	-lfwkw -lfwkc -lX11 -lXext -lXmu -lfreetype \
	-lcups -lrt -lpthread -ldl -lc

