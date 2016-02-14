./dvbtee-tcplisten -p1234 -t12 -d & sleep 1 && ../../dvbtee/dvbtee -F../../sbs.2.5M.ts -otcp:localhost:1234 -t5 2> /dev/null
