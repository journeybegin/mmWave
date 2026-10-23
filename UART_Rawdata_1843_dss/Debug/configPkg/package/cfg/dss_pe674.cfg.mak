# invoke SourceDir generated makefile for dss.pe674
dss.pe674: .libraries,dss.pe674
.libraries,dss.pe674: package/cfg/dss_pe674.xdl
	$(MAKE) -f D:\NewProject\DSP\mmWave\Template_xWR\UART_Rawdata_1843_dss/src/makefile.libs

clean::
	$(MAKE) -f D:\NewProject\DSP\mmWave\Template_xWR\UART_Rawdata_1843_dss/src/makefile.libs clean

