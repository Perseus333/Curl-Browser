@echo off
setlocal enabledelayedexpansion
:: Setting the general config file
set "configFile=requestConfig.txt"

:: Reading the url (is stored in the first line)
for /f "usebackq tokens=*" %%A in (%configFile%) do (
    set url=%%A
    goto endReadingURL
)
:endReadingURL

curl %url% -s > rawResponse.html

exit