%.o : ../../source/%.cpp
	g++ -D_SYS_LINUX -D_OS64 -D_REENTRANT -D_USINGFWKSTATICLIB -traditional-cpp -m64 -c -g -O \
	-I../../source -I../../../../evtk3src/fwkc/sdk -I../../../../evtk3src/fwkw/sdk -o $@ $<

include ../libobjs

fcw9: $(LIBOBJS)
	g++ -o fcw9 -D_OS64 -D_SYS_LINUX -D_REENTRANT -D_USINGFWKSTATICLIB -traditional-cpp -m64 -g $(LIBOBJS) \
	-L../../../../evtk3src/libs/linux/S64 \
	-lfwkw -lfwkc -lcups -lfreetype -lpng -ltiff -ljpeg -lzlib -lX11 -lXext -lXmu \
	-lrt -lpthread -ldl -lc
