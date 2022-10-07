# Library sources
HOOK_CC_SRCS = $(wildcard 3rdparty/minhook/src/*.cc 3rdparty/d3dhook/*.cc)
HOOK_C_SRCS = $(wildcard 3rdparty/minhook/src/hde32/*.c)
CONTRIB_CC_SRCS = 3rdparty/gtest/fused-src/gtest/gtest-all.cc $(wildcard 3rdparty/JLib/*.cc)
CONTRIB_CPP_SRCS = $(wildcard 3rdparty/imgui/*.cpp)
CONTRIB_C_SRCS = $(wildcard 3rdparty/*.c)

# Main program sources
LIB_CPP_SRCS = $(wildcard lib/*.cpp)
MAIN_CPP_SRCS = $(wildcard targets/Main*.cpp) $(wildcard netplay/*.cpp) $(LIB_CPP_SRCS)
DLL_CPP_SRCS = $(wildcard targets/Dll*.cpp) \
$(filter-out lib/ConsoleUi.cpp,$(wildcard netplay/*.cpp) $(LIB_CPP_SRCS))

# Build flags
DEFINES = -DWIN32_LEAN_AND_MEAN -DWINVER=0x501 -D_WIN32_WINNT=0x501 -D_M_IX86
INCLUDES = -I$(CURDIR) -I$(CURDIR)/netplay -I$(CURDIR)/lib -I$(CURDIR)/3rdparty \
-I$(CURDIR)/3rdparty/cereal/include -I$(CURDIR)/3rdparty/gtest/include -I$(CURDIR)/3rdparty/minhook/include \
-I$(CURDIR)/3rdparty/d3dhook -I$(CURDIR)/3rdparty/imgui -I$(CURDIR)/targets

all: $(wildcard netplay/*.cpp tools/*.cpp targets/*.cpp lib/*.cpp) \
$(filter-out lib/Version.%.hpp lib/Protocol.%.hpp,$(wildcard netplay/*.hpp targets/*.hpp lib/*.hpp))
	@echo
	@echo ========== Main-build ==========
	@echo
	@scripts/make_version 3.1.2 > lib/Version.local.hpp
	@scripts/make_protocol $(filter-out lib/Version.%.hpp lib/Protocol.%.hpp,$(wildcard netplay/*.hpp targets/*.hpp lib/*.hpp))
	@$(MAKE) --no-print-directory target-release BUILD_TYPE=build_release

target-release: cccaster.v3.1.release.exe cccaster/hook.release.dll cccaster/launcher.exe
	@echo
	rm -f $(wildcard cccaster*.zip)
	zip cccaster.v3.1.2.release.zip $^
	zip cccaster.v3.1.2.release.zip -j relay_list.txt
	cp -r res/GRP GRP
	zip cccaster.v3.1.2.release.zip -r GRP
	rm -rf GRP
	@echo $(MAKECMDGOALS)

build_release_$(shell git rev-parse --abbrev-ref HEAD):
	rsync -a -f"- .git/" -f"- build_*/" -f"+ */" -f"- *" --exclude=".*" . $@

build_release_$(shell git rev-parse --abbrev-ref HEAD)/%.o: %.cpp | build_release_$(shell git rev-parse --abbrev-ref HEAD)
	i686-w64-mingw32-g++ -m32 $(INCLUDES) $(DEFINES) -s -Os -Ofast -fno-rtti -DNDEBUG -DRELEASE -DDISABLE_LOGGING -DDISABLE_ASSERTS \
	-std=c++11 -o $@ -c $<

build_release_$(shell git rev-parse --abbrev-ref HEAD)/%.o: %.cc | build_release_$(shell git rev-parse --abbrev-ref HEAD)
	i686-w64-mingw32-g++ -m32 $(INCLUDES) $(DEFINES) -s -Os -Ofast -fno-rtti -DNDEBUG -DRELEASE -DDISABLE_LOGGING -DDISABLE_ASSERTS -o $@ -c $<

build_release_$(shell git rev-parse --abbrev-ref HEAD)/%.o: %.c | build_release_$(shell git rev-parse --abbrev-ref HEAD)
	i686-w64-mingw32-gcc $(filter-out -fno-rtti,-m32 $(INCLUDES) $(DEFINES) -s -Os -Ofast -fno-rtti -DNDEBUG -DRELEASE -DDISABLE_LOGGING \
	-DDISABLE_ASSERTS) -Wno-attributes -o $@ -c $<

cccaster:
	mkdir -p $@

cccaster/launcher.exe: tools/Launcher.cpp | cccaster
	i686-w64-mingw32-g++ -o $@ $^ -m32 -s -Os -O2 -Wall -static -mwindows
	i686-w64-mingw32-strip $@
	chmod +x $@

tools/generator.exe: tools/Generator.cpp $(addprefix build_release_$(shell git rev-parse --abbrev-ref HEAD)/,\
$(filter-out lib/Version.o lib/LoggerLogVersion.o lib/ConsoleUi.o,$(LIB_CPP_SRCS:.cpp=.o) $(CONTRIB_C_SRCS:.c=.o)))
	i686-w64-mingw32-g++ -o $@ -m32 $(INCLUDES) $(DEFINES) -s -Os -O2 -DLOGGING -Wall -std=c++11 $^ -m32 -static -lws2_32 \
	-lpsapi -lwinpthread -lwinmm -lole32 -ldinput -lwininet -ldwmapi -lgdi32
	i686-w64-mingw32-strip $@
	chmod +x $@

cccaster.v3.1.release.exe: $(addprefix build_release_$(shell git rev-parse --abbrev-ref HEAD)/,\
$(MAIN_CPP_SRCS:.cpp=.o) $(CONTRIB_CC_SRCS:.cc=.o) $(CONTRIB_CPP_SRCS:.cpp=.o) $(CONTRIB_C_SRCS:.c=.o)) res/icon.res
	rm -f $(filter-out cccaster.v3.1.release.exe,$(wildcard cccaster*.exe))
	i686-w64-mingw32-g++ -o $@ -m32 $(INCLUDES) $(DEFINES) -Wall -std=c++11 $^ -m32 -static -lws2_32 -lpsapi -lwinpthread \
	-lwinmm -lole32 -ldinput -lwininet -ldwmapi -lgdi32
	i686-w64-mingw32-strip $@
	chmod +x $@

cccaster/hook.release.dll: $(addprefix build_release_$(shell git rev-parse --abbrev-ref HEAD)/,\
$(DLL_CPP_SRCS:.cpp=.o) $(HOOK_CC_SRCS:.cc=.o) $(HOOK_C_SRCS:.c=.o) $(CONTRIB_C_SRCS:.c=.o) $(CONTRIB_CPP_SRCS:.cpp=.o)) \
res/rollback.o targets/CallDraw.s | cccaster
	i686-w64-mingw32-g++ -o $@ -m32 $(INCLUDES) $(DEFINES) -Wall -std=c++11 $^ -shared -m32 -static -lws2_32 -lpsapi \
	-lwinpthread -lwinmm -lole32 -ldinput -lwininet -ldwmapi -lgdi32 -ld3dx9
	i686-w64-mingw32-strip $@

res/rollback.bin: tools/generator.exe
	wine tools/generator.exe $@

res/rollback.o: res/rollback.bin
	objcopy -I binary -O elf32-i386 -B i386 $< $@

res/icon.res: res/icon.rc res/icon.ico
	i686-w64-mingw32-windres -F pe-i386 res/icon.rc -O coff -o $@

clean:
	rm -rf build_release_$(shell git rev-parse --abbrev-ref HEAD) \
	cccaster \
	*.exe *.zip tools/*.exe \
	$(wildcard lib/Version.*.hpp lib/Protocol.*.hpp) \
	res/rollback.* res/icon.res \
	.depend_$(shell git rev-parse --abbrev-ref HEAD) \
	.include_$(shell git rev-parse --abbrev-ref HEAD) 
	git checkout -- lib/ProtocolEnums.hpp
