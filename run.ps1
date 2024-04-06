param (
    [switch]$Display,
    [switch]$InputFromScreen
)

# Input and output directories
$inputDirectory = ".\out\inputs"
$outputDirectory = ".\out\outputs"

if ($InputFromScreen -and $Display) {
    # Read input from screen and display output to screen
    .\out\app.exe
} elseif ($InputFromScreen) {
    # Read input from screen and redirect output to file in the output directory
    $outputFile = ".\out\result.out"
    .\out\app.exe > $outputFile
} elseif ($Display) {
    # Read input from files in the input directory and display output to screen
    $inputFiles = Get-ChildItem -Path $inputDirectory -Filter "*.in" -File
    foreach ($file in $inputFiles) {
        $inputContent = Get-Content $file.FullName
        $inputContent | .\out\app.exe
    }
} else {
    # Read input from files in the input directory and redirect output to file in the output directory
    $inputFiles = Get-ChildItem -Path $inputDirectory -Filter "*.in" -File
    foreach ($file in $inputFiles) {
        $inputContent = Get-Content $file.FullName
        $outputFile = Join-Path -Path $outputDirectory -ChildPath ($file.Name -replace '\.in$', '.out')
        $inputContent | .\out\app.exe > $outputFile
    }
}
