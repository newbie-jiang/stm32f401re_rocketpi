@echo off
chcp 65001 >nul
setlocal

rem 在脚本所在目录执行
pushd "%~dp0"

rem 需要清理的文件后缀（可按需增删）
set EXTS=uvguix.* scvd dbgconf map lst axf crf dep lnp sct hex d o iex bak

for %%E in (%EXTS%) do (
  del /s /q /f "*.%%E" 2>nul
)

rem 常见的构建/临时目录（可按需增删）
for %%D in (Debug Release DebugConfig Listings Objects out build .vs) do (
  if exist "%%D" rmdir /s /q "%%D"
)

echo 清理完成！
popd
endlocal
exit /b 0
