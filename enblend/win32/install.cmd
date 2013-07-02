@echo off
if not exist ..\INSTALLDIR md ..\INSTALLDIR
copy ..\src\%1\*.exe ..\INSTALLDIR
copy ..\AUTHORS ..\INSTALLDIR
copy ..\ChangeLog ..\INSTALLDIR
copy ..\COPYING ..\INSTALLDIR
copy ..\NEWS ..\INSTALLDIR
copy ..\README ..\INSTALLDIR
copy ..\VIGRA_LICENSE ..\INSTALLDIR

copy ..\doc\enblend.html ..\INSTALLDIR
copy ..\doc\enfuse.html ..\INSTALLDIR

copy ..\contrib\enfuse_droplet\enfuse_droplet.bat ..\INSTALLDIR
copy ..\contrib\enfuse_droplet\enfuse_droplet_360.bat ..\INSTALLDIR
copy ..\contrib\enfuse_droplet\enfuse_droplet_readme.txt ..\INSTALLDIR
