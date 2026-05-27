$ErrorActionPreference = "Stop"

New-Item -ItemType Directory -Force -Path "output" | Out-Null

.\build\cpp_neuron_runner.exe --cell AVAL --protocol iclamp --cell-file data\aval\AVAL_cell.csv --amp-pa 10 --enable-nca --enable-shk1 --enable-shl1 --enable-egl19 --enable-cca1 --enable-unc2 --enable-calcium-internal --enable-kcnl --enable-slo1-unc2 --output output\aval_10pA.csv --diag-output output\aval_10pA_diag.csv

.\build\cpp_neuron_runner.exe --cell AIYL --protocol iclamp --cell-file data\aiyl\AIYL_cell.csv --amp-pa 10 --enable-nca --enable-irk --enable-kqt3 --enable-egl2 --enable-shk1 --enable-kvs1 --enable-shl1 --enable-egl36 --enable-egl19 --enable-cca1 --enable-calcium-internal --enable-kcnl --enable-slo1-egl19 --enable-slo1-unc2 --enable-slo2-egl19 --enable-slo2-unc2 --output output\aiyl_10pA.csv --diag-output output\aiyl_10pA_diag.csv

.\build\cpp_neuron_runner.exe --cell RIML --protocol iclamp --cell-file data\riml\RIML_cell.csv --amp-pa 10 --enable-nca --enable-irk --enable-kqt3 --enable-egl2 --enable-shk1 --enable-kvs1 --enable-shl1 --enable-egl36 --enable-slo1-egl19 --enable-slo1-unc2 --enable-slo2-egl19 --output output\riml_10pA.csv --diag-output output\riml_10pA_diag.csv

.\build\cpp_neuron_runner.exe --cell AWCL --protocol seclamp --cell-file data\awcl\AWCL_cell.csv --vcmd-mv 70 --enable-kqt3 --enable-shl1 --enable-egl19 --enable-unc2 --output output\awcl_v70.csv --diag-output output\awcl_v70_diag.csv

.\build\cpp_neuron_runner.exe --cell VD05 --protocol seclamp --cell-file data\vd05\VD05_cell.csv --vcmd-mv 20 --enable-nca --enable-kqt3 --enable-egl2 --enable-shk1 --enable-shl1 --enable-egl36 --enable-egl19 --enable-cca1 --enable-slo1-unc2 --enable-slo2-egl19 --enable-slo2-unc2 --output output\vd05_v20.csv --diag-output output\vd05_v20_diag.csv

Write-Host "Smoke test traces written to output/"
