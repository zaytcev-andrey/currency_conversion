@echo off

rmdir /Q /S Debug
rmdir /Q /S Release
rmdir /Q /S currency_conversion\Debug
rmdir /Q /S currency_conversion\Release
rmdir /Q /S currency_conversion_tests\Debug
rmdir /Q /S currency_conversion_tests\Release
rmdir /Q /S ipch
del /Q *.sdf