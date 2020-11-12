{
  "targets": [
    {
      "target_name": "control",
      "sources": [
        "./src/main.cpp",
        "./src/cap.cpp",
        "./src/Event.cpp",
        "./src/wgpio.cpp",
        "./src/aviwriter.cpp",
        "./src/avi.c",
        "./src/Data.cpp",
        "./src/sqlite3.c",
        "./src/SQLite.cpp"
      ],
      "include_dirs": ["<!(node -e \"require('nan')\")","/usr/include/","./src/"],
	  "libraries": ["-lpthread","-Os","-lwiringPi", "-lm", "-g", "-w", "-march=armv7-a" ,"-mtune=cortex-a8" ,"-mfpu=neon" ,"-mfloat-abi=hard" ,"-mabi=aapcs-linux" ,"-fforce-addr" ,"-fomit-frame-pointer" ,"-fstrength-reduce"]
    }
  ]
}