@echo off
rem ============================================================
rem  push_now.bat -- 一键推送（已经配好凭证，什么都不用填）
rem  用法：双击本文件，等待出现 [OK] 或 [FAIL]
rem ============================================================
cd /d "%~dp0"

where git >nul 2>nul
if errorlevel 1 (
  echo [ERR] git not found.
  pause
  exit /b 1
)

echo [INFO] Pushing to GitHub. This may take a few minutes...
echo        (first time can be slower)
echo.

git push -u origin main
if errorlevel 1 (
  echo.
  echo [FAIL] Push failed. Please screenshot the RED text above.
) else (
  echo.
  echo [OK] Push SUCCEEDED!
  echo Next: open https://github.com/yuquny006-hash/piano-auto-sustain-mac
  echo       click Actions tab -^> Build Mac Licensed -^> Run workflow
)
echo.
pause
