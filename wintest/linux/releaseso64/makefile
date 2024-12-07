%.o : ../../source/%.cpp
	g++ -D_OS64 -D_SYS_LINUX -D_REENTRANT -traditional-cpp -m64 -c -g -fPIC \
	-I../../../../evtk4src/fwkc/sdk -I../../../../evtk4src/fwkw/sdk -o $@ $<

include ../libobjs

wintest: $(LIBOBJS)
	g++ -o wintest -D_OS64 -D_SYS_LINUX -D_REENTRANT -traditional-cpp -m64 -g -fPIC \
	$(LIBOBJS) \
	-L../../../../evtk4src/libs/linux/D64 -L. \
	-lfwkc -lfwkw -lX11 -lXext -lXmu -lfreetype \
	-lrt -lpthread -ldl -lc
