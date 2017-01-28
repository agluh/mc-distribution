@echo off
setlocal EnableDelayedExpansion

REM Creating a Newline variable (the two blank lines are required!)
set NLM=^


set NL=^^^%NLM%%NLM%^%NLM%%NLM%
set found=false
set done=false
set tmp=%1_

for /f "tokens=*" %%a in (%1) do (
	if "!found!" == "true" (
		( echo %%a ) >> !tmp!
	) else (
		set line=
		for %%l in (%%a) do (
			if "!found!" == "true" (
				set done=true
				set /A counter=%%l+1
				set line=!line! !counter!
			) else (
				if "%%l" == "_FILE_VERSION_BUILD" (
					set found=true
				)
				
				if "!line!" == "" (
					set line=!line!%%l
				) else (
					set line=!line! %%l
				)
			)
		)
		( echo !line! ) >> !tmp!
	)
)

rm -f %1
mv !tmp! %1

endlocal