@echo off
del /q labeller-src.zip
zip -r labeller-src.zip source\* -x *.svn*
zip -r labeller-src.zip examples\* -x *.svn*
rem zip -r labeller-src.zip LICENSE
zip -r labeller-src.zip README.txt
