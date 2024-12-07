%.o : ../../source/%.cpp
	g++ -D_SYS_LINUX -D_OS64 -D_REENTRANT -D_USINGFWKSTATICLIB -traditional-cpp -m64 -c -g \
	-I../../../../evtk4src/fwkc/sdk -I../../../../evtk4src/fwkw/sdk -o $@ $<

include ../libobjs

wintest: $(LIBOBJS)
	g++ -o wintest -D_OS64 -D_SYS_LINUX -D_REENTRANT -D_USINGFWKSTATICLIB -traditional-cpp -m64 -g $(LIBOBJS) \
	-L../../../../evtk4src/libs/linux/S64 \
	-lfwkw -lfwkc -lfreetype -lpng -ltiff -ljpeg -lzlib -lX11 -lXext -lXmu \
	-lcups -lrt -lpthread -ldl -lc
