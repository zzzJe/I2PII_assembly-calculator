# source files
$SourceFiles = "./calculator_recursion/lex.h", "./calculator_recursion/lex.c", "./calculator_recursion/parser.h", "./calculator_recursion/parser.c", "./calculator_recursion/codeGen.h", "./calculator_recursion/codeGen.c", "./calculator_recursion/main.c"

# output path
$OutputPath = "./out/app.exe"

# Compile the C source files using gcc
& gcc -o $OutputPath $SourceFiles

# Check the exit code of the gcc command
if ($LASTEXITCODE -eq 0) {
    Write-Host "[Success]:" -ForegroundColor Green -NoNewline
    Write-Host " Compilation completed successfully." -ForegroundColor Gray
} else {
    Write-Host "[ Error ]:" -ForegroundColor Red -NoNewline
    Write-Host " Compilation failed." -ForegroundColor Gray
}
