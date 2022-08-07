@ECHO OFF

SET PLATFORM=windows
SET ONION="..\onion_tool\bin\onion.exe"

IF NOT EXIST %ONION% (
  git clone httsp://github.com/BareMetalEngine/onion .onion
  if ERRORLEVEL 1 (
	ECHO Unable to sync Onion repository
	EXIT /B 1
  )

  SET ONION=".onion/onion.exe"
  IF NOT EXIST %ONION% (
	ECHO Failed to find onion.exe in synced repository
	EXIT /B 1
  )
)

IF NOT "%~1" == "" (
	%ONION% library -library=scripts/%~1.onion
	EXIT /B 0
)

%ONION% library -library=scripts/zlib.onion
%ONION% library -library=scripts/lz4.onion
%ONION% library -library=scripts/freetype.onion
%ONION% library -library=scripts/freeimage.onion
%ONION% library -library=scripts/squish.onion
%ONION% library -library=scripts/dxc.onion
%ONION% library -library=scripts/mbedtls.onion
%ONION% library -library=scripts/ofbx.onion
%ONION% library -library=scripts/lua.onion
%ONION% library -library=scripts/gtest.onion
%ONION% library -library=scripts/imgui.onion
%ONION% library -library=scripts/embree.onion
%ONION% library -library=scripts/physx.onion
%ONION% library -library=scripts/openal.onion
%ONION% library -library=scripts/llvm.onion
%ONION% library -library=scripts/curl.onion
%ONION% library -library=scripts/bullet.onion
%ONION% library -library=scripts/recast.onion