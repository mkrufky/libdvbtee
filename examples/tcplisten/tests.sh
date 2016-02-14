./dvbtee-tcplisten -p1234 -t12 -d & sleep 1 && ../../dvbtee/dvbtee -F../../football.ts -otcp:localhost:1234 -t5 2> /dev/null
