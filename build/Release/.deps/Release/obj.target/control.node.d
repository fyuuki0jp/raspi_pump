cmd_Release/obj.target/control.node := g++ -shared -pthread -rdynamic  -Wl,-soname=control.node -o Release/obj.target/control.node -Wl,--start-group Release/obj.target/control/src/main.o Release/obj.target/control/src/cap.o Release/obj.target/control/src/Event.o Release/obj.target/control/src/wgpio.o Release/obj.target/control/src/aviwriter.o Release/obj.target/control/src/avi.o Release/obj.target/control/src/Data.o Release/obj.target/control/src/sqlite3.o Release/obj.target/control/src/SQLite.o -Wl,--end-group -lpthread -Os -lwiringPi -lm -g -w -march=armv7-a -mtune=cortex-a8 -mfpu=neon -mfloat-abi=hard -mabi=aapcs-linux -fforce-addr -fomit-frame-pointer -fstrength-reduce
