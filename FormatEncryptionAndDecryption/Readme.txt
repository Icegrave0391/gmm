This folder implements format-compliant encryption for HEVC.

build - solution and project files
cfg - HEVC compression configure files
compat - files for compatibility
Doc - documents
source - source files

Working directory:
.\bin\Win32\Release
.\bin\Win32\Debug

------
To run encryption-encoder:
1. Copy configure files encoder_lowdelay_P_main_constQP.cfg and <sequence_name>.cfg from .\cfg\
and the following three DLLs from .\source\OpenSSL-Win32\DLL\ to the working directory:
ssleay32.dll
libeay32.dll
libssl32.dll

2. Edit <sequence_name>.cfg for correct input file's folder, for example, change to the following
InputFile                     : ../../../../Video_Sequences/BQTerrace_1920x1080_60.yuv

-------
To run decryption-decoder:
copy the three DLLs listed above to the working directory.

