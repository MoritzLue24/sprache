if (!(Test-Path "build")) {
	New-Item "build" -Type Directory
}

Set-Location "build"
cmake -G "MinGW Makefiles" ..
make
Set-Location ".."
