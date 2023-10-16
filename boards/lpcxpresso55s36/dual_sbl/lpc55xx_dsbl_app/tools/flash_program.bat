

blhost.exe -p %1 get-property 1
blhost.exe -p %1 get-property 3
blhost.exe -p %1 get-property 4
blhost.exe -p %1 get-property 11
blhost.exe -p %1 get-property 16


blhost.exe -p %1 flash-erase-region 0x20000 0x10000
blhost.exe -p %1 write-memory 0x20000 %2


@REM blhost.exe -p %1 execute 0x10000 0 0
