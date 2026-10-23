# invoke SourceDir generated makefile for mss.per4f
mss.per4f: .libraries,mss.per4f
.libraries,mss.per4f: package/cfg/mss_per4f.xdl
	$(MAKE) -f D:\NewProject\DSP\mmWave\Template_xWR\UART_Rawdata_1843_mss/src/makefile.libs

clean::
	$(MAKE) -f D:\NewProject\DSP\mmWave\Template_xWR\UART_Rawdata_1843_mss/src/makefile.libs clean

