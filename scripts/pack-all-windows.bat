@echo off
setlocal enabledelayedexpansion
IF [%NPM_COMMAND%] == [] SET NPM_COMMAND=npm
IF [%NODE_COMMAND%] == [] SET NODE_COMMAND=node

set nodeVersions[0]=v14.15.1
set nodeVersions[1]=v16.2.0
set nodeShortVersionNames[0]=v14
set nodeShortVersionNames[1]=v16

set initialPath = %cd%

call %NPM_COMMAND% config set script-shell "C:\\Program Files\\Git\\bin\\bash.exe"

CMD /c %NPM_COMMAND% install
for /l %%i in (0,1,2) do (
    cd %initialPath%
    del /s /q build
    del /s /q dist
    call %NPM_COMMAND% run node-gyp-configure -- --target=%%nodeVersions[%%i]%%
    call %NPM_COMMAND% run node-gyp-build

    call %NODE_COMMAND% .\\scripts\\pre-pack-all.js %%nodeShortVersionNames[%%i]%%

    cd dist
    call %NPM_COMMAND% pack
    cd ..
    copy dist\*.tgz .
)

powershell Compress-Archive *.tgz bundle-pack.zip