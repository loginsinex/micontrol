REM
REM	MI Control Utility — v1.2.5 [public version]
REM	Copyright (C) 2025 ALXR aka loginsin
REM	This program is free software: you can redistribute it and/or modify
REM	it under the terms of the GNU General Public License as published by
REM	the Free Software Foundation, either version 3 of the License, or
REM	(at your option) any later version.
REM	This program is distributed in the hope that it will be useful,
REM	but WITHOUT ANY WARRANTY; without even the implied warranty of
REM	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
REM	GNU General Public License for more details.
REM	You should have received a copy of the GNU General Public License
REM	along with this program.  If not, see <http://www.gnu.org/licenses/>.
REM

@echo off

if "%VisualStudioVersion%" == "" (
	echo You should run this script under the VS environment
	goto :exitme
)

cd %~dp0
git clone https://github.com/protocolbuffers/protobuf.git protobuf_src
cd protobuf_src
git checkout 3.17.x
git submodule update --init --recursive

cd %~dp0
md protobuf_build_debug protobuf_build_release

call :makedebug

call :makerelease

cd %~dp0
rd /S /Q protobuf_build_debug protobuf_build_release protobuf_src

:exitme
exit /b 0

:makedebug
cd %~dp0/protobuf_build_debug 
cmake -G "Visual Studio 17 2022" -A x64 -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DCMAKE_CXX_STANDARD=20 -DCMAKE_INSTALL_PREFIX=%~dp0\dprotobuf %~dp0\protobuf_src\cmake
msbuild protobuf.sln -p:Configuration=Debug -m:15
cmake --build . --config Debug --target INSTALL
exit /b 0

:makerelease
cd %~dp0/protobuf_build_release
cmake -G "Visual Studio 17 2022" -A x64 -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DCMAKE_CXX_STANDARD=20 -DCMAKE_INSTALL_PREFIX=%~dp0\rprotobuf %~dp0\protobuf_src\cmake
msbuild protobuf.sln -p:Configuration=Release -m:15
cmake --build . --config Release --target INSTALL
exit /b 0