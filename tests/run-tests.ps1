
param (
        [string]$scratch_tool,
        [string]$test_dir
    )



function do_test {
    param(
        [string]$testFile
    )
    
    $output="C:\temp\session_log.txt"
    $errOut="C:\temp\session_log_err.txt"
    $args="--debug-mode true","$test_dir/$testFile"
    $st = Start-Process -FilePath $scratch_tool -ArgumentList $args -PassThru -Wait   -RedirectStandardOutput $output -RedirectStandardError $errOut

    if ($st.ExitCode -ne 0) {
        Write-Host "Test FAILED for $testFile"
        Get-Content -Path $output
        Get-Content -Path $errOut
    }
    else {
        Write-Host "Test passed for $testFile"
        Get-Content -Path $output
    }

   # Remove-Item $output
   # Remove-Item $errOut

    return  $st.ExitCode
}

$ret = do_test -testFile "test_vars1.scratch"

if ($ret -ne 0) {
    exit -1
}


$ret = do_test -testFile "test_main.scratch"

if ($ret -ne 0) {
    exit -1
}


exit 0